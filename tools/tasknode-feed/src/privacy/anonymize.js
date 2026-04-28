import crypto from 'node:crypto';
import { redactText } from './redaction.js';

function hmacId(value, pepper, prefix = 'node') {
  const seed = String(value || 'unknown');
  const digest = crypto.createHmac('sha256', String(pepper || 'public-fixture-pepper'))
    .update(seed)
    .digest('hex')
    .slice(0, 8);
  return `${prefix}:${digest}`;
}

function eventTimestamp(event) {
  return event.feed_timestamp
    || event.submitted_at
    || event.verified_at
    || event.accepted_at
    || event.created_at
    || new Date().toISOString();
}

export function anonymizeEvent(event, options = {}) {
  const titleSource = event.board_user_title || event.user_title || event.title || 'Task Node update';
  const descriptionSource = event.activity_anonymized_summary
    || event.board_user_description
    || event.user_description
    || event.description
    || event.reward_summary
    || '';

  return {
    source_id: String(event.id || ''),
    task_ref: hmacId(event.id, options.anonPepper, 'task'),
    actor: hmacId(event.user_id || event.board_task_id || event.id, options.anonPepper, 'node'),
    timestamp: new Date(eventTimestamp(event)).toISOString(),
    type: event.status === 'completed' || event.status === 'verified' || event.status === 'rewarded'
      ? 'task_completed'
      : 'task_published',
    status: String(event.status || 'published'),
    department: String(event.board_department || event.department || event.task_category || 'task-node'),
    title: redactText(titleSource, options),
    body: redactText(descriptionSource, options),
    raw_context: redactText([
      titleSource,
      descriptionSource,
      event.board_task_details,
      event.board_expected_impact,
      event.activity_anonymized_summary,
      event.reward_summary,
    ].filter(Boolean).join('\n'), options),
  };
}
