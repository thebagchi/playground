#!/usr/bin/env bash

# Run prefetching benchmark with core pinning for consistent results
# Usage: ./run.sh [core_number]
#   core_number: CPU core to pin to (default: 0)

CORE=${1:-0}

if [[ ! -f prefetching.bin ]]; then
    echo "Error: prefetching.bin not found. Run build.sh first."
    exit 1
fi

echo "Running prefetching benchmark pinned to CPU core ${CORE}..."
echo "============================================================"
taskset -c ${CORE} ./prefetching.bin
