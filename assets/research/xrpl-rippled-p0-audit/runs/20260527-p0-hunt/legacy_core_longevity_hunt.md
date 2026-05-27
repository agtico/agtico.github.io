# XRPL Legacy-Core Longevity P0 Hunt Plan

Date: 2026-05-27

Scope: find live XRPL bugs that are old, simple, and transaction-visible. This
is deliberately not another sweep of newly activated AMM/MPT/Vault/Lending
surfaces unless a new feature exposes an older primitive that was already wrong.

Do not touch the public article during this hunt. Promote only packet, repro,
or triage evidence after the gates below pass.

## Objective

The strongest target is a bug that has probably been live for years and looks
obvious once isolated:

- an old transaction or ledger primitive;
- a current `3.1.3` reproduction;
- preferably the same reproduction or equivalent source proof on an older tag;
- a small invariant violation, not a subtle product debate;
- a clean control case showing the intended path rejects while the alternate
  path succeeds, or showing state/accounting corruption that should be
  impossible.

If this works, the finding is materially stronger than "a new amendment shipped
with a bug." It says an old live primitive sat broken through many releases.

## Promotion Gates

### 1. Oldness Gate

A candidate may enter the legacy lane only if one of these holds:

- the transaction path existed before 2020;
- the amendment has been live for several years;
- the allegedly broken helper is used by a pre-2020 transaction path.

For every promoted candidate, record:

```text
first_seen_commit
first_seen_tag_or_release
oldest_reproduced_tag_if_buildable
current_reproduced_tag
years_live_estimate
amendment_or_mainnet_surface_status
```

If an old tag will not build quickly, keep the candidate alive only if the code
path and invariant are substantially unchanged across history.

### 2. Stupidity Gate

The bug must be easy to explain. Preferred patterns:

- direct control rejects, alternate path succeeds;
- two-step directory insertion leaves partial state or owner-count drift;
- reserve/accounting rule is applied in one path and skipped in another;
- freeze/auth/deposit rule is enforced in one receive path and skipped in
  another old receive path;
- normal transaction input reaches `tefINTERNAL`, `tefEXCEPTION`, or an
  invariant failure;
- sequence, ticket, or object-key construction uses the wrong sequence source.

Do not promote merely because code is ugly. The witness needs a ledger effect,
incorrect result code, or impossible state.

### 3. Live Gate

Only current mainnet-enabled surfaces count for the live packet:

- reproduce against the current checked upstream target;
- prove required amendments are enabled on live XRPL using direct ledger status,
  not explorer summaries;
- exclude disabled surfaces even if the code is present in `rippled`.

### 4. Remediation Gate

Classify every candidate before promotion:

- no confirmed upstream fix in `3.2.0-b7` or `origin/develop`;
- fixed after `3.1.3`, but still important because it is live until upgrade;
- historical/replay-era only;
- disabled/future-only;
- product semantics, not a P0.

For this hunt, prefer "no confirmed fix" and "multi-year old" over
"already-known next-release cleanup."

## Initial Investigation Notes

The first source/history pass points at old files with simple-looking state
machinery:

| Surface | First source signal | Why it is worth probing |
|---|---:|---|
| `Payment.cpp` | 2012 | Oldest receive/pathing core; current code has explicit `tefEXCEPTION` guard for amount issue extraction. |
| `SetTrust.cpp` | 2012 | Trustline reserve carveout intentionally skips reserve enforcement while `OwnerCount < 2`; reserve and positive-balance rules are already a productive defect family. |
| `CreateOffer.cpp` / path steps | 2014-2016 | Old offer crossing allows partial execution even when the signer lacks reserve to place a remainder; path/offer settlement is where old accounting shortcuts hide. |
| `Escrow.cpp` | 2015 | Legacy XRP escrow is old; TokenEscrow is newer, but old cancel/finish/delete semantics can still expose ownership and destination edge cases. |
| `PayChan.cpp` | 2016 | Payment-channel owner-directory handling has explicit recipient-directory amendment code and a 2025 fix family; this is a strong old-code migration target. |
| `CreateCheck.cpp` / `CashCheck.cpp` | 2018 | Check creation uses object insertion plus two owner-directory insertions; CheckCash can auto-create trustlines. Both are old and mechanically simple. |
| `CreateTicket.cpp` / `Transactor.cpp` | 2014+ | Tickets reduce sequence ordering constraints; raw sequence vs ticket sequence mistakes have already produced real bugs. |

