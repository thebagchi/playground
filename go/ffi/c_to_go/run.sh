#!/usr/bin/env bash

# Script to run main executable and check thread count
# Usage: ./run.sh

echo "Starting main executable..."
echo "PID: $$"

# Run the main executable in background
./main &
MAIN_PID=$!

echo "Main process started with PID: $MAIN_PID"

# Wait a moment for the process to start
sleep 2

# Check thread count using ps
echo "Thread information:"
ps -p $MAIN_PID -L

echo ""
echo "Thread count: $(ps -p $MAIN_PID -L | wc -l)"

# Wait for the process to finish
echo "Waiting for process to complete..."
wait $MAIN_PID

echo "Script completed."