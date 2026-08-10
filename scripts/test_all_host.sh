#!/usr/bin/env bash
set -euo pipefail

bash scripts/test_beacon_protocol_host.sh
bash scripts/test_beacon_metrics_host.sh
bash scripts/test_sx1262_api_host.sh
bash scripts/test_sx1262_params_host.sh
bash scripts/test_sx1262_transport_host.sh
