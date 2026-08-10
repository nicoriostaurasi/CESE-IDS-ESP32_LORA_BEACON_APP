#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="${BUILD_DIR:-build_beacon_node_0}"
ESPPORT="${ESPPORT:-/dev/ttyUSB0}"
BUILD_DIR="${BUILD_DIR}" bash scripts/idf_build_beacon_node_0.sh
idf.py -B "${BUILD_DIR}" -p "${ESPPORT}" flash
