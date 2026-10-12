# Pearl update research scripts (AGTI, October 2026)

Published with [Engaging with PRL](https://agtico.github.io/intelligence-reports/2026/10/12/engaging-with-prl/).

| Script | Purpose |
|--------|---------|
| `fetch_chain.py` | Pearl mainnet history from the public Blockbook indexer (height, time, difficulty, coinbase) |
| `fetch_market.py` | PRL daily price, market cap and volume from CoinGecko (`pearl-2`) |
| `economics.py` | Implied hashrate, emission and supply, H100 revenue, issuance vs volume, margin heatmap |
| `gpu_economics.py` | Per-GPU breakevens (power only, full owned, rented), the FP8 fork scenario, forward PRL yield |
| `compare_api_pricing.py` | Pearl Inference Platform list prices vs the cheapest live OpenRouter provider |

Data snapshots in `data/` and charts in `charts/` were produced on 2026-10-12. Capex figures in `gpu_economics.py` are
AGTI assumptions; benchmarks and rents are cited in the article.
