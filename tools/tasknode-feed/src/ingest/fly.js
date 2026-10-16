import { execFile } from 'node:child_process';
import { promisify } from 'node:util';

const execFileAsync = promisify(execFile);
const MARKER = '__TASKNODE_FEED_JSON__';

const REMOTE_SCRIPT = `
const { Pool } = require('pg');

const requestedLimit = Math.max(1, Math.min(Number.parseInt(process.argv[1] || '24', 10) || 24, 100));
const limit = Math.min(Math.max(requestedLimit * 4, requestedLimit), 100);
const lookbackDays = Math.max(1, Math.min(Number.parseInt(process.argv[2] || '14', 10) || 14, 90));
const connectionString = process.env.DATABASE_READONLY || process.env.DATABASE_URL;

if (!connectionString) {
  throw new Error('DATABASE_READONLY or DATABASE_URL is required inside the Fly app');
}

const pool = new Pool({
  connectionString,
  max: 1,
  application_name: 'agtico-tasknode-feed-fly-export',
  ssl: { rejectUnauthorized: false },
});

const query = \`
SELECT
  ace.id,
  ace.task_id,
  COALESCE(ace.user_id, t.user_id) AS user_id,
  t.title,
  t.description,
  t.status,
  t.task_category,
  t.task_metadata,
  t.created_at,
  t.accepted_at,
  t.submitted_at,
  t.verified_at,
  COALESCE(ace.posted_at, ace.created_at) AS feed_timestamp,
  t.reward_summary,
  t.reward_tx_hash,
  NULL::text AS verification_tx_hash,
  t.board_task_id,
  bt.department AS board_department,
  bt.user_title AS board_user_title,
  bt.user_description AS board_user_description,
  bt.task_details AS board_task_details,
  bt.expected_impact AS board_expected_impact,
  NULL::jsonb AS board_metadata,
  latest_submission.pftl_tx_hash AS submission_pftl_tx_hash,
  latest_submission.reward_tx_hash AS submission_reward_tx_hash,
  latest_submission.verification_tx_hash AS submission_verification_tx_hash,
  latest_submission.created_at AS submission_created_at,
  ace.anonymized_summary AS activity_anonymized_summary,
  ace.tickers AS activity_tickers,
  ace.pftl_tx_hash AS activity_pftl_tx_hash,
  ace.task_tag AS activity_task_tag,
  ace.action_type AS activity_action_type,
  ace.event_type AS activity_event_type,
  ace.relevance_score AS activity_relevance_score,
  COALESCE(ace.posted_at, ace.created_at) AS activity_created_at
FROM public.activity_channel_events ace
JOIN public.tasks t ON t.id = ace.task_id
LEFT JOIN public.board_tasks bt ON bt.id = t.board_task_id
LEFT JOIN LATERAL (
  SELECT ts.pftl_tx_hash, ts.reward_tx_hash, ts.verification_tx_hash, ts.created_at
  FROM public.task_submissions ts
  WHERE ts.task_id = t.id
  ORDER BY ts.created_at DESC
  LIMIT 1
) latest_submission ON true
WHERE ace.status = 'posted'
  AND ace.posted_at IS NOT NULL
  AND ace.posted_at >= now() - ($2::int * interval '1 day')
  AND NULLIF(BTRIM(COALESCE(ace.anonymized_summary, '')), '') IS NOT NULL
  AND COALESCE(t.status, '') NOT IN ('cancelled', 'expired', 'rejected')
ORDER BY ace.posted_at DESC, ace.created_at DESC
LIMIT $1::int
\`;

const fallbackQuery = \`
SELECT
  t.id,
  t.id AS task_id,
  t.user_id,
  t.title,
  t.description,
  t.status,
  t.task_category,
  t.task_metadata,
  t.created_at,
  t.accepted_at,
  t.submitted_at,
  t.verified_at,
  COALESCE(t.submitted_at, t.verified_at, latest_submission.created_at, t.accepted_at, t.created_at) AS feed_timestamp,
  t.reward_summary,
  t.reward_tx_hash,
  NULL::text AS verification_tx_hash,
  t.board_task_id,
  bt.department AS board_department,
  bt.user_title AS board_user_title,
  bt.user_description AS board_user_description,
  bt.task_details AS board_task_details,
  bt.expected_impact AS board_expected_impact,
  NULL::jsonb AS board_metadata,
  latest_submission.pftl_tx_hash AS submission_pftl_tx_hash,
  latest_submission.reward_tx_hash AS submission_reward_tx_hash,
  latest_submission.verification_tx_hash AS submission_verification_tx_hash,
  latest_submission.created_at AS submission_created_at,
  NULL::text AS activity_anonymized_summary,
  NULL::jsonb AS activity_tickers,
  NULL::text AS activity_pftl_tx_hash,
  NULL::text AS activity_task_tag,
  NULL::text AS activity_action_type,
  NULL::text AS activity_event_type,
  NULL::numeric AS activity_relevance_score,
  NULL::timestamptz AS activity_created_at
FROM public.tasks t
LEFT JOIN public.board_tasks bt ON bt.id = t.board_task_id
LEFT JOIN LATERAL (
  SELECT ts.pftl_tx_hash, ts.reward_tx_hash, ts.verification_tx_hash, ts.created_at
  FROM public.task_submissions ts
  WHERE ts.task_id = t.id
  ORDER BY ts.created_at DESC
  LIMIT 1
) latest_submission ON true
WHERE COALESCE(t.status, '') NOT IN ('cancelled', 'expired', 'rejected')
  AND COALESCE(t.submitted_at, t.verified_at, latest_submission.created_at, t.accepted_at, t.created_at) >= now() - ($2::int * interval '1 day')
ORDER BY feed_timestamp DESC
LIMIT $1::int
\`;

pool.query(query, [limit, lookbackDays])
  .then(async (result) => {
    const rows = result.rows.length > 0
      ? result.rows
      : (await pool.query(fallbackQuery, [limit, lookbackDays])).rows;
    console.log('${MARKER}');
    console.log(JSON.stringify(rows));
  })
  .catch((error) => {
    console.error('TASKNODE_FEED_FLY_EXPORT_ERROR:', error.message);
    process.exitCode = 1;
  })
  .finally(() => pool.end());
`;

