---
layout: report
title: "RippleD 3.1.3 Audit: Live Mainnet-Enabled High/Critical Findings"
date: "2026-05-26 20:00:00 +0000"
summary: "Post Fiat evaluated a RippleD-derived implementation path. This live-filtered report includes only reproduced high/critical findings whose required XRPL mainnet amendments were enabled in a direct XRPL validated-ledger query."
category: Post Fiat Research
xrpl_report: true
copy_article: true
report_css_version: 20260528c
tags:
  - AGTI
  - Post Fiat
  - XRPL
  - rippled
  - Security
---

<div class="pearl-primer-box">
  <p><strong>Context:</strong> Post Fiat evaluated a <strong>RippleD-derived implementation path</strong>. This report is the live-filtered reproducibility packet for upstream <code>XRPLF/rippled</code>, baseline <code>3.1.3</code>, commit <code>46b241ace8b30d9c9775d60ffba7d24b21903896</code>.</p>
  <p style="margin-top:12px"><strong>Live scope:</strong> This page includes eight reproduced high/critical findings on XRPL mainnet-enabled amendment surfaces verified through direct XRPL public JSON-RPC against a validated ledger.</p>
  <p style="margin-top:12px"><strong>Severity:</strong> This is a serious protocol-quality report. The included findings cover live-enabled authorization policy, MPT state, TokenEscrow, AMM state, and PermissionedDEX behavior in code that directly processes ledger transactions or enabled MPT accounting.</p>
  <p style="margin-top:12px"><strong>Proof surface:</strong> The proof surface is a clean local upstream jtx build, not a public-testnet anecdote. That is intentional: local standalone jtx fixes the upstream tag, amendment profile, ledger setup, expected marker, and result code.</p>
</div>

---

## Executive Summary

This public report contains **8 reproduced high/critical findings** on XRPL mainnet-enabled amendment surfaces. The findings cover MPT authorization and accounting, TokenEscrow cancellation, AMM state carryover, and PermissionedDEX invariants.

<div class="pearl-hero-grid">
  <div class="pearl-scorecard warn">
    <span class="label">Live findings</span>
    <span class="value">8</span>
    <span class="hint">Only high/critical findings with enabled amendment surfaces.</span>
  </div>
  <div class="pearl-scorecard warn">
    <span class="label">Proof markers</span>
    <span class="value">8</span>
    <span class="hint">All markers are present in the bound proof extract.</span>
  </div>
  <div class="pearl-scorecard good">
    <span class="label">Mainnet wallet</span>
    <span class="value">Not required for proof</span>
    <span class="hint">jtx standalone mints local test accounts.</span>
  </div>
</div>

## Live Amendment Filter

Live status source: direct XRPL public JSON-RPC. The packet calls `feature` for named amendment status and `ledger_entry` for the raw on-ledger `Amendments` object at index `7DB0788C020F02780A673DC74757F23823FA3014C1866E72CC4CD8B226CD6EF4`. The saved receipt is [`direct_xrpl_amendment_status_20260527.json`](/assets/research/xrpl-rippled-p0-audit/direct_xrpl_amendment_status_20260527.json).

| Amendment | Status in snapshot | Used by findings |
|---|---|---|
| `AMM` | Enabled | `AMM-STALE-AUTH-001` |
| `MPTokensV1` | Enabled | `MPT-DOMAIN-AUTH-001`, `MPT-LOCK-UNAUTH-001`, `MPT-NONCANONICAL-AMOUNT-001`, `MPT-TRANSFER-RATE-OVERFLOW-001` |
| `PermissionedDomains` | Enabled | `MPT-DOMAIN-AUTH-001`, `PDEX-HYBRID-QUALITY-001`, `PDEX-CANCEL-INVARIANT-001` |
| `PermissionedDEX` | Enabled | `PDEX-HYBRID-QUALITY-001`, `PDEX-CANCEL-INVARIANT-001` |
| `TokenEscrow` | Enabled | `ESCROW-CANCEL-IOU-001` |

## Evidence Packet

