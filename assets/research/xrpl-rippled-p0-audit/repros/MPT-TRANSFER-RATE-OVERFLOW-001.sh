#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
exec "$ROOT/run_repro.sh" "MPT-TRANSFER-RATE-OVERFLOW-001"
