# XRPL RippleD Live Mainnet-Enabled Evidence Packet

This directory is the public, live-filtered evidence packet for the AGTI report:

`/intelligence-reports/2026/05/26/xrpl-rippled-open-p0-freeze-audit/`

The public report includes only reproduced high/critical findings whose required
XRPL mainnet amendment surfaces were enabled in a direct XRPL validated-ledger
query checked at `2026-05-27T15:25:31Z`. Public XRPL servers checked in
`direct_xrpl_mainnet_runtime_status_20260527.json` reported
`rippled_version=3.1.3`.

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
| `repro_manifest.json` | Canonical live manifest: 8 public high/critical findings, markers, risk labels, and proof bindings. |
| `direct_xrpl_amendment_status_20260527.json` | Direct XRPL `feature` and `ledger_entry` receipt used for the live filter. |
| `direct_xrpl_mainnet_runtime_status_20260527.json` | Direct XRPL server/runtime receipt showing public server versions and live amendment state. |
| `upstream_remediation_status_20260527.json` | Git-ancestry remediation receipt for `3.2.0-b7` and `origin/develop`. |
| `runs/20260527-p0-hunt/live_mainnet_enabled_proof_extract_20260527.log` | Live-only proof extract with marker coverage and zero-failure footer. |
| `run_repro.sh` | Common runner used by every per-finding wrapper. |
| `repros/*.sh` | Per-finding wrappers. The public article links only wrappers for live findings. |
| `verify_packet.py` | Static verifier for article links, manifest records, scripts, markers, amendment snapshot, and proof hash. |

The public packet is scoped to the live findings list.

## Remediation Boundary

Six public findings have confirmed post-3.1.3 remediation in `3.2.0-b7` or
`origin/develop`. Two public MPT findings do not:

- `MPT-TRANSFER-RATE-OVERFLOW-001`
- `MPT-LOCK-UNAUTH-001`

Those two are the only public findings in this packet for which the verifier
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
./repros/MPT-DOMAIN-AUTH-001.sh
```

Expected proof footer:

```text
ripple.tx.OpenP0Repro had 0 failures.
47 cases, 9119 tests total, 0 failures
ripple.tx.OpenP0ReproCrash had 0 failures.
1 case, 12 tests total, 0 failures
```

## Boundary

This packet is not a vendor advisory, CVE package, or mainnet exploit recipe.
It is a reproducibility packet for clean local upstream jtx proofs over
live-enabled amendment surfaces. Public testnet demonstrations are secondary
because amendment state, node configuration, and server build selection move.
