#!/usr/bin/env bash
# AGTI — XRPL rippled P0 repro runner (no mainnet wallet required)
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
FINDING_ID="${1:-}"
cd "$ROOT"

markers=()
if [[ -n "$FINDING_ID" ]]; then
  mapfile -t markers < <(python3 - "$ROOT/repro_manifest.json" "$FINDING_ID" <<'PY'
import json
import sys
manifest = json.load(open(sys.argv[1], encoding="utf-8"))
finding_id = sys.argv[2]
for record in manifest["records"]:
    if record["id"] == finding_id:
        for marker in record["markers"]:
            print(marker)
        break
else:
    raise SystemExit(f"unknown finding id: {finding_id}")
PY
  )
  echo "=== targeted finding: $FINDING_ID ==="
  printf 'expected marker: %s\n' "${markers[@]}"
fi

RIPPLED_ROOT="${RIPPLED_ROOT:-/home/postfiat/repos/rippled}"
RIPPLED="${RIPPLED:-$RIPPLED_ROOT/.build/rippled}"
if [[ ! -x "$RIPPLED" && -x "$RIPPLED_ROOT/.build/xrpld" ]]; then
  RIPPLED="$RIPPLED_ROOT/.build/xrpld"
elif [[ ! -x "$RIPPLED" && -x "$RIPPLED_ROOT/build/rippled" ]]; then
  RIPPLED="$RIPPLED_ROOT/build/rippled"
fi
if [[ -x "$RIPPLED" ]]; then
  echo ""
  echo "=== rippled unittest (OpenP0Repro — local repro suite, live-filtered output) ==="
  tmp_log="$(mktemp)"
  trap 'rm -f "$tmp_log"' EXIT
  "$RIPPLED" --unittest OpenP0Repro >"$tmp_log" 2>&1
  grep -F "ripple.tx.OpenP0Repro had 0 failures." "$tmp_log" >/dev/null
  grep -F "67 cases, 16563 tests total, 0 failures" "$tmp_log" >/dev/null
  if [[ ${#markers[@]} -gt 0 ]]; then
    for marker in "${markers[@]}"; do
      grep -F "$marker" "$tmp_log" >/dev/null
      grep -F "$marker" "$tmp_log"
    done
    echo "targeted finding $FINDING_ID reproduced by marker assertion."
  fi
  grep -F "ripple.tx.OpenP0Repro had 0 failures." "$tmp_log"
  grep -F "67 cases, 16563 tests total, 0 failures" "$tmp_log"
else
  echo ""
  echo "rippled not built at $RIPPLED — skipping jtx unittest."
  echo "Build rippled release-3.1.3 to run on-ledger repros."
  if [[ -n "$FINDING_ID" ]]; then
    exit 1
  fi
fi
