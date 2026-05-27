#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
exec "$ROOT/run_repro.sh" "TOKENESCROW-DISALLOW-INCOMING-FINISH-001"
