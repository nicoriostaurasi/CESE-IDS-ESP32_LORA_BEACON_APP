#!/usr/bin/env bash
set -euo pipefail

PORT="${1:-${ESPPORT:-/dev/ttyUSB0}}"
BAUD="${ESPBAUD:-115200}"

idf.py -p "${PORT}" -b "${BAUD}" flash
