#!/usr/bin/bash

# Create results folder if it doesn't exist.
mkdir -p results

# Defaults
ROW_VALUES=(80 100 1000 2000)
ZERO_PCTS=(20 60 90)
MULTS=(5 10 30)
THREAD_COUNTS=(1 2 4)
REPS=5

# Run experiments for each parameter setup
for THREAD_COUNT in "${THREAD_COUNTS[@]}"; do
    for ROWS in "${ROW_VALUES[@]}"; do
        for ZERO in "${ZERO_PCTS[@]}"; do
            for MULT in "${MULTS[@]}"; do
                for (( i=0; i<REPS; i++)); do
                    ./sparse_array "$ROWS" "$ZERO" "$MULT" "$THREAD_COUNT"
                done
                FILE="sparse_array_${ROWS}rows_${ZERO}per_${MULT}iter_${THREAD_COUNT}thr.csv"
                mv "$FILE" "./results/$FILE"
            done
        done
    done
done