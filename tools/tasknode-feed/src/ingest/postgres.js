import pg from 'pg';

const { Pool } = pg;

const RECENT_TASKS_QUERY = `
SELECT
  t.id,
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
  GREATEST(
    COALESCE(latest_activity.activity_at, '-infinity'::timestamptz),
    COALESCE(latest_submission.created_at, '-infinity'::timestamptz),
    COALESCE(t.verified_at, '-infinity'::timestamptz),
    COALESCE(t.submitted_at, '-infinity'::timestamptz),
    COALESCE(t.accepted_at, '-infinity'::timestamptz),
    t.created_at
  ) AS feed_timestamp,
  t.reward_summary,
  t.reward_tx_hash,
  t.verification_tx_hash,
  t.board_task_id,
  bt.department AS board_department,
  bt.user_title AS board_user_title,
  bt.user_description AS board_user_description,
  bt.task_details AS board_task_details,
  bt.expected_impact AS board_expected_impact,
  bt.board_metadata AS board_metadata,
  latest_submission.pftl_tx_hash AS submission_pftl_tx_hash,
  latest_submission.reward_tx_hash AS submission_reward_tx_hash,
  latest_submission.created_at AS submission_created_at,
  latest_activity.anonymized_summary AS activity_anonymized_summary,
  latest_activity.tickers AS activity_tickers,
  latest_activity.pftl_tx_hash AS activity_pftl_tx_hash,
  latest_activity.task_tag AS activity_task_tag,
  latest_activity.action_type AS activity_action_type,
  latest_activity.event_type AS activity_event_type,
  latest_activity.relevance_score AS activity_relevance_score,
  latest_activity.activity_at AS activity_created_at
FROM public.tasks t
LEFT JOIN public.board_tasks bt ON bt.id = t.board_task_id
LEFT JOIN LATERAL (
  SELECT ts.pftl_tx_hash, ts.reward_tx_hash, ts.created_at
  FROM public.task_submissions ts
  WHERE ts.task_id = t.id
  ORDER BY ts.created_at DESC
  LIMIT 1
) latest_submission ON true
LEFT JOIN LATERAL (
  SELECT
    ace.anonymized_summary,
    ace.tickers,
    ace.pftl_tx_hash,
    ace.task_tag,
    ace.action_type,
    ace.event_type,
    ace.relevance_score,
    COALESCE(ace.posted_at, ace.created_at) AS activity_at
  FROM public.activity_channel_events ace
  WHERE ace.task_id = t.id
    AND NULLIF(BTRIM(COALESCE(ace.anonymized_summary, '')), '') IS NOT NULL
  ORDER BY COALESCE(ace.posted_at, ace.created_at) DESC
  LIMIT 1
) latest_activity ON true
WHERE GREATEST(
    COALESCE(latest_activity.activity_at, '-infinity'::timestamptz),
    COALESCE(latest_submission.created_at, '-infinity'::timestamptz),
    COALESCE(t.verified_at, '-infinity'::timestamptz),
    COALESCE(t.submitted_at, '-infinity'::timestamptz),
    COALESCE(t.accepted_at, '-infinity'::timestamptz),
    t.created_at
  ) >= now() - ($2::int * interval '1 day')
  AND COALESCE(t.status, '') NOT IN ('cancelled', 'expired', 'rejected')
ORDER BY feed_timestamp DESC, t.created_at DESC
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
    const result = await pool.query(RECENT_TASKS_QUERY, [config.limit, config.lookbackDays]);
    return result.rows;
  } finally {
    await pool.end();
  }
}