## Candidate Queue

### 1. `CHECK-LEGACY-DIRFULL-PARTIAL-001`

Hypothesis: `CreateCheck` inserts the `Check` object, then inserts it into the
destination owner directory, then inserts it into the source owner directory.
If the second directory insertion fails, there may be leaked object state,
owner-dir inconsistency, or rollback behavior worth proving.

Why this is high yield:

- Checks date to 2018.
- The code is simple.
- The bug class is not philosophical: either rollback is clean or it is not.

Probe:

1. Fill or force the source owner directory boundary while destination insertion
   remains possible.
2. Submit `CheckCreate`.
3. Assert result code, absence/presence of the `Check` object, source and
   destination owner directories, and both owner counts.
4. Repeat with destination directory boundary first.
5. If current reproduces, test an older checks-era tag or source-compare the
   same order of operations.

Promotion condition: `tecDIR_FULL` or another failure leaves any persistent
object, owner-dir entry, or owner-count drift.

### 2. `PAYCHAN-LEGACY-CLOSE-OWNERDIR-001`

Hypothesis: old payment-channel close/refund code or the
`fixPayChanRecipientOwnerDir` migration surface leaves a mismatch between
source owner directory, destination owner directory, owner count, and channel
object lifecycle.

Why this is high yield:

- Payment channels date to 2016.
- A later recipient owner-dir fix exists, which is a useful source signal.
- Close paths refund XRP and delete an object, so a bad edge can be serious.

Probe:

1. Create channels across old/new recipient-owner-dir amendment settings where
   possible in unit harness.
2. Exercise close-by-source, close-by-destination-after-expiry, and claim-close.
3. Test destination deletion or obligation accounting around recipient owner-dir
   entries.
4. Assert source balance refund, owner count, channel deletion, and destination
   owner directory cleanup.

Promotion condition: live transaction can strand a channel, leak an owner-dir
entry, delete with unresolved obligation, or return an internal error under
normal inputs.

### 3. `TRUSTSET-LEGACY-RESERVE-CARVEOUT-001`

Hypothesis: the old `SetTrust` reserve carveout for accounts with fewer than
two owned objects interacts badly with positive-balance trustlines, offer
crossing, or deletion, creating reserve/accounting states that should be
impossible.

Why this is high yield:

- Trustlines date to the original transaction engine.
- Existing current findings already show trustline reserve/accounting pressure.
- The code comment explicitly documents a product exception that may have
  metastasized into inconsistent enforcement.

Probe:

1. Create under-reserved accounts with `OwnerCount` 0, 1, and 2.
2. Compare direct `TrustSet`, offer-created trustline, CheckCash-created
   trustline, and TokenEscrow-created trustline.
3. Track `OwnerCount`, reserve flags, balance sign, default-state deletion, and
   spendability.
4. Try old tags/source history to establish whether the behavior is long-lived.

Promotion condition: an account holds spendable positive IOU balance or a
non-default trustline without the required reserve/owner accounting, or a direct
control rejects while an old alternate path succeeds.

### 4. `OFFER-LEGACY-RESERVE-CROSSED-001`

Hypothesis: `OfferCreate` allows crossing to execute even when the signer lacks
the reserve required to place the remainder. A boundary case may leave an offer,
trustline, owner count, or fee/result-code state inconsistent with the
reserve-check model.

Why this is high yield:

- Offer crossing is old, core, and economically important.
- The source explicitly allows crossed execution under insufficient reserve.
- The existing positive-balance/reserve finding suggests this region is fertile.

Probe:

1. Build crossed and partially crossed offers with exact reserve boundary
   accounts.
2. Compare fully crossed, partially crossed, and no-cross controls.
3. Assert open offer existence, owner count, balance, reserve, and result code.
4. Vary `tfSell`, transfer rates, freeze/auth, and no-ripple only after the
   simple reserve boundary is exhausted.

Promotion condition: partially crossed execution leaves persistent state that a
reserve-equivalent direct path would reject, or returns success with accounting
that violates owner/reserve invariants.

### 5. `PAYMENT-LEGACY-TEFEXCEPTION-PATH-001`

Hypothesis: old payment/pathing inputs can still route normal malformed-but
accepted transactions to `tefEXCEPTION`, `tefINTERNAL`, or fee-burning behavior
instead of deterministic preflight/preclaim rejection.

