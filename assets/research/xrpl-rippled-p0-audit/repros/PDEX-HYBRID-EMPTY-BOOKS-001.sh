#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
exec "$ROOT/run_repro.sh" "PDEX-HYBRID-EMPTY-BOOKS-001"
