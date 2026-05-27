#!/usr/bin/env python3
"""
Model IOU freeze flag semantics vs rippled checkFrozen / checkDeepFrozen.

This is a logic model only — not a ledger simulation. It shows why
regular-only freeze bypasses every path that uses checkDeepFrozen alone.

Run: python3 freeze_check_model.py
"""

from __future__ import annotations

from dataclasses import dataclass


@dataclass(frozen=True)
class TrustLineFlags:
    regular_freeze: bool = False
    deep_freeze: bool = False

    def valid(self) -> bool:
        # On XRPL, deep freeze requires regular freeze first.
        return not self.deep_freeze or self.regular_freeze


def check_frozen(flags: TrustLineFlags) -> bool:
    """Rippled checkFrozen / isFrozen on IOU receiver — blocks both levels."""
    return flags.regular_freeze or flags.deep_freeze


def check_deep_frozen(flags: TrustLineFlags) -> bool:
    """Rippled checkDeepFrozen / isDeepFrozen — blocks deep only."""
    return flags.deep_freeze


def tx_allowed(wrong_check) -> bool:
    """If check returns 'frozen' (True), transactor should reject (tecFROZEN)."""
    return not wrong_check


SCENARIOS = [
    ("No freeze", TrustLineFlags()),
    ("Regular freeze only (compliance case)", TrustLineFlags(regular_freeze=True)),
    ("Deep + regular freeze", TrustLineFlags(regular_freeze=True, deep_freeze=True)),
]


def main() -> None:
    print("XRPL IOU freeze check model\n")
    print(f"{'Scenario':<40} {'checkFrozen blocks?':<22} {'checkDeepFrozen blocks?':<26} {'Bug path allows IOU?'}")
    print("-" * 110)
    for name, flags in SCENARIOS:
        if not flags.valid():
            continue
        cf = check_frozen(flags)
        cd = check_deep_frozen(flags)
        bug_allows = tx_allowed(check_deep_frozen(flags)) and not tx_allowed(check_frozen(flags))
        print(
            f"{name:<40} {str(cf):<22} {str(cd):<26} {str(bug_allows)}"
        )

    print("\nRows where 'Bug path allows IOU?' is True are the regular-freeze bypass class.")
    print("This model covers the freeze predicate. It does not prove each transaction path by itself.")
    print("Existing jtx often sets tfSetFreeze | tfSetDeepFreeze together — misses the regular-only row.\n")

    jtx_confirmed = [
        ("F3.3", "LoanBrokerCoverWithdraw", "destination", "checkDeepFrozen"),
        ("F3.5", "LoanBrokerDelete", "broker owner", "checkDeepFrozen"),
        ("F3.6", "LoanPay", "broker owner fee routing", "isDeepFrozen"),
        ("F3.7", "LoanSet", "broker owner origination fee", "checkDeepFrozen"),
        ("F3.8", "LoanPay", "vault pseudo", "checkDeepFrozen"),
        ("F3.9", "LoanBrokerCoverDeposit", "broker pseudo", "checkDeepFrozen"),
        ("F3.10", "LoanPay", "broker pseudo fallback fee", "checkDeepFrozen"),
    ]
    not_counted = [
        ("F4.6", "VaultWithdraw", "vault pseudo source", "missing checkFrozen"),
        ("B3-1", "VaultDeposit", "vault pseudo destination", "missing checkFrozen"),
    ]

    print("JTX-confirmed regular-freeze lending sites:")
    for fid, tx, recv, wrong in jtx_confirmed:
        print(f"  {fid:6} {tx:28} receiver={recv:24} wrong={wrong}")

    print("\nNot counted as reproduced fund-movement findings in this kit:")
    for fid, tx, recv, wrong in not_counted:
        print(f"  {fid:6} {tx:28} receiver={recv:24} wrong={wrong}")


if __name__ == "__main__":
    main()
