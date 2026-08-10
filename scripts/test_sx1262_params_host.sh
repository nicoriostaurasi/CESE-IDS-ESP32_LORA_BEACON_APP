#!/usr/bin/env bash
set -euo pipefail

(
  cd components/sx1262_radio
  tr -d '\r' < scripts/test_sx1262_params_host.sh | bash
)
