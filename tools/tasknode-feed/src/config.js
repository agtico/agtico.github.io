import path from 'node:path';
import { fileURLToPath } from 'node:url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);
const TOOL_ROOT = path.resolve(__dirname, '..');
const REPO_ROOT = path.resolve(TOOL_ROOT, '..', '..');

function readInt(name, fallback) {
  const raw = process.env[name];
  if (raw === undefined || raw === '') {
    return fallback;
  }
  const parsed = Number.parseInt(raw, 10);
  return Number.isFinite(parsed) ? parsed : fallback;
}

function readBool(name, fallback = false) {
  const raw = process.env[name];
  if (raw === undefined || raw === '') {
    return fallback;
  }
  return ['1', 'true', 'yes', 'on'].includes(String(raw).trim().toLowerCase());
}

function readMs(name, fallback) {
  const parsed = readInt(name, fallback);
  return parsed > 0 ? parsed : fallback;
}

function splitCsv(value) {
  return String(value || '')
    .split(',')
    .map((entry) => entry.trim())
    .filter(Boolean);
}

export function loadConfig() {
  const databaseUrl = process.env.TASKNODE_DATABASE_URL
    || process.env.PFTASKS_DATABASE_URL
    || process.env.DATABASE_URL
    || '';
  const requestedSource = String(process.env.TASKNODE_FEED_SOURCE || 'auto').trim().toLowerCase();
  const source = requestedSource === 'auto'
    ? (databaseUrl ? 'postgres' : 'fixture')
    : requestedSource;

  return {
    repoRoot: REPO_ROOT,
    toolRoot: TOOL_ROOT,
    source,
    sourceLabel: process.env.TASKNODE_FEED_SOURCE_LABEL || '',
    flyApp: process.env.TASKNODE_FEED_FLY_APP || 'pftasks-api',
    flyctlPath: process.env.TASKNODE_FEED_FLYCTL || 'flyctl',
    flyTimeoutMs: readMs('TASKNODE_FEED_FLY_TIMEOUT_MS', 120_000),
    databaseUrl,
    limit: readInt('TASKNODE_FEED_LIMIT', 24),
    lookbackDays: readInt('TASKNODE_FEED_LOOKBACK_DAYS', 14),
    anonPepper: process.env.FEED_ANON_PEPPER || process.env.TASKNODE_FEED_ANON_PEPPER || 'public-fixture-pepper',
    redactionTerms: splitCsv(process.env.TASKNODE_FEED_REDACT_TERMS),
    explorerBase: process.env.PFTL_EXPLORER_BASE || 'https://explorer.testnet.postfiat.org/transactions/{hash}/detailed',
    deepseekApiBase: process.env.DEEPSEEK_API_BASE
      || process.env.DEEPSEEK_BASE_URL
      || process.env.TASKNODE_DEEPSEEK_API_BASE
      || process.env.SPRS_DEEPSEEK_API_BASE
      || process.env.SPRS_TELEGRAM_DEEPSEEK_API_BASE
      || 'https://api.deepseek.com',
    deepseekApiKey: process.env.DEEPSEEK_API_KEY
      || process.env.TASKNODE_DEEPSEEK_API_KEY
      || process.env.SPRS_DEEPSEEK_API_KEY
      || process.env.SPRS_TELEGRAM_DEEPSEEK_API_KEY
      || '',
    deepseekModel: process.env.DEEPSEEK_MODEL
      || process.env.TASKNODE_DEEPSEEK_MODEL
      || process.env.SPRS_TELEGRAM_CONTEXT_SUMMARY_MODEL
      || 'deepseek-v4-flash',
    deepseekDoNotTrain: readBool('DEEPSEEK_DO_NOT_TRAIN', true),
    deepseekEnabled: readBool(
      'TASKNODE_FEED_DEEPSEEK_ENABLED',
      Boolean(process.env.DEEPSEEK_API_KEY
        || process.env.TASKNODE_DEEPSEEK_API_KEY
        || process.env.SPRS_DEEPSEEK_API_KEY
        || process.env.SPRS_TELEGRAM_DEEPSEEK_API_KEY)
    ),
    dryRun: readBool('TASKNODE_FEED_DRY_RUN', false),
    outputJson: process.env.TASKNODE_FEED_OUTPUT_JSON || path.join(REPO_ROOT, 'assets', 'data', 'tasknode-feed.json'),
    outputData: process.env.TASKNODE_FEED_OUTPUT_DATA || path.join(REPO_ROOT, '_data', 'tasknode_feed.json'),
    fixturePath: process.env.TASKNODE_FEED_FIXTURE || path.join(TOOL_ROOT, 'examples', 'tasknode-events.fixture.json'),
  };
}
