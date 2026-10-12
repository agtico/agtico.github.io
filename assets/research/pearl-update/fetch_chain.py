"""Pearl mainnet history from the public Blockbook indexer: one row every STEP blocks (height, time, difficulty, block
reward). Implied hashrate is calibrated later (economics.py) from a published network hashrate, since Pearl's
difficulty is not in hash units.

    python fetch_chain.py [--step 500]  -> data/chain.csv
"""
import argparse
import csv
import json
import time
import urllib.request
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

API = "https://blockbook.pearlresearch.ai/api/v2"
OUT = Path(__file__).resolve().parent / "data" / "chain.csv"


def get(url, tries=4):
    for k in range(tries):
        try:
            with urllib.request.urlopen(urllib.request.Request(url, headers={"User-Agent": "agti-research"}), timeout=30) as r:
                return json.load(r)
        except Exception:
            if k == tries - 1:
                raise
            time.sleep(1.5 * (k + 1))


def row(h):
    b = get(f"{API}/block/{get(f'{API}/block-index/{h}')['blockHash']}?pageSize=1")
    reward = sum(int(o.get("value", 0)) for o in b["txs"][0].get("vout", [])) / 1e8   # coinbase incl. fees
    return {"height": h, "time": b["time"], "difficulty": float(b["difficulty"]), "coinbase_prl": round(reward, 4)}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--step", type=int, default=500)
    a = ap.parse_args()
    tip = get(f"{API}/")["backend"]["blocks"]
    heights = list(range(1, tip, a.step)) + [tip]
    with ThreadPoolExecutor(8) as pool:
        rows = list(pool.map(row, heights))
    OUT.parent.mkdir(exist_ok=True)
    with OUT.open("w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=list(rows[0]))
        w.writeheader()
        w.writerows(rows)
    print(f"{len(rows)} rows to {OUT} (tip {tip})")


if __name__ == "__main__":
    main()
