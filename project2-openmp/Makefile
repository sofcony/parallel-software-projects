#Definitions
CC = gcc
FLAGS = -g -Wall -fopenmp

all: poly_mult mergesort sparse_array

poly_mult: ./2a_polynomial_multiplication/poly_mult.c
	$(CC) $(FLAGS) ./2a_polynomial_multiplication/poly_mult.c -o $@

sparse_array: ./2b_sparse_array/sparse_array.c
	$(CC) $(FLAGS) ./2b_sparse_array/sparse_array.c -o $@

mergesort: ./2c_mergesort/mergesort.c
	$(CC) $(FLAGS) ./2c_mergesort/mergesort.c -o $@

clean:
	rm -f poly_mult mergesort sparse_array
