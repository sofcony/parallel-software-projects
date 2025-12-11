#!/usr/bin/env bash

# Executables built by the Makefile
# - ./1c   -> compiled with -DUSE_PADDING=0
# - ./1c_padded  -> compiled with -DUSE_PADDING=1
PROG_ORIG=./1c
PROG_PAD=./1c_padded

# OUTFILE="/csv/1c_results.csv"
# OUTFILE_PAD="/csv/1c_results_padded.csv"
OUTFILE="1c_results.csv"
OUTFILE_PAD="1c_results_padded.csv"

# How many times to repeat each experiment 
# (can override: REPEATS=10 ./1c_experiments.sh)
REPEATS=${REPEATS:-5}

# Lists of values for scenarios
num_elements=(50 100 500 1000 10000 100000 1000000 10000000)

set -euo pipefail

# Write headers (aligned to 1c program output)
# Columns: array_size, use_padding(0/1), creation_time_s, serial_time_s, parallel_time_s, run
echo "array_size,use_padding,creation_time_s,serial_time_s,parallel_time_s,run" > "$OUTFILE"
echo "array_size,use_padding,creation_time_s,serial_time_s,parallel_time_s,run" > "$OUTFILE_PAD"

# Function 
parse_and_append() {
  local prog="$1" ; shift
  local use_padding="$1" ; shift
  local outfile="$1" ; shift
  local size="$1" ; shift
  local run_idx="$1" ; shift

  echo "Running: array_size=$size use_padding=$use_padding (run $run_idx/$REPEATS) — binary: $prog"
  local output
  output=$($prog "$size")

  # Extract times from program output
  local creation_time serial_time parallel_time
  creation_time=$(grep -Eo "> Array creation time: [0-9]+\.[0-9]+" <<< "$output" | awk '{print $5}')
  serial_time=$(grep -Eo "> Serial execution time: [0-9]+\.[0-9]+" <<< "$output" | awk '{print $5}')
  parallel_time=$(grep -Eo "> Parallel execution time: [0-9]+\.[0-9]+" <<< "$output" | awk '{print $5}')
  
  # 
  creation_fmt=$(awk -v v="$creation_time" 'BEGIN{printf "%.8f", v+0}')
  serial_fmt=$(awk -v v="$serial_time" 'BEGIN{printf "%.8f", v+0}')
  parallel_fmt=$(awk -v v="$parallel_time" 'BEGIN{printf "%.8f", v+0}')

  echo "$size,$use_padding,$creation_fmt,$serial_fmt,$parallel_fmt,$run_idx" >> "$outfile"
}

# Original (no padding)
for na in "${num_elements[@]}"; do
  for run_idx in $(seq 1 "$REPEATS"); do
    parse_and_append "$PROG_ORIG" 0 "$OUTFILE" "$na" "$run_idx"
  done
done

echo "Results saved in $OUTFILE (use_padding=0)"

# Padded
for na in "${num_elements[@]}"; do
  for run_idx in $(seq 1 "$REPEATS"); do
    parse_and_append "$PROG_PAD" 1 "$OUTFILE_PAD" "$na" "$run_idx"
  done
done

echo "Results saved in $OUTFILE_PAD (use_padding=1)"