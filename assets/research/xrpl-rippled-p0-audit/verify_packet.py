#!/usr/bin/env python3
"""Verify the live-mainnet XRPL P0 evidence packet.

This is a packet-only static consistency check. It verifies that the manifest,
direct XRPL live-state receipts, remediation receipt, proof log, and per-finding
wrappers agree. It does not run rippled.
"""

from __future__ import annotations

import hashlib
import json
import os
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parent
MANIFEST = ROOT / "repro_manifest.json"
AMENDMENT_STATUS = ROOT / "direct_xrpl_amendment_status_20260527.json"
RUNTIME_STATUS = ROOT / "direct_xrpl_mainnet_runtime_status_20260527.json"
REMEDIATION_STATUS = ROOT / "upstream_remediation_status_20260527.json"
MOBY_DICK_LIVE_STATE = ROOT / "runs/20260527-p0-hunt/live_state_snapshot_20260528_moby_dick.json"

EXPECTED_RECORD_COUNT = 19

REQUIRED_ENABLED = {
    "AMM",
    "AMMClawback",
    "Checks",
    "CheckCashMakesTrustLine",
    "DepositAuth",
    "DisallowIncoming",
    "fixDisallowIncomingV1",
    "MPTokensV1",
    "NonFungibleTokensV1_1",
    "fixEnforceNFTokenTrustline",
    "fixEnforceNFTokenTrustlineV2",
    "fixRemoveNFTokenAutoTrustLine",
    "fixNFTokenReserve",
    "fixNFTokenRemint",
    "NFTokenMintOffer",
    "PermissionedDomains",
    "PermissionedDEX",
    "TokenEscrow",
    "Credentials",
    "fixMPTDeliveredAmount",
    "fixAMMv1_3",
    "fixTokenEscrowV1",
    "fixAMMClawbackRounding",
}

REQUIRED_DISABLED = {
    "LendingProtocol",
    "SingleAssetVault",
    "PermissionDelegation",
    "Batch",
    "fixDelegateV1_1",
    "fixDisallowIncomingV1_1",
}

FORBIDDEN_IDS = {
    "MPT-DOMAIN-AUTH-001",
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


def read_json(path: Path) -> dict:
    return json.loads(read_text(path))


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def amendment_enabled(amendment_status: dict, name: str) -> bool:
    public = amendment_status.get("public_enabled_features", {})
    scope = amendment_status.get("scope_features", {})
    if name in public:
        return public[name]["enabled"] is True
    return scope[name]["enabled"] is True


def runtime_enabled(runtime_status: dict, name: str) -> bool:
    return runtime_status["feature_status"][name]["enabled"] is True


def check_required_feature_sets(amendment_status: dict, runtime_status: dict) -> None:
    for amendment in REQUIRED_ENABLED:
        require(amendment_enabled(amendment_status, amendment), f"required live amendment is not enabled: {amendment}")
        require(runtime_enabled(runtime_status, amendment), f"runtime receipt missing enabled amendment: {amendment}")

    for disabled in REQUIRED_DISABLED:
        require(
            amendment_status["scope_features"][disabled]["enabled"] is False,
            f"disabled surface unexpectedly enabled in amendment receipt: {disabled}",
        )
        require(
            runtime_status["feature_status"][disabled]["enabled"] is False,
            f"disabled surface unexpectedly enabled in runtime receipt: {disabled}",
        )

    cleanup = amendment_status["scope_features"]["fixCleanup3_1_3"]
    require(cleanup["enabled"] is True, "fixCleanup3_1_3 must be enabled by raw Amendments hash")
    require(
        cleanup["id"] == "303ACB16CF8DBD3B5C34F131A9D19A7DE01AE05F480A8A682B869D1B4AAC8CFC",
        "unexpected fixCleanup3_1_3 amendment hash",
    )
    require(cleanup["id_source"] == "sha512_half_name", "fixCleanup3_1_3 must be raw-hash checked")


def check_direct_receipts(manifest: dict, amendment_status: dict, runtime_status: dict) -> None:
    require(
        amendment_status["source"] == "direct XRPL public JSON-RPC",
        "amendment receipt must be direct XRPL JSON-RPC",
    )
    require(amendment_status["feature_rpc"]["validated"] is True, "feature RPC result must be validated")
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
        runtime_status["source"] == "direct XRPL public JSON-RPC",
        "runtime receipt must be direct XRPL JSON-RPC",
    )
    require(
        runtime_status["feature_rpc"]["ledger_hash"] == runtime_status["amendments_ledger_entry"]["ledger_hash"],
        "runtime feature and amendments checks must bind to the same ledger hash",
    )
    require(
        runtime_status["feature_rpc"]["ledger_index"] == runtime_status["amendments_ledger_entry"]["ledger_index"],
        "runtime feature and amendments checks must bind to the same ledger index",
    )
    for server in runtime_status["server_info"]:
        require(server["rippled_version"] == "3.1.3", "public server runtime version must be 3.1.3")

    check_required_feature_sets(amendment_status, runtime_status)


