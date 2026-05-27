#!/usr/bin/env python3
"""Verify the public XRPL P0 audit packet.

This is a static consistency check. It verifies that the article, manifest,
proof log, per-finding wrappers, markers, and local asset links agree.
It does not run rippled.
"""

from __future__ import annotations

import hashlib
import json
import os
import re
import sys
import urllib.parse
from pathlib import Path


ROOT = Path(__file__).resolve().parent
SITE_ROOT = ROOT.parents[2]
ARTICLE = SITE_ROOT / "_posts" / "2026-05-26-xrpl-rippled-open-p0-freeze-audit.md"
MANIFEST = ROOT / "repro_manifest.json"
AMENDMENT_STATUS = ROOT / "direct_xrpl_amendment_status_20260527.json"


def fail(message: str) -> None:
    print(f"FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)


def require(condition: bool, message: str) -> None:
    if not condition:
        fail(message)


def read_text(path: Path) -> str:
    try:
        return path.read_text(encoding="utf-8")
    except FileNotFoundError:
        fail(f"missing file: {path}")


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def local_asset_links(markdown: str) -> set[str]:
    links: set[str] = set()
    patterns = [
        r"\]\((/assets/[^)#]+)",
        r"href=\"(/assets/[^\"]+)\"",
        r"src=\"(/assets/[^\"]+)\"",
    ]
    for pattern in patterns:
        links.update(re.findall(pattern, markdown))
    return links


def main() -> int:
    manifest = json.loads(read_text(MANIFEST))
    amendment_status = json.loads(read_text(AMENDMENT_STATUS))
    article = read_text(ARTICLE)
    proof = manifest["proof"]
    records = manifest["records"]
    proof_log = ROOT / proof["log"]
    proof_text = read_text(proof_log)

    require(manifest["target"]["repo"] == "XRPLF/rippled", "unexpected target repo")
    require(manifest["target"]["tag"] == "3.1.3", "unexpected target tag")
    require(
        manifest["target"]["commit"] == "46b241ace8b30d9c9775d60ffba7d24b21903896",
        "unexpected target commit",
    )
    require(sha256(proof_log) == proof["sha256"], "proof log SHA-256 mismatch")

    require(
        "RippleD 3.1.3 Audit: Live Mainnet-Enabled High/Critical Findings" in article,
        "article title mismatch",
    )
    require(len(records) == 8, "live manifest must contain exactly 8 public findings")
    require("## Live Amendment Filter" in article, "article missing live amendment filter")
    require("## Evidence Packet" in article, "article missing evidence packet section")
    require("## Table Of Contents" in article, "article missing table of contents")
    blocked_article_terms = [
        "XR" + "PSCAN",
        "live" + "_amendment" + "_status_" + "20260527.json",
        "What Was " + "Removed",
    ]
    for blocked in blocked_article_terms:
        require(blocked not in article, f"article contains blocked term: {blocked}")
    require("47 cases, 9119 tests total, 0 failures" in proof_text, "proof log missing OpenP0Repro footer")
    require("ripple.tx.OpenP0ReproCrash had 0 failures." in proof_text, "proof log missing crash-control footer")

    require(
        amendment_status["source"] == "direct XRPL public JSON-RPC",
        "amendment receipt must be direct XRPL JSON-RPC",
    )
    require(
        amendment_status["feature_rpc"]["validated"] is True,
        "feature RPC result must be validated",
    )
    require(
        amendment_status["amendments_ledger_entry"]["validated"] is True,
        "Amendments ledger entry must be validated",
    )
    require(
        amendment_status["feature_rpc"]["ledger_hash"] == amendment_status["amendments_ledger_entry"]["ledger_hash"],
        "feature and Amendments ledger-entry checks must bind to the same ledger hash",
    )
    require(
        amendment_status["feature_rpc"]["ledger_index"] == amendment_status["amendments_ledger_entry"]["ledger_index"],
        "feature and Amendments ledger-entry checks must bind to the same ledger index",
    )
    require(
        amendment_status["checked_utc"] == manifest["live_scope"]["checked_utc"],
        "manifest live-scope timestamp must match amendment receipt",
    )
    require(
        amendment_status["checked_utc"] in article,
        "article must name the amendment receipt timestamp",
    )
    required_enabled = {
        "AMM",
        "MPTokensV1",
        "PermissionedDomains",
        "PermissionedDEX",
        "TokenEscrow",
    }
    for amendment in required_enabled:
        actual = amendment_status["public_enabled_features"][amendment]["enabled"]
        require(actual is True, f"required public amendment is not enabled: {amendment}")

    forbidden_ids = {
        "LEND-FREEZE-001",
        "LOANBROKER-COVER-PRECISION-001",
        "LOAN-MINCOVER-SCALE-001",
        "VAULT-SHARE-MPT-TRANSFER-001",
        "LOANBROKER-LOCKED-MPT-001",
        "LOAN-PAYMENT-FACTOR-001",
        "VAULT-WITHDRAW-SCALE-BOUNDARY-001",
        "VAULT-DEPOSIT-ISSUER-EDGE-001",
        "VAULT-SOLE-SHAREHOLDER-IMPAIRED-001",
        "VAULT-DEPOSIT-OPPOSITE-LIMIT-001",
        "DELEGATE-DELETE-STALE-001",
        "DELEGATE-FEE-RESERVE-001",
        "DELEGATE-SAV-001",
        "DELEGATE-MULTISIGN-001",
        "DELEGATE-MPT-GRANULAR-MUTATION-001",
        "DELEGATE-EMPTY-ACCOUNTSET-001",
        "BATCH-SIGNER-OUTER-REPLAY-001",
        "MPT-LOCK-UNAUTH-NOSAV-001",
        "MPT-STISSUE-WIRE-001",
        "NUMBER-CUSP-UPWARD-001",
        "NUMBER-DIVISION-UPWARD-001",
        "PDOMAIN-TICKET-001",
        "MPT-MULTISEND-001",
        "VAULT-WITHDRAW-001",
        "VAULT-MPT-ESCROW-001",
        "VAULT-CLAWBACK-001",
        "LOANPAY-FEE-001",
        "INVARIANT-BOOL-OVERWRITE-001",
        "CREDENTIAL-EXPIRED-DELETE-001",
        "PDEX-HYBRID-EMPTY-BOOKS-001",
    }
    for forbidden_id in forbidden_ids:
        require(forbidden_id not in article, f"finding outside live manifest leaked into article: {forbidden_id}")

    anchors = set(re.findall(r'<a id="([^"]+)"></a>', article))
    seen_ids: set[str] = set()
    marker_count = 0

    for record in records:
        rid = record["id"]
        require(rid not in seen_ids, f"duplicate record id: {rid}")
        seen_ids.add(rid)

        script = ROOT / record["script"]
        require(script.exists(), f"missing repro script for {rid}: {script}")
        require(os.access(script, os.X_OK), f"repro script is not executable: {script}")
        script_text = read_text(script)
        require(f'"{rid}"' in script_text, f"repro script does not target {rid}: {script}")

        source = ROOT / record["source"]
        require(source.exists(), f"missing source for {rid}: {source}")
        require(record["anchor"] in anchors, f"missing article anchor for {rid}: {record['anchor']}")
        require(f"### {rid} - " in article, f"missing article section for {rid}")
        require(f"repros/{rid}.sh" in article, f"article missing repro link for {rid}")

        for marker in record["markers"]:
            marker_count += 1
            require(marker in proof_text, f"proof log missing marker for {rid}: {marker}")

    for link in local_asset_links(article):
        path = SITE_ROOT / urllib.parse.unquote(link.lstrip("/"))
        require(path.exists(), f"article links missing asset: {link}")

    print("packet-ok")
    print(f"records={len(records)} markers={marker_count} proof_sha256={proof['sha256']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
