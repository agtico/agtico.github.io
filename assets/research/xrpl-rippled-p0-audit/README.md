# XRPL rippled open P0 audit — repro kit (AGTI)

Evidence kit for the AGTI intelligence report on **10 confirmed open P0s** in rippled `release-3.1.3` (F4.6/B3-1 vault pseudo bypass **ruled out by jtx** — not counted).

**Charts:** `build_release_rolling_chart.py` → `rippled_release_rolling_12m.svg` (12-month rolling stable release count since 2013).

| Script | Purpose |
|--------|---------|
| [`freeze_check_model.py`](freeze_check_model.py) | Logic model: why `checkDeepFrozen` misses regular-only freeze |
| [`repro_f3_3_regular_freeze.jtx.cpp`](repro_f3_3_regular_freeze.jtx.cpp) | jtx snippet for F3.3 (needs rippled build) |
| [`run_repro.sh`](run_repro.sh) | Runs Python model; optional rippled unittest if built |

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
python3 freeze_check_model.py
./run_repro.sh
```

## Full rippled jtx repro (optional)

```bash
git clone https://github.com/XRPLF/rippled.git
cd rippled && git checkout release-3.1.3
mkdir build && cd build
cmake -GNinja -DCMAKE_BUILD_TYPE=Release ..
ninja rippled
# Add repro snippet to src/test/app/ and register in CMakeLists, then:
./rippled --unittest LoanBroker
```

Upstream paths (3.1.3 layout): `src/xrpld/app/tx/detail/LoanBrokerCoverWithdraw.cpp` ~109–112.

## Related audit docs

Local fork audit: `rippled/bug_hunt_plan/P0_INVENTORY.md`

## Disclaimer

Independent AGTI research. Not investment or legal advice. Describes protocol behavior on audited source; verify on your own node before acting.
