---
layout: report
title: "RippleD 3.1.3 Audit: Live Mainnet Findings And Remediation Inventory"
date: "2026-05-26 20:00:00 +0000"
summary: "Post Fiat's live-filtered XRPL rippled 3.1.3 packet currently contains 19 reproduced high findings on mainnet-enabled surfaces: 14 without a confirmed fix in the checked 3.2.0-b7/origin-develop refs and 5 with post-3.1.3 remediation evidence."
category: Post Fiat Research
xrpl_report: true
copy_article: true
report_css_version: 20260528d
tags:
  - AGTI
  - Post Fiat
  - XRPL
  - rippled
  - Security
---

<div class="pearl-primer-box">
  <p><strong>Context:</strong> Post Fiat evaluated a <strong>RippleD-derived implementation path</strong>. This report is the live-filtered reproducibility inventory for upstream <code>XRPLF/rippled</code>, baseline <code>3.1.3</code>, commit <code>46b241ace8b30d9c9775d60ffba7d24b21903896</code>.</p>
  <p style="margin-top:12px"><strong>Current packet:</strong> The packet now contains <strong>19 reproduced high findings</strong> on XRPL mainnet-enabled surfaces. The split is <strong>14 findings with no confirmed fix</strong> in the checked <code>3.2.0-b7</code> / <code>origin/develop</code> refs and <strong>5 findings with post-3.1.3 remediation evidence</strong>.</p>
  <p style="margin-top:12px"><strong>Proof model:</strong> Findings are reproduced in a clean local upstream jtx harness against the fixed <code>3.1.3</code> target. Live eligibility is gated by direct XRPL JSON-RPC amendment and runtime receipts, not explorer pages.</p>
</div>

---

## Executive Summary

This report is an inventory of the current AGTI/Post Fiat XRPL `rippled` 3.1.3 evidence packet.

The packet is not a list of speculative code smells. A finding only enters the public inventory if it has a local reproduction wrapper, an expected marker in the `OpenP0Repro` proof log, a risk label, a live-amendment dependency, and remediation status checked against `3.2.0-b7` and `origin/develop`.

The important update is that the article is no longer the earlier 8-finding snapshot. The current packet has 19 live-mainnet-eligible findings:

<div class="pearl-hero-grid">
  <div class="pearl-scorecard warn">
    <span class="label">Packet findings</span>
    <span class="value">19</span>
    <span class="hint">Reproduced high findings on live-enabled surfaces.</span>
  </div>
  <div class="pearl-scorecard warn">
    <span class="label">No confirmed fix</span>
    <span class="value">14</span>
    <span class="hint">No confirmed fix in checked 3.2.0-b7 / origin-develop refs.</span>
  </div>
  <div class="pearl-scorecard good">
    <span class="label">Remediating</span>
    <span class="value">5</span>
    <span class="hint">Post-3.1.3 remediation present in beta/develop evidence.</span>
  </div>
</div>

## Current Mainnet State

Direct XRPL public JSON-RPC checks on 2026-05-27 produced the current-state packet:

- Runtime receipt: [`direct_xrpl_mainnet_runtime_status_20260527.json`](/assets/research/xrpl-rippled-p0-audit/direct_xrpl_mainnet_runtime_status_20260527.json)
- Amendment receipt: [`direct_xrpl_amendment_status_20260527.json`](/assets/research/xrpl-rippled-p0-audit/direct_xrpl_amendment_status_20260527.json)
- Remediation receipt: [`upstream_remediation_status_20260527.json`](/assets/research/xrpl-rippled-p0-audit/upstream_remediation_status_20260527.json)
- Canonical manifest: [`repro_manifest.json`](/assets/research/xrpl-rippled-p0-audit/repro_manifest.json)

| Check | Result |
|---|---|
| Target release | `rippled 3.1.3`, commit `46b241ace8b30d9c9775d60ffba7d24b21903896` |
| Public server versions checked | `s1.ripple.com` and `s2.ripple.com` reported `rippled_version=3.1.3` |
| Latest checked beta | `3.2.0-b7` |
| Live enabled surfaces used here | `AMM`, `AMMClawback`, `Checks`, `CheckCashMakesTrustLine`, `Credentials`, `DepositAuth`, `DisallowIncoming`, `fixDisallowIncomingV1`, `MPTokensV1`, `NonFungibleTokensV1_1`, `PermissionedDomains`, `PermissionedDEX`, `TokenEscrow`, `fixMPTDeliveredAmount`, `fixAMMv1_3`, `fixTokenEscrowV1`, `fixAMMClawbackRounding`, `fixCleanup3_1_3` |
| Disabled surfaces excluded | `LendingProtocol`, `SingleAssetVault`, `PermissionDelegation`, `Batch`, `fixDelegateV1_1`, `fixDisallowIncomingV1_1` |
| Cleanup-era gate | `fixCleanup3_1_3` was enabled by raw on-ledger amendment hash, so cleanup-era-only candidates are excluded |

## Inventory

