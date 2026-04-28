import { loadFixtureEvents } from './ingest/fixture.js';
import { buildPftlLinks } from './links/pftl.js';
import { anonymizeEvent } from './privacy/anonymize.js';
import { hasPublicLeak } from './privacy/redaction.js';
import { summarizeEvent } from './summarize/deepseek.js';
import { extractTickers } from './tickers/extract.js';

async function loadEvents(config) {
  if (config.source === 'fixture') {
    return loadFixtureEvents(config);
  }
  if (config.source === 'postgres') {
    const { loadPostgresEvents } = await import('./ingest/postgres.js');
    return loadPostgresEvents(config);
  }
  if (config.source === 'fly') {
    const { loadFlyEvents } = await import('./ingest/fly.js');
    return loadFlyEvents(config);
  }
  throw new Error(`Unsupported TASKNODE_FEED_SOURCE: ${config.source}`);
}

function buildTags(summaryTags, tickers, event) {
  const tags = new Set();
  for (const tag of summaryTags || []) {
    const normalized = String(tag || '').trim().toLowerCase();
    if (normalized) {
      tags.add(normalized);
    }
  }
  tags.add('task-node');
  if (event.department) {
    tags.add(String(event.department).toLowerCase().replace(/[^a-z0-9]+/g, '-'));
  }
  for (const ticker of tickers) {
    tags.add(`$${ticker}`);
  }
  return Array.from(tags).slice(0, 12);
}

function taskContextKey(event) {
  return event?.task_id || event?.taskId || event?.board_task_id || event?.id || '';
}

function eventTime(event) {
  const raw = event?.feed_timestamp
    || event?.activity_created_at
    || event?.created_at
    || event?.submitted_at
    || event?.verified_at
    || '';
  const parsed = Date.parse(raw);
  return Number.isFinite(parsed) ? parsed : 0;
}

export function buildTaskTickerContext(events) {
  const context = new Map();
  const sortedEvents = [...(events || [])].sort((a, b) => eventTime(b) - eventTime(a));
  for (const event of sortedEvents) {
    const key = taskContextKey(event);
    if (!key || context.has(key)) {
      continue;
    }
    const tickers = extractTickers(event);
    if (!tickers.length) {
      continue;
    }
    context.set(key, tickers.slice(0, 8));
  }
  return context;
}

export function resolveEventTickers(event, tickerContext) {
  const directTickers = extractTickers(event);
  if (directTickers.length) {
    return directTickers;
  }
  const key = taskContextKey(event);
  if (!key || !tickerContext?.has(key)) {
    return [];
  }
  return tickerContext.get(key).slice(0, 8);
}

function assertPublicSafe(item) {
  const probe = JSON.stringify({
    title: item.title,
    summary: item.summary,
    tags: item.tags,
    actor: item.actor,
  });
  if (hasPublicLeak(probe)) {
    throw new Error(`Public feed item ${item.id} still appears to contain private identifiers`);
  }
}

function modelLabel(config) {
  if (config.deepseekEnabled && config.deepseekApiKey) {
    return config.deepseekModel;
  }
  if (config.source === 'fly' || config.source === 'postgres') {
    return 'tasknode-activity-channel';
  }
  return 'local-redacted-fallback';
}

export async function buildFeed(config) {
  const rawEvents = await loadEvents(config);
  const items = [];
  const publicSource = config.sourceLabel || config.source;
  const tickerContext = buildTaskTickerContext(rawEvents);

  for (const rawEvent of rawEvents.slice(0, config.limit)) {
    const anonymized = anonymizeEvent(rawEvent, config);
    const summary = await summarizeEvent(anonymized, config);
    const tickers = resolveEventTickers(rawEvent, tickerContext);
    const links = buildPftlLinks(rawEvent, config.explorerBase);
    const item = {
      id: anonymized.task_ref,
      timestamp: anonymized.timestamp,
      type: anonymized.type,
      status: anonymized.status,
      actor: anonymized.actor,
      category: summary.category || anonymized.department,
      title: summary.title,
      summary: summary.summary,
      tickers,
      tags: buildTags(summary.tags, tickers, anonymized),
      links,
      source: {
        system: 'pftasks',
        source_id: anonymized.task_ref,
      },
    };
    assertPublicSafe(item);
    items.push(item);
  }

  return {
    version: 1,
    generated_at: new Date().toISOString(),
    source: publicSource,
    model: modelLabel(config),
    privacy: {
      mode: 'semi-anonymous',
      raw_task_content_published: false,
      actor_ids: 'hmac',
    },
    items,
  };
}
