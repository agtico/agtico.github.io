import test from 'node:test';
import assert from 'node:assert/strict';
import { normalizeTxHash, buildExplorerUrl } from '../src/links/pftl.js';
import { redactText, hasPublicLeak } from '../src/privacy/redaction.js';
import { anonymizeEvent } from '../src/privacy/anonymize.js';
import { extractTickers } from '../src/tickers/extract.js';
import { classifyTaskTickers } from '../src/tickers/classify.js';

test('redacts direct identifiers before public rendering', () => {
  const redacted = redactText('Client: Acme Capital. Email me@test.com. Wallet rDTXLQ7ZKZVKz33zJbHjgVShjsBnqMBhmN. Task 6f2d0db5-7d28-41d0-a088-cc0cf3c49f7b.');
  assert.match(redacted, /client: \[redacted-client\]/i);
  assert.match(redacted, /\[redacted-email\]/);
  assert.match(redacted, /\[redacted-wallet\]/);
  assert.match(redacted, /\[redacted-id\]/);
  assert.equal(hasPublicLeak(redacted), false);
});

test('redacts configured sensitive terms', () => {
  const redacted = redactText('Coordinate Project Nightfall scan.', {
    redactionTerms: ['Project Nightfall'],
  });
  assert.equal(redacted, 'Coordinate [redacted] scan.');
});

test('privacy leak checks are deterministic across repeated calls', () => {
  const leaked = 'Escalate to founder@example.com for handling.';
  assert.equal(hasPublicLeak(leaked), true);
  assert.equal(hasPublicLeak(leaked), true);
  assert.equal(hasPublicLeak(redactText(leaked)), false);
});

test('anonymizes actor and task references deterministically', () => {
  const event = {
    id: '6f2d0db5-7d28-41d0-a088-cc0cf3c49f7b',
    user_id: '1dc5f8af-18a2-48b4-9b7f-1cf07bd34a1e',
    title: 'Client: SecretCo $XRP scan',
    description: 'Email secret@example.com',
    created_at: '2026-04-28T13:10:00.000Z',
  };
  const one = anonymizeEvent(event, { anonPepper: 'pepper' });
  const two = anonymizeEvent(event, { anonPepper: 'pepper' });
  assert.equal(one.actor, two.actor);
  assert.match(one.actor, /^node:[0-9a-f]{8}$/);
  assert.match(one.task_ref, /^task:[0-9a-f]{8}$/);
  assert.doesNotMatch(one.title, /SecretCo|secret@example\.com/);
});

test('extracts ticker tags from metadata and task text', () => {
  const event = {
    title: 'Build $XRP and $NAV scan',
    description: 'Include $MSFT comparison but ignore UX acronym.',
    activity_tickers: ['BTC CRYPTO', 'COIN US EQUITY'],
    task_metadata: {
      alpha_tickers: ['xrp', 'NAV'],
      security_mapping: { tickers: ['MSFT'] },
    },
    board_metadata: {
      associated_tickers: [{ symbol: 'GOOGL' }],
    },
  };
  assert.deepEqual(extractTickers(event), ['BTC', 'COIN', 'XRP', 'NAV', 'GOOGL', 'MSFT']);
});

test('validates and formats PFTL explorer links', () => {
  const hash = 'abcdef1234567890abcdef1234567890abcdef1234567890abcdef1234567890';
  assert.equal(normalizeTxHash(hash), hash.toUpperCase());
  assert.equal(
    buildExplorerUrl(hash, 'https://explorer.test/tx/{hash}'),
    `https://explorer.test/tx/${hash.toUpperCase()}`
  );
});

test('classifies Telegram futures work to TON and exchange tickers', () => {
  const event = {
    task_category: 'personal',
    activity_anonymized_summary: 'Requested verification for Telegram risk monitor updates to show pending futures exposure and post-fill exposure estimates.',
    activity_tickers: ['BTC CRYPTO', 'ETH CRYPTO', 'XRP CRYPTO'],
  };
  assert.deepEqual(
    classifyTaskTickers(event, { title: event.activity_anonymized_summary, summary: event.activity_anonymized_summary }),
    ['TON', 'HYPE', 'BNB', 'COIN', 'CME', 'ICE', 'CBOE']
  );
});

test('does not preserve generic underlyings without direct textual support', () => {
  const event = {
    task_category: 'personal',
    activity_anonymized_summary: 'Submitted evidence for a configurable delta-hedging trigger bot design.',
    activity_tickers: ['BTC CRYPTO', 'ETH CRYPTO'],
  };
  assert.deepEqual(
    classifyTaskTickers(event, { title: event.activity_anonymized_summary, summary: event.activity_anonymized_summary }),
    ['HYPE', 'BNB', 'COIN', 'CME', 'ICE', 'CBOE']
  );
});

test('does not treat future planning language as futures trading signal', () => {
  const event = {
    task_category: 'network',
    activity_anonymized_summary: 'Planning a collaborative review of task handoff friction and acceptance criteria for future network improvements.',
    activity_tickers: ['XRP CRYPTO'],
  };
  assert.deepEqual(
    classifyTaskTickers(event, { title: event.activity_anonymized_summary, summary: event.activity_anonymized_summary }),
    ['PFT', 'XRP']
  );
});
