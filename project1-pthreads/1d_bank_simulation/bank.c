#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#define SEED 4
#define DEBUG 0
#define CRITICAL_SECTION_DELAY 100000
// #define USE_DELAY 0

// Global data
int *accounts;
int num_accounts;
int num_threads;
int transactions_per_thread;
float query_percentage;

int lock_type;                          // 1=coarse mutex, 2=fine mutex, 3=coarse rwlock, 4=fine rwlock
int use_delay;                          // 0=no delay, 1=add delay to balance queries

// Locks for different schemes
pthread_mutex_t coarse_mutex;           // Single lock for all accounts
pthread_mutex_t *fine_mutexes;          // One lock per account
pthread_rwlock_t coarse_rwlock;         // Single rwlock for all accounts
pthread_rwlock_t *fine_rwlocks;         // One rwlock per account


// --- Function declarations ---
void* threads_transactions(void* arg);
// Lock management
void init_locks();
void destroy_locks();
const char* get_lock_name();
// Coarse-grained (single mutex) API
void transfer_coarse_mutex(int from, int to, int amount);
int  query_coarse_mutex(int account);
// Fine-grained (per-account mutex) API
void transfer_fine_mutex(int from, int to, int amount);
int query_fine_mutex(int account);
// Coarse-grained (single rwlock) API
void transfer_coarse_rwlock(int from, int to, int amount);
int query_coarse_rwlock(int account);
// Fine-grained (per-account rwlock) API
void transfer_fine_rwlock(int from, int to, int amount);
int query_fine_rwlock(int account);


