import pg from 'pg';

const { Pool } = pg;

const RECENT_ACTIVITY_QUERY = `
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
`;

export async function loadPostgresEvents(config) {
  if (!config.databaseUrl) {
    throw new Error('TASKNODE_DATABASE_URL, PFTASKS_DATABASE_URL, or DATABASE_URL is required for postgres source');
  }

  const pool = new Pool({
    connectionString: config.databaseUrl,
    max: 2,
    application_name: 'agtico-tasknode-feed',
    ssl: process.env.TASKNODE_DATABASE_SSL === 'disable' ? false : { rejectUnauthorized: false },
  });

  try {
    const queryLimit = Math.min(Math.max(config.limit * 4, config.limit), 100);
    const result = await pool.query(RECENT_ACTIVITY_QUERY, [queryLimit, config.lookbackDays]);
    return result.rows;
  } finally {
    await pool.end();
  }
}