def check_remediation(remediation_status: dict) -> None:
    require(
        remediation_status["source"] == "local XRPLF/rippled git ancestry after fetch --all --tags --prune",
        "unexpected remediation source",
    )
    unresolved = set(remediation_status["summary"]["not_confirmed_fixed_in_3_2_0_b7_or_develop"])
    require(
        unresolved == {
            "MPT-TRANSFER-RATE-OVERFLOW-001",
            "MPT-LOCK-UNAUTH-001",
            "TRUSTLINE-POSITIVE-BALANCE-RESERVE-001",
            "TRUSTLINE-DISALLOW-INCOMING-OFFER-001",
            "NFTOKEN-DISALLOW-INCOMING-ACCEPT-001",
            "NFTOKEN-BROKER-FEE-DISALLOW-INCOMING-TRUSTLINE-001",
            "CHECKCASH-DISALLOW-INCOMING-TRUSTLINE-001",
            "TOKENESCROW-DISALLOW-INCOMING-FINISH-001",
            "AMMWITHDRAW-DISALLOW-INCOMING-TRUSTLINE-001",
            "AMMCREATE-DISALLOW-INCOMING-TRUSTLINE-001",
            "AMMDEPOSIT-EMPTY-DISALLOW-INCOMING-TRUSTLINE-001",
            "AMMCLAWBACK-DISALLOW-INCOMING-PAIRED-ASSET-001",
            "AMMCLAWBACK-DEPOSITAUTH-PAIRED-ASSET-001",
            "AMMBID-DEPOSITAUTH-REFUND-001",
        },
        "unexpected unresolved remediation set",
    )
    patched = set(remediation_status["summary"]["patched_in_3_2_0_b7_or_develop"])
    require(len(patched) == 5, "unexpected remediated finding count")
    for rid in unresolved:
        record = remediation_status["records"][rid]
        for fix in record["fix_commits"]:
            require(fix["in_3_2_0_b7"] is False, f"{rid} unexpectedly fixed in 3.2.0-b7")
            require(fix["in_origin_develop"] is False, f"{rid} unexpectedly fixed in origin/develop")


def check_old_tag_trustline_repros() -> None:
    expected = {
        "runs/20260527-p0-hunt/trustline_positive_balance_2_5_0_repro.patch": (
            "f2b8bb9a31890aa91c7389b9a79157b64dc3d9824d22d6876d3716c4fc28e48d"
        ),
        "runs/20260527-p0-hunt/trustline_positive_balance_2_5_0_repro.log": (
            "3e9968cf305851dbd2cdf9f8a50d55b3dab784649af1fb7d9993fdd0fcccb494"
        ),
        "runs/20260527-p0-hunt/trustline_positive_balance_2_0_0_repro.patch": (
            "07651b66c8e5f0e3e39fac776d57933a767988b8e663efb072eae1804be08cd9"
        ),
        "runs/20260527-p0-hunt/trustline_positive_balance_2_0_0_repro.log": (
            "8fc64bef728cb84a1f38e4763877e530a6ce54716706ac34ffb3957b070794b2"
        ),
        "runs/20260527-p0-hunt/trustline_positive_balance_1_5_0_repro.patch": (
            "b30a319451845797e1f55416990018b7c73a6759d5d2b9b6c853a83c57f5b047"
        ),
        "runs/20260527-p0-hunt/trustline_positive_balance_1_5_0_repro.log": (
            "cba00449acaaad475b6e3d8824ffb0ee0abb4b981bbb62e44ca4ae9777c7c12f"
        ),
    }

    for relative, expected_hash in expected.items():
        path = ROOT / relative
        require(path.exists(), f"missing old-tag trustline repro artifact: {relative}")
        require(sha256(path) == expected_hash, f"old-tag trustline repro hash mismatch: {relative}")

    checks = {
        "2_5_0": (
            "Legacy 2.5.0 -- offer crossing creates positive balance without reserve",
            "32 cases, 1656 tests total, 0 failures",
        ),
        "2_0_0": (
            "Legacy 2.0.0 -- offer crossing creates positive balance without reserve",
            "22 cases, 700 tests total, 0 failures",
        ),
        "1_5_0": (
            "Legacy 1.5.0 -- offer crossing creates positive balance without reserve",
            "9 cases, 271 tests total, 0 failures",
        ),
    }

    for tag_key, (marker, footer) in checks.items():
        log = read_text(ROOT / f"runs/20260527-p0-hunt/trustline_positive_balance_{tag_key}_repro.log")
        require(marker in log, f"old-tag trustline repro log missing marker: {tag_key}")
        require(footer in log, f"old-tag trustline repro log missing zero-failure footer: {tag_key}")


