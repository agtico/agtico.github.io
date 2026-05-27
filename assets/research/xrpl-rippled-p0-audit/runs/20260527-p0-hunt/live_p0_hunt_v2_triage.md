# Live XRPL P0 Hunt V2 Triage

Checked: `2026-05-27T20:06:02Z`

## Live Scope

Direct XRPL JSON-RPC showed the public servers checked for the packet reporting
`rippled_version=3.1.3`.

Enabled live surfaces for this packet:

- `AMM`
- `AMMClawback`
- `Checks`
- `CheckCashMakesTrustLine`
- `DepositAuth`
- `DisallowIncoming`
- `fixDisallowIncomingV1`
- `MPTokensV1`
- `NonFungibleTokensV1_1`
- `fixEnforceNFTokenTrustline`
- `fixEnforceNFTokenTrustlineV2`
- `fixRemoveNFTokenAutoTrustLine`
- `fixNFTokenReserve`
- `fixNFTokenRemint`
- `NFTokenMintOffer`
- `PermissionedDomains`
- `PermissionedDEX`
- `TokenEscrow`
- `Credentials`
- `fixMPTDeliveredAmount`
- `fixAMMv1_3`
- `fixTokenEscrowV1`
- `fixAMMClawbackRounding`
- `fixCleanup3_1_3`

Disabled surfaces that remain excluded:

- `LendingProtocol`
- `SingleAssetVault`
- `PermissionDelegation`
- `Batch`
- `fixDelegateV1_1`
- `fixDisallowIncomingV1_1`

`fixCleanup3_1_3` is included by raw amendment hash
`303ACB16CF8DBD3B5C34F131A9D19A7DE01AE05F480A8A682B869D1B4AAC8CFC`.
The public `feature` name lookup did not expose that name, so the gate checks
the raw on-ledger `Amendments` object rather than relying only on feature-name
visibility.

## Live Dependency Correction

`MPT-DOMAIN-AUTH-001` is excluded from the live packet. The reproduced path uses
MPT `DomainID`, and current `MPTokenIssuanceCreate` / `MPTokenIssuanceSet`
feature gates require `PermissionedDomains && SingleAssetVault` for that field.
Direct XRPL mainnet status shows `SingleAssetVault=false`, so this finding does
not satisfy the live-mainnet-only constraint.

## Promotion Result

Five additional findings were promoted during the live-only continuation:

- `TRUSTLINE-POSITIVE-BALANCE-RESERVE-001`: standard IOU offer crossing can
  create a positive balance for a receiver while leaving `OwnerCount=0` and the
  receiver reserve flag unset after that receiver previously cleared the
  trustline limit and balance.
- `TRUSTLINE-DISALLOW-INCOMING-OFFER-001`: an issuer can set
  `asfDisallowIncomingTrustline`, but `OfferCreate` can still cross into that
  issuer's IOU for a taker with no existing trustline, creating the incoming
  trustline through the offer path.
- `NFTOKEN-DISALLOW-INCOMING-ACCEPT-001`: the same issuer opt-out is bypassed
  by `NFTokenAcceptOffer`; direct `TrustSet` is rejected, but NFT sell-offer
  settlement in the issuer's IOU creates the seller trustline anyway.
- `CHECKCASH-DISALLOW-INCOMING-TRUSTLINE-001`: the same issuer opt-out is
  bypassed by `CheckCash`; direct `TrustSet` is rejected, but cashing an IOU
  check auto-creates the receiver trustline anyway.
- `TOKENESCROW-DISALLOW-INCOMING-FINISH-001`: the same issuer opt-out is
  bypassed by `EscrowFinish`; direct `TrustSet` is rejected, but finishing an
  IOU TokenEscrow auto-creates the destination trustline anyway.

## Latest Demotion

- `MPT-ESCROW-SENDER-LOCK-001`: tested whether MPT TokenEscrow finish ignores
  a sender lock applied after escrow creation. The scratch `OpenP0Repro` case
  returned `tecNO_PERMISSION`, not `tesSUCCESS`, so no live-mainnet P0 was
  promoted and the scratch repro was removed.

A later AMMClawback slice promoted one more unresolved live finding:

- `AMMCLAWBACK-DISALLOW-INCOMING-PAIRED-ASSET-001`: in a two-issuer AMM, issuer
  A can claw back its asset and force-return issuer B's paired IOU to a holder
  after issuer B has set `asfDisallowIncomingTrustline`, recreating issuer B's
  trustline through the AMMWithdraw/accountSend path even though direct
  `TrustSet` is rejected.

