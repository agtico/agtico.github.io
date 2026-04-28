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

function truncateTitle(value, maxChars = 96) {
  const text = String(value || '').replace(/\s+/g, ' ').trim();
  if (!text) {
    return 'Task Node update';
  }
  if (text.length <= maxChars) {
    return text;
  }
  return `${text.slice(0, Math.max(maxChars - 3, 1)).trimEnd()}...`;
}

function normalizeActivityTickers(value) {
  if (!Array.isArray(value)) {
    return [];
  }
  const tickers = [];
  for (const entry of value) {
    let symbol = String(entry || '').trim().toUpperCase();
    if (symbol.includes(' ')) {
      symbol = symbol.split(/\s+/)[0] || symbol;
    }
    if (/^[A-Z0-9.\-]{1,15}$/.test(symbol) && !tickers.includes(symbol)) {
      tickers.push(symbol);
    }
  }
  return tickers.slice(0, 8);
}

function hasActivityChannelSummary(event) {
  return Boolean(String(event.activity_anonymized_summary || '').trim());
}

async function summarizeFeedEvent(rawEvent, anonymized, config) {
  if (hasActivityChannelSummary(rawEvent)) {
    const summary = String(rawEvent.activity_anonymized_summary || '').replace(/\s+/g, ' ').trim();
    return {
      title: truncateTitle(summary),
      summary,
      category: rawEvent.activity_task_tag || anonymized.department,
      tags: rawEvent.activity_task_tag ? [rawEvent.activity_task_tag] : [],
      model: 'activity-channel',
    };
  }
  return summarizeEvent(anonymized, config);
}

export function resolveEventTickers(event) {
  const activityTickers = normalizeActivityTickers(event.activity_tickers);
  if (activityTickers.length) {
    return activityTickers;
  }
  if (hasActivityChannelSummary(event)) {
    return [];
  }
  return extractTickers(event);
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

  for (const rawEvent of rawEvents.slice(0, config.limit)) {
    const anonymized = anonymizeEvent(rawEvent, config);
    const summary = await summarizeFeedEvent(rawEvent, anonymized, config);
    const tickers = resolveEventTickers(rawEvent);
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