Why this is high yield:

- Payment is the core XRPL transaction path.
- Current source contains explicit exception guards.
- Any old payment-path result-code defect has high rhetorical value if clean.

Probe:

1. Sweep IOU/XRP path combinations, malformed path elements, no-line states,
   require-auth, freeze, and deposit-auth controls.
2. Reject future-only MPT variants unless the old payment primitive is the
   actual failing root.
3. Assert result code class and whether the sequence/fee behavior is
   appropriate for the reject layer.

Promotion condition: normal submitted transaction reaches `tefEXCEPTION`,
`tefINTERNAL`, invariant failure, or burns a fee where the equivalent bad input
should fail before fee claim.

### 6. `ESCROW-LEGACY-XRP-DELETE-EDGE-001`

Hypothesis: legacy XRP escrow cancel/finish interactions with account deletion,
owner directories, or ticket sequence handling can strand obligations or return
internal errors.

Why this is high yield:

- SuspendedPayment/Escrow lineage dates to 2015.
- XRP escrow is older than TokenEscrow and therefore more valuable if broken.
- Account-delete plus obligations is a historically productive bug shape.

Probe:

1. Create XRP escrows with source/destination boundary conditions.
2. Attempt source and destination account deletion before cancel/finish.
3. Exercise cancel/finish through tickets and ordinary sequences.
4. Assert escrow object, owner count, owner directory, and balance after failure.

Promotion condition: deletion succeeds with live obligation, cancel/finish hits
internal failure under normal state, or owner accounting drifts.

### 7. `TICKET-LEGACY-SEQUENCE-COLLISION-001`

Hypothesis: a remaining create-object path uses raw `sfSequence` instead of the
sequence proxy, causing ticket-paid transactions to collide or derive duplicate
object keys.

Why this is high yield:

- Tickets date to 2014.
- Raw sequence bugs are easy to understand and have prior evidence.
- A generalized lint can cover many transaction types quickly.

Probe:

1. Static scan every object-creating transactor for raw `sfSequence`,
   `getSeqValue`, and keylet construction.
2. For each suspicious path, run two ticket-paid creates from the same account.
3. Assert duplicate-key exception, result code, and rollback behavior.

Promotion condition: any live transaction type reaches duplicate insertion,
`tefEXCEPTION`, or object collision from normal ticket-paid use.

## First Probe Order

1. `CHECK-LEGACY-DIRFULL-PARTIAL-001`
2. `PAYCHAN-LEGACY-CLOSE-OWNERDIR-001`
3. `TRUSTSET-LEGACY-RESERVE-CARVEOUT-001`
4. `OFFER-LEGACY-RESERVE-CROSSED-001`
5. `TICKET-LEGACY-SEQUENCE-COLLISION-001`
6. `PAYMENT-LEGACY-TEFEXCEPTION-PATH-001`
7. `ESCROW-LEGACY-XRP-DELETE-EDGE-001`

This order is chosen for fastest kill/promote cycle. Directory and sequence
bugs should fail loudly if present. Reserve/accounting bugs require more state
setup but are likely to be more serious if they reproduce.

## Evidence Packet Template

Every promoted legacy finding gets a compact packet:

```text
id:
title:
legacy_surface:
first_seen_commit:
first_seen_release_or_tag:
current_target:
oldest_reproduced_tag:
live_amendment_status:
control_rejects:
alternate_path_succeeds:
broken_behavior:
state_before:
state_after:
repro_script_or_test:
proof_hash:
upstream_fix_status:
severity_rationale:
article_status: not-public-until-authorized
```

## Do Not Promote

- disabled mainnet surfaces;
- future-branch-only paths;
- new AMM/MPT/Vault/Lending issues unless they expose an old primitive;
- helper math without a transaction-visible effect;
- already scheduled fixes unless the oldness proof materially changes the
  significance;
- product-semantics disputes without a control rejection or invariant break;
- broad "XRPL bad" framing without a minimized repro.

## Immediate Next Action

Write one scratch jtx test for `CHECK-LEGACY-DIRFULL-PARTIAL-001`. If it dies
cleanly, move directly to `PAYCHAN-LEGACY-CLOSE-OWNERDIR-001`. Do not spend more
than one hour on a candidate without either a reproducing marker or a written
source-kill note.
