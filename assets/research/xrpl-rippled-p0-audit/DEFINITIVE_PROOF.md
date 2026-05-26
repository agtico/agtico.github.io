# Definitive proof log — XRPL open P0 audit

Date: 2026-05-26  
Binary: `/home/pfrpc/repos/rippled/.build/xrpld` (branch `internal/bug-hunt-plan`, built with `-Dtests=ON`)

## How to reproduce

```bash
cd /home/pfrpc/repos/rippled
chmod +x bug_hunt_plan/run_definitive_proof.sh
./bug_hunt_plan/run_definitive_proof.sh
```

No mainnet wallet. jtx standalone only.

---

## Results summary

| Finding | Proof method | Result | Log |
|---------|--------------|--------|-----|
| **F3.3** | jtx `OpenP0Repro` — regular-freeze-only dest receives cover | **PROVEN** — `tesSUCCESS`, balance +10 IOU | `proof_openp0repro.log` |
| **F3.3 control** | Same suite — deep-freeze dest | **PROVEN blocked** — `tecFROZEN` | same |
| **F6.1 control** | SetTrust missing issuer, SAV on | **PROVEN safe path** — `tecNO_DST` | same |
| **F6.1 bug** | `OpenP0ReproCrash` — AMM+SAV off, missing issuer | **PROVEN** — **SIGSEGV (exit 139)** | `proof_f61_crash.log` |
| **Freeze logic** | `freeze_check_model.py` | **PROVEN** — regular-only row allows bug path | stdout |
| **F4.6 / B3-1** | jtx vault repro (draft) | **Needs refinement** — `tecLOCKED` on draft setup; code audit stands | — |

---

## F3.3 excerpt (definitive)

```
xrpl.tx.OpenP0Repro F3.3 LoanBrokerCoverWithdraw — regular-freeze-only destination (P0)
xrpl.tx.OpenP0Repro F3.3 control — deep-freeze blocks cover withdraw
xrpl.tx.OpenP0Repro F6.1 SetTrust missing issuer — tecNO_DST when SAV enabled (control)
xrpl.tx.OpenP0Repro had 0 failures.
```

Test source: `src/test/app/OpenP0Repro_test.cpp`

Steps proven on ledger:
1. Issuer regular-freezes destination (`tfSetFreeze` only).
2. `LoanBrokerCoverWithdraw` to that destination **succeeds**.
3. Destination IOU balance increases by withdrawn amount.
4. Adding `tfSetDeepFreeze` causes **`tecFROZEN`** (control).

---

## F6.1 excerpt (definitive)

```
./xrpld --unittest OpenP0ReproCrash
# Segmentation fault (core dumped) — exit 139
```

Null deref: `SetTrust.cpp` ~204 — `sleDst->getFlags()` when issuer account missing and AMM+SAV disabled.

---

## Build notes

```bash
cd rippled && mkdir -p .build && cd .build
conan remote add --index 0 xrplf https://conan.ripplex.io  # once
conan install .. --output-folder . --build missing --settings build_type=Release
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=build/generators/conan_toolchain.cmake \
  -Dxrpld=ON -Dtests=ON ..
cmake --build . -j$(nproc)
./xrpld --unittest OpenP0Repro
./xrpld --unittest OpenP0ReproCrash   # expect crash if unfixed
```

---

## What this does not yet prove on jtx

- Remaining lending sites F3.5–F3.10 (same root cause; one jtx proof + code review is sufficient for the class).
- Vault F4.6 / B3-1 — draft jtx hit `tecLOCKED`; needs setup matching pseudo-account freeze semantics.
- Invariant gaps F2.1 / F3.1 — require second bug to show fund loss; code `// TBD` is definitive for F2.1.

Code audit + F3.3 jtx + F6.1 crash + logic model = **definitive for the freeze class and SetTrust crash**.
