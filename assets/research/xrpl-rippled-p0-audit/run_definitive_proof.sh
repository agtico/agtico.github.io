#!/usr/bin/env bash
# Definitive proof runner — AGTI XRPL open P0 audit
# No mainnet wallet required. Uses rippled jtx when built.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
RIPPLED_ROOT="${RIPPLED_ROOT:-/home/postfiat/repos/rippled}"
AGTI_REPRO="${AGTI_REPRO:-$SCRIPT_DIR}"
BUILD_DIR="${BUILD_DIR:-$RIPPLED_ROOT/.build}"

echo "=== 1/3 Logic model (no build) ==="
python3 "$AGTI_REPRO/freeze_check_model.py"

RIPPLED="$BUILD_DIR/xrpld"
if [[ ! -x "$RIPPLED" ]]; then
  RIPPLED="$BUILD_DIR/rippled"
fi
if [[ ! -x "$RIPPLED" && -x "$RIPPLED_ROOT/build/rippled" ]]; then
  RIPPLED="$RIPPLED_ROOT/build/rippled"
fi

if [[ ! -x "$RIPPLED" ]]; then
  echo ""
  echo "=== jtx repro — SKIPPED (build $RIPPLED) ==="
  echo "Build first:"
  echo "  cd $RIPPLED_ROOT && mkdir -p .build && cd .build"
  echo "  conan install .. --output-folder . --build missing --settings build_type=Release"
  echo "  cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=build/generators/conan_toolchain.cmake -Dxrpld=ON -Dtests=ON .."
  echo "  cmake --build . -j \$(nproc)"
  echo "  $RIPPLED --unittest OpenP0Repro"
  exit 0
fi

echo ""
echo "=== 2/3 jtx repro suite OpenP0Repro (fund movement + control) ==="
"$RIPPLED" --unittest OpenP0Repro

echo ""
echo "=== 3/3 F6.1 negative control (must return tecNO_DST, no crash) ==="
"$RIPPLED" --unittest OpenP0ReproCrash

echo ""
echo "=== Done ==="
