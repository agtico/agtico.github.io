const EMAIL_PATTERN = /\b[A-Z0-9._%+-]+@[A-Z0-9.-]+\.[A-Z]{2,}\b/gi;
const PHONE_PATTERN = /(?:\+?\d[\d\s().-]{8,}\d)/g;
const UUID_PATTERN = /\b[0-9a-f]{8}-[0-9a-f]{4}-[1-5][0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}\b/gi;
const WALLET_PATTERN = /\br[1-9A-HJ-NP-Za-km-z]{25,35}\b/g;
const TX_PATTERN = /\b[A-Fa-f0-9]{32,128}\b/g;
const URL_PATTERN = /\bhttps?:\/\/[^\s<>"']+/gi;
const CLIENT_FIELD_PATTERN = /\b(client|customer|company|organization|org|account|contact)\s*[:=]\s*([^.;\n\r]+)/gi;

function isLikelyPhone(value) {
  return String(value || '').replace(/\D/g, '').length >= 10;
}

function redactPhone(value) {
  return isLikelyPhone(value) ? '[redacted-phone]' : value;
}

function escapeRegex(value) {
  return String(value).replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
}

function containsPattern(pattern, text, predicate = () => true) {
  pattern.lastIndex = 0;
  let match = pattern.exec(text);
  while (match) {
    if (predicate(match[0])) {
      return true;
    }
    match = pattern.exec(text);
  }
  return false;
}

export function redactText(value, options = {}) {
  let text = String(value || '');
  if (!text) {
    return '';
  }

  text = text
    .replace(EMAIL_PATTERN, '[redacted-email]')
    .replace(PHONE_PATTERN, redactPhone)
    .replace(URL_PATTERN, '[redacted-url]')
    .replace(WALLET_PATTERN, '[redacted-wallet]')
    .replace(UUID_PATTERN, '[redacted-id]')
    .replace(TX_PATTERN, '[redacted-tx]')
    .replace(CLIENT_FIELD_PATTERN, (_match, label) => `${label}: [redacted-client]`);

  const terms = Array.isArray(options.redactionTerms) ? options.redactionTerms : [];
  for (const term of terms) {
    const trimmed = String(term || '').trim();
    if (trimmed.length < 2) {
      continue;
    }
    text = text.replace(new RegExp(escapeRegex(trimmed), 'gi'), '[redacted]');
  }

  return text.replace(/\s+/g, ' ').trim();
}

export function hasPublicLeak(value) {
  const text = String(value || '');
  return containsPattern(EMAIL_PATTERN, text)
    || containsPattern(PHONE_PATTERN, text, isLikelyPhone)
    || containsPattern(WALLET_PATTERN, text)
    || containsPattern(UUID_PATTERN, text);
}
