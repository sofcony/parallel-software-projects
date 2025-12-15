# Makefile for Project 2 - OpenMP

# Compiler and flags
CC      := gcc
CFLAGS  ?= -g -Wall -fopenmp
LDLIBS  := 

# Custom flags for specific programs


# Binaries
BIN_1A := 1a
BIN_1B := 1b
BIN_1C := 1c

# Sources
SRC_1A := 1a.c
SRC_1B := 1b.c
SRC_1C := 1c.c

# Build all 
# all: $(BIN_1A) $(BIN_1B) $(BIN_1C)
all: $(BIN_1B) $(BIN_1C)

# $(BIN_1A): $(SRC_1A)
# 	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

$(BIN_1B): $(SRC_1B)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

$(BIN_1C): $(SRC_1C)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)


# Run by user (override ARGS="...")
# run1a: $(BIN_1A)
# 	./$(BIN_1A) $(ARGS)

run1b: $(BIN_1B)
	./$(BIN_1B) $(ARGS)

run1c: $(BIN_1C)
	./$(BIN_1C) $(ARGS)


# Examples tests 
# ------ Examples for 1a -----

# ------ Examples for 1b -----

# ------ Examples for 1c -----

clean:
	rm -f $(BIN_1A) $(BIN_1B) $(BIN_1C) *.o

.PHONY: all clean run1a run1b run1c