# Makefile for Project 2 - OpenMP

# Compiler and flags
CC      := gcc
CFLAGS  ?= -g -Wall -fopenmp
LDLIBS  := 

# Custom flags for specific programs


# Binaries
BIN_2A := 2a
BIN_2B := 2b
BIN_2C := 2c

# Sources
SRC_2A := 2a_polynomial_multiplication/poly_mult.c
SRC_2B := 2b_sparse_array/sparse_array.c
SRC_2C := 2c_mergesort/mergesort.c

# Build all 
all: $(BIN_2A) $(BIN_2B) $(BIN_2C)

$(BIN_2A): $(SRC_2A)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

$(BIN_2B): $(SRC_2B)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

$(BIN_2C): $(SRC_2C)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)


# Run by user (override ARGS="...")
# run1a: $(BIN_2A)
# 	./$(BIN_2A) $(ARGS)

run1b: $(BIN_2B)
	./$(BIN_2B) $(ARGS)

run1c: $(BIN_2C)
	./$(BIN_2C) $(ARGS)


# Examples tests 
# ------ Examples for 2a -----

# ------ Examples for 2b -----

# ------ Examples for 2c -----

clean:
	rm -f $(BIN_2A) $(BIN_2B) $(BIN_2C) *.o

.PHONY: all clean run2a run2b run2c