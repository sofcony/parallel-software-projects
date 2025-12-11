# Makefile for Project 1 - Pthreads

# Compiler and flags
CC      := gcc
CFLAGS  ?= 
LDLIBS  := -lpthread -D_GNU_SOURCE

# Custom flags for specific programs
USE_PADDING ?= 0

# Binaries
BIN_1A := 1a
BIN_1B := 1b
BIN_1C := 1c
BIN_1C_PAD  := 1c_padded
BIN_1D := 1d
BIN_1E := 1e

# Sources
# SRC_1A := 1a_mult_polyn.c
# SRC_1B := 
SRC_1C := 1c_matrix.c
SRC_1D := 1d_bank.c
# SRC_1E := 

# Build all 
all: $(BIN_1C) $(BIN_1C_ORIG) $(BIN_1C_PAD) $(BIN_1D)

$(BIN_1A): $(SRC_1A)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

$(BIN_1C): $(SRC_1C)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

$(BIN_1C_PAD): $(SRC_1C)
	$(CC) $(CFLAGS) -DUSE_PADDING=1 $^ -o $@ $(LDFLAGS) $(LDLIBS)

$(BIN_1D): $(SRC_1D)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)


# Run by user (override ARGS="...")
run1a: $(BIN_1A)
	./$(BIN_1A) $(ARGS)

run1c: $(BIN_1C)
	./$(BIN_1C) $(ARGS)

run1c_pad: $(BIN_1C_PAD)
	./$(BIN_1C_PAD) $(ARGS)

run1d: $(BIN_1D)
	./$(BIN_1D) $(ARGS)


# ------ Examples for 1a (degree threads) -----
# 1) Small matrix (10,000 rows) with varying threads
test1a-small: $(BIN_1A)
	./$(BIN_1A) 10000 1 ; echo ; \
	./$(BIN_1A) 10000 2 ; echo ; \
	./$(BIN_1A) 10000 4 ; echo ; \
	./$(BIN_1A) 10000 8
# 2) Large matrix (20,000 rows) with varying threads
test1a-large: $(BIN_1A)
	./$(BIN_1A) 20000 1 ; echo ; \
	./$(BIN_1A) 20000 2 ; echo ; \
	./$(BIN_1A) 20000 4 ; echo ; \
	./$(BIN_1A) 20000 8


# ----- Examples for 1c (matrix stats) -----
# 1) Original structure without padding
test1c: $(BIN_1C)
	./$(BIN_1C) 1000 ; echo
	./$(BIN_1C) 10000 ; echo 
	./$(BIN_1C) 100000 ; echo 
	./$(BIN_1C) 1000000 ; echo 
	./$(BIN_1C) 10000000 ; echo 
	./$(BIN_1C) 100000000
# 2) Structure with padding to avoid false sharing
test1c-padded: $(BIN_1C_PAD)
	./$(BIN_1C_PAD) 1000 ; echo
	./$(BIN_1C_PAD) 10000 ; echo 
	./$(BIN_1C_PAD) 100000 ; echo 
	./$(BIN_1C_PAD) 1000000 ; echo 
	./$(BIN_1C_PAD) 10000000 ; echo 
	./$(BIN_1C_PAD) 100000000


# ----- Examples for 1d (bank with locks) -----
# 1) 80% queries, 4 threads, coarse mutex / coarse RWLock, with & without delay
test1d-80q-4t: $(BIN_1D)
	@echo "== 80% queries, 4 threads, coarse mutex / coarse RWLock, with & without delay: =="
	./$(BIN_1D) 1000 100000 80 1 4 0 ; echo
	./$(BIN_1D) 1000 100000 80 1 4 1 ; echo
	./$(BIN_1D) 1000 100000 80 3 4 0 ; echo
	./$(BIN_1D) 1000 100000 80 3 4 1 ; echo

# 2) 100% queries, 8 threads, all the locks, with delay
test1d-100q-8t: $(BIN_1D)
	@echo "== 100% queries, 8 threads, all the locks, with delay: =="
	./$(BIN_1D) 1000 100000 100 1 8 1 ; echo
	./$(BIN_1D) 1000 100000 100 3 8 1 ; echo
	./$(BIN_1D) 1000 100000 100 2 8 1 ; echo
	./$(BIN_1D) 1000 100000 100 4 8 1 ; echo

# 3) 20% queries, 8 threads, all the locks, with delay (write-heavy)
test1d-20q-8t: $(BIN_1D)
	@echo "== 20% queries, 8 threads, all the locks, with delay: =="
	./$(BIN_1D) 1000 1000or1 8 1 ; echo
	./$(BIN_1D) 1000 100000 20 3 8 1 ; echo
	./$(BIN_1D) 1000 100000 20 2 8 1 ; echo
	./$(BIN_1D) 1000 100000 20 4 8 1 ; echo

clean:
	rm -f $(BIN_1A) $(BIN_1C) $(BIN_1C_PAD) $(BIN_1D) *.o

.PHONY: all clean run1a run1c run1d test1a-small test1a-large test1c test1c-padded test1d-80q-4t test1d-100q-8t test1d-20q-8t