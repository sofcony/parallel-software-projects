#Definitions
CC = gcc
FLAGS = -g -Wall -fopenmp

all: poly_mult mergesort sparse_array

SRC_2A := 2a_polynomial_multiplication/poly_mult.c
SRC_2B := 2b_sparse_array/sparse_array.c
SRC_2C := 2c_mergesort/mergesort.c

poly_mult: $(SRC_2A)
	$(CC) $(FLAGS) $^ -o $@

sparse_array: $(SRC_2B)
	$(CC) $(FLAGS) $^ -o $@

mergesort: $(SRC_2C)
	$(CC) $(FLAGS) $^ -o $@

clean:
	rm -f poly_mult mergesort sparse_array *.o
