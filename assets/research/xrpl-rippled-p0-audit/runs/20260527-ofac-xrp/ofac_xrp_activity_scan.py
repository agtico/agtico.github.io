#!/usr/bin/env python3
"""Scan direct XRPL account_tx activity for the OFAC XRP address.

This is a triage helper, not an exploit. It fetches account_tx from a public
XRPL JSON-RPC endpoint, separates own-signed activity from counterparty-signed
activity, and checks whether the target address is visibly present in the
transaction or metadata JSON returned by account_tx.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import sys
import urllib.request
from datetime import datetime, timezone
from typing import Any


RIPPLE_EPOCH_OFFSET = 946_684_800


def ripple_iso(ripple_time: int | None) -> str | None:
    if ripple_time is None:
        return None
    return datetime.fromtimestamp(
        ripple_time + RIPPLE_EPOCH_OFFSET, tz=timezone.utc
    ).isoformat().replace("+00:00", "Z")


def sha256_json(value: Any) -> str:
    payload = json.dumps(value, sort_keys=True, separators=(",", ":")).encode()
    return hashlib.sha256(payload).hexdigest()


def rpc(url: str, method: str, params: list[dict[str, Any]]) -> dict[str, Any]:
    req = urllib.request.Request(
        url,
        data=json.dumps({"method": method, "params": params}).encode(),
        headers={"Content-Type": "application/json"},
    )
    with urllib.request.urlopen(req, timeout=60) as handle:
        result = json.load(handle)
    if "error" in result:
        raise RuntimeError(result)
    body = result.get("result", {})
    if body.get("status") == "error":
        raise RuntimeError(body)
    return body


def contains_address(value: Any, address: str) -> bool:
    if isinstance(value, str):
        return value == address
    if isinstance(value, list):
        return any(contains_address(item, address) for item in value)
    if isinstance(value, dict):
        return any(
            contains_address(k, address) or contains_address(v, address)
            for k, v in value.items()
        )
    return False


def trustset_flags(flags: int | None) -> list[str]:
    if flags is None:
        return []
    mapping = {
        0x00010000: "tfSetfAuth",
        0x00020000: "tfSetNoRipple",
        0x00040000: "tfClearNoRipple",
        0x00100000: "tfSetFreeze",
        0x00200000: "tfClearFreeze",
        0x00400000: "tfSetDeepFreeze",
        0x00800000: "tfClearDeepFreeze",
    }
    return [name for bit, name in mapping.items() if flags & bit]


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--account", required=True)
    parser.add_argument("--rpc-url", default="https://s1.ripple.com:51234/")
    parser.add_argument("--stop-before", default="2021-11-08T00:00:00Z")
    parser.add_argument("--limit-pages", type=int, default=12)
    parser.add_argument("--page-size", type=int, default=400)
    args = parser.parse_args()

    stop_dt = datetime.fromisoformat(args.stop_before.replace("Z", "+00:00"))
    stop_ripple = int(stop_dt.timestamp()) - RIPPLE_EPOCH_OFFSET

    marker = None
    pages = 0
    transactions: list[dict[str, Any]] = []
    while pages < args.limit_pages:
        params: dict[str, Any] = {
            "account": args.account,
            "ledger_index_min": -1,
            "ledger_index_max": -1,
            "limit": args.page_size,
            "binary": False,
            "forward": False,
        }
        if marker is not None:
            params["marker"] = marker
        page = rpc(args.rpc_url, "account_tx", [params])
        pages += 1
        batch = page.get("transactions", [])
        transactions.extend(batch)
        if any((entry.get("tx", {}).get("date") or 0) < stop_ripple for entry in batch):
            break
        marker = page.get("marker")
        if marker is None:
            break

    post_stop = [
        entry
        for entry in transactions
        if (entry.get("tx", {}).get("date") or 0) >= stop_ripple
    ]

    compact = []
    own = []
    counterparty = []
    missing_visible_address = []
    counterparty_fee_drops = 0
    counterparty_success_fee_drops = 0

    for entry in post_stop:
        tx = entry.get("tx", {})
        meta = entry.get("meta", {})
        visible_in_tx = contains_address(tx, args.account)
        visible_in_meta = contains_address(meta, args.account)
        if not (visible_in_tx or visible_in_meta):
            missing_visible_address.append(tx.get("hash"))
        fee = int(tx.get("Fee", "0")) if str(tx.get("Fee", "0")).isdigit() else 0
        row = {
            "hash": tx.get("hash"),
            "ledger_index": tx.get("ledger_index") or tx.get("inLedger"),
            "date": ripple_iso(tx.get("date")),
            "transaction_type": tx.get("TransactionType"),
            "account": tx.get("Account"),
            "result": meta.get("TransactionResult"),
            "fee_drops": fee,
            "visible_in_tx": visible_in_tx,
            "visible_in_meta": visible_in_meta,
            "trustset_flags": trustset_flags(tx.get("Flags"))
            if tx.get("TransactionType") == "TrustSet"
            else [],
        }
        compact.append(row)
        if tx.get("Account") == args.account:
            own.append(row)
        else:
            counterparty.append(row)
            counterparty_fee_drops += fee
            if meta.get("TransactionResult") == "tesSUCCESS":
                counterparty_success_fee_drops += fee

    summary = {
        "source": "direct XRPL public JSON-RPC account_tx",
        "rpc_url": args.rpc_url,
        "retrieved_utc": datetime.now(timezone.utc)
        .isoformat()
        .replace("+00:00", "Z"),
        "account": args.account,
        "stop_before": args.stop_before,
        "pages_fetched": pages,
        "transactions_scanned": len(transactions),
        "transactions_at_or_after_stop": len(post_stop),
        "own_signed_at_or_after_stop": len(own),
        "counterparty_signed_at_or_after_stop": len(counterparty),
        "latest_account_involving_tx": compact[0] if compact else None,
        "latest_own_signed_tx": own[0] if own else None,
        "latest_counterparty_signed_tx": counterparty[0] if counterparty else None,
        "counterparty_fee_drops_at_or_after_stop": counterparty_fee_drops,
        "counterparty_success_fee_drops_at_or_after_stop": counterparty_success_fee_drops,
        "counterparty_success_fee_xrp_at_or_after_stop": counterparty_success_fee_drops
        / 1_000_000,
        "missing_visible_address_hashes": missing_visible_address,
        "visible_address_check": "pass"
        if not missing_visible_address
        else "fail",
        "transactions": compact,
    }
    summary["summary_sha256"] = sha256_json(summary)
    json.dump(summary, sys.stdout, indent=2, sort_keys=True)
    sys.stdout.write("\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
