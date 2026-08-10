#!/usr/bin/env bash
set -euo pipefail

LOG_FILE="${1:-}"
if [[ -z "${LOG_FILE}" || ! -f "${LOG_FILE}" ]]; then
  echo "Uso: bash scripts/check_acceptance_log.sh <monitor.log>" >&2
  exit 2
fi

python3 scripts/check_acceptance_log.py "${LOG_FILE}"