def check_moby_dick_live_state() -> None:
    require(MOBY_DICK_LIVE_STATE.exists(), "missing Moby Dick live-state snapshot")
    require(
        sha256(MOBY_DICK_LIVE_STATE) == "e4f756f7ae60087a90ff7e4eaf15292fe092caecfab69ff3c22116e6a546c972",
        "Moby Dick live-state snapshot SHA-256 mismatch",
    )
    snapshot = read_json(MOBY_DICK_LIVE_STATE)
    require(snapshot["source"] == "direct XRPL public JSON-RPC", "unexpected Moby Dick snapshot source")
    require(snapshot["feature_rpc"]["validated"] is True, "Moby Dick feature RPC must be validated")
    require(
        snapshot["feature_rpc"]["ledger_hash"] == snapshot["amendments_ledger_entry"]["ledger_hash"],
        "Moby Dick feature and Amendments checks must bind to the same ledger hash",
    )
    for server in snapshot["server_info"]:
        require(server["rippled_version"] == "3.1.3", "Moby Dick snapshot server runtime must be 3.1.3")
    require(
        snapshot["feature_status"]["fixCleanup3_1_3"]["enabled"] is True,
        "Moby Dick snapshot must show fixCleanup3_1_3 enabled",
    )
    require(
        snapshot["feature_status"]["Checks"]["enabled"] is True
        and snapshot["feature_status"]["CheckCashMakesTrustLine"]["enabled"] is True,
        "Moby Dick snapshot must show CheckCash live features enabled",
    )


def check_manifest_records(
    manifest: dict,
    amendment_status: dict,
    runtime_status: dict,
    proof_text: str,
) -> int:
    records = manifest["records"]
    require(len(records) == EXPECTED_RECORD_COUNT, f"live manifest must contain exactly {EXPECTED_RECORD_COUNT} findings")
    require(
        manifest["live_scope"]["included_ids"] == [record["id"] for record in records],
        "live-scope included IDs must match manifest record order",
    )

    seen_ids: set[str] = set()
    marker_count = 0
    for record in records:
        rid = record["id"]
        require(rid not in seen_ids, f"duplicate record id: {rid}")
        require(rid not in FORBIDDEN_IDS, f"finding outside live manifest leaked into packet: {rid}")
        seen_ids.add(rid)

        required = record.get("required_amendments")
        require(isinstance(required, list), f"{rid} missing required_amendments")
        if not required:
            require(
                isinstance(record.get("required_baseline_surface"), str) and record["required_baseline_surface"],
                f"{rid} has no amendment gate and must name required_baseline_surface",
            )
        for amendment in required:
            require(amendment_enabled(amendment_status, amendment), f"{rid} requires disabled amendment: {amendment}")
            require(runtime_enabled(runtime_status, amendment), f"{rid} runtime receipt missing amendment: {amendment}")

        for amendment in record.get("required_disabled_amendments", []):
            require(
                amendment_status["scope_features"][amendment]["enabled"] is False,
                f"{rid} requires amendment to be disabled, but direct receipt shows enabled: {amendment}",
            )
            require(
                runtime_status["feature_status"][amendment]["enabled"] is False,
                f"{rid} requires amendment to be disabled, but runtime receipt shows enabled: {amendment}",
            )

        script = ROOT / record["script"]
        require(script.exists(), f"missing repro script for {rid}: {script}")
        require(os.access(script, os.X_OK), f"repro script is not executable: {script}")
        script_text = read_text(script)
        require(f'"{rid}"' in script_text, f"repro script does not target {rid}: {script}")

        source = ROOT / record["source"]
        require(source.exists(), f"missing source for {rid}: {source}")

        for marker in record["markers"]:
            marker_count += 1
            require(marker in proof_text, f"proof log missing marker for {rid}: {marker}")

    return marker_count


def main() -> int:
    manifest = read_json(MANIFEST)
    amendment_status = read_json(AMENDMENT_STATUS)
    runtime_status = read_json(RUNTIME_STATUS)
    remediation_status = read_json(REMEDIATION_STATUS)
    proof = manifest["proof"]
    proof_log = ROOT / proof["log"]
    proof_text = read_text(proof_log)

    require(manifest["target"]["repo"] == "XRPLF/rippled", "unexpected target repo")
    require(manifest["target"]["tag"] == "3.1.3", "unexpected target tag")
    require(
        manifest["target"]["commit"] == "46b241ace8b30d9c9775d60ffba7d24b21903896",
        "unexpected target commit",
    )
    require(sha256(proof_log) == proof["sha256"], "proof log SHA-256 mismatch")
    require("64 cases, 16354 tests total, 0 failures" in proof_text, "proof log missing OpenP0Repro footer")
    require("ripple.tx.OpenP0ReproCrash had 0 failures." in proof_text, "proof log missing crash-control footer")

    check_direct_receipts(manifest, amendment_status, runtime_status)
    check_remediation(remediation_status)
    check_old_tag_trustline_repros()
    check_moby_dick_live_state()
    marker_count = check_manifest_records(manifest, amendment_status, runtime_status, proof_text)

    print("packet-ok")
    print(f"records={len(manifest['records'])} markers={marker_count} proof_sha256={proof['sha256']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
