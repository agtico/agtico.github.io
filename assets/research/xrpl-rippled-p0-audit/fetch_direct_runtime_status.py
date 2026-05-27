#!/usr/bin/env python3
"""Fetch direct XRPL runtime and amendment status for the public audit packet."""

from __future__ import annotations

import hashlib
import json
import urllib.request
from datetime import datetime, timezone
from pathlib import Path


ROOT = Path(__file__).resolve().parent
OUT = ROOT / "direct_xrpl_mainnet_runtime_status_20260527.json"
RPC_URLS = ["https://s1.ripple.com:51234/", "https://s2.ripple.com:51234/"]
SCOPE_FEATURES = [
    "AMM",
    "AMMClawback",
    "MPTokensV1",
    "PermissionedDomains",
    "PermissionedDEX",
    "TokenEscrow",
    "Credentials",
    "LendingProtocol",
    "SingleAssetVault",
    "PermissionDelegation",
    "Batch",
    "fixDelegateV1_1",
    "fixMPTDeliveredAmount",
    "fixAMMv1_3",
    "fixCleanup3_1_3",
]


def rpc(url: str, method: str, params: dict | None = None) -> dict:
    payload = {"method": method, "params": [params or {}]}
    request = urllib.request.Request(
        url,
        data=json.dumps(payload).encode("utf-8"),
        headers={"Content-Type": "application/json"},
    )
    with urllib.request.urlopen(request, timeout=30) as response:
        data = json.load(response)
    result = data.get("result", {})
    if result.get("status") == "error":
        raise RuntimeError(f"{method} failed at {url}: {result}")
    return result


def amendments_index() -> str:
    # rippled keylet::amendments() = sha512Half(uint16('f')).
    return hashlib.sha512(bytes.fromhex("0066")).digest()[:32].hex().upper()


def feature_id(name: str, by_name: dict[str, dict]) -> tuple[str, str, bool | None]:
    if name in by_name:
        item = by_name[name]
        return item["id"], "feature_rpc", item["supported"]
    return hashlib.sha512(name.encode("utf-8")).digest()[:32].hex().upper(), "sha512_half_name", None


def server_info(url: str) -> dict:
    result = rpc(url, "server_info")
    info = result["info"]
    validated = info.get("validated_ledger", {})
    return {
        "rpc_url": url,
        "rippled_version": info.get("rippled_version") or info.get("build_version"),
        "clio_version": info.get("clio_version"),
        "libxrpl_version": info.get("libxrpl_version"),
        "validated_ledger": validated,
        "network_id": info.get("network_id"),
        "time": info.get("time"),
    }


def main() -> int:
    servers = [server_info(url) for url in RPC_URLS]
    feature_result = rpc(RPC_URLS[0], "feature")
    by_name = {
        value["name"]: {"id": key, **value}
        for key, value in feature_result["features"].items()
    }

    index = amendments_index()
    ledger_entry = rpc(RPC_URLS[0], "ledger_entry", {"ledger_hash": feature_result["ledger_hash"], "index": index})
    enabled_hashes = set(ledger_entry["node"]["Amendments"])

    feature_status = {}
    for name in SCOPE_FEATURES:
        amendment_id, id_source, supported = feature_id(name, by_name)
        feature_status[name] = {
            "id": amendment_id,
            "enabled": amendment_id in enabled_hashes,
            "supported": supported,
            "id_source": id_source,
            "feature_rpc_visible": name in by_name,
        }

    output = {
        "source": "direct XRPL public JSON-RPC",
        "checked_utc": datetime.now(timezone.utc).replace(microsecond=0).isoformat().replace("+00:00", "Z"),
        "server_info": servers,
        "feature_rpc": {
            "rpc_url": RPC_URLS[0],
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
        "feature_status": feature_status,
    }
    OUT.write_text(json.dumps(output, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    print(f"wrote {OUT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
