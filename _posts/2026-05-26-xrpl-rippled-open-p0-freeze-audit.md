---
layout: report
title: "Honing XRP Code Quality: Post Fiat Evaluation of the RippleD Codebase"
date: "2026-05-26 20:00:00 +0000"
summary: "Post Fiat runs on a RippleD fork. This is our internal code-quality evaluation of upstream rippled 3.1.3 before we decide how to proceed."
category: Post Fiat Research
xrpl_report: true
report_css_version: 20260527e
tags:
  - AGTI
  - Post Fiat
  - XRPL
  - rippled
  - Security
---

<div class="pearl-primer-box">
  <p><strong>Context:</strong> Post Fiat is a <strong>RippleD fork</strong>. Before we commit to building on or migrating off this stack, we ran an internal audit of the upstream codebase — baseline <code>release-3.1.3</code>, May 2026 — to understand what is actually broken, what is proven, and what we can rule out.</p>
  <p style="margin-top:12px">This report is that evaluation write-up: plain-English observations, hypothesized exploit paths where our testing supported them, diagrams, and local jtx reproductions. It reflects our internal research only, published for transparency.</p>
</div>

<div class="pearl-hero-grid">
  <div class="pearl-scorecard warn">
    <span class="label">Issues in scope</span>
    <span class="value">10</span>
    <span class="hint">F2.1, F3.1, F3.3–F3.10, F6.1 — observed in upstream <code>release-3.1.3</code> at time of review.</span>
  </div>
  <div class="pearl-scorecard warn">
    <span class="label">Locally reproduced</span>
    <span class="value">7 lending</span>
    <span class="hint">Regular-freeze behavior — balance change observed in rippled unittest (F3.3).</span>
  </div>
  <div class="pearl-scorecard good">
    <span class="label">Repro wallet</span>
    <span class="value">Not required</span>
    <span class="hint">jtx standalone mints test XRP.</span>
  </div>
</div>

<div class="pearl-verdict-banner">
  <strong>How to read this report</strong>
  <p>Every section below uses the same three-part layout:</p>
  <ul class="pearl-verdict-list">
    <li><strong>Plain English</strong> — what the bug means without protocol jargon</li>
    <li><strong>How it could be exploited</strong> — who does what, and what breaks in the real world</li>
    <li><strong>Correct vs existing</strong> — diagram comparing intended behavior to rippled today</li>
  </ul>
  <p class="pearl-verdict-foot">Sections below cover issues we are still tracking. Lending freeze behavior and the SetTrust crash were reproduced locally. F4.6 / B3-1 vault pseudo fund movement was investigated and <strong>not reproduced</strong> in our jtx setup.</p>
</div>

---

## Section A — IOU regular freeze vs deep freeze (root cause)

### Plain English

An IOU issuer can freeze an account in two steps. **Regular freeze** means “this account should not send or receive my token.” **Deep freeze** is a stronger lock layered on top. You can have **regular-only** freeze — and that is the normal compliance case (suspect wallet, court order, fraud response).

Rippled has two API checks: **`checkFrozen`** (blocks both) and **`checkDeepFrozen`** (blocks deep only). Lending code often uses the wrong one on **receivers**.

### How it could be exploited

1. Issuer regular-freezes account **D** (does not need deep freeze).
2. Someone submits a lending transaction that pays **D** (or a frozen broker owner, vault pseudo, etc.).
3. Preclaim calls **`checkDeepFrozen` only** → not blocked.
4. IOU is delivered to an account the issuer thought was frozen.

**Not required:** hacking validators, fake signatures, or deep freeze.

### Correct vs existing functionality

<div class="pearl-split pearl-diagram-split">
  <div class="pearl-panel good">
    <h4>Correct behavior</h4>
    <div class="pearl-mermaid"><div class="mermaid">
flowchart TB
  C1[Issuer regular-freezes D] --> C2[Tx pays D]
  C2 --> C3{checkFrozen D?}
  C3 -->|frozen| C4[tecFROZEN]
  C3 -->|not frozen| C5[IOU delivered]
    </div></div>
  </div>
  <div class="pearl-panel bad">
    <h4>Existing — lending</h4>
    <div class="pearl-mermaid"><div class="mermaid">
