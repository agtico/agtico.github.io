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

export async function buildFeed(config) {
  const rawEvents = await loadEvents(config);
  const items = [];

  for (const rawEvent of rawEvents.slice(0, config.limit)) {
    const anonymized = anonymizeEvent(rawEvent, config);
    const tickers = extractTickers(rawEvent);
    const summary = await summarizeEvent(anonymized, config);
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
    source: config.source,
    model: config.deepseekEnabled && config.deepseekApiKey ? config.deepseekModel : 'local-redacted-fallback',
    privacy: {
      mode: 'semi-anonymous',
      raw_task_content_published: false,
      actor_ids: 'hmac',
    },
    items,
  };
}
