#!/usr/bin/env python3
"""Fetch direct XRPL amendment status for the public live audit packet."""

from __future__ import annotations

import hashlib
import json
import urllib.request
from datetime import datetime, timezone
from pathlib import Path


ROOT = Path(__file__).resolve().parent
OUT = ROOT / "direct_xrpl_amendment_status_20260527.json"
RPC_URL = "https://s1.ripple.com:51234/"
PUBLIC_FEATURES = [
    "AMM",
    "MPTokensV1",
    "PermissionedDomains",
    "PermissionedDEX",
    "TokenEscrow",
]


def rpc(method: str, params: dict | None = None) -> dict:
    payload = {"method": method, "params": [params or {}]}
    request = urllib.request.Request(
        RPC_URL,
        data=json.dumps(payload).encode("utf-8"),
        headers={"Content-Type": "application/json"},
    )
    with urllib.request.urlopen(request, timeout=30) as response:
        data = json.load(response)
    result = data.get("result", {})
    if result.get("status") == "error":
        raise RuntimeError(f"{method} failed: {result}")
    return result


def amendments_index() -> str:
    # rippled keylet::amendments() = sha512Half(uint16('f')).
    return hashlib.sha512(bytes.fromhex("0066")).digest()[:32].hex().upper()


def main() -> int:
    feature_result = rpc("feature")
    features = feature_result["features"]
    by_name = {value["name"]: {"id": key, **value} for key, value in features.items()}

    index = amendments_index()
    ledger_entry = rpc("ledger_entry", {"ledger_hash": feature_result["ledger_hash"], "index": index})
    enabled_hashes = set(ledger_entry["node"]["Amendments"])

    public = {}
    for name in PUBLIC_FEATURES:
        item = by_name[name]
        if item["id"] not in enabled_hashes:
            raise RuntimeError(f"{name} is not in the on-ledger Amendments object")
        if item["enabled"] is not True:
            raise RuntimeError(f"{name} is not enabled")
        public[name] = {
            "id": item["id"],
            "enabled": item["enabled"],
            "supported": item["supported"],
        }

    output = {
        "source": "direct XRPL public JSON-RPC",
        "rpc_url": RPC_URL,
        "checked_utc": datetime.now(timezone.utc).replace(microsecond=0).isoformat().replace("+00:00", "Z"),
        "feature_rpc": {
            "ledger_hash": feature_result["ledger_hash"],
            "ledger_index": feature_result["ledger_index"],
            "validated": feature_result["validated"],
        },
        "amendments_ledger_entry": {
            "index": index,
            "ledger_hash": ledger_entry["ledger_hash"],
            "ledger_index": ledger_entry["ledger_index"],
            "validated": ledger_entry["validated"],
            "enabled_count": len(ledger_entry["node"]["Amendments"]),
        },
        "public_enabled_features": public,
    }
    OUT.write_text(json.dumps(output, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    print(f"wrote {OUT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
