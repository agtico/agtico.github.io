# XRPL RippleD P0 Evidence Packet

This directory is the public, curated evidence packet for the AGTI report:

`/intelligence-reports/2026/05/26/xrpl-rippled-open-p0-freeze-audit/`

It is intentionally smaller than the raw overnight hunt workspace. The packet
keeps the report, manifest, proof log, reproduction source, and per-finding
wrappers needed for audit. It excludes duplicate model-triage dumps and broad
scratch logs that are not needed to verify the published article.

## Target

- Upstream repo: `XRPLF/rippled`
- Target tag: `3.1.3`
- Target commit: `46b241ace8b30d9c9775d60ffba7d24b21903896`
- Local proof suite: `OpenP0Repro`
- Negative-control suite: `OpenP0ReproCrash`

## Packet Contents

| File | Purpose |
|---|---|
| `repro_manifest.json` | Canonical list of 37 published findings, markers, risk labels, and proof bindings. |
| `OpenP0Repro_test.cpp` | Reproduction source to copy into `rippled/src/test/app/`. |
| `run_definitive_proof.sh` | Runs the logic model, `OpenP0Repro`, and `OpenP0ReproCrash`. |
| `run_repro.sh` | Common runner used by every per-finding wrapper. |
| `repros/*.sh` | One wrapper per published finding; each asserts its manifest markers. |
| `runs/20260527-p0-hunt/definitive_proof_batch_signer_outer_replay_20260527.log` | Final proof log bound by SHA-256 in the manifest and article. |
| `runs/20260527-p0-hunt/candidate_matrix.md` | Promoted and demoted candidate disposition. |
| `runs/20260527-p0-hunt/repro_results.md` | Reproduction journal excerpts. |
| `verify_packet.py` | Static verifier for article links, manifest records, scripts, markers, and proof hash. |

## Audit Commands

Static packet verification:

```bash
cd assets/research/xrpl-rippled-p0-audit
python3 verify_packet.py
```

Run the full local proof, assuming a matching upstream rippled checkout exists
at `/home/postfiat/repos/rippled`:

```bash
cd assets/research/xrpl-rippled-p0-audit
./run_definitive_proof.sh
```

Run one finding:

```bash
cd assets/research/xrpl-rippled-p0-audit
./repros/BATCH-SIGNER-OUTER-REPLAY-001.sh
```

Expected proof footer:

```text
ripple.tx.OpenP0Repro had 0 failures.
47 cases, 9119 tests total, 0 failures
ripple.tx.OpenP0ReproCrash had 0 failures.
1 case, 12 tests total, 0 failures
```

## Boundary

This packet is not a vendor advisory, CVE package, or mainnet exploit guide.
It is a reproducibility packet for a clean local upstream jtx build and direct
helper/protocol-wire checks in the same suite. Public testnet demonstrations are
secondary because amendment state, node configuration, and server build
selection are not fixed there. That boundary is evidentiary, not exculpatory:
a clean local upstream jtx repro is sufficient to prove the transaction-path
behavior it exercises.
