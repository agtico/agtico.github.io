import { extractTickers } from './extract.js';

const MAX_TICKERS = 8;
const VENUE_TICKERS = ['HYPE', 'BNB', 'COIN', 'CME', 'ICE', 'CBOE'];
const AI_INFRA_TICKERS = ['MSFT', 'GOOGL', 'AMZN', 'NVDA'];
const HOSPITALITY_TICKERS = ['BKNG', 'ABNB', 'MAR', 'HLT'];
const NETWORK_TICKERS = ['PFT', 'XRP'];
const UNSUPPORTED_GENERIC_TICKERS = new Set(['BTC', 'ETH', 'XRP', 'PFT']);

function normalizedText(parts) {
  return parts
    .filter(Boolean)
    .map((part) => String(part))
    .join(' ')
    .replace(/\s+/g, ' ')
    .trim()
    .toLowerCase();
}

function addMany(target, values) {
  for (const value of values) {
    if (value && !target.includes(value)) {
      target.push(value);
    }
  }
}

function hasAny(text, patterns) {
  return patterns.some((pattern) => pattern.test(text));
}

function isNetworkTopic(event, text) {
  const category = String(event.task_category || event.category || '').toLowerCase();
  const activityTag = String(event.activity_task_tag || '').toLowerCase();
  return category === 'network'
    || activityTag === 'network'
    || hasAny(text, [
      /\bdynamic unl\b/,
      /\bvalidator(?:s| mapping)?\b/,
      /\bpost fiat network\b/,
      /\btask node network\b/,
      /\bnetwork scoring\b/,
      /\bpftl network\b/,
      /\bxrpl\b/,
    ]);
}

function hasDirectSupport(ticker, text, networkTopic) {
  if (!UNSUPPORTED_GENERIC_TICKERS.has(ticker)) {
    return true;
  }
  if (ticker === 'BTC') {
    return /\b(bitcoin|btc)\b/.test(text);
  }
  if (ticker === 'ETH') {
    return /\b(ethereum|eth)\b/.test(text);
  }
  if (ticker === 'XRP') {
    return networkTopic || /\b(xrp|xrpl)\b/.test(text);
  }
  if (ticker === 'PFT') {
    return networkTopic || /\b(post fiat|pft\b|pftl network)\b/.test(text);
  }
  return true;
}

function heuristicTickers(event, text, networkTopic) {
  const tickers = [];

  if (hasAny(text, [/\btelegram\b/])) {
    addMany(tickers, ['TON']);
  }

  if (hasAny(text, [
    /\bfutures\b/,
    /\bperp(?:etual)?s?\b/,
    /\bdelta[-\s]?hedg(?:e|ing)\b/,
    /\blimit[-\s]?orders?\b/,
    /\bopen futures orders?\b/,
    /\bpost[-\s]?fill\b/,
    /\bhedge sizing\b/,
  ])) {
    addMany(tickers, VENUE_TICKERS);
  }

  if (hasAny(text, [
    /\brag\b/,
    /\bretrieval\b/,
    /\banswer extraction\b/,
    /\bllm\b/,
    /\bprompt privacy\b/,
    /\bscoring pipeline\b/,
    /\bevaluation set\b/,
  ])) {
    addMany(tickers, AI_INFRA_TICKERS);
  }

  if (hasAny(text, [
    /\bvps\b/,
    /\bcustomer agent\b/,
    /\bservice supervision\b/,
    /\bruntime\b/,
    /\bsecrets?\b/,
  ])) {
    addMany(tickers, ['AMZN', 'MSFT', 'NET']);
  }

  if (hasAny(text, [/\bhospitality\b/, /\bosaka\b/, /\boutreach\b/])) {
    addMany(tickers, HOSPITALITY_TICKERS);
  }

  if (networkTopic) {
    addMany(tickers, NETWORK_TICKERS);
  }

  return tickers;
}

export function classifyTaskTickers(event, summary = {}, maxTickers = MAX_TICKERS) {
  const text = normalizedText([
    summary.title,
    summary.summary,
    event.activity_anonymized_summary,
    event.activity_task_tag,
    event.activity_action_type,
    event.title,
    event.description,
    event.board_user_title,
    event.board_user_description,
    event.board_task_details,
  ]);
  const networkTopic = isNetworkTopic(event, text);
  const existing = extractTickers(event)
    .filter((ticker) => hasDirectSupport(ticker, text, networkTopic));
  const inferred = heuristicTickers(event, text, networkTopic);
  const ordered = [];

  addMany(ordered, inferred);
  addMany(ordered, existing);

  return ordered.slice(0, maxTickers);
}