| Evidence object | Link |
|---|---|
| Live audit packet index | [`AUDIT_PACKET.md`](/assets/research/xrpl-rippled-p0-audit/AUDIT_PACKET.md) |
| Live repro manifest | [`repro_manifest.json`](/assets/research/xrpl-rippled-p0-audit/repro_manifest.json) |
| Direct XRPL amendment receipt | [`direct_xrpl_amendment_status_20260527.json`](/assets/research/xrpl-rippled-p0-audit/direct_xrpl_amendment_status_20260527.json) |
| Static packet verifier | [`verify_packet.py`](/assets/research/xrpl-rippled-p0-audit/verify_packet.py) |
| Common runner | [`run_repro.sh`](/assets/research/xrpl-rippled-p0-audit/run_repro.sh) |
| Live proof extract | [`live_mainnet_enabled_proof_extract_20260527.log`](/assets/research/xrpl-rippled-p0-audit/runs/20260527-p0-hunt/live_mainnet_enabled_proof_extract_20260527.log) |

Proof extract hash:

```text
f50e5a25081533c7ed5f32823e76a927a06dd2013a51eea82b386bf87ab4461c  runs/20260527-p0-hunt/live_mainnet_enabled_proof_extract_20260527.log
```

The extract is derived from the full local proof log and contains only the live-public markers listed in this report plus the zero-failure proof footer.

## Risk Scoring

| Score band | Meaning |
|---|---|
| 9.0-10.0 | Direct authorization-policy bypass, issuer/control-policy bypass, or unauthorized ledger mutation on an enabled surface. |
| 8.0-8.9 | High-severity state corruption, stale authority, transfer restriction bypass, or deterministic transaction-path failure on an enabled surface. |
| 7.0-7.9 | Important enabled-surface transaction/helper/invariant correctness issue with credible consensus or product-security impact. |

## Table Of Contents

