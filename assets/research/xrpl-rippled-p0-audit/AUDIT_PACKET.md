# XRPL RippleD Live Mainnet-Enabled Evidence Packet

This directory is the live-filtered evidence packet for the AGTI report:

`/intelligence-reports/2026/05/26/xrpl-rippled-open-p0-freeze-audit/`

The packet includes only reproduced high/critical findings whose required
XRPL mainnet amendment surfaces were enabled in a direct XRPL validated-ledger
query checked at `2026-05-27T16:36:40Z`. Public XRPL servers checked in
`direct_xrpl_mainnet_runtime_status_20260527.json` reported
`rippled_version=3.1.3`.

The live filter also checks raw amendment hashes from the on-ledger
`Amendments` object. That matters because public Clio feature-name lookup did
not expose `fixCleanup3_1_3`, while the raw Amendments object did contain its
hash. Cleanup-era candidates are therefore excluded unless the reproduced
behavior survives with `fixCleanup3_1_3` enabled.

## Target

- Upstream repo: `XRPLF/rippled`
- Target tag: `3.1.3`
- Target commit: `46b241ace8b30d9c9775d60ffba7d24b21903896`
- Local proof suite: `OpenP0Repro`
- Negative-control suite: `OpenP0ReproCrash`
- Live amendment source: direct XRPL public JSON-RPC at `https://s1.ripple.com:51234/`

## Live Inclusion Rule

Included findings must satisfy both conditions:

1. The reproduced behavior applies to a current `rippled 3.1.3` code path.
2. The required XRPL mainnet amendment surface is enabled in the direct
   validated-ledger amendment receipt.

## Packet Contents

| File | Purpose |
|---|---|
| `repro_manifest.json` | Canonical live manifest: 9 live-mainnet eligible high/critical findings, markers, risk labels, and proof bindings. |
| `direct_xrpl_amendment_status_20260527.json` | Direct XRPL `feature` and `ledger_entry` receipt used for the live filter. |
| `direct_xrpl_mainnet_runtime_status_20260527.json` | Direct XRPL server/runtime receipt showing public server versions and live amendment state. |
| `upstream_remediation_status_20260527.json` | Git-ancestry remediation receipt for `3.2.0-b7` and `origin/develop`. |
| `runs/20260527-p0-hunt/live_p0_hunt_v2_triage.md` | Continuation triage for the live-only promotions and excluded candidates. |
| `runs/20260527-p0-hunt/live_mainnet_enabled_proof_extract_20260527_v3.log` | Live-only proof extract with marker coverage and zero-failure footer. |
| `fetch_direct_amendment_status.py` | Refreshes direct amendment and raw-hash live-status evidence. |
| `fetch_direct_runtime_status.py` | Refreshes direct public-server runtime and amendment evidence. |
| `run_repro.sh` | Common runner used by every per-finding wrapper. |
| `repros/*.sh` | Per-finding wrappers. Only IDs present in `repro_manifest.json` are packet findings. |
| `verify_packet.py` | Packet-only static verifier for manifest records, scripts, markers, amendment snapshots, remediation status, and proof hash. |

The packet is scoped to the live findings list.

`MPT-DOMAIN-AUTH-001` is explicitly excluded from the live packet because the
reproduced MPT `DomainID` path requires `SingleAssetVault` in the current
`MPTokenIssuanceCreate` / `MPTokenIssuanceSet` feature gate, and direct XRPL
mainnet status shows `SingleAssetVault=false`.

## Remediation Boundary

Five packet findings have confirmed post-3.1.3 remediation in `3.2.0-b7` or
`origin/develop`. Four packet findings do not:

- `MPT-TRANSFER-RATE-OVERFLOW-001`
- `MPT-LOCK-UNAUTH-001`
- `TRUSTLINE-POSITIVE-BALANCE-RESERVE-001`
- `TRUSTLINE-DISALLOW-INCOMING-OFFER-001`

Those four are the only findings in this packet for which the verifier
evidence did not confirm an upstream fix in the checked refs.

## Audit Commands

Static packet verification:

```bash
cd assets/research/xrpl-rippled-p0-audit
python3 verify_packet.py
```

Run one live finding:

```bash
cd assets/research/xrpl-rippled-p0-audit
./repros/MPT-LOCK-UNAUTH-001.sh
```

Expected proof footer:

```text
ripple.tx.OpenP0Repro had 0 failures.
49 cases, 9229 tests total, 0 failures
ripple.tx.OpenP0ReproCrash had 0 failures.
1 case, 12 tests total, 0 failures
```

## Boundary

This packet is not a vendor advisory, CVE package, or mainnet exploit recipe.
It is a reproducibility packet for clean local upstream jtx proofs over
live-enabled amendment surfaces. Public testnet demonstrations are secondary
because amendment state, node configuration, and server build selection move.
