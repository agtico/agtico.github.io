"""PRL daily price, market cap and volume from CoinGecko's public API (coin id pearl-2, homepage pearlresearch.ai).

    python fetch_market.py  -> data/market.csv
"""
import csv
import datetime as dt
import json
import urllib.request
from pathlib import Path

URL = "https://api.coingecko.com/api/v3/coins/pearl-2/market_chart?vs_currency=usd&days=365&interval=daily"
OUT = Path(__file__).resolve().parent / "data" / "market.csv"


def main():
    req = urllib.request.Request(URL, headers={"User-Agent": "Mozilla/5.0 (agti-research)"})
    d = json.load(urllib.request.urlopen(req, timeout=60))
    rows = {}
    for key, col in (("prices", "price_usd"), ("market_caps", "market_cap_usd"), ("total_volumes", "volume_usd")):
        for ms, v in d[key]:
            day = dt.datetime.fromtimestamp(ms / 1000, dt.timezone.utc).strftime("%Y-%m-%d")
            rows.setdefault(day, {"date": day})[col] = v
    OUT.parent.mkdir(exist_ok=True)
    with OUT.open("w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=["date", "price_usd", "market_cap_usd", "volume_usd"])
        w.writeheader()
        w.writerows(sorted(rows.values(), key=lambda r: r["date"]))
    print(f"{len(rows)} days to {OUT}")


if __name__ == "__main__":
    main()
