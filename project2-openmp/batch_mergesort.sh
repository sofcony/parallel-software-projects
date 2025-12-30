#!/usr/bin/bash

# Create results folder if it doesn't exist.
mkdir -p results

# Defaults
SIZES=(1000 100000 1000000)
MODES=(parallel serial)
THREAD_COUNTS=(1 2 4)
REPS=5

# Run experiments for each parameter setup
for THREAD_COUNT in "${THREAD_COUNTS[@]}"; do
    for MODE in "${MODES[@]}"; do
        for SIZE in "${SIZES[@]}"; do
            for (( i=0; i<REPS; i++)); do
                ./mergesort "$SIZE" "$MODE" "$THREAD_COUNT"
            done
            FILE="mergesort_${MODE}_${THREAD_COUNT}thr_${SIZE}elem.csv"
            mv "$FILE" "./results/$FILE"
        done
    done
done