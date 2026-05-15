#!/bin/bash
set -Eeuo pipefail

# Find all executable files starting with 'test_'
tests=$(find . -maxdepth 1 -type f -executable -name 'test_*')

for test_exe in $tests; do
  echo "Running test: $test_exe"

  # Stop any existing server instances
  pkill fs || true

  # Create a new filesystem
  ./createfs

  # Start the server in the background
  ./fs 8000 &
  server_pid=$!

  # Give the server time to initialize
  sleep 1

  # Run the test executable
  $test_exe localhost 8000

  # Stop the server
  kill $server_pid

  # Wait for the server process to terminate
  wait $server_pid 2>/dev/null || true

  echo "Finished test: $test_exe"
  echo "--------------------------------------"
done