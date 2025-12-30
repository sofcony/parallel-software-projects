#!/usr/bin/bash

#Create results folder if it doesn't exist.
mkdir -p results

ORDERS=(1000 10000)
THREAD_COUNTS=(1 2 4)
REPS=5

# Run experiments for each parameter setup
for THREAD_COUNT in "${THREAD_COUNTS[@]}"; do
	for ORDER in "${ORDERS[@]}"; do
		for (( i=0; i<REPS; i++)); do
			./poly_mult "$ORDER" "$THREAD_COUNT"
			# Move csv file to results folder.			
		done
		FILE="poly_mult_${THREAD_COUNT}thr_${ORDER}order.csv"
		mv "$FILE" "./results/$FILE"
	done
done