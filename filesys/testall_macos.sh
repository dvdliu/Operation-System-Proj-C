#!/bin/bash
set -Eeuo pipefail

# Find all executable files starting with 'test_'
tests=$(find . -maxdepth 1 -type f -perm +111 -name 'test_*')

for test_exe in $tests; do
  echo "Running test: $test_exe"

  # Extract the base name of the test executable
  test_name=$(basename "$test_exe")

  # Check if a corresponding .fs file exists
  fs_file="${test_name}.fs"

  # Stop any existing server instances
  pgrep fs && pkill fs || true

  # Create a new filesystem
  # Pass .fs file as argument if it exists, otherwise create empty
  if [[ -f "$fs_file" ]]; then
    ./createfs_macos "$fs_file"
  else
    ./createfs_macos
  fi

  # Start the server in the background
  ./fs 8000 &
  server_pid=$!

  # Give the server time to initialize
  sleep 1

  # Run the test executable
  "$test_exe" localhost 8000

  # Stop the server
  kill "$server_pid"

  # Wait for the server process to terminate
  wait "$server_pid" 2>/dev/null || true

  echo "Finished test: $test_exe"
  echo "--------------------------------------"
done