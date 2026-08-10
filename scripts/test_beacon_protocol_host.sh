#!/usr/bin/env bash
set -euo pipefail

mkdir -p build_host
cc -std=c11 -Wall -Wextra -Werror \
  -Icomponents/beacon_protocol/include \
  components/beacon_protocol/beacon_protocol.c \
  test/host/test_beacon_protocol/test_beacon_protocol.c \
  -o build_host/test_beacon_protocol
build_host/test_beacon_protocol