flowchart TB
  B1[Issuer regular-freezes D] --> B2[Tx pays D]
  B2 --> B3{checkDeepFrozen only?}
  B3 -->|regular-only| B4[Not blocked]
  B4 --> B5[IOU delivered — bug]
    </div></div>
  </div>
</div>

---

## Section B — Lending freeze bypass (F3.3, F3.5–F3.10) · locally reproduced

### Plain English

The **XLS-66 lending** feature (loan brokers, loan pay, cover withdraw, etc.) checks the **wrong freeze level** on almost every path where IOU goes to a **receiver**. The correct pattern already existed in **`VaultWithdraw`** (destination check) but was not copied into lending.

Seven transaction paths share one mistake introduced in PR [#5270](https://github.com/XRPLF/rippled/pull/5270).

### How it could be exploited

| ID | Transaction | Exploit in one sentence |
|----|-------------|-------------------------|
| **F3.3** | CoverWithdraw | Broker sends cover to a **regular-frozen** destination — IOU arrives anyway. |
| **F3.5** | BrokerDelete | Deleting broker sends leftover cover to a **regular-frozen** owner. |
| **F3.6** | LoanPay | Borrower pays loan; **broker fee** still routed to **regular-frozen** owner. |
| **F3.7** | LoanSet | New loan; **origination fee** paid to **regular-frozen** broker owner. |
| **F3.8** | LoanPay | Payment credits a **regular-frozen** vault pseudo-account. |
| **F3.9** | CoverDeposit | Cover deposited into **regular-frozen** broker pseudo. |
| **F3.10** | LoanSet | Fee sent to **regular-frozen** broker pseudo. |

**Observed in testing:** issuer compliance / fraud containment may fail — frozen wallets can still receive IOU via lending in our jtx run. **F3.3 reproduction:** destination balance increased by 10 IOU while regular-frozen.

### Correct vs existing functionality

```mermaid
flowchart LR
  subgraph correct [Correct — e.g. VaultWithdraw dest]
    direction TB
    R1[Receiver may get IOU] --> R2[checkFrozen receiver]
    R2 --> R3{Regular or deep frozen?}
    R3 -->|yes| R4[tecFROZEN]
    R3 -->|no| R5[Transfer OK]
  end
  subgraph broken [Existing — lending receive paths]
    direction TB
    L1[Receiver may get IOU] --> L2[checkDeepFrozen receiver only]
    L2 --> L3{Deep frozen?}
    L3 -->|no — regular-only OK| L4[Transfer OK — BUG]
    L3 -->|yes| L5[tecFROZEN]
  end
```

**Representative code (F3.3):**

```cpp
if (auto const ret = checkDeepFrozen(ctx.view, dstAcct, vaultAsset))
    return ret;
// MISSING: checkFrozen(ctx.view, dstAcct, vaultAsset)
```

---

## Section C — SetTrust validator crash (F6.1) · locally reproduced

### Plain English

If someone submits a **SetTrust** (trust line) transaction pointing at an **issuer account that does not exist**, validators should return a clean error (`tecNO_DST`). When AMM and Single-Asset-Vault features are **off**, the code can **skip that check** and **crash** by reading flags from a null account pointer.

This is a **network availability** bug, not a “steal IOU” bug.

### How it could be exploited

1. Attacker crafts SetTrust with a non-existent issuer; features configured so the null guard is off.
2. Transaction reaches **preclaim** on validators.
3. **`sleDst->getFlags()` on null** → validator process segfaults (jtx: **exit 139**).

**Harm:** validator crash, potential consensus disruption if enough nodes hit the same malformed tx — not direct fund theft.

### Correct vs existing functionality

```mermaid
flowchart LR
  subgraph correct [Correct behavior]
    direction TB
    S1[SetTrust: issuer account missing] --> S2{sleDst null?}
    S2 -->|yes| S3[Return tecNO_DST — always]
    S2 -->|no| S4[Continue preclaim checks]
  end
  subgraph broken [Existing — AMM+SAV off]
    direction TB
    B1[SetTrust: issuer account missing] --> B2{sleDst null AND AMM/SAV on?}
    B2 -->|no — guard skipped| B3[sleDst->getFlags — CRASH]
    B2 -->|yes| B4[tecNO_DST]
  end
```

---

## Section D — VaultInvariant loan gap (F2.1)

### Plain English

After every successful transaction, **invariants** are safety nets that assert ledger accounting still makes sense. For **loan** transactions (`LoanSet`, `LoanManage`, `LoanPay`), the vault invariant literally contains **`// TBD`** and **returns true** — no checks run.

### How it could be exploited

**Not directly.** You need a **second bug** in loan math or routing that corrupts vault/broker balances. Without invariants, that corruption **validates successfully** instead of failing the ledger.

**Analogy:** smoke alarm with no battery — fire only hurts you if something else ignites.

### Correct vs existing functionality

```mermaid
flowchart LR
  subgraph correct [Correct behavior]
    direction TB
    V1[LoanPay succeeds] --> V2[VaultInvariant runs]
    V2 --> V3[Assert vault AssetsAvailable / broker cover / loan fields consistent]
    V3 --> V4{Mismatch?}
    V4 -->|yes| V5[Invariant fail — ledger rejected]
    V4 -->|no| V6[Ledger stands]
  end
  subgraph broken [Existing behavior]
    direction TB
    W1[LoanPay succeeds] --> W2[VaultInvariant: case ttLOAN_*]
    W2 --> W3["// TBD — return true"]
    W3 --> W4[Always passes — no safety net]
  end
```

---

## Section E — FreezeInvariant MPT gap (F3.1)

### Plain English

`FreezeInvariant` watches IOU trust line balance changes to catch forbidden transfers while frozen. It **only looks at `ltRIPPLE_STATE`** — **MPT token** balance changes are **invisible** to this invariant.

### How it could be exploited

**Not directly.** If any transactor allows a frozen MPT to move (a separate bug), this invariant **will not catch it**. IOU freeze enforcement and MPT freeze enforcement are asymmetric at the invariant layer.

### Correct vs existing functionality

```mermaid
flowchart LR
  subgraph correct [Correct behavior]
    direction TB
    M1[Any balance change in tx] --> M2[FreezeInvariant tracks IOU lines]
    M2 --> M3[FreezeInvariant tracks MPT tokens]
    M3 --> M4{Frozen party received funds?}
    M4 -->|yes| M5[Invariant fail]
  end
  subgraph broken [Existing behavior]
    direction TB
    X1[Balance change in tx] --> X2{Entry type?}
    X2 -->|ltRIPPLE_STATE| X3[Checked]
    X2 -->|ltMPTOKEN| X4[Skipped — blind spot]
    X4 --> X5[Invariant passes even if MPT moved while frozen]
  end
```

---

## Section F — EscrowFinish IOU (F3.11 · under review)

### Plain English

When finishing an **IOU escrow**, the destination freeze check uses **`isDeepFrozen` only**. The **MPT escrow path in the same file** uses **`isFrozen`**. Tests expect IOU finish to **succeed** when the destination is regular-frozen after escrow was created.

**Status:** under review — may be intentional legacy behavior for in-flight escrows; we have not promoted this to our main findings list.

### How it could be exploited

1. Escrow created to destination **D**.
2. Issuer regular-freezes **D** (not deep).
3. Finisher submits **EscrowFinish** → IOU may still pay **D**.

Same freeze API mistake as lending, but product intent is unclear.

### Correct vs existing functionality

```mermaid
flowchart LR
  subgraph correct [Correct — aligned with MPT path]
    direction TB
    E1[EscrowFinish IOU to dest D] --> E2{isFrozen D?}
    E2 -->|yes| E3[tecFROZEN / block finish]
    E2 -->|no| E4[Deliver escrowed IOU]
  end
  subgraph broken [Existing — IOU template]
    direction TB
    F1[EscrowFinish IOU to dest D] --> F2{isDeepFrozen D only?}
    F2 -->|regular-only freeze| F3[Finish succeeds — IOU delivered]
    F2 -->|deep freeze| F4[Blocked]
  end
```

---

## Section G — fixCleanup3_1_3 vs open issues

### Plain English

**fixCleanup3_1_3** (rippled 3.1.3, May 2026) bundles real fixes: NFT offer cleanup, permissioned-domain failed-tx invariant, vault withdraw trust-line limits, loan accounting patches, LoanPay overpay error code, LoanBroker cover upper bound.

It **does not** fix lending regular-freeze bypass, SetTrust crash, invariant TBD gaps, or Escrow IOU finish semantics.

### How it could be exploited

The amendment itself is not an exploit — the risk we note is **false confidence**: assuming “3.1.3 + fixCleanup = lending/vault security closed” while our local tests still showed lending freeze behavior on the paths below.

### Correct vs existing functionality

```mermaid
flowchart LR
  subgraph marketed [What fixCleanup fixes]
    direction TB
    MC1[NFT expired offer cleanup]
    MC2[VaultWithdraw trust-line limit]
    MC3[LoanManage accounting patches]
    MC4[LoanPay overpay error code]
  end
  subgraph stillopen [Still open after activation]
    direction TB
    SO1[Lending checkDeepFrozen on receivers — locally reproduced]
    SO2[SetTrust null deref — locally reproduced]
    SO3[VaultInvariant loan TBD]
    SO4[FreezeInvariant MPT blind spot]
  end
  marketed -.->|does not close| stillopen
```

---

## Section H — Local reproduction notes

### Plain English

We ran rippled’s built-in **jtx** test ledger locally — no testnet, no mainnet XRP. Tests mint accounts and assert balances.

### Results

| Finding | Method | Result |
|---------|--------|--------|
| **F3.3** lending freeze | `OpenP0Repro` | Reproduced locally — `tesSUCCESS`, dest +10 IOU |
| **F3.3 control** | deep freeze added | Reproduced locally — `tecFROZEN` |
| **F6.1** SetTrust crash | `OpenP0ReproCrash` | Reproduced locally — segfault exit 139 |
| **Freeze logic** | `freeze_check_model.py` | Consistent with code paths reviewed |

### Not reproduced in our jtx setup

| Finding | Method | Result |
|---------|--------|--------|
| **F4.6 / B3-1** vault pseudo fund movement | `OpenP0Repro` | Not reproduced — `tecLOCKED`; code-review observation only |

Repro kit: [assets/research/xrpl-rippled-p0-audit/](https://agti.net/assets/research/xrpl-rippled-p0-audit/) · [`OpenP0Repro_test.cpp`](https://agti.net/assets/research/xrpl-rippled-p0-audit/OpenP0Repro_test.cpp)

---

## Implications for Post Fiat

1. **Do not treat fixCleanup as a full code-quality closure** based on this review alone.
2. **Issuer regular-freeze on lending paths** behaved inconsistently in our local tests (7 call sites reviewed).
3. **Invariant gaps** (F2.1, F3.1) may reduce safety-net coverage if other issues appear.
4. **SetTrust crash** is an availability concern in our malformed-tx reproduction.
5. **F4.6 / B3-1** were code-review observations; we did not reproduce fund movement locally.

---

<div class="pearl-disclaimer">

**Disclaimer**

This document is published by **Post Fiat / AGTI** for informational purposes only. It describes our **internal code-quality evaluation** of the open-source **RippleD** codebase (baseline `release-3.1.3`, May 2026). Post Fiat maintains a RippleD-derived fork; we are **not** speaking on behalf of Ripple, Ripple Labs, the XRP Ledger Foundation (XRPLF), or any other third party.

Nothing here is legal, investment, tax, or security advice. Observations are based on static code review, local unit tests (jtx), and our interpretation of upstream behavior at a point in time. **We may be wrong.** Upstream code, amendments, and deployment configurations change. Readers should perform their own due diligence and consult qualified professionals before acting.

Issue identifiers (e.g. F3.3, F6.1) are **internal audit labels**, not official CVEs or vendor advisories. Descriptions of hypothetical exploit paths are **research scenarios**, not allegations of wrongdoing, negligence, or breach of duty by any person or organization. Mention of pull requests or contributors is for traceability only.

**No warranty.** This report is provided “as is” without warranty of any kind. To the fullest extent permitted by law, Post Fiat / AGTI disclaims liability for any loss or damage arising from use of or reliance on this material.

**Trademarks.** Ripple, XRP, XRPL, rippled, and related names are trademarks of their respective owners. Post Fiat is an independent project.

**Corrections.** If you believe any statement is inaccurate, contact us with reproducible evidence and we will review updates in good faith.

*Baseline: upstream rippled `release-3.1.3` · Post Fiat internal review · May 2026*

</div>
