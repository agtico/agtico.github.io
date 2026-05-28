#!/usr/bin/env python3
"""Fetch direct XRPL account state for XRP addresses extracted from OFAC."""

from __future__ import annotations

import argparse
import hashlib
import json
import sys
import urllib.request
from datetime import datetime, timezone
from typing import Any


DEFAULT_RPC_URLS = ("https://s1.ripple.com:51234/", "https://s2.ripple.com:51234/")


def sha256_json(value: Any) -> str:
    payload = json.dumps(value, sort_keys=True, separators=(",", ":")).encode()
    return hashlib.sha256(payload).hexdigest()


def rpc(url: str, method: str, params: list[dict[str, Any]] | None = None) -> dict[str, Any]:
    body = {"method": method}
    if params is not None:
        body["params"] = params
    request = urllib.request.Request(
        url,
        data=json.dumps(body).encode(),
        headers={"Content-Type": "application/json"},
    )
    with urllib.request.urlopen(request, timeout=60) as handle:
        response = json.load(handle)
    if "error" in response:
        raise RuntimeError(response)
    result = response.get("result", {})
    if result.get("status") == "error":
        raise RuntimeError(result)
    return result


def query_account(url: str, account: str) -> dict[str, Any]:
    queries: dict[str, Any] = {}
    for method, params in (
        ("account_info", {"account": account, "ledger_index": "validated", "strict": True}),
        ("account_objects", {"account": account, "ledger_index": "validated", "limit": 400}),
        ("account_lines", {"account": account, "ledger_index": "validated", "limit": 400}),
        ("account_offers", {"account": account, "ledger_index": "validated", "limit": 400}),
        ("account_channels", {"account": account, "ledger_index": "validated", "limit": 400}),
    ):
        try:
            queries[method] = rpc(url, method, [params])
        except Exception as exc:  # keep the receipt complete even when one RPC lacks support
            queries[method] = {"status": "error", "error": str(exc)}
    return {
        "rpc_url": url,
        "server_info": rpc(url, "server_info").get("info", {}),
        "queries": queries,
    }


def load_addresses(path: str) -> list[str]:
    with open(path, "r", encoding="utf-8") as handle:
        snapshot = json.load(handle)
    addresses = snapshot.get("xrp_addresses")
    if not isinstance(addresses, list) or not all(isinstance(a, str) for a in addresses):
        raise ValueError(f"{path} does not contain xrp_addresses")
    return sorted(set(addresses))


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--sdn-snapshot", required=True)
    parser.add_argument("--rpc-url", action="append", default=[])
    args = parser.parse_args()

    urls = args.rpc_url or list(DEFAULT_RPC_URLS)
    addresses = load_addresses(args.sdn_snapshot)
    responses = {
        address: [query_account(url, address) for url in urls] for address in addresses
    }
    snapshot = {
        "source": "direct XRPL public JSON-RPC",
        "checked_utc": datetime.now(timezone.utc).isoformat().replace("+00:00", "Z"),
        "sdn_snapshot": args.sdn_snapshot,
        "xrp_addresses": addresses,
        "responses": responses,
    }
    snapshot["snapshot_sha256"] = sha256_json(snapshot)
    json.dump(snapshot, sys.stdout, indent=2, sort_keys=True)
    sys.stdout.write("\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
