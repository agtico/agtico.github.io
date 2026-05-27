# Live XRPL P0 Hunt V2 Triage

Checked: `2026-05-27T16:14:26Z`

## Live Scope

Direct XRPL JSON-RPC showed the public servers checked for the packet reporting
`rippled_version=3.1.3`.

Enabled live surfaces for this packet:

- `AMM`
- `AMMClawback`
- `MPTokensV1`
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

No additional finding was promoted in this slice.

Reason: the highest-volume candidate set in the existing matrix is blocked by
one of the WHIP gates:

- cleanup-era candidates are excluded because `fixCleanup3_1_3` is enabled by
  raw amendment hash;
- lending, vault, delegation, and batch candidates are excluded because their
  required amendment surfaces remain disabled;
- protocol-wire and arithmetic-helper candidates remain below the P0 severity
  bar unless a transaction-visible live surface is demonstrated;
- already-remediated findings remain release-line evidence, not novel unfixed
  P0s.

## Still-Promoted Unfixed Set

The packet still has two live-enabled findings with no confirmed upstream fix in
the checked `3.2.0-b7` or `origin/develop` refs:

- `MPT-TRANSFER-RATE-OVERFLOW-001`
- `MPT-LOCK-UNAUTH-001`

## Next Hunt Targets

The next high-yield live-only review should stay inside:

1. MPT auth, lock, transfer-rate, and RequireAuth paths.
2. AMM and AMMClawback authorization, freeze, and invariant paths.
3. PermissionedDEX domain/credential quality and cancellation invariants.
4. TokenEscrow result-code and owner-count/invariant paths.
5. Credentials current-tag transaction paths that do not rely on disabled
   cleanup-era behavior.
