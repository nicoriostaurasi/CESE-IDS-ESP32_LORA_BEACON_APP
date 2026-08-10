#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="${BUILD_DIR:-build_beacon_node_0}"
idf.py -B "${BUILD_DIR}" \
  -D IDF_TARGET=esp32s3 \
  -D PROJECT_VER=ids-beacon-node-0 \
  -D CMAKE_C_FLAGS="-DBEACON_NODE_ID=0" \
  build
