#!/usr/bin/env bash
set -euo pipefail

# Build script for prefetching benchmark
# Usage: ./build.sh [clean]

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC_FILE="prefetching.cc"
OUT_NAME="prefetching.bin"

CXX=${CXX:-g++}
CXXFLAGS="-O2 -march=native -std=c++17 -Wall -Wextra -Wpedantic"

if [[ ${1:-} == "clean" ]]; then
  echo "Removing ${SCRIPT_DIR}/${OUT_NAME}"
  rm -f "${SCRIPT_DIR}/${OUT_NAME}"
  exit 0
fi

echo "Compiling ${SRC_FILE} -> ${OUT_NAME} using ${CXX}"
${CXX} ${CXXFLAGS} "${SCRIPT_DIR}/${SRC_FILE}" -o "${SCRIPT_DIR}/${OUT_NAME}" -lm

if [[ $? -eq 0 ]]; then
  echo "Build succeeded: ${SCRIPT_DIR}/${OUT_NAME}"
else
  echo "Build failed"
  exit 1
fi
