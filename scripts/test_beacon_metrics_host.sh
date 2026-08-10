#!/usr/bin/env bash
set -euo pipefail

mkdir -p build_host
cc -std=c11 -Wall -Wextra -Werror \
  -Icomponents/beacon_metrics/include \
  components/beacon_metrics/beacon_metrics.c \
  test/host/test_beacon_metrics/test_beacon_metrics.c \
  -o build_host/test_beacon_metrics
build_host/test_beacon_metrics
