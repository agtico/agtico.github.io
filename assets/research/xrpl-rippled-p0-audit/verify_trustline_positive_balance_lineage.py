#!/usr/bin/env python3
"""Verify the source-lineage evidence for TRUSTLINE-POSITIVE-BALANCE-RESERVE-001.

This is not an old-binary transaction repro. It checks the narrower claim that
the sender-side-only reserve transition logic exists across sampled historical
release tags, while the receiver-side fix exists only on the observed fix
branch and is not contained in the latest checked release/develop refs.
"""

from __future__ import annotations

import json
import os
import subprocess
from datetime import datetime, timezone
from pathlib import Path


ROOT = Path(__file__).resolve().parent
RIPPLED_ROOT = Path(os.environ.get("RIPPLED_ROOT", "/home/postfiat/repos/rippled"))
OUT = (
    ROOT
    / "runs/20260527-p0-hunt/trustline_positive_balance_lineage_20260527.json"
)

SAMPLED_REFS = [
    "0.12.0",
    "0.20.0",
    "0.30.0",
    "0.50.0",
    "0.80.0",
    "1.0.0",
    "1.5.0",
    "2.0.0",
    "2.5.0",
    "3.1.3",
]

FIX_COMMIT = "b4a45f1f0f49d3caf56d2c790960380b5e648a60"
FIX_BRANCH = "origin/vvysokikh1/fix-positive-balance-trustline-pay-no-reserve"
LATEST_REFS = ["3.2.0-b7", "origin/develop"]


def run_git(args: list[str], check: bool = True) -> subprocess.CompletedProcess[str]:
    result = subprocess.run(
        ["git", *args],
        cwd=RIPPLED_ROOT,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )
    if check and result.returncode != 0:
        raise RuntimeError(
            f"git {' '.join(args)} failed with {result.returncode}\n"
            f"stdout={result.stdout}\nstderr={result.stderr}"
        )
    return result


def git_show(ref: str) -> str:
    # Dereference annotated tags so the report binds to the release commit
    # instead of embedding the tag object's PGP signature block.
    return run_git(
        ["show", "-s", "--format=%H %ad %an %s", "--date=short", f"{ref}^{{commit}}"]
    ).stdout.strip()


def grep_first(ref: str, pattern: str) -> str | None:
    result = run_git(["grep", "-n", "--fixed-strings", pattern, ref], check=False)
    if result.returncode == 0:
        return result.stdout.splitlines()[0]
    if result.returncode == 1:
        return None
    raise RuntimeError(
        f"git grep failed for {ref} pattern={pattern!r}\n"
        f"stdout={result.stdout}\nstderr={result.stderr}"
    )


def is_ancestor(commit: str, ref: str) -> bool:
    return run_git(["merge-base", "--is-ancestor", commit, ref], check=False).returncode == 0


def main() -> int:
    sampled = []
    for ref in SAMPLED_REFS:
        sender_branch = grep_first(ref, "Sender balance was positive.")
        receiver_clear = grep_first(ref, "Receiver reserve is clear.")
        receiver_fix = grep_first(ref, "fixTrustLineOwnerCount")
        sampled.append(
            {
                "ref": ref,
                "commit": git_show(ref),
                "sender_side_reserve_transition": sender_branch,
                "receiver_reserve_clear_comment": receiver_clear,
                "receiver_side_fix_symbol": receiver_fix,
                "passes_lineage_shape": bool(sender_branch and receiver_clear and receiver_fix is None),
            }
        )

    fix_branch = {
        "ref": FIX_BRANCH,
        "commit_contains_fix": is_ancestor(FIX_COMMIT, FIX_BRANCH),
        "receiver_transition_comment": grep_first(
            FIX_BRANCH, "receiver's balance went from zero/negative to positive"
        ),
        "fix_symbol": grep_first(FIX_BRANCH, "fixTrustLineOwnerCount"),
        "receiver_reserve_not_set_comment": grep_first(FIX_BRANCH, "Receiver reserve is not set."),
    }

    latest_refs = {
        ref: {"contains_fix_commit": is_ancestor(FIX_COMMIT, ref), "commit": git_show(ref)}
        for ref in LATEST_REFS
    }

    passed = (
        all(item["passes_lineage_shape"] for item in sampled)
        and fix_branch["commit_contains_fix"]
        and bool(fix_branch["receiver_transition_comment"])
        and bool(fix_branch["fix_symbol"])
        and bool(fix_branch["receiver_reserve_not_set_comment"])
        and all(not item["contains_fix_commit"] for item in latest_refs.values())
    )

    output = {
        "source": "local XRPLF/rippled git source-lineage check",
        "checked_utc": datetime.now(timezone.utc).replace(microsecond=0).isoformat().replace("+00:00", "Z"),
        "finding_id": "TRUSTLINE-POSITIVE-BALANCE-RESERVE-001",
        "rippled_root": str(RIPPLED_ROOT),
        "scope_note": (
            "This proves source-lineage shape only. It does not replace the "
            "current 3.1.3 binary repro and does not prove an old binary tag."
        ),
        "sampled_refs": sampled,
        "fix_commit": FIX_COMMIT,
        "fix_branch": fix_branch,
        "latest_refs": latest_refs,
        "passed": passed,
    }

    OUT.write_text(json.dumps(output, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    print(f"wrote {OUT}")
    print(f"passed={passed}")
    return 0 if passed else 1


if __name__ == "__main__":
    raise SystemExit(main())
