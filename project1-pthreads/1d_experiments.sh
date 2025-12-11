#!/usr/bin/env bash

# Executable name as built by the Makefile
PROG=./1d

# OUTFILE="csv/1d_bank_results.csv"
OUTFILE="1d_bank_results.csv"

# How many times to repeat each experiment 
# (can override: REPEATS=10 ./1d_experiments.sh)
REPEATS=${REPEATS:-5}

# Lists of values for scenarios
accounts_list=(100 1000 5000)
tx_list=(1000 10000 40000 70000)
query_list=(20 50 90)              # percentages
lock_types=(1 2 3 4)            # 1=coarse mutex, 2=fine mutex, 3=coarse rwlock, 4=fine rwlock
threads_list=(2 4)
use_delay_list=(0)            # 0 no delay, 1 with delay


# Write header in the CSV file
echo "num_accounts,transactions_per_thread,query_pct,lock_type,num_threads,use_delay,execution_time,throughput,run" > "$OUTFILE"


for na in "${accounts_list[@]}"; do
  for tx in "${tx_list[@]}"; do
    for q in "${query_list[@]}"; do
      for lt in "${lock_types[@]}"; do
        for th in "${threads_list[@]}"; do
          for delay in "${use_delay_list[@]}"; do
            for run_idx in $(seq 1 "$REPEATS"); do
              echo "Running: accounts=$na tx=$tx q=$q lock=$lt threads=$th delay=$delay (run $run_idx/$REPEATS)"

              # Run the program and capture all output in a variable
              output=$($PROG "$na" "$tx" "$q" "$lt" "$th" "$delay")

              # Capture the execution time and throughput using regex parsing
              exec_time=$(grep -Eo "> Execution time: [0-9]+\.[0-9]+" <<< "$output" | awk '{print $4}')
              throughput=$(grep -Eo "> Throughput: [0-9]+(\.[0-9]+)?" <<< "$output" | awk '{print $3}')

              # Write a line to the CSV
              echo "$na,$tx,$q,$lt,$th,$delay,$exec_time,$throughput,$run_idx" >> "$OUTFILE"
            done
          done
        done
      done
    done
  done
done

echo "Results saved in $OUTFILE"