int main (int argc, char *argv[]) {

    // if (argc != 6) {
    //     printf("Usage: %s <num_accounts> <transactions_per_thread> <query_percentage> <lock_type> <num_threads>\n", argv[0]);
    //     printf("  lock_type: 1=coarse mutex, 2=fine mutex, 3=coarse rwlock, 4=fine rwlock\n");
    //     printf("Example: %s 100 1000 20 1 4\n", argv[0]);
    //     return 1;
    // }


    // // Parse arguments
    // num_accounts = atoi(argv[1]);
    // transactions_per_thread = atoi(argv[2]);
    // query_percentage = atof(argv[3]) / 100.0;
    // lock_type = atoi(argv[4]);
    // num_threads = atoi(argv[5]);
    // use_delay = USE_DELAY;

    if (argc != 6 && argc != 7) {
        printf("Usage: %s <num_accounts> <transactions_per_thread> <query_percentage> <lock_type> <num_threads> [use_delay]\n", argv[0]);
        printf("  lock_type: 1=coarse mutex, 2=fine mutex, 3=coarse rwlock, 4=fine rwlock\n");
        printf("  use_delay: 0=no delay (default), 1=add delay to queries\n");
        printf("Example: %s 100 1000 20 1 4\n", argv[0]);
        return 1;
    }


    // Parse arguments
    num_accounts = atoi(argv[1]);
    transactions_per_thread = atoi(argv[2]);
    query_percentage = atof(argv[3]) / 100.0;
    lock_type = atoi(argv[4]);
    num_threads = atoi(argv[5]);
    use_delay = (argc == 7) ? atoi(argv[6]) : 0; // If 7th arg provided, use it; else default to 0

    // Validation of input num_accounts, transactions_per_thread, num_threads, lock_type, use_delay, query_percentage
    if (num_accounts <= 0 || transactions_per_thread <= 0 || num_threads <= 0) {
        fprintf(stderr, "num_accounts, transactions_per_thread, and num_threads must be positive integers.\n");
        return 1;
    }
    if (lock_type < 1 || lock_type > 4) {
        fprintf(stderr, "Invalid lock_type. Must be 1, 2, 3, or 4.\n");
        return 1;
    }
    if (use_delay < 0 || use_delay > 1) {
        fprintf(stderr, "Invalid use_delay. Must be 0 or 1.\n");
        return 1;
    }
    if (query_percentage < 0.0 || query_percentage > 1.0) {
        fprintf(stderr, "Invalid query_percentage. Must be between 0 and 100.\n");
        return 1;
    }

    // Print configuration
    printf("\n---- Bank Simulation ----\n");
    printf("Accounts: %d\n", num_accounts);
    printf("Transactions per thread: %d\n", transactions_per_thread);
    printf("Query percentage: %.1f %%\n", query_percentage * 100);
    printf("Lock type: %s\n", get_lock_name());
    printf("Threads: %d\n", num_threads);
    printf("Use delay: %s\n", use_delay ? "Yes" : "No");

    srand(time(NULL));
    // srand(SEED); // For reproducibility   

    // STEP 1: Allocate and initialize bank accounts array ----
    accounts = (int *)malloc(num_accounts * sizeof(int));
    for (int i = 0; i < num_accounts; ++i) {
        accounts[i] = rand() % 10000; // [0, 9999]
    }

    // STEP 2: Initialize locks ----
    init_locks();

    // STEP 3: Precompute initial total amount from all accounts ------
    long initial_total = 0;
    for (int i = 0; i < num_accounts; ++i) initial_total += accounts[i];

    // STEP 4: Start timing and create threads ----
    struct timeval start, end;
    gettimeofday(&start, NULL);

    pthread_t* threads = malloc(num_threads * sizeof(pthread_t));
    long thread;

    for (thread = 0; thread < num_threads; thread++) {
        pthread_create(&threads[thread], NULL, threads_transactions, (void*) thread);
    }
    // Wait for all threads to complete
    for (thread = 0; thread < num_threads; thread++) {
        pthread_join(threads[thread], NULL);
    }

    gettimeofday(&end, NULL);


    // STEP 5: Compute final total amount and timing ----
    long final_total = 0;
    for (int i = 0; i < num_accounts; ++i) final_total += accounts[i];


    // STEP 6: Print results ----
    printf("--- Results ---\n");
    printf("> Initial total: %ld\n", initial_total);
    printf("> Final total: %ld\n", final_total);

    if (initial_total == final_total) {
        printf("> Status: SUCCESS - Total money preserved!\n");
    } else {
        printf("> Status: ERROR - Money difference: %ld\n", final_total - initial_total);
    }

    double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
    printf("> Execution time: %.6f seconds\n", time_taken);
    printf("> Throughput: %.2f transactions/second\n", (num_threads * transactions_per_thread) / time_taken);
    printf("\n");

    // ---- Cleanup - Free memory
    destroy_locks();
    free(accounts);
    free(threads);

    return 0;
}



// PARALLEL IMPLEMENTATION
// --- Thread function to perform transactions (transfers and queries)
void* threads_transactions(void* arg) {
    
    long thread_id = (long)arg;
    unsigned int seed = time(NULL) ^ thread_id;
    
    for (int t = 0; t < transactions_per_thread; t++) {

        float r = (float)rand_r(&seed) / RAND_MAX; // Random float in [0.0, 1.0)
        if (r < query_percentage) { // Perform query            
            int acc;
            int balance;

            acc = rand_r(&seed) % num_accounts;

            switch (lock_type) {
                case 1: balance = query_coarse_mutex(acc); break;
                case 2: balance = query_fine_mutex(acc); break;
                case 3: balance = query_coarse_rwlock(acc); break;
                case 4: balance = query_fine_rwlock(acc); break;
                default: balance = query_coarse_mutex(acc); break;
            }

            if (DEBUG) {
                printf("Thread %ld: Queried account %d, balance = %d\n", thread_id, acc, balance);
            }
        } else { // Perform transfer

            int from;
            int to;
            int amount;

            from = rand_r(&seed) % num_accounts;
            do { to = rand_r(&seed) % num_accounts; } while (to == from); // Ensure different accounts
            amount = rand_r(&seed) % 1000; // Random amount to transfer [0, 999]

            // Choose the lock scheme
            switch (lock_type) {
                case 1: transfer_coarse_mutex(from, to, amount); break;
                case 2: transfer_fine_mutex(from, to, amount); break;
                case 3: transfer_coarse_rwlock(from, to, amount); break;
                case 4: transfer_fine_rwlock(from, to, amount); break;
                default: transfer_coarse_mutex(from, to, amount); break;
            }
            
            if (DEBUG) {
                printf("Thread %ld: Transferred %d from account %d to account %d\n", thread_id, amount, from, to);
            }
        }
    }

    return NULL;
}


