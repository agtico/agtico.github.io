#!/usr/bin/env bash
# AGTI — XRPL rippled P0 repro runner (no mainnet wallet required)
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
cd "$ROOT"

echo "=== freeze_check_model.py ==="
python3 freeze_check_model.py

RIPPLED="${RIPPLED:-/home/pfrpc/repos/rippled/build/rippled}"
if [[ -x "$RIPPLED" ]]; then
  echo ""
  echo "=== rippled unittest (LoanBroker — existing suite) ==="
  "$RIPPLED" --unittest LoanBroker 2>&1 | tail -20
else
  echo ""
  echo "rippled not built at $RIPPLED — skipping jtx unittest."
  echo "Build rippled release-3.1.3 to run on-ledger repros."
fi
