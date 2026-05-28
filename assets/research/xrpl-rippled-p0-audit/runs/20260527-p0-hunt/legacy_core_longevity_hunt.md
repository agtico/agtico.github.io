# XRPL Legacy-Core Longevity P0 Hunt Plan

Date: 2026-05-28

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

## Current Lead For The Next 8-Hour Pass

`TRUSTLINE-POSITIVE-BALANCE-RESERVE-001` is now the model candidate for this
lane. The bug shape is old core reserve/accounting drift:

1. an account clears its IOU trustline limit and returns to zero balance;
2. the trustline remains with `OwnerCount=0` and no receiver reserve flag;
3. offer crossing gives the account a positive IOU balance again;
4. `OwnerCount` remains `0` and the receiver reserve flag remains unset.

Packet-bound proof already exists on the current `3.1.3` target, `2.5.0`, and
`2.0.0`. A `1.5.0` Ubuntu 20.04 Docker build produced a passing terminal repro
with marker:

```text
Legacy 1.5.0 -- offer crossing creates positive balance without reserve
2.5s, 1 suite, 9 cases, 271 tests total, 0 failures
```

That `1.5.0` proof is not yet packet evidence until the patch/log are saved,
hashed, verifier-bound, committed, and the temporary worktree is cleaned.

The next hunt should look for siblings of this shape before broadening:
alternate old receive/settlement paths that create positive balance,
non-default trustline state, object ownership, or directory entries without the
matching reserve/owner-count transition.

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

## 8-Hour Operator Queue

### Phase 0. Package `1.5.0` Trustline Proof

Timebox: 45 minutes.

Tasks:

1. Save the `1.5.0` repro patch and full proof log.
2. Record tag commit, tag date, Docker image, build flags, proof marker, and
   result count.
3. Add the artifact hashes to this run packet.
4. Run the packet verifier and the trustline repro wrapper.
5. Commit only if checks pass.
6. Return the temporary `rippled-1.5.0` worktree to a clean/non-misleading
   state after artifacting.

### Phase 1. Trustline/Offer Reserve Siblings

Timebox: 2 hours.

Probe first:

1. Path payment/rippling creates positive balance after a cleared trustline.
2. `CheckCash` creates or resurrects positive IOU balance without reserve.
3. Offer crossing with transfer rate, quality-in/out, and partial crossing at
   reserve boundaries.
4. Trustline default-state deletion followed by alternate-path resurrection.
5. Direct `Payment`/`TrustSet` controls that reject or charge reserve while the
   alternate path succeeds.

Promotion condition: positive balance, owned object, directory entry, or
non-default trustline state exists without the expected owner/reserve
accounting.

### Phase 2. Old Object Lifecycle And Directory State

Timebox: 90 minutes.

Probe:

1. `CreateCheck` directory-full and two-owner-directory failure paths.
2. `PayChan` close/claim/delete around recipient owner-directory migration.
3. XRP `Escrow` cancel/finish/delete around account deletion and owner counts.
4. Ticket-paid create paths that may derive object keys from raw `sfSequence`.

Promotion condition: a live transaction can strand an object, leak a directory
entry, leave owner count wrong, collide object keys, or reach an internal
exception from normal input.

### Phase 3. Authorization/Freeze Receive-Path Siblings

Timebox: 90 minutes.

Probe:

1. Direct payment control versus path payment, offer crossing, check cash, and
   deferred settlement.
2. Local freeze and global freeze on IOU receive/send/offer paths.
3. `RequireAuth` on direct and indirect receive paths.
4. Clawback/deep-freeze interactions only where direct RPC proves the surface
   live.

Demote pure semantics disputes. Promote only if a direct path rejects but an
indirect path accepts the same ledger effect and creates durable state or value
movement.

### Phase 4. Deterministic Exception And Arithmetic Sweep

Timebox: 75 minutes.

Probe:

1. `STAmount`, quality, and transfer-rate boundaries in old IOU/offer paths.
2. Path-payment amount extraction and issue mismatch paths.
3. Offer book crossing with tiny/huge qualities and partial crossing.
4. Owner-directory traversal and object deletion failure paths.

Promotion condition: transaction-visible `tefINTERNAL`, `tefEXCEPTION`,
overflow, assertion, or invariant failure from normal input.

### Phase 5. Source-Signal Clustering

Timebox: 45 minutes.

Use commit history, branch names, and touched files as source signals, not
evidence. Cluster around:

- `fix-positive-balance-trustline-pay-no-reserve`;
- owner-count and reserve edits;
- `rippleCreditIOU`, `accountSend`, `trustCreate`, `trustDelete`,
  `adjustOwnerCount`;
- directory insert/delete helpers;
- result-code changes in `Payment`, `OfferCreate`, `SetTrust`, `CreateCheck`,
  `CashCheck`, `Escrow`, and `PayChan`.

If a signal does not produce a repro quickly, write a source-kill note and move
on.

### Phase 6. Packet Hardening

Timebox: final 30 minutes.

For every promoted candidate: save patch/log/hash, update repro wrapper,
manifest, packet docs, triage, verifier, run checks, and commit only after
checks pass.

## Legacy Candidate Details

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

## Legacy Fallback Probe Order

Use this queue only after the `1.5.0` proof is packet-bound and the
trustline/offer reserve sibling probes are exhausted or source-killed:

1. `CHECK-LEGACY-DIRFULL-PARTIAL-001`
2. `PAYCHAN-LEGACY-CLOSE-OWNERDIR-001`
3. `TRUSTSET-LEGACY-RESERVE-CARVEOUT-001`
4. `OFFER-LEGACY-RESERVE-CROSSED-001`
5. `TICKET-LEGACY-SEQUENCE-COLLISION-001`
6. `PAYMENT-LEGACY-TEFEXCEPTION-PATH-001`
7. `ESCROW-LEGACY-XRP-DELETE-EDGE-001`

This order is still useful for negative inventory, but the new evidence says
reserve/accounting siblings should get first attention.

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

Package the observed `1.5.0`
`TRUSTLINE-POSITIVE-BALANCE-RESERVE-001` proof into the packet. If that cannot
be done within 45 minutes, write the precise blocker. Then start Phase 1 and
hunt sibling trustline/offer reserve bugs before returning to the legacy
fallback queue. Do not spend more than one hour on a candidate without either a
reproducing marker or a written source-kill note.