// -------- Coarse Grained Mutex API --------
void transfer_coarse_mutex(int from, int to, int amount) {
    if (amount <= 0 || from == to) return; // Invalid transfer
    if (from == to) return; // No self-transfer
    if (from < 0 || to < 0 || from >= num_accounts || to >= num_accounts) return; // Invalid accounts

    pthread_mutex_lock(&coarse_mutex);
    if (accounts[from] >= amount) {
        accounts[from] -= amount;
        accounts[to] += amount;
    }
    pthread_mutex_unlock(&coarse_mutex);
}
int query_coarse_mutex(int account) {    
    if (account < 0 || account >= num_accounts) return 0; // Invalid account
    
    int balance;
    pthread_mutex_lock(&coarse_mutex);
    balance = accounts[account];
    if (use_delay) { // Simulate delay inside critical section            
        for (volatile int i = 0; i < CRITICAL_SECTION_DELAY; i++);
    }
    pthread_mutex_unlock(&coarse_mutex);

    return balance;
}

// -------- Fine Grained Mutex API --------
void transfer_fine_mutex(int from, int to, int amount) {
    if (amount <= 0) return; // Invalid transfer
    if (from == to) return; // No self-transfer
    if (from < 0 || to < 0 || from >= num_accounts || to >= num_accounts) return; // Invalid accounts

    // To avoid deadlock, always lock in order of account number
    int first = (from < to) ? from : to;
    int second = (from < to) ? to : from;

    pthread_mutex_lock(&fine_mutexes[first]);
    pthread_mutex_lock(&fine_mutexes[second]);
    if (accounts[from] >= amount) {
        accounts[from] -= amount;
        accounts[to] += amount;
    }
    pthread_mutex_unlock(&fine_mutexes[second]);
    pthread_mutex_unlock(&fine_mutexes[first]);
}
int query_fine_mutex(int account) {
    if (account < 0 || account >= num_accounts) return 0; // Invalid account

    int balance;
    pthread_mutex_lock(&fine_mutexes[account]);
    balance = accounts[account];
    if (use_delay) { // Simulate delay inside critical section            
        for (volatile int i = 0; i < CRITICAL_SECTION_DELAY; i++);
    }
    pthread_mutex_unlock(&fine_mutexes[account]);

    return balance;
}

// -------- Coarse Grained RW Lock API --------
void transfer_coarse_rwlock(int from, int to, int amount) {
    if (amount <= 0) return; // Invalid transfer
    if (from == to) return; // No self-transfer
    if (from < 0 || to < 0 || from >= num_accounts || to >= num_accounts) return; // Invalid accounts

    pthread_rwlock_wrlock(&coarse_rwlock);
    if (accounts[from] >= amount) {
        accounts[from] -= amount;
        accounts[to] += amount;
    }
    pthread_rwlock_unlock(&coarse_rwlock);
}
int query_coarse_rwlock(int account) {
    if (account < 0 || account >= num_accounts) return 0; // Invalid account
    
    int balance;
    pthread_rwlock_rdlock(&coarse_rwlock);
    balance = accounts[account];
    if (use_delay) { // Simulate delay inside critical section            
        for (volatile int i = 0; i < CRITICAL_SECTION_DELAY; i++);
    }
    pthread_rwlock_unlock(&coarse_rwlock);

    return balance;
}

