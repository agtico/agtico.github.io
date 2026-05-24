# Pearl PoUW research scripts (AGTI)

Published with the [Pearl PoUW AGTI report](https://agtico.github.io/intelligence-reports/2026/05/23/pearl-pouw-useful-work-asic-analysis/).

| Script | Purpose |
|--------|---------|
| [`simulate.py`](https://agtico.github.io/assets/research/pearl-economics/simulate.py) | Fleet dynamics: dual-use GPU vs bare GPU vs ASIC |
| [`compare_inference_pricing.py`](https://agtico.github.io/assets/research/pearl-economics/compare_inference_pricing.py) | OpenRouter vs Together Pearl API price check |
| [`requirements.txt`](https://agtico.github.io/assets/research/pearl-economics/requirements.txt) | `numpy`, `matplotlib` |

## Quick start

```bash
mkdir pearl-economics && cd pearl-economics
curl -LO https://agtico.github.io/assets/research/pearl-economics/requirements.txt
curl -LO https://agtico.github.io/assets/research/pearl-economics/simulate.py
curl -LO https://agtico.github.io/assets/research/pearl-economics/compare_inference_pricing.py
python3 -m venv .venv
.venv/bin/pip install -r requirements.txt
.venv/bin/python simulate.py --all --plot
.venv/bin/python compare_inference_pricing.py
```

Parameters are illustrative — not calibrated to Pearl mainnet.