| ID | Risk | Status | Surface | Exploit class | Repro |
|---|---:|---|---|---|---|
| [MPT-LOCK-UNAUTH-001](#mpt-lock-unauth-001) | 8.2 | No confirmed fix | `MPTokensV1` | Lock-state deletion | [`sh`](/assets/research/xrpl-rippled-p0-audit/repros/MPT-LOCK-UNAUTH-001.sh) |
| [TRUSTLINE-POSITIVE-BALANCE-RESERVE-001](#trustline-positive-balance-reserve-001) | 8.1 | No confirmed fix | Baseline IOU trustlines | Reserve/owner-count bypass | [`sh`](/assets/research/xrpl-rippled-p0-audit/repros/TRUSTLINE-POSITIVE-BALANCE-RESERVE-001.sh) |
| [TRUSTLINE-DISALLOW-INCOMING-OFFER-001](#trustline-disallow-incoming-offer-001) | 8.0 | No confirmed fix | `DisallowIncoming` + offers | Issuer policy bypass | [`sh`](/assets/research/xrpl-rippled-p0-audit/repros/TRUSTLINE-DISALLOW-INCOMING-OFFER-001.sh) |
| [NFTOKEN-DISALLOW-INCOMING-ACCEPT-001](#nftoken-disallow-incoming-accept-001) | 8.0 | No confirmed fix | NFT settlement | Issuer policy bypass | [`sh`](/assets/research/xrpl-rippled-p0-audit/repros/NFTOKEN-DISALLOW-INCOMING-ACCEPT-001.sh) |
| [NFTOKEN-BROKER-FEE-DISALLOW-INCOMING-TRUSTLINE-001](#nftoken-broker-fee-disallow-incoming-trustline-001) | 8.0 | No confirmed fix | NFT broker fee | Issuer policy bypass | [`sh`](/assets/research/xrpl-rippled-p0-audit/repros/NFTOKEN-BROKER-FEE-DISALLOW-INCOMING-TRUSTLINE-001.sh) |
| [CHECKCASH-DISALLOW-INCOMING-TRUSTLINE-001](#checkcash-disallow-incoming-trustline-001) | 8.0 | No confirmed fix | Checks | Issuer policy bypass | [`sh`](/assets/research/xrpl-rippled-p0-audit/repros/CHECKCASH-DISALLOW-INCOMING-TRUSTLINE-001.sh) |
| [TOKENESCROW-DISALLOW-INCOMING-FINISH-001](#tokenescrow-disallow-incoming-finish-001) | 8.0 | No confirmed fix | TokenEscrow | Issuer policy bypass | [`sh`](/assets/research/xrpl-rippled-p0-audit/repros/TOKENESCROW-DISALLOW-INCOMING-FINISH-001.sh) |
| [AMMWITHDRAW-DISALLOW-INCOMING-TRUSTLINE-001](#ammwithdraw-disallow-incoming-trustline-001) | 8.0 | No confirmed fix | AMM withdraw | Issuer policy bypass | [`sh`](/assets/research/xrpl-rippled-p0-audit/repros/AMMWITHDRAW-DISALLOW-INCOMING-TRUSTLINE-001.sh) |
| [AMMCREATE-DISALLOW-INCOMING-TRUSTLINE-001](#ammcreate-disallow-incoming-trustline-001) | 8.0 | No confirmed fix | AMM create | Issuer policy bypass | [`sh`](/assets/research/xrpl-rippled-p0-audit/repros/AMMCREATE-DISALLOW-INCOMING-TRUSTLINE-001.sh) |
| [AMMDEPOSIT-EMPTY-DISALLOW-INCOMING-TRUSTLINE-001](#ammdeposit-empty-disallow-incoming-trustline-001) | 8.0 | No confirmed fix | AMM empty-pool deposit | Issuer policy bypass | [`sh`](/assets/research/xrpl-rippled-p0-audit/repros/AMMDEPOSIT-EMPTY-DISALLOW-INCOMING-TRUSTLINE-001.sh) |
| [AMMCLAWBACK-DISALLOW-INCOMING-PAIRED-ASSET-001](#ammclawback-disallow-incoming-paired-asset-001) | 8.0 | No confirmed fix | AMMClawback | Issuer policy bypass | [`sh`](/assets/research/xrpl-rippled-p0-audit/repros/AMMCLAWBACK-DISALLOW-INCOMING-PAIRED-ASSET-001.sh) |
| [AMMCLAWBACK-DEPOSITAUTH-PAIRED-ASSET-001](#ammclawback-depositauth-paired-asset-001) | 8.0 | No confirmed fix | AMMClawback | Holder receive-policy bypass | [`sh`](/assets/research/xrpl-rippled-p0-audit/repros/AMMCLAWBACK-DEPOSITAUTH-PAIRED-ASSET-001.sh) |
| [AMMBID-DEPOSITAUTH-REFUND-001](#ammbid-depositauth-refund-001) | 8.0 | No confirmed fix | AMMBid | Holder receive-policy bypass | [`sh`](/assets/research/xrpl-rippled-p0-audit/repros/AMMBID-DEPOSITAUTH-REFUND-001.sh) |
| [MPT-TRANSFER-RATE-OVERFLOW-001](#mpt-transfer-rate-overflow-001) | 7.4 | No confirmed fix | `MPTokensV1` | Arithmetic overflow | [`sh`](/assets/research/xrpl-rippled-p0-audit/repros/MPT-TRANSFER-RATE-OVERFLOW-001.sh) |
| [ESCROW-CANCEL-IOU-001](#escrow-cancel-iou-001) | 8.1 | Remediated after 3.1.3 | TokenEscrow | Deterministic exception | [`sh`](/assets/research/xrpl-rippled-p0-audit/repros/ESCROW-CANCEL-IOU-001.sh) |
| [AMM-STALE-AUTH-001](#amm-stale-auth-001) | 8.0 | Remediated after 3.1.3 | AMM | Stale authorization state | [`sh`](/assets/research/xrpl-rippled-p0-audit/repros/AMM-STALE-AUTH-001.sh) |
| [MPT-NONCANONICAL-AMOUNT-001](#mpt-noncanonical-amount-001) | 7.6 | Fixed in develop, not confirmed in `3.2.0-b7` | `MPTokensV1` | Non-canonical amount validation | [`sh`](/assets/research/xrpl-rippled-p0-audit/repros/MPT-NONCANONICAL-AMOUNT-001.sh) |
| [PDEX-HYBRID-QUALITY-001](#pdex-hybrid-quality-001) | 7.7 | Remediated after 3.1.3 | PermissionedDEX | Order-book metadata corruption | [`sh`](/assets/research/xrpl-rippled-p0-audit/repros/PDEX-HYBRID-QUALITY-001.sh) |
| [PDEX-CANCEL-INVARIANT-001](#pdex-cancel-invariant-001) | 7.5 | Remediated after 3.1.3 | PermissionedDEX | Valid transaction invariant failure | [`sh`](/assets/research/xrpl-rippled-p0-audit/repros/PDEX-CANCEL-INVARIANT-001.sh) |

## Why The 14 Unfixed Findings Matter

The dominant pattern is not one isolated transaction bug. It is incomplete receive-policy enforcement across multiple ledger paths.

`asfDisallowIncomingTrustline` and `asfDepositAuth` are account-level controls. Direct paths respect them. Several indirect paths do not. Offers, NFT settlement, broker fees, CheckCash, TokenEscrow finish, AMM create/deposit/withdraw, AMMClawback paired-asset returns, and AMMBid refunds can reach the same economic effect through a different transaction family.

That is a serious implementation-quality signal for anyone inheriting `rippled 3.1.3`: policy checks must be centralized or every new transaction family becomes another bypass candidate.

## Plain-English Finding Guide

This section explains the packet in human terms before the raw evidence links.
Each finding should be read as a state-transition mismatch: the transaction
family reaches a ledger effect that the surrounding account policy, reserve
rule, arithmetic rule, or invariant says should not happen.

### Unfixed In Checked 3.2.0-b7 / origin-develop

**MPT-LOCK-UNAUTH-001**

- **What is this?** MPTokens are XRPL multi-purpose tokens. Issuers can authorize holders and mark holder token objects as locked.
- **Why it matters.** A lock is supposed to be durable issuer control, not state a holder can erase by deleting and recreating its token object.
- **What is the bug?** A holder can `tfMPTUnauthorize` a locked zero-balance MPToken, deleting the lock state, then re-authorize without `lsfMPTLocked`.
- **Intended behavior.** Locked-token deletion should preserve the issuer lock or reject deletion while locked.
- **Actual behavior.** The reproduced path deletes the locked holder object and recreates it unlocked.
- **Remediation.** Enforce locked MPToken deletion checks in the MPT authorization path. No confirmed fix was found in the checked refs.

**TRUSTLINE-POSITIVE-BALANCE-RESERVE-001**

- **What is this?** XRPL IOUs live on trustlines; a positive holder balance normally creates owned ledger state and consumes owner reserve.
- **Why it matters.** Reserve accounting is XRPL's anti-state-spam mechanism. A positive balance without owner reserve means durable ledger state exists without the normal cost.
- **What is the bug?** Offer crossing can give a holder a positive IOU balance while `OwnerCount` stays zero and the receiver reserve flag stays unset.
- **Intended behavior.** A receiver crossing from non-positive to positive balance should pay owner reserve or the transaction should fail.
- **Actual behavior.** The reproduced path creates the positive balance without the reserve-side accounting.
- **Remediation.** Charge receiver owner reserve on the balance transition, or fail if reserve is unavailable. No confirmed fix was found in the checked refs.

**TRUSTLINE-DISALLOW-INCOMING-OFFER-001**

- **What is this?** `asfDisallowIncomingTrustline` is an issuer flag intended to block new incoming trustlines. `OfferCreate` is the DEX path for crossing IOU offers.
- **Why it matters.** If direct `TrustSet` is blocked but DEX settlement creates the same trustline, the issuer policy is not actually enforced.
- **What is the bug?** An issuer can block direct trustline creation, but a taker without a trustline can still cross an offer and receive the issuer IOU.
- **Intended behavior.** `OfferCreate` should apply the same incoming-trustline opt-out check before creating the trustline.
- **Actual behavior.** Direct `TrustSet` is rejected, then offer crossing creates the trustline anyway.
- **Remediation.** Reject offer acceptance that would create a blocked issuer trustline. The checked refs do not contain a confirmed fix.

**NFTOKEN-DISALLOW-INCOMING-ACCEPT-001**

- **What is this?** `NFTokenAcceptOffer` settles NFT sales and can pay the seller in an issued IOU.
- **Why it matters.** NFT settlement should not be a second route to create a trustline that direct issuer policy forbids.
- **What is the bug?** The seller can receive an issuer IOU through NFT settlement despite the issuer setting `asfDisallowIncomingTrustline`.
- **Intended behavior.** NFT IOU settlement should enforce the issuer's incoming-trustline opt-out.
- **Actual behavior.** Direct `TrustSet` is rejected, but `NFTokenAcceptOffer` creates the seller trustline.
- **Remediation.** Add the same issuer-policy check to NFT IOU settlement. No confirmed fix was found in the checked refs.

**NFTOKEN-BROKER-FEE-DISALLOW-INCOMING-TRUSTLINE-001**

- **What is this?** Brokered NFT settlement can pay a broker fee in an issuer IOU.
- **Why it matters.** Broker fees are easy to miss because the broker is neither buyer nor seller; this tests whether receive-policy enforcement is centralized.
- **What is the bug?** The broker can receive an issuer IOU fee and get a new trustline despite the issuer opt-out.
- **Intended behavior.** Broker-fee payment should enforce `asfDisallowIncomingTrustline` before creating a broker trustline.
- **Actual behavior.** The broker-fee path creates the trustline through settlement.
- **Remediation.** Apply issuer-policy checks to NFT broker-fee IOU payment. No confirmed fix was found in the checked refs.

**CHECKCASH-DISALLOW-INCOMING-TRUSTLINE-001**

- **What is this?** Checks allow delayed settlement; with `CheckCashMakesTrustLine`, cashing an IOU check can create the receiver trustline.
- **Why it matters.** Delayed settlement should not bypass the same issuer policy that direct trustline creation must obey.
- **What is the bug?** `CheckCash` can create an incoming trustline to an issuer that has opted out of new incoming trustlines.
- **Intended behavior.** `CheckCash` should reject IOU cashing when it would create a blocked trustline.
- **Actual behavior.** Direct `TrustSet` is blocked, then the check-cash path creates the trustline.
- **Remediation.** Add issuer-policy checks to automatic trustline creation during `CheckCash`. No confirmed fix was found in the checked refs.

**TOKENESCROW-DISALLOW-INCOMING-FINISH-001**

- **What is this?** TokenEscrow releases issued assets when `EscrowFinish` completes.
- **Why it matters.** Escrow completion is non-interactive for the destination; it should not force a policy-blocked trustline onto the account.
- **What is the bug?** `EscrowFinish` can deliver an IOU and create a destination trustline despite issuer `DisallowIncomingTrustline`.
- **Intended behavior.** Finishing an IOU escrow should enforce the issuer's incoming-trustline opt-out.
- **Actual behavior.** Direct `TrustSet` is rejected, then escrow completion creates the trustline.
- **Remediation.** Add issuer-policy checks to TokenEscrow finish settlement. No confirmed fix was found in the checked refs.

**AMMWITHDRAW-DISALLOW-INCOMING-TRUSTLINE-001**

- **What is this?** `AMMWithdraw` returns pooled assets to a liquidity provider.
- **Why it matters.** AMMs are a major indirect settlement surface. If withdrawal skips issuer policy, liquidity mechanics can create blocked trustlines.
- **What is the bug?** A withdrawal can send an issuer IOU to an account with no trustline even after the issuer has opted out.
- **Intended behavior.** AMM withdrawal should enforce issuer trustline policy before creating a receiver trustline.
- **Actual behavior.** The AMM withdrawal path creates the trustline through `accountSend`.
- **Remediation.** Apply issuer-policy checks to AMM withdrawal sends. No confirmed fix was found in the checked refs.

**AMMCREATE-DISALLOW-INCOMING-TRUSTLINE-001**

- **What is this?** `AMMCreate` creates the special AMM account and the initial pool.
- **Why it matters.** Pool creation creates durable ledger state. It should not create an AMM-account trustline to an issuer that opted out of new incoming trustlines.
- **What is the bug?** A pool can be created for an issuer IOU despite issuer `DisallowIncomingTrustline`.
- **Intended behavior.** AMM creation should reject pool creation when it would create a blocked issuer trustline.
- **Actual behavior.** The AMM account trustline is created through the pool creation path.
- **Remediation.** Apply issuer-policy checks to AMM account trustline creation. No confirmed fix was found in the checked refs.

**AMMDEPOSIT-EMPTY-DISALLOW-INCOMING-TRUSTLINE-001**

- **What is this?** `AMMDeposit` with `tfTwoAssetIfEmpty` can reinitialize an empty pool.
- **Why it matters.** Reinitialization is a lifecycle edge case where old state is recreated; those paths must re-run the same policy checks as first creation.
- **What is the bug?** Empty-pool reinitialization can recreate an AMM trustline to an issuer that has opted out.
- **Intended behavior.** Empty-pool deposit should enforce issuer policy before recreating the AMM account trustline.
- **Actual behavior.** The reinit path recreates the trustline despite `DisallowIncomingTrustline`.
- **Remediation.** Apply issuer-policy checks to empty-pool reinitialization. No confirmed fix was found in the checked refs.

**AMMCLAWBACK-DISALLOW-INCOMING-PAIRED-ASSET-001**

- **What is this?** `AMMClawback` lets issuer A claw back its asset from a two-asset AMM pool, which can return issuer B's paired asset to a holder.
- **Why it matters.** Cross-issuer AMM operations must respect both issuers' policies, not only the issuer initiating the clawback.
- **What is the bug?** Issuer A's clawback can force-return issuer B's IOU to a holder after issuer B opted out of incoming trustlines.
- **Intended behavior.** Returning the paired asset should enforce issuer B's trustline policy.
- **Actual behavior.** The paired asset is returned and the issuer B trustline is recreated.
- **Remediation.** Apply issuer-policy checks to paired-asset returns in AMM clawback. No confirmed fix was found in the checked refs.

**AMMCLAWBACK-DEPOSITAUTH-PAIRED-ASSET-001**

- **What is this?** `DepositAuth` is a receiver-side flag that requires authorization before unsolicited funds can be delivered.
- **Why it matters.** A protocol-generated AMM return is still a delivery to the receiver; it should not bypass the receiver's explicit no-unsolicited-deposits policy.
- **What is the bug?** AMM clawback can force-return a paired IOU to a holder that rejects direct payment under `DepositAuth`.
- **Intended behavior.** AMM clawback should enforce the holder's receive authorization before delivering paired assets.
- **Actual behavior.** Direct payment is rejected, but the AMM clawback return delivers the asset.
- **Remediation.** Apply `DepositAuth` checks to paired-asset returns. No confirmed fix was found in the checked refs.

**AMMBID-DEPOSITAUTH-REFUND-001**

- **What is this?** `AMMBid` replaces the current AMM auction-slot owner and refunds LP tokens to the previous owner.
- **Why it matters.** The previous owner is not signing the later bid. Protocol-generated refunds still need to obey receiver policy.
- **What is the bug?** The previous owner can set `DepositAuth`, reject direct LP-token payment, and still receive an LP-token refund through a later `AMMBid`.
- **Intended behavior.** AMM bid refunds should respect the previous owner's `DepositAuth` state.
- **Actual behavior.** The refund path delivers LP tokens despite the receiver policy.
- **Remediation.** Apply `DepositAuth` checks to AMM bid refunds. No confirmed fix was found in the checked refs.

**MPT-TRANSFER-RATE-OVERFLOW-001**

- **What is this?** MPT transfer rates scale token movements to account for issuer transfer fees.
- **Why it matters.** Consensus transaction code should not throw arithmetic exceptions on transaction amounts; it should either compute deterministically or reject cleanly.
- **What is the bug?** A large integral MPT amount with a 1.5 transfer rate reaches a scaled-mantissa overflow path.
- **Intended behavior.** Transfer-rate math should be bounded and deterministic, or fail before application.
- **Actual behavior.** The reproduced path hits an `overflow_error`.
- **Remediation.** Route MPT transfer-rate math through bounded consensus arithmetic. A fix-looking commit exists, but no confirmed fix was found in the checked refs.

### Remediated Or Remediating After 3.1.3

**ESCROW-CANCEL-IOU-001**

- **What is this?** TokenEscrow cancellation should unwind escrow accounting after normal trustline lifecycle changes.
- **Why it matters.** Cancellation should not strand state or throw a deterministic exception because a related trustline was deleted.
- **What is the bug?** Canceling an IOU escrow after sender trustline deletion returns `tefEXCEPTION` / owner-count template-field failure.
- **Intended behavior.** Escrow cancellation should account from durable account state, not require the old trustline to still exist.
- **Actual behavior.** The cancellation path depends on deleted trustline state and throws.
- **Remediation.** Patched after 3.1.3 by using the account ledger entry for cancellation accounting; confirmed in `3.2.0-b7` and `origin/develop`.

**AMM-STALE-AUTH-001**

- **What is this?** AMM auction authorization state controls the current discounted trading slot.
- **Why it matters.** Empty-pool reinitialization should not inherit privilege metadata from a prior pool lifecycle.
- **What is the bug?** Reinitializing an empty AMM leaves stale `sfAuthAccounts` from the previous auction slot.
- **Intended behavior.** Empty-pool reinit should clear stale auction authorization state.
- **Actual behavior.** The old authorization list survives into the new pool lifecycle.
- **Remediation.** Patched after 3.1.3 by clearing `AuthAccounts` during empty-pool reinitialization; confirmed in `3.2.0-b7` and `origin/develop`.

**MPT-NONCANONICAL-AMOUNT-001**

- **What is this?** XRPL amount encodings are supposed to be canonical before ledger application.
- **Why it matters.** Malformed values should fail preflight, not reach fee-burning application paths.
- **What is the bug?** A non-canonical MPT amount reaches transaction application and returns `tecPATH_PARTIAL` instead of `temBAD_AMOUNT`.
- **Intended behavior.** Non-canonical MPT amounts should be rejected before application.
- **Actual behavior.** The malformed amount reaches the ledger engine and burns a fee.
- **Remediation.** Patched in `origin/develop`; not confirmed in checked `3.2.0-b7`.

**PDEX-HYBRID-QUALITY-001**

- **What is this?** Permissioned DEX hybrid offers are indexed by quality for matching and settlement metadata.
- **Why it matters.** Offer quality is not cosmetic; mismatched quality changes order-book interpretation and can corrupt market metadata.
- **What is the bug?** A partially crossed hybrid offer leaves its open-book directory key at one quality while `sfExchangeRate` records another.
- **Intended behavior.** Directory key quality and `sfExchangeRate` should agree after partial crossing.
- **Actual behavior.** The reproduced path leaves those values inconsistent.
- **Remediation.** Patched after 3.1.3 by fixing hybrid offer placement and metadata repair; confirmed in `3.2.0-b7` and `origin/develop`.

**PDEX-CANCEL-INVARIANT-001**

- **What is this?** Permissioned DEX offers can cancel or interact with regular offers from the same account.
- **Why it matters.** Invariants should catch impossible ledger mutation, not reject a valid transaction because two offer families interact.
- **What is the bug?** A valid domain `OfferCreate` that cancels a regular offer fails with `tecINVARIANT_FAILED`.
- **Intended behavior.** The invariant should permit the valid deletion caused by the domain offer path.
- **Actual behavior.** The invariant treats the deleted regular offer as forbidden mutation.
- **Remediation.** Patched after 3.1.3 by updating the permissioned-DEX invariant; confirmed in `3.2.0-b7` and `origin/develop`.

## Evidence Packet

| Evidence object | Link |
|---|---|
| Packet index | [`AUDIT_PACKET.md`](/assets/research/xrpl-rippled-p0-audit/AUDIT_PACKET.md) |
| Canonical manifest | [`repro_manifest.json`](/assets/research/xrpl-rippled-p0-audit/repro_manifest.json) |
| Direct XRPL amendment receipt | [`direct_xrpl_amendment_status_20260527.json`](/assets/research/xrpl-rippled-p0-audit/direct_xrpl_amendment_status_20260527.json) |
| Direct XRPL runtime receipt | [`direct_xrpl_mainnet_runtime_status_20260527.json`](/assets/research/xrpl-rippled-p0-audit/direct_xrpl_mainnet_runtime_status_20260527.json) |
| Upstream remediation receipt | [`upstream_remediation_status_20260527.json`](/assets/research/xrpl-rippled-p0-audit/upstream_remediation_status_20260527.json) |
| Live-only triage | [`live_p0_hunt_v2_triage.md`](/assets/research/xrpl-rippled-p0-audit/runs/20260527-p0-hunt/live_p0_hunt_v2_triage.md) |
| Static packet verifier | [`verify_packet.py`](/assets/research/xrpl-rippled-p0-audit/verify_packet.py) |
| Common repro runner | [`run_repro.sh`](/assets/research/xrpl-rippled-p0-audit/run_repro.sh) |
| Proof extract | [`live_mainnet_enabled_proof_extract_20260527_v13.log`](/assets/research/xrpl-rippled-p0-audit/runs/20260527-p0-hunt/live_mainnet_enabled_proof_extract_20260527_v13.log) |

Proof extract hash:

```text
302da1ccf25b3ab103cdccf231be443515e81561593c34912aca87849a22cfd6
```

## Reproduction Model

Run the static packet verifier:

```bash
cd /home/postfiat/repos/agtico.github.io/assets/research/xrpl-rippled-p0-audit
python3 verify_packet.py
```

Run one finding:

```bash
cd /home/postfiat/repos/agtico.github.io/assets/research/xrpl-rippled-p0-audit
./repros/TRUSTLINE-POSITIVE-BALANCE-RESERVE-001.sh
```

Expected proof footer:

```text
ripple.tx.OpenP0Repro had 0 failures.
59 cases, 16068 tests total, 0 failures
```

The per-finding wrapper reads `repro_manifest.json`, runs the upstream local jtx proof suite, asserts the targeted marker, and requires the zero-failure proof footer.

## Findings Without Confirmed Fix

<a id="mpt-lock-unauth-001"></a>
### MPT-LOCK-UNAUTH-001 - MPT locked holder lock-state deletion

**Risk:** 8.2 / High. **Surface:** `MPTokensV1`; `SingleAssetVault` disabled. **Class:** feature-gated lock-state deletion.

**Broken behavior.** A holder can `tfMPTUnauthorize` a locked zero-balance MPToken, deleting issuer lock state, then re-authorize without `lsfMPTLocked`.

**Expected behavior.** Locked-token deletion checks should preserve issuer lock state.

**Repro:** [`MPT-LOCK-UNAUTH-001.sh`](/assets/research/xrpl-rippled-p0-audit/repros/MPT-LOCK-UNAUTH-001.sh). Marker: `MPT current — locked holder can delete lock state without SAV`.

**Upstream status.** Fix-looking commits exist, but the checked `3.2.0-b7` and `origin/develop` source still gates locked-token deletion on `SingleAssetVault`. No confirmed fix.

<a id="trustline-positive-balance-reserve-001"></a>
### TRUSTLINE-POSITIVE-BALANCE-RESERVE-001 - Positive IOU balance without receiver owner reserve

**Risk:** 8.1 / High. **Surface:** baseline IOU trustline reserve accounting. **Class:** reserve and owner-count bypass.

**Broken behavior.** After a holder clears its trust limit and balance, offer crossing can give the holder a positive IOU balance while `OwnerCount` remains zero and the receiver reserve flag remains unset.

**Expected behavior.** A receiver whose trustline balance moves from zero or negative to positive should either receive the reserve flag and `OwnerCount` increment, or the transaction should fail for insufficient reserve.

**Repro:** [`TRUSTLINE-POSITIVE-BALANCE-RESERVE-001.sh`](/assets/research/xrpl-rippled-p0-audit/repros/TRUSTLINE-POSITIVE-BALANCE-RESERVE-001.sh). Marker: `TrustLine current — offer crossing creates positive balance without reserve`.

**Upstream status.** A fix-looking branch exists (`origin/vvysokikh1/fix-positive-balance-trustline-pay-no-reserve`), but no confirmed fix is present in checked `3.2.0-b7` or `origin/develop`.

<a id="trustline-disallow-incoming-offer-001"></a>
### TRUSTLINE-DISALLOW-INCOMING-OFFER-001 - OfferCreate bypasses DisallowIncomingTrustline

**Risk:** 8.0 / High. **Surface:** `DisallowIncoming`, `fixDisallowIncomingV1`, offers. **Class:** issuer policy bypass.

**Broken behavior.** An issuer with `asfDisallowIncomingTrustline` set blocks direct `TrustSet` incoming trustlines, but a taker without an existing trustline can still cross an offer and receive the issuer's IOU, creating the trustline through `OfferCreate`.

**Expected behavior.** `OfferCreate` should enforce the same incoming-trustline opt-out policy before allowing a taker without an existing trustline to receive the issuer's IOU.

**Repro:** [`TRUSTLINE-DISALLOW-INCOMING-OFFER-001.sh`](/assets/research/xrpl-rippled-p0-audit/repros/TRUSTLINE-DISALLOW-INCOMING-OFFER-001.sh). Marker: `TrustLine current — OfferCreate bypasses DisallowIncomingTrustline`.

**Upstream status.** Fix-looking branch `origin/copilot/apply-asfdisallowincomingtrustline` adds an OfferCreate check behind proposed `fixDisallowIncomingV1_1`, but no confirmed fix is present in checked `3.2.0-b7` or `origin/develop`.

<a id="nftoken-disallow-incoming-accept-001"></a>
### NFTOKEN-DISALLOW-INCOMING-ACCEPT-001 - NFTokenAcceptOffer bypasses DisallowIncomingTrustline

**Risk:** 8.0 / High. **Surface:** `NonFungibleTokensV1_1`, `fixEnforceNFTokenTrustlineV2`, `DisallowIncoming`, `fixDisallowIncomingV1`. **Class:** issuer policy bypass.

**Broken behavior.** Direct `TrustSet` is rejected, but `NFTokenAcceptOffer` can settle a sell offer in the issuer's IOU to a seller with no existing trustline, creating the incoming trustline through NFT settlement.

**Expected behavior.** `NFTokenAcceptOffer` should enforce the same incoming-trustline opt-out policy before allowing a seller without an existing trustline to receive the issuer's IOU.

**Repro:** [`NFTOKEN-DISALLOW-INCOMING-ACCEPT-001.sh`](/assets/research/xrpl-rippled-p0-audit/repros/NFTOKEN-DISALLOW-INCOMING-ACCEPT-001.sh). Marker: `NFToken current — AcceptOffer bypasses DisallowIncomingTrustline`.

**Upstream status.** No confirmed fix in checked `3.2.0-b7` or `origin/develop`; the OfferCreate branch does not cover this path.

<a id="nftoken-broker-fee-disallow-incoming-trustline-001"></a>
### NFTOKEN-BROKER-FEE-DISALLOW-INCOMING-TRUSTLINE-001 - NFToken broker fee bypasses DisallowIncomingTrustline

**Risk:** 8.0 / High. **Surface:** `NonFungibleTokensV1_1`, `fixEnforceNFTokenTrustlineV2`, `DisallowIncoming`, `fixDisallowIncomingV1`. **Class:** issuer policy bypass.

**Broken behavior.** A brokered `NFTokenAcceptOffer` can pay the broker fee in an issuer's IOU to a broker with no existing trustline, creating the incoming trustline through `NFTokenAcceptOffer::pay` / `accountSend`.

**Expected behavior.** `NFTokenAcceptOffer` should enforce the incoming-trustline opt-out before allowing an IOU broker fee to create a broker trustline.

**Repro:** [`NFTOKEN-BROKER-FEE-DISALLOW-INCOMING-TRUSTLINE-001.sh`](/assets/research/xrpl-rippled-p0-audit/repros/NFTOKEN-BROKER-FEE-DISALLOW-INCOMING-TRUSTLINE-001.sh). Marker: `NFToken current — broker fee bypasses DisallowIncomingTrustline`.

**Upstream status.** No confirmed fix in checked `3.2.0-b7` or `origin/develop`; the OfferCreate branch does not cover broker-fee settlement.

<a id="checkcash-disallow-incoming-trustline-001"></a>
### CHECKCASH-DISALLOW-INCOMING-TRUSTLINE-001 - CheckCash bypasses DisallowIncomingTrustline

**Risk:** 8.0 / High. **Surface:** `Checks`, `CheckCashMakesTrustLine`, `DisallowIncoming`, `fixDisallowIncomingV1`. **Class:** issuer policy bypass.

**Broken behavior.** Direct `TrustSet` is rejected, but `CheckCash` can cash an IOU check to a receiver with no existing trustline, creating the incoming trustline through the automatic CheckCash path.

**Expected behavior.** `CheckCash` should enforce the same incoming-trustline opt-out policy before auto-creating a receiver trustline.

**Repro:** [`CHECKCASH-DISALLOW-INCOMING-TRUSTLINE-001.sh`](/assets/research/xrpl-rippled-p0-audit/repros/CHECKCASH-DISALLOW-INCOMING-TRUSTLINE-001.sh). Marker: `CheckCash current — bypasses DisallowIncomingTrustline`.

**Upstream status.** No confirmed fix in checked `3.2.0-b7` or `origin/develop`.

<a id="tokenescrow-disallow-incoming-finish-001"></a>
### TOKENESCROW-DISALLOW-INCOMING-FINISH-001 - EscrowFinish bypasses DisallowIncomingTrustline

**Risk:** 8.0 / High. **Surface:** `TokenEscrow`, `fixTokenEscrowV1`, `DisallowIncoming`, `fixDisallowIncomingV1`. **Class:** issuer policy bypass.

**Broken behavior.** Direct `TrustSet` is rejected, but `EscrowFinish` can finish an IOU escrow to a destination with no existing trustline, creating the incoming trustline through the TokenEscrow finish path.

**Expected behavior.** `EscrowFinish` should enforce the same incoming-trustline opt-out policy before auto-creating a destination trustline.

**Repro:** [`TOKENESCROW-DISALLOW-INCOMING-FINISH-001.sh`](/assets/research/xrpl-rippled-p0-audit/repros/TOKENESCROW-DISALLOW-INCOMING-FINISH-001.sh). Marker: `TokenEscrow current — Finish bypasses DisallowIncomingTrustline`.

**Upstream status.** No confirmed fix in checked `3.2.0-b7` or `origin/develop`.

<a id="ammwithdraw-disallow-incoming-trustline-001"></a>
### AMMWITHDRAW-DISALLOW-INCOMING-TRUSTLINE-001 - AMMWithdraw bypasses DisallowIncomingTrustline

**Risk:** 8.0 / High. **Surface:** `AMM`, `DisallowIncoming`, `fixDisallowIncomingV1`. **Class:** issuer policy bypass.

**Broken behavior.** `AMMWithdraw` can withdraw an issuer's IOU from a live AMM pool to a user with no existing issuer trustline, creating the incoming trustline through the AMM withdrawal `accountSend` path.

**Expected behavior.** `AMMWithdraw` should enforce the same incoming-trustline opt-out policy before allowing pool withdrawal to create a receiver trustline.

**Repro:** [`AMMWITHDRAW-DISALLOW-INCOMING-TRUSTLINE-001.sh`](/assets/research/xrpl-rippled-p0-audit/repros/AMMWITHDRAW-DISALLOW-INCOMING-TRUSTLINE-001.sh). Marker: `AMM current — Withdraw bypasses DisallowIncomingTrustline`.

**Upstream status.** No confirmed fix in checked `3.2.0-b7` or `origin/develop`.

<a id="ammcreate-disallow-incoming-trustline-001"></a>
### AMMCREATE-DISALLOW-INCOMING-TRUSTLINE-001 - AMMCreate bypasses DisallowIncomingTrustline

**Risk:** 8.0 / High. **Surface:** `AMM`, `DisallowIncoming`, `fixDisallowIncomingV1`. **Class:** issuer policy bypass.

**Broken behavior.** `AMMCreate` can create a new pool containing an issuer's IOU after that issuer has opted out of incoming trustlines, creating the AMM account trustline through the AMM creation `accountSend` path.

**Expected behavior.** `AMMCreate` should enforce the same incoming-trustline opt-out policy before creating an AMM account trustline for the issuer's IOU.

**Repro:** [`AMMCREATE-DISALLOW-INCOMING-TRUSTLINE-001.sh`](/assets/research/xrpl-rippled-p0-audit/repros/AMMCREATE-DISALLOW-INCOMING-TRUSTLINE-001.sh). Marker: `AMM current — Create bypasses DisallowIncomingTrustline`.

**Upstream status.** No confirmed fix in checked `3.2.0-b7` or `origin/develop`.

<a id="ammdeposit-empty-disallow-incoming-trustline-001"></a>
### AMMDEPOSIT-EMPTY-DISALLOW-INCOMING-TRUSTLINE-001 - AMMDeposit empty-pool bypass

**Risk:** 8.0 / High. **Surface:** `AMM`, `DisallowIncoming`, `fixDisallowIncomingV1`. **Class:** issuer policy bypass.

**Broken behavior.** `AMMDeposit` with `tfTwoAssetIfEmpty` can reinitialize an empty AMM pool and recreate the AMM account trustline to an issuer that has set `asfDisallowIncomingTrustline`.

**Expected behavior.** Empty-pool reinitialization should enforce the same incoming-trustline opt-out policy before recreating the AMM account trustline.

**Repro:** [`AMMDEPOSIT-EMPTY-DISALLOW-INCOMING-TRUSTLINE-001.sh`](/assets/research/xrpl-rippled-p0-audit/repros/AMMDEPOSIT-EMPTY-DISALLOW-INCOMING-TRUSTLINE-001.sh). Marker: `AMM current — Empty deposit bypasses DisallowIncomingTrustline`.

**Upstream status.** No confirmed fix in checked `3.2.0-b7` or `origin/develop`.

<a id="ammclawback-disallow-incoming-paired-asset-001"></a>
### AMMCLAWBACK-DISALLOW-INCOMING-PAIRED-ASSET-001 - AMMClawback paired-asset DisallowIncoming bypass

**Risk:** 8.0 / High. **Surface:** `AMM`, `AMMClawback`, `DisallowIncoming`, `fixDisallowIncomingV1`, `fixAMMClawbackRounding`. **Class:** issuer policy bypass.

**Broken behavior.** In a two-issuer AMM, issuer A can claw back its asset and force-return issuer B's paired IOU to a holder after issuer B has set `asfDisallowIncomingTrustline`, recreating issuer B's trustline through `AMMWithdraw` / `accountSend`.

**Expected behavior.** `AMMClawback` should enforce the paired-asset issuer's incoming-trustline opt-out before returning that issuer's IOU to a holder with no existing trustline.

**Repro:** [`AMMCLAWBACK-DISALLOW-INCOMING-PAIRED-ASSET-001.sh`](/assets/research/xrpl-rippled-p0-audit/repros/AMMCLAWBACK-DISALLOW-INCOMING-PAIRED-ASSET-001.sh). Marker: `AMM current — Clawback returns paired asset through DisallowIncomingTrustline`.

**Upstream status.** No confirmed fix in checked `3.2.0-b7` or `origin/develop`.

<a id="ammclawback-depositauth-paired-asset-001"></a>
### AMMCLAWBACK-DEPOSITAUTH-PAIRED-ASSET-001 - AMMClawback paired-asset DepositAuth bypass

**Risk:** 8.0 / High. **Surface:** `AMM`, `AMMClawback`, `DepositAuth`, `fixAMMClawbackRounding`. **Class:** holder receive-policy bypass.

**Broken behavior.** A holder can set `asfDepositAuth` and reject direct issuer payment with `tecNO_PERMISSION`, but `AMMClawback` can still force-return the paired IOU to that holder through the AMM withdrawal path.

**Expected behavior.** `AMMClawback` should enforce the holder's `DepositAuth` receive policy before returning paired IOU assets.

**Repro:** [`AMMCLAWBACK-DEPOSITAUTH-PAIRED-ASSET-001.sh`](/assets/research/xrpl-rippled-p0-audit/repros/AMMCLAWBACK-DEPOSITAUTH-PAIRED-ASSET-001.sh). Marker: `AMM current — Clawback bypasses DepositAuth paired asset`.

**Upstream status.** No confirmed fix in checked `3.2.0-b7` or `origin/develop`.

<a id="ammbid-depositauth-refund-001"></a>
### AMMBID-DEPOSITAUTH-REFUND-001 - AMMBid auction refund bypasses DepositAuth

**Risk:** 8.0 / High. **Surface:** `AMM`, `DepositAuth`, `fixAMMv1_3`. **Class:** holder receive-policy bypass.

**Broken behavior.** A previous AMM auction-slot owner can set `asfDepositAuth`, and direct LP-token payment to that account is rejected. A later `AMMBid` by another account can still refund LP tokens to the previous owner through `AMMBid::applyBid` / `accountSend`.

**Expected behavior.** `AMMBid` should enforce the previous owner's `DepositAuth` receive policy before refunding LP tokens to a non-signing account.

**Repro:** [`AMMBID-DEPOSITAUTH-REFUND-001.sh`](/assets/research/xrpl-rippled-p0-audit/repros/AMMBID-DEPOSITAUTH-REFUND-001.sh). Marker: `AMM current — Bid refund bypasses DepositAuth`.

**Upstream status.** No confirmed fix in checked `3.2.0-b7` or `origin/develop`.

<a id="mpt-transfer-rate-overflow-001"></a>
### MPT-TRANSFER-RATE-OVERFLOW-001 - MPT transfer-rate scaling overflow

**Risk:** 7.4 / High. **Surface:** `MPTokensV1`. **Class:** arithmetic overflow.

**Broken behavior.** Applying a 1.5 transfer rate to a large integral MPT amount throws `overflow_error` in the legacy scaled-mantissa path.

**Expected behavior.** Valid integral-token amounts should scale through bounded consensus arithmetic or fail cleanly before overflow.

**Repro:** [`MPT-TRANSFER-RATE-OVERFLOW-001.sh`](/assets/research/xrpl-rippled-p0-audit/repros/MPT-TRANSFER-RATE-OVERFLOW-001.sh). Marker: `MPT current — transfer-rate scaling overflows large integral amount`.

**Upstream status.** Fix-looking commit `22fbf4d06` exists, but was not contained in checked `3.2.0-b7` or `origin/develop`. No confirmed fix.

## Findings With Post-3.1.3 Remediation Evidence

<a id="escrow-cancel-iou-001"></a>
### ESCROW-CANCEL-IOU-001 - EscrowCancel deleted IOU trustline exception

**Risk:** 8.1 / High. **Surface:** `TokenEscrow`, `fixTokenEscrowV1`. **Class:** deterministic transaction exception.

**Broken behavior.** Canceling an IOU escrow after the sender trustline was deleted returns `tefEXCEPTION` / owner-count template-field error.

**Expected behavior.** Escrow cancellation accounting should not depend on a deleted sender trustline.

**Repro:** [`ESCROW-CANCEL-IOU-001.sh`](/assets/research/xrpl-rippled-p0-audit/repros/ESCROW-CANCEL-IOU-001.sh). Marker: `EscrowCancel current — deleted IOU trustline returns tefEXCEPTION`.

**Upstream status.** Patched after 3.1.3 in `ad3d172a1`; confirmed present in `3.2.0-b7` and `origin/develop`.

<a id="amm-stale-auth-001"></a>
### AMM-STALE-AUTH-001 - AMM stale AuthAccounts after empty reinit

**Risk:** 8.0 / High. **Surface:** `AMM`, `fixAMMv1_3`. **Class:** stale authorization state.

**Broken behavior.** Empty-pool reinitialization with `tfTwoAssetIfEmpty` leaves stale `sfAuthAccounts` from the prior auction slot.

**Expected behavior.** Reinitializing an empty AMM should clear stale auction authorization state.

**Repro:** [`AMM-STALE-AUTH-001.sh`](/assets/research/xrpl-rippled-p0-audit/repros/AMM-STALE-AUTH-001.sh). Marker: `AMM current — stale AuthAccounts survive empty reinit`.

**Upstream status.** Patched after 3.1.3 in `779b49cd9`; confirmed present in `3.2.0-b7` and `origin/develop`.

<a id="mpt-noncanonical-amount-001"></a>
### MPT-NONCANONICAL-AMOUNT-001 - Non-canonical MPT amount reaches ledger engine

**Risk:** 7.6 / High. **Surface:** `MPTokensV1`, `fixMPTDeliveredAmount`. **Class:** malformed amount accepted into application path.

**Broken behavior.** A non-canonical MPT amount reaches transaction application and returns fee-burning `tecPATH_PARTIAL` instead of failing preflight as `temBAD_AMOUNT`.

**Expected behavior.** Non-canonical MPT amounts should fail before ledger application.

**Repro:** [`MPT-NONCANONICAL-AMOUNT-001.sh`](/assets/research/xrpl-rippled-p0-audit/repros/MPT-NONCANONICAL-AMOUNT-001.sh). Marker: `MPT current — non-canonical amount reaches ledger engine`.

**Upstream status.** Patched after 3.1.3 in `dcd2ff0b5`; confirmed present in `origin/develop`, not confirmed in `3.2.0-b7`.

<a id="pdex-hybrid-quality-001"></a>
### PDEX-HYBRID-QUALITY-001 - Permissioned-DEX hybrid-offer quality mismatch

**Risk:** 7.7 / High. **Surface:** `PermissionedDEX`, `PermissionedDomains`, `Credentials`. **Class:** order-book metadata corruption.

**Broken behavior.** A partially crossed hybrid offer leaves its open-book directory key at one quality while `sfExchangeRate` records another.

**Expected behavior.** Open-book directory key quality and `sfExchangeRate` metadata must match after partial crossing.

**Repro:** [`PDEX-HYBRID-QUALITY-001.sh`](/assets/research/xrpl-rippled-p0-audit/repros/PDEX-HYBRID-QUALITY-001.sh). Marker: `Permissioned DEX current — hybrid offer open-book quality mismatch`.

**Upstream status.** Patched after 3.1.3 in `28cc20c81`; confirmed present in `3.2.0-b7` and `origin/develop`.

<a id="pdex-cancel-invariant-001"></a>
### PDEX-CANCEL-INVARIANT-001 - Permissioned-DEX regular-offer cancel invariant failure

**Risk:** 7.5 / High. **Surface:** `PermissionedDEX`, `PermissionedDomains`, `Credentials`. **Class:** valid transaction invariant failure.

**Broken behavior.** A valid domain `OfferCreate` that cancels the user regular offer fails with `tecINVARIANT_FAILED` because the invariant treats the deleted regular offer as forbidden mutation.

**Expected behavior.** The invariant should ignore regular offers deleted as part of a valid domain offer cancellation.

**Repro:** [`PDEX-CANCEL-INVARIANT-001.sh`](/assets/research/xrpl-rippled-p0-audit/repros/PDEX-CANCEL-INVARIANT-001.sh). Marker: `Permissioned DEX current — cancel regular offer via domain offer invariant`.

**Upstream status.** Patched after 3.1.3 in `8c0080020`; confirmed present in `3.2.0-b7` and `origin/develop`.

## Excluded Boundary

`MPT-DOMAIN-AUTH-001` is excluded from the live packet. The reproduced MPT `DomainID` path requires `SingleAssetVault` in the current `MPTokenIssuanceCreate` / `MPTokenIssuanceSet` feature gate, and direct XRPL mainnet status shows `SingleAssetVault=false`.

Cleanup-era candidates are also excluded unless they reproduce with `fixCleanup3_1_3` enabled. The raw on-ledger `Amendments` object contains the `fixCleanup3_1_3` hash, so old pre-cleanup reproduction alone is not enough for this public live inventory.

## Implications For Post Fiat

1. A RippleD-derived implementation path cannot inherit enabled XRPL surfaces blindly. The current packet shows repeated gaps between direct policy checks and indirect settlement paths.
2. Receive-policy enforcement should be centralized. If every transaction family is responsible for remembering `DisallowIncomingTrustline`, `DepositAuth`, freeze, authorization, reserve, and owner-count rules, the surface grows faster than review coverage.
3. Fork authors should treat `3.1.3` as unsafe to inherit without this packet's fixes or equivalent negative controls.

---

<div class="pearl-disclaimer">

**Disclaimer**

This document is published by **Post Fiat / AGTI** for informational purposes only. It describes our **internal code-quality evaluation** of the open-source **RippleD** codebase (baseline `release-3.1.3`, May 2026). Post Fiat evaluated RippleD-derived implementation paths; we are **not** speaking on behalf of Ripple, Ripple Labs, the XRP Ledger Foundation (XRPLF), or any other third party.

Nothing here is legal, investment, tax, or security advice. Observations are based on static code review, local unit tests (jtx), helper/protocol-wire checks, live amendment-status filtering, and our interpretation of upstream behavior at a point in time. Upstream code, amendments, and deployment configurations change. Readers should perform their own due diligence and consult qualified professionals before acting.

Issue identifiers are **internal audit labels**, not official CVEs or vendor advisories. Descriptions of hypothetical exploit paths are **research scenarios**, not allegations of wrongdoing, negligence, or breach of duty by any person or organization.

**No warranty.** This report is provided "as is" without warranty of any kind. To the fullest extent permitted by law, Post Fiat / AGTI disclaims liability for any loss or damage arising from use of or reliance on this material.

**Trademarks.** Ripple, XRP, XRPL, rippled, and related names are trademarks of their respective owners. Post Fiat is an independent project.

**Corrections.** If you believe any statement is inaccurate, contact us with reproducible evidence and we will review updates in good faith.

*Baseline: upstream rippled `release-3.1.3` - direct XRPL validated-ledger amendment receipt checked 2026-05-27T20:06:02Z*

</div>
