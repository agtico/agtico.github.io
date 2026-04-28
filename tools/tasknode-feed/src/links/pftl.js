const TX_HASH_PATTERN = /^[A-Fa-f0-9]{32,128}$/;

export function normalizeTxHash(value) {
  const raw = String(value || '').trim();
  if (!TX_HASH_PATTERN.test(raw)) {
    return '';
  }
  return raw.toUpperCase();
}

export function buildExplorerUrl(txHash, explorerBase) {
  const hash = normalizeTxHash(txHash);
  if (!hash) {
    return '';
  }
  const base = String(explorerBase || '').trim().replace(/\/+$/, '');
  if (!base) {
    return '';
  }
  if (base.includes('{hash}')) {
    return base.replace('{hash}', encodeURIComponent(hash));
  }
  return `${base}/${encodeURIComponent(hash)}`;
}

export function buildPftlLinks(event, explorerBase) {
  const candidates = [
    ['activity', event.activity_pftl_tx_hash],
    ['submission', event.submission_pftl_tx_hash || event.pftl_tx_hash],
    ['reward', event.submission_reward_tx_hash || event.reward_tx_hash],
    ['verification', event.submission_verification_tx_hash || event.verification_tx_hash],
  ];
  const seen = new Set();
  const links = [];

  for (const [kind, candidate] of candidates) {
    const hash = normalizeTxHash(candidate);
    if (!hash || seen.has(hash)) {
      continue;
    }
    const url = buildExplorerUrl(hash, explorerBase);
    if (!url) {
      continue;
    }
    seen.add(hash);
    links.push({
      type: 'pftl_tx',
      label: kind === 'submission' || kind === 'activity' ? 'PFTL proof' : `PFTL ${kind}`,
      hash,
      url,
    });
  }

  return links;
}