// -------- Fine Grained RW Lock API --------
void transfer_fine_rwlock(int from, int to, int amount) {
    if (amount <= 0) return; // Invalid transfer
    if (from == to) return; // No self-transfer
    if (from < 0 || to < 0 || from >= num_accounts || to >= num_accounts) return; // Invalid accounts

    // To avoid deadlock, always lock in order of account number
    int first = (from < to) ? from : to;
    int second = (from < to) ? to : from;

    pthread_rwlock_wrlock(&fine_rwlocks[first]);
    pthread_rwlock_wrlock(&fine_rwlocks[second]);
    if (accounts[from] >= amount) {
        accounts[from] -= amount;
        accounts[to] += amount;
    }
    pthread_rwlock_unlock(&fine_rwlocks[second]);
    pthread_rwlock_unlock(&fine_rwlocks[first]);
}
int query_fine_rwlock(int account) {
    if (account < 0 || account >= num_accounts) return 0; // Invalid account

    int balance;
    pthread_rwlock_rdlock(&fine_rwlocks[account]);
    balance = accounts[account];
    if (use_delay) { // Simulate delay inside critical section            
        for (volatile int i = 0; i < CRITICAL_SECTION_DELAY; i++);
    }
    pthread_rwlock_unlock(&fine_rwlocks[account]);

    return balance;
}


// -------- Lock Management --------
// --- Initialize locks based on lock_type ---
void init_locks() {
    switch(lock_type) {
        case 1:
            // Coarse-grained mutex
            pthread_mutex_init(&coarse_mutex, NULL);
            break;
        case 2:
            // Fine-grained mutex
            fine_mutexes = (pthread_mutex_t*)malloc(num_accounts * sizeof(pthread_mutex_t));
            for (int i = 0; i < num_accounts; i++) {
                pthread_mutex_init(&fine_mutexes[i], NULL);
            }
            break;
        case 3:
            // Coarse-grained rwlock
            pthread_rwlock_init(&coarse_rwlock, NULL);
            break;
        case 4:
            // Fine-grained rwlock with writer preference
            {
                pthread_rwlockattr_t attr;
                pthread_rwlockattr_init(&attr);
                pthread_rwlockattr_setkind_np(&attr, PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP);
                fine_rwlocks = (pthread_rwlock_t*)malloc(num_accounts * sizeof(pthread_rwlock_t));
                for (int i = 0; i < num_accounts; i++) {
                    pthread_rwlock_init(&fine_rwlocks[i], &attr);
                }
                pthread_rwlockattr_destroy(&attr);
            }
            break;
        default:
            // Fallback: coarse mutex
            pthread_mutex_init(&coarse_mutex, NULL);
            lock_type = 1;
    }
}

// --- Destroy locks and free resources ---
void destroy_locks() {
    switch(lock_type) {
        case 1:
            pthread_mutex_destroy(&coarse_mutex);
            break;
        case 2:
            if (fine_mutexes) {
                for (int i = 0; i < num_accounts; i++) {
                    pthread_mutex_destroy(&fine_mutexes[i]);
                }
                free(fine_mutexes);
                fine_mutexes = NULL;
            }
            break;
        case 3:
            pthread_rwlock_destroy(&coarse_rwlock);
            break;
        case 4:
            if (fine_rwlocks) {
                for (int i = 0; i < num_accounts; i++) {
                    pthread_rwlock_destroy(&fine_rwlocks[i]);
                }
                free(fine_rwlocks);
                fine_rwlocks = NULL;
            }
            break;
    }
}

// --- Get lock type name ---
const char* get_lock_name() {
    switch(lock_type) {
        case 1: return "Coarse-grained Mutex";
        case 2: return "Fine-grained Mutex";
        case 3: return "Coarse-grained RWLock";
        case 4: return "Fine-grained RWLock";
        default: return "Unknown";
    }
}