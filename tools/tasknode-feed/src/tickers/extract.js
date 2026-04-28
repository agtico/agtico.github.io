const MAX_TICKERS = 8;
const COMMON_FALSE_POSITIVES = new Set([
  'AGTI',
  'API',
  'AI',
  'BFR',
  'CSV',
  'DAO',
  'JSON',
  'KPI',
  'LLM',
  'NFT',
  'PFT',
  'PFTL',
  'ROI',
  'SQL',
  'URL',
  'UX',
]);

export function normalizeTickerSymbol(value) {
  if (value === null || value === undefined) {
    return '';
  }
  const raw = String(value).trim().toUpperCase();
  if (!raw || !/^[A-Z0-9.\-]{1,15}$/.test(raw)) {
    return '';
  }
  return raw;
}

function addTicker(target, value) {
  const symbol = normalizeTickerSymbol(value);
  if (!symbol || COMMON_FALSE_POSITIVES.has(symbol) || target.has(symbol)) {
    return;
  }
  target.add(symbol);
}

function collectFromArray(target, entries) {
  if (!Array.isArray(entries)) {
    return;
  }
  for (const entry of entries) {
    if (typeof entry === 'string') {
      addTicker(target, entry);
    } else if (entry && typeof entry === 'object') {
      addTicker(target, entry.symbol || entry.ticker || entry.code);
    }
  }
}

export function extractTickers(event, maxTickers = MAX_TICKERS) {
  const metadata = event.task_metadata || event.metadata || {};
  const boardMetadata = event.board_metadata || {};
  const securityMapping = metadata.security_mapping || metadata.securityMapping || {};
  const alphaSignal = metadata.alpha_llm_signal || metadata.alphaLlmSignal || {};
  const tickers = new Set();

  collectFromArray(tickers, event.activity_tickers);
  collectFromArray(tickers, metadata.alpha_tickers || metadata.alphaTickers);
  collectFromArray(tickers, boardMetadata.alpha_tickers || boardMetadata.alphaTickers);
  collectFromArray(tickers, boardMetadata.associated_tickers || boardMetadata.associatedTickers);
  collectFromArray(tickers, boardMetadata.tickers);
  collectFromArray(tickers, securityMapping.tickers);
  collectFromArray(tickers, alphaSignal.per_ticker);

  const text = [
    event.title,
    event.description,
    event.board_user_title,
    event.board_user_description,
    event.board_task_details,
    event.activity_anonymized_summary,
    metadata.alpha_question,
  ].filter(Boolean).join(' ');

  for (const match of text.matchAll(/(?:\$|#)([A-Za-z][A-Za-z0-9.\-]{1,14})\b/g)) {
    addTicker(tickers, match[1]);
  }

  return Array.from(tickers).slice(0, maxTickers);
}