| ID | Risk | Enabled surface | Exploit class | Repro |
|---|---:|---|---|---|
| [MPT-DOMAIN-AUTH-001](#mpt-domain-auth-001) | 8.7 / Critical | MPTokensV1 + PermissionedDomains enabled | Authorization-policy bypass | [`MPT-DOMAIN-AUTH-001.sh`](/assets/research/xrpl-rippled-p0-audit/repros/MPT-DOMAIN-AUTH-001.sh) |
| [MPT-LOCK-UNAUTH-001](#mpt-lock-unauth-001) | 8.2 / High | MPTokensV1 enabled | Feature-gated lock-state deletion | [`MPT-LOCK-UNAUTH-001.sh`](/assets/research/xrpl-rippled-p0-audit/repros/MPT-LOCK-UNAUTH-001.sh) |
| [ESCROW-CANCEL-IOU-001](#escrow-cancel-iou-001) | 8.1 / High | TokenEscrow enabled | Deterministic transaction exception | [`ESCROW-CANCEL-IOU-001.sh`](/assets/research/xrpl-rippled-p0-audit/repros/ESCROW-CANCEL-IOU-001.sh) |
| [AMM-STALE-AUTH-001](#amm-stale-auth-001) | 8.0 / High | AMM enabled | Stale authorization state | [`AMM-STALE-AUTH-001.sh`](/assets/research/xrpl-rippled-p0-audit/repros/AMM-STALE-AUTH-001.sh) |
| [MPT-NONCANONICAL-AMOUNT-001](#mpt-noncanonical-amount-001) | 7.6 / High | MPTokensV1 enabled | Malformed amount accepted into application path | [`MPT-NONCANONICAL-AMOUNT-001.sh`](/assets/research/xrpl-rippled-p0-audit/repros/MPT-NONCANONICAL-AMOUNT-001.sh) |
| [MPT-TRANSFER-RATE-OVERFLOW-001](#mpt-transfer-rate-overflow-001) | 7.4 / High | MPTokensV1 enabled | Arithmetic overflow | [`MPT-TRANSFER-RATE-OVERFLOW-001.sh`](/assets/research/xrpl-rippled-p0-audit/repros/MPT-TRANSFER-RATE-OVERFLOW-001.sh) |
| [PDEX-HYBRID-QUALITY-001](#pdex-hybrid-quality-001) | 7.7 / High | PermissionedDEX + PermissionedDomains enabled | Order-book metadata corruption | [`PDEX-HYBRID-QUALITY-001.sh`](/assets/research/xrpl-rippled-p0-audit/repros/PDEX-HYBRID-QUALITY-001.sh) |
| [PDEX-CANCEL-INVARIANT-001](#pdex-cancel-invariant-001) | 7.5 / High | PermissionedDEX + PermissionedDomains enabled | Valid transaction invariant failure | [`PDEX-CANCEL-INVARIANT-001.sh`](/assets/research/xrpl-rippled-p0-audit/repros/PDEX-CANCEL-INVARIANT-001.sh) |

## Reproduction Model

Run the static packet check:

```bash
cd /home/postfiat/repos/agtico.github.io/assets/research/xrpl-rippled-p0-audit
python3 verify_packet.py
```

Run one finding:

```bash
cd /home/postfiat/repos/agtico.github.io/assets/research/xrpl-rippled-p0-audit
./repros/MPT-DOMAIN-AUTH-001.sh
```

The wrapper reads the live manifest, runs the local upstream jtx repro suite, asserts the targeted marker, and requires the proof footer `47 cases, 9119 tests total, 0 failures`.

## Live Mainnet-Enabled Findings

<a id="mpt-domain-auth-001"></a>
### MPT-DOMAIN-AUTH-001 - Domain-bound MPT RequireAuth clearing

| Field | Value |
|---|---|
| Risk | **8.7 / Critical** |
| Enabled surface | MPTokensV1 + PermissionedDomains enabled |
| Category | Current transaction-path |
| Exploit type | Authorization-policy bypass |
| Affected target | rippled 3.1.3 MPT issuance authorization |
| Repro script | [`MPT-DOMAIN-AUTH-001.sh`](/assets/research/xrpl-rippled-p0-audit/repros/MPT-DOMAIN-AUTH-001.sh) |
| Proof extract | [`live_mainnet_enabled_proof_extract_20260527.log`](/assets/research/xrpl-rippled-p0-audit/runs/20260527-p0-hunt/live_mainnet_enabled_proof_extract_20260527.log) |

**Broken behavior.** An issuer can clear RequireAuth while retaining DomainID, leaving a domain-bound issuance permissionless in authorization state.

**Expected behavior.** Domain-bound issuances should not clear RequireAuth while DomainID remains.

**Deterministic demonstration.** Run:

```bash
cd /home/postfiat/repos/agtico.github.io/assets/research/xrpl-rippled-p0-audit
./repros/MPT-DOMAIN-AUTH-001.sh
```

**Required marker(s).**

- `MPT current — domain-bound RequireAuth can be cleared`

**Source signal.** Later upstream commit 366899d5 / PR #6712.

**Remediation prompt.** Disallow MPTClearRequireAuth when DomainID is set.

<a id="mpt-lock-unauth-001"></a>
### MPT-LOCK-UNAUTH-001 - MPT locked holder lock-state deletion

| Field | Value |
|---|---|
| Risk | **8.2 / High** |
| Enabled surface | MPTokensV1 enabled |
| Category | Current feature-bound |
| Exploit type | Feature-gated lock-state deletion |
| Affected target | rippled 3.1.3 MPT lock-state handling |
| Repro script | [`MPT-LOCK-UNAUTH-001.sh`](/assets/research/xrpl-rippled-p0-audit/repros/MPT-LOCK-UNAUTH-001.sh) |
| Proof extract | [`live_mainnet_enabled_proof_extract_20260527.log`](/assets/research/xrpl-rippled-p0-audit/runs/20260527-p0-hunt/live_mainnet_enabled_proof_extract_20260527.log) |

**Broken behavior.** A holder can tfMPTUnauthorize a locked zero-balance MPToken, deleting issuer lock state, then re-authorize without lsfMPTLocked.

**Expected behavior.** Locked-token deletion checks should preserve issuer lock state.

**Deterministic demonstration.** Run:

```bash
cd /home/postfiat/repos/agtico.github.io/assets/research/xrpl-rippled-p0-audit
./repros/MPT-LOCK-UNAUTH-001.sh
```

**Required marker(s).**

- `MPT current - locked holder can delete lock state`

**Source signal.** Source review of MPTokenAuthorize::preclaim plus upstream lock/delete coverage.

**Remediation prompt.** Enforce locked MPToken deletion checks in the MPT authorization path.

<a id="escrow-cancel-iou-001"></a>
### ESCROW-CANCEL-IOU-001 - EscrowCancel deleted IOU trustline exception

| Field | Value |
|---|---|
| Risk | **8.1 / High** |
| Enabled surface | TokenEscrow enabled |
| Category | Current transaction-path |
| Exploit type | Deterministic transaction exception |
| Affected target | rippled 3.1.3 token escrow cancellation |
| Repro script | [`ESCROW-CANCEL-IOU-001.sh`](/assets/research/xrpl-rippled-p0-audit/repros/ESCROW-CANCEL-IOU-001.sh) |
| Proof extract | [`live_mainnet_enabled_proof_extract_20260527.log`](/assets/research/xrpl-rippled-p0-audit/runs/20260527-p0-hunt/live_mainnet_enabled_proof_extract_20260527.log) |

**Broken behavior.** Canceling an IOU escrow after the sender trustline was deleted returns tefEXCEPTION / OwnerCount template-field error.

**Expected behavior.** Escrow cancellation accounting should not depend on a deleted sender trustline.

**Deterministic demonstration.** Run:

```bash
cd /home/postfiat/repos/agtico.github.io/assets/research/xrpl-rippled-p0-audit
./repros/ESCROW-CANCEL-IOU-001.sh
```

**Required marker(s).**

- `EscrowCancel current — deleted IOU trustline returns tefEXCEPTION`

**Source signal.** Later upstream commit ad3d172a1 / PR #6171.

**Remediation prompt.** Switch token escrow cancellation accounting to the account ledger entry rather than the deleted trustline.

<a id="amm-stale-auth-001"></a>
### AMM-STALE-AUTH-001 - AMM stale AuthAccounts after empty reinit

| Field | Value |
|---|---|
| Risk | **8.0 / High** |
| Enabled surface | AMM enabled |
| Category | Current transaction-path |
| Exploit type | Stale authorization state |
| Affected target | rippled 3.1.3 AMM empty-pool reinitialization |
| Repro script | [`AMM-STALE-AUTH-001.sh`](/assets/research/xrpl-rippled-p0-audit/repros/AMM-STALE-AUTH-001.sh) |
| Proof extract | [`live_mainnet_enabled_proof_extract_20260527.log`](/assets/research/xrpl-rippled-p0-audit/runs/20260527-p0-hunt/live_mainnet_enabled_proof_extract_20260527.log) |

**Broken behavior.** Empty-pool reinitialization with tfTwoAssetIfEmpty leaves stale sfAuthAccounts from the prior auction slot.

**Expected behavior.** Reinitializing an empty AMM should clear stale auction authorization state.

**Deterministic demonstration.** Run:

```bash
cd /home/postfiat/repos/agtico.github.io/assets/research/xrpl-rippled-p0-audit
./repros/AMM-STALE-AUTH-001.sh
```

**Required marker(s).**

- `AMM current — stale AuthAccounts survive empty reinit`

**Source signal.** Later upstream commit e1fe35993 / PR #6996.

**Remediation prompt.** Clear AuthAccounts during empty-pool AMM reinitialization.

<a id="mpt-noncanonical-amount-001"></a>
### MPT-NONCANONICAL-AMOUNT-001 - Non-canonical MPT amount reaches ledger engine

| Field | Value |
|---|---|
| Risk | **7.6 / High** |
| Enabled surface | MPTokensV1 enabled |
| Category | Current transaction-path |
| Exploit type | Malformed amount accepted into application path |
| Affected target | rippled 3.1.3 MPT amount validation |
| Repro script | [`MPT-NONCANONICAL-AMOUNT-001.sh`](/assets/research/xrpl-rippled-p0-audit/repros/MPT-NONCANONICAL-AMOUNT-001.sh) |
| Proof extract | [`live_mainnet_enabled_proof_extract_20260527.log`](/assets/research/xrpl-rippled-p0-audit/runs/20260527-p0-hunt/live_mainnet_enabled_proof_extract_20260527.log) |

**Broken behavior.** A non-canonical MPT amount reaches transaction application and returns fee-burning tecPATH_PARTIAL instead of failing preflight as temBAD_AMOUNT.

**Expected behavior.** Non-canonical MPT amounts should fail before ledger application.

**Deterministic demonstration.** Run:

```bash
cd /home/postfiat/repos/agtico.github.io/assets/research/xrpl-rippled-p0-audit
./repros/MPT-NONCANONICAL-AMOUNT-001.sh
```

**Required marker(s).**

- `MPT current — non-canonical amount reaches ledger engine`

**Source signal.** Later upstream commit dcd2ff0b5 / PR #7117.

**Remediation prompt.** Reject non-canonical MPT amounts during preflight/preclaim before fee-burning application.

<a id="mpt-transfer-rate-overflow-001"></a>
### MPT-TRANSFER-RATE-OVERFLOW-001 - MPT transfer-rate scaling overflow

| Field | Value |
|---|---|
| Risk | **7.4 / High** |
| Enabled surface | MPTokensV1 enabled |
| Category | Current helper/accounting |
| Exploit type | Arithmetic overflow |
| Affected target | rippled 3.1.3 MPT transfer-rate helper |
| Repro script | [`MPT-TRANSFER-RATE-OVERFLOW-001.sh`](/assets/research/xrpl-rippled-p0-audit/repros/MPT-TRANSFER-RATE-OVERFLOW-001.sh) |
| Proof extract | [`live_mainnet_enabled_proof_extract_20260527.log`](/assets/research/xrpl-rippled-p0-audit/runs/20260527-p0-hunt/live_mainnet_enabled_proof_extract_20260527.log) |

**Broken behavior.** Applying a 1.5 transfer rate to a large integral MPT amount throws overflow_error in the legacy scaled-mantissa path.

**Expected behavior.** Valid integral-token amounts should scale through bounded consensus arithmetic or fail cleanly before overflow.

**Deterministic demonstration.** Run:

```bash
cd /home/postfiat/repos/agtico.github.io/assets/research/xrpl-rippled-p0-audit
./repros/MPT-TRANSFER-RATE-OVERFLOW-001.sh
```

**Required marker(s).**

- `MPT current — transfer-rate scaling overflows large integral amount`

**Source signal.** Later upstream commit 22fbf4d06.

**Remediation prompt.** Route MPT/V2 transfer-rate math through Number arithmetic.

<a id="pdex-hybrid-quality-001"></a>
### PDEX-HYBRID-QUALITY-001 - Permissioned-DEX hybrid-offer quality mismatch

| Field | Value |
|---|---|
| Risk | **7.7 / High** |
| Enabled surface | PermissionedDEX + PermissionedDomains enabled |
| Category | Current transaction-path |
| Exploit type | Order-book metadata corruption |
| Affected target | rippled 3.1.3 permissioned DEX hybrid offers |
| Repro script | [`PDEX-HYBRID-QUALITY-001.sh`](/assets/research/xrpl-rippled-p0-audit/repros/PDEX-HYBRID-QUALITY-001.sh) |
| Proof extract | [`live_mainnet_enabled_proof_extract_20260527.log`](/assets/research/xrpl-rippled-p0-audit/runs/20260527-p0-hunt/live_mainnet_enabled_proof_extract_20260527.log) |

**Broken behavior.** A partially crossed hybrid offer leaves its open-book directory key at one quality while sfExchangeRate records another.

**Expected behavior.** Open-book directory key quality and sfExchangeRate metadata must match after partial crossing.

**Deterministic demonstration.** Run:

```bash
cd /home/postfiat/repos/agtico.github.io/assets/research/xrpl-rippled-p0-audit
./repros/PDEX-HYBRID-QUALITY-001.sh
```

**Required marker(s).**

- `Permissioned DEX current — hybrid offer open-book quality mismatch`

**Source signal.** Later upstream commit 28cc20c81 / PR #7087.

**Remediation prompt.** Use the correct open-book placement rate and repair existing bad sfExchangeRate metadata.

<a id="pdex-cancel-invariant-001"></a>
### PDEX-CANCEL-INVARIANT-001 - Permissioned-DEX regular-offer cancel invariant failure

| Field | Value |
|---|---|
| Risk | **7.5 / High** |
| Enabled surface | PermissionedDEX + PermissionedDomains enabled |
| Category | Current transaction-path |
| Exploit type | Valid transaction invariant failure |
| Affected target | rippled 3.1.3 permissioned DEX OfferCreate |
| Repro script | [`PDEX-CANCEL-INVARIANT-001.sh`](/assets/research/xrpl-rippled-p0-audit/repros/PDEX-CANCEL-INVARIANT-001.sh) |
| Proof extract | [`live_mainnet_enabled_proof_extract_20260527.log`](/assets/research/xrpl-rippled-p0-audit/runs/20260527-p0-hunt/live_mainnet_enabled_proof_extract_20260527.log) |

**Broken behavior.** A valid domain OfferCreate that cancels the user regular offer fails with tecINVARIANT_FAILED because the invariant treats the deleted regular offer as forbidden mutation.

**Expected behavior.** The invariant should ignore regular offers deleted as part of a valid domain offer cancellation.

**Deterministic demonstration.** Run:

```bash
cd /home/postfiat/repos/agtico.github.io/assets/research/xrpl-rippled-p0-audit
./repros/PDEX-CANCEL-INVARIANT-001.sh
```

**Required marker(s).**

- `Permissioned DEX current — cancel regular offer via domain offer invariant`

**Source signal.** Later upstream commit 8c0080020 / PR #7118.

**Remediation prompt.** Update the permissioned-DEX invariant to ignore deleted regular offers in this path.

## Implications For Post Fiat

1. A RippleD-derived path cannot inherit enabled XRPL surfaces blindly. The live-enabled findings cluster around authorization policy, MPT state, TokenEscrow state handling, AMM state carryover, and PermissionedDEX invariants.
2. The correct engineering response is not rhetorical. Any reused concept needs independent tests, manifest-bound repros, amendment-status gating, and negative controls.
3. This report is intentionally scoped to the enabled surfaces listed above.

---

<div class="pearl-disclaimer">

**Disclaimer**

This document is published by **Post Fiat / AGTI** for informational purposes only. It describes our **internal code-quality evaluation** of the open-source **RippleD** codebase (baseline `release-3.1.3`, May 2026). Post Fiat evaluated RippleD-derived implementation paths; we are **not** speaking on behalf of Ripple, Ripple Labs, the XRP Ledger Foundation (XRPLF), or any other third party.

Nothing here is legal, investment, tax, or security advice. Observations are based on static code review, local unit tests (jtx), helper/protocol-wire checks, live amendment-status filtering, and our interpretation of upstream behavior at a point in time. Upstream code, amendments, and deployment configurations change. Readers should perform their own due diligence and consult qualified professionals before acting.

Issue identifiers are **internal audit labels**, not official CVEs or vendor advisories. Descriptions of hypothetical exploit paths are **research scenarios**, not allegations of wrongdoing, negligence, or breach of duty by any person or organization.

**No warranty.** This report is provided "as is" without warranty of any kind. To the fullest extent permitted by law, Post Fiat / AGTI disclaims liability for any loss or damage arising from use of or reliance on this material.

**Trademarks.** Ripple, XRP, XRPL, rippled, and related names are trademarks of their respective owners. Post Fiat is an independent project.

**Corrections.** If you believe any statement is inaccurate, contact us with reproducible evidence and we will review updates in good faith.

*Baseline: upstream rippled `release-3.1.3` - direct XRPL validated-ledger amendment receipt checked 2026-05-27T14:48:14Z*

</div>