export function remoteScriptForTest() {
  return REMOTE_SCRIPT;
}

function shellQuote(value) {
  return `'${String(value).replaceAll("'", "'\\''")}'`;
}

function parseRows(stdout) {
  const markerIndex = stdout.lastIndexOf(MARKER);
  if (markerIndex === -1) {
    throw new Error('Fly export did not return a JSON marker');
  }

  const payload = stdout.slice(markerIndex + MARKER.length).trim();
  const rows = JSON.parse(payload);
  if (!Array.isArray(rows)) {
    throw new Error('Fly export JSON payload was not an array');
  }
  return rows;
}

function sanitizeFlyOutput(value) {
  return String(value || '')
    .replace(/postgres(?:ql)?:\/\/\\S+/gi, '[redacted-postgres-url]')
    .split('\\n')
    .filter(Boolean)
    .slice(-8)
    .join('\\n');
}

export async function loadFlyEvents(config) {
  const remoteCommand = [
    'node',
    '-e',
    shellQuote(REMOTE_SCRIPT),
    String(config.limit),
    String(config.lookbackDays),
  ].join(' ');

  try {
    const result = await execFileAsync(config.flyctlPath, [
      'ssh',
      'console',
      '-a',
      config.flyApp,
      '-C',
      remoteCommand,
    ], {
      timeout: config.flyTimeoutMs,
      maxBuffer: 10 * 1024 * 1024,
    });
    return parseRows(result.stdout);
  } catch (error) {
    const output = sanitizeFlyOutput(`${error.stdout || ''}\n${error.stderr || ''}`);
    const suffix = output ? `\n${output}` : '';
    throw new Error(`Fly Task Node feed export failed.${suffix}`);
  }
}