A later NFToken broker-fee slice promoted one more unresolved live finding:

- `NFTOKEN-BROKER-FEE-DISALLOW-INCOMING-TRUSTLINE-001`: a brokered
  `NFTokenAcceptOffer` can pay the broker fee in an issuer's IOU to a broker
  with no existing trustline after that issuer has set
  `asfDisallowIncomingTrustline`, creating the broker trustline through
  NFTokenAcceptOffer::pay/accountSend even though direct `TrustSet` is rejected.

A later AMMClawback/DepositAuth slice promoted one more unresolved live finding:

- `AMMCLAWBACK-DEPOSITAUTH-PAIRED-ASSET-001`: in a two-issuer AMM, a holder can
  set `asfDepositAuth` and reject direct issuer payment with `tecNO_PERMISSION`,
  but AMMClawback can still force-return the paired IOU to that holder and
  recreate the paired-asset trustline through the AMMWithdraw/accountSend path.

A later AMMBid/DepositAuth slice promoted one more unresolved live finding:

- `AMMBID-DEPOSITAUTH-REFUND-001`: a previous AMM auction-slot owner can set
  `asfDepositAuth` and reject direct LP-token payment with `tecNO_PERMISSION`,
  but a later `AMMBid` by another account can still refund LP tokens to that
  previous owner through the AMMBid/accountSend path.

The remaining high-volume candidate set in the existing matrix is blocked by
one of the WHIP gates:

- cleanup-era candidates are excluded because `fixCleanup3_1_3` is enabled by
  raw amendment hash;
- lending, vault, delegation, and batch candidates are excluded because their
  required amendment surfaces remain disabled;
- protocol-wire and arithmetic-helper candidates remain below the P0 severity
  bar unless a transaction-visible live surface is demonstrated;
- already-remediated findings remain release-line evidence, not novel unfixed
  P0s.
- `CRED-DIRFULL-001` was tested on the live `Credentials` surface after direct
  status refresh. The scratch proof forced the subject owner directory full and
  returned `tecDIR_FULL` without leaving a `Credential` object or issuer/subject
  owner-count drift, so it is source-killed and not packet eligible.
- `PAYMENT-DISALLOW-INCOMING-TRUSTLINE-001` was tested as a direct
  holder-to-recipient IOU `Payment` variant of the live
  `DisallowIncomingTrustline` family. It returned `tecPATH_DRY` and left no
  recipient trustline, so the plain Payment path is source-killed and not packet
  eligible.

## Still-Promoted Unfixed Set

The packet now has fourteen live-enabled findings with no confirmed upstream fix in
the checked `3.2.0-b7` or `origin/develop` refs:

- `MPT-TRANSFER-RATE-OVERFLOW-001`
- `MPT-LOCK-UNAUTH-001`
- `TRUSTLINE-POSITIVE-BALANCE-RESERVE-001`
- `TRUSTLINE-DISALLOW-INCOMING-OFFER-001`
- `NFTOKEN-DISALLOW-INCOMING-ACCEPT-001`
- `NFTOKEN-BROKER-FEE-DISALLOW-INCOMING-TRUSTLINE-001`
- `CHECKCASH-DISALLOW-INCOMING-TRUSTLINE-001`
- `TOKENESCROW-DISALLOW-INCOMING-FINISH-001`
- `AMMWITHDRAW-DISALLOW-INCOMING-TRUSTLINE-001`
- `AMMCREATE-DISALLOW-INCOMING-TRUSTLINE-001`
- `AMMDEPOSIT-EMPTY-DISALLOW-INCOMING-TRUSTLINE-001`
- `AMMCLAWBACK-DISALLOW-INCOMING-PAIRED-ASSET-001`
- `AMMCLAWBACK-DEPOSITAUTH-PAIRED-ASSET-001`
- `AMMBID-DEPOSITAUTH-REFUND-001`

## Next Hunt Targets

The next high-yield live-only review should stay inside:

1. MPT auth, lock, transfer-rate, and RequireAuth paths.
2. AMM and AMMClawback authorization, freeze, and invariant paths.
3. PermissionedDEX domain/credential quality and cancellation invariants.
4. TokenEscrow result-code and owner-count/invariant paths.
5. Credentials current-tag transaction paths that do not rely on disabled
   cleanup-era behavior.
