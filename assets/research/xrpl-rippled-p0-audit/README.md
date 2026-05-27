# XRPL rippled lending freeze audit — repro kit (AGTI)

Evidence kit for the AGTI intelligence report on rippled `release-3.1.3`. The local jtx suite reproduces seven current regular-freeze lending receive paths: F3.3 and F3.5-F3.10.

A 2026-05-27 follow-up hunt also reproduced twenty-seven additional current `3.1.3` transaction/helper root causes found from upstream history and source review, covering twenty-nine current paths, one feature-bound MPT lock-state root cause with `MPTokensV1` active and `SingleAssetVault` inactive, plus one current MPT `STIssue` protocol-wire serialization proof beyond the lending-freeze class. The current paths include `LoanBrokerCover` precision drift across deposit, withdraw, and clawback, vault/loan precision and invariant failures, AMM stale `AuthAccounts`, multiple delegation authority defects, MPT authorization/serialization/arithmetic defects, permissioned-DEX state defects, and IOU trustline policy bypasses through offers, NFT settlement, CheckCash auto-trustline creation, and TokenEscrow finish.

The same hunt reproduced nine pre-`fixCleanup3_1_3` historical/replay-era root causes with fixed-path negative controls: a `PermissionedDomainSet` ticket-sequence collision, an MPT multi-send aggregate `MaximumAmount` bypass, a `VaultWithdraw` share-limit bypass, vault share-MPT deletion while escrowed shares remained locked, zero-amount `VaultClawback` unclamped clawback, `LoanPay` fee-cap DoS, invariant bool-overwrite, expired-credential cleanup delete failure, and permissioned-DEX empty `sfAdditionalBooks`. F6.1 SetTrust crash and F4.6/B3-1 vault pseudo fund movement were **not reproduced by jtx** and are not counted as reproduced findings.

**Charts:** `build_release_rolling_chart.py` → `rippled_release_rolling_12m.svg` (12-month rolling stable release count since 2013).

| Script | Purpose |
|--------|---------|
| [`AUDIT_PACKET.md`](AUDIT_PACKET.md) | Public packet index and audit boundary |
| [`verify_packet.py`](verify_packet.py) | Static verifier for the article, manifest, proof hash, scripts, anchors, and markers |
| [`repro_manifest.json`](repro_manifest.json) | Canonical finding list and proof binding |
| [`freeze_check_model.py`](freeze_check_model.py) | Logic model: why `checkDeepFrozen` misses regular-only freeze |
| [`OpenP0Repro_test.cpp`](OpenP0Repro_test.cpp) | jtx suite for F3.3/F3.5-F3.10 lending proofs, additional current `3.1.3` repros, pre-`fixCleanup3_1_3` historical/replay-era repros, and negative controls (copy into rippled `src/test/app/`) |
| [`run_repro.sh`](run_repro.sh) | Runs Python model and targeted marker checks; optional rippled unittest if built |
| [`run_definitive_proof.sh`](run_definitive_proof.sh) | Full proof runner for `OpenP0Repro` plus `OpenP0ReproCrash` |
| [`runs/20260527-p0-hunt/candidate_matrix.md`](runs/20260527-p0-hunt/candidate_matrix.md) | Curated candidate matrix, including promoted and demoted candidates |
| [`runs/20260527-p0-hunt/repro_results.md`](runs/20260527-p0-hunt/repro_results.md) | Curated reproduction journal excerpts |

## Do you need funded XRP wallets?

| Mode | Funded wallet? | Notes |
|------|----------------|-------|
| **This Python model** | No | Pure logic |
| **rippled jtx / `--unittest`** | No | Standalone env mints test XRP |
| **XRPL devnet / testnet** | Faucet only | Optional live demo |
| **XRPL mainnet** | Yes | Not required for proof; real IOU + fees if you demo live |

**For migration research:** jtx standalone repro is enough. No mainnet wallet required.

## Quick start

```bash
cd assets/research/xrpl-rippled-p0-audit
python3 verify_packet.py
python3 freeze_check_model.py
./run_repro.sh
./run_definitive_proof.sh
```

## Full rippled jtx repro (optional)

```bash
git clone https://github.com/XRPLF/rippled.git
cd rippled && git checkout 3.1.3
cp ../agtico.github.io/assets/research/xrpl-rippled-p0-audit/OpenP0Repro_test.cpp src/test/app/
conan profile detect --force
conan remote add --index 0 xrplf https://conan.ripplex.io || true
conan install . --output-folder .build --build missing --settings build_type=Release --options xrpld=True --options tests=True
cmake -S . -B .build -GNinja -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=.build/build/generators/conan_toolchain.cmake -Dxrpld=ON -Dtests=ON
cmake --build .build --target rippled -j "$(nproc)"
.build/rippled --unittest OpenP0Repro
.build/rippled --unittest OpenP0ReproCrash
```

Upstream paths (3.1.3 layout): `src/xrpld/app/tx/detail/LoanBrokerCoverWithdraw.cpp`, `LoanBrokerDelete.cpp`, `LoanBrokerCoverDeposit.cpp`, `LoanSet.cpp`, and `LoanPay.cpp`.

## Related audit docs

Local fork audit: `rippled/bug_hunt_plan/P0_INVENTORY.md`

## Disclaimer

Independent AGTI research. Not investment or legal advice. Describes protocol behavior on audited source; verify on your own node before acting.
