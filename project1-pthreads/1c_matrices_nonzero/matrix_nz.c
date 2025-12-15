#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#define SEED 4
#define DEBUG 0

#ifndef USE_PADDING
#define USE_PADDING 0
#endif

// --- Original Structure to hold the non zero counts for each array ---   
// When USE_PADDING is set to 0, this structure will be used
struct array_stats_s0 {
    long long int info_array_0;
    long long int info_array_1;
    long long int info_array_2;
    long long int info_array_3;
} array_stats_0;

// --- Better Structure with padding to avoid false sharing ---
// When USE_PADDING is set to 1, this structure will be used
struct array_stats_s1 {
    long long int info_array_0;
    char padding0[56];  // 64 - 8 = 56 bytes padding
    long long int info_array_1;
    char padding1[56];
    long long int info_array_2;
    char padding2[56];
    long long int info_array_3;
    char padding3[56];
} array_stats_1;


#if USE_PADDING
#define array_stats array_stats_1
#else
#define array_stats array_stats_0
#endif


// The 4 arrays
int *array_0;
int *array_1;
int *array_2;
int *array_3;
int array_size;  // Size of each array


// --- Function declarations ---
void* count_nonzero(void *arg);
void serial_count(long long *result_0, long long *result_1, long long *result_2, long long *result_3);
double get_time_diff(struct timeval start, struct timeval end);


// Main function
int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Usage: %s <array_size>\n", argv[0]);
        printf("Example: %s 10000 \n", argv[0]);
        return 1;
    }

    // Parse arguments
    array_size = atoi(argv[1]);

    // Print configuration
    printf("---- Non Zero Counter for 4 arrays ----\n");
    printf("Array size: %d elements\n", array_size);

    srand(time(NULL));
    // srand(SEED); // For reproducibility


    // STEP 1: Memory allocation and initialization of the 4 arrays and start timing
    struct timeval start, end;
    int i;
    gettimeofday(&start, NULL);
    
    array_0 = (int*)malloc(array_size * sizeof(int));
    array_1 = (int*)malloc(array_size * sizeof(int));
    array_2 = (int*)malloc(array_size * sizeof(int));
    array_3 = (int*)malloc(array_size * sizeof(int));
    
    // Initialize with random numbers 0-9
    for (i = 0; i < array_size; i++) {
        array_0[i] = rand() % 10;
        array_1[i] = rand() % 10;
        array_2[i] = rand() % 10;
        array_3[i] = rand() % 10;
    }
    
    gettimeofday(&end, NULL);

    printf("--- Results ---\n");
    printf("> Array creation time: %.6f seconds\n", get_time_diff(start, end));
    
    // STEP 2: Serial execution
    long long serial_0, serial_1, serial_2, serial_3;
    
    gettimeofday(&start, NULL);
    serial_count(&serial_0, &serial_1, &serial_2, &serial_3);
    gettimeofday(&end, NULL);
    
    printf("> Serial execution time: %.6f seconds\n", get_time_diff(start, end));
    if (DEBUG) {printf("> Serial results: %lld, %lld, %lld, %lld\n\n", serial_0, serial_1, serial_2, serial_3); }


    // STEP 3: Parallel execution with threads    
    // Reset the shared structure
    array_stats.info_array_0 = 0;
    array_stats.info_array_1 = 0;
    array_stats.info_array_2 = 0;
    array_stats.info_array_3 = 0;

    // Create the 4 threads
    pthread_t thread_0, thread_1, thread_2, thread_3;
    int id_0 = 0, id_1 = 1, id_2 = 2, id_3 = 3;
    
    gettimeofday(&start, NULL);
    
    pthread_create(&thread_0, NULL, count_nonzero, &id_0);
    pthread_create(&thread_1, NULL, count_nonzero, &id_1);
    pthread_create(&thread_2, NULL, count_nonzero, &id_2);
    pthread_create(&thread_3, NULL, count_nonzero, &id_3);
    
    // Wait for all 4 threads to finish
    pthread_join(thread_0, NULL);
    pthread_join(thread_1, NULL);
    pthread_join(thread_2, NULL);
    pthread_join(thread_3, NULL);
    
    gettimeofday(&end, NULL);
    
    printf("> Parallel execution time: %.6f seconds\n", get_time_diff(start, end));
    if (DEBUG) {printf("> Parallel results: %lld, %lld, %lld, %lld\n\n", array_stats.info_array_0, array_stats.info_array_1, array_stats.info_array_2, array_stats.info_array_3); }
    
    // STEP 4: Correctness check
    if (serial_0 == array_stats.info_array_0 && serial_1 == array_stats.info_array_1 &&
        serial_2 == array_stats.info_array_2 && serial_3 == array_stats.info_array_3) {
        printf("> Status: SUCCESS - Serial and parallel results are correct!\n");
    } else {
        printf("> Status: ERROR - Serial and parallel results are different!\n");
    }
    printf("\n");

    // Cleanup - Free memory
    free(array_0);
    free(array_1);
    free(array_2);
    free(array_3);
    
    return 0;
}


// PARALLEL IMPLEMENTATION
// --- Thread function to count non-zero elements in an array
void* count_nonzero(void *arg) {

    int thread_id = *(int*)arg;  // Which array (0, 1, 2, or 3)
    
    // Select the appropriate array based on thread_id
    int *my_array;
    if (thread_id == 0) my_array = array_0;
    else if (thread_id == 1) my_array = array_1;
    else if (thread_id == 2) my_array = array_2;
    else my_array = array_3;
    
    // Count non-zero elements
    int i;
    for (i = 0; i < array_size; i++) {
        if (my_array[i] != 0) {
            // Update the correct field based on thread_id
            if (thread_id == 0) array_stats.info_array_0++;
            else if (thread_id == 1) array_stats.info_array_1++;
            else if (thread_id == 2) array_stats.info_array_2++;
            else array_stats.info_array_3++;
        }
    }
    
    return NULL;
}

// SERIAL IMPLEMENTATION
void serial_count(long long *result_0, long long *result_1, long long *result_2, long long *result_3) {
    
    int i;
    *result_0 = 0;
    *result_1 = 0;
    *result_2 = 0;
    *result_3 = 0;
    
    // Count for each array separately
    for (i = 0; i < array_size; i++) {
        if (array_0[i] != 0) (*result_0)++;
    }
    for (i = 0; i < array_size; i++) {
        if (array_1[i] != 0) (*result_1)++;
    }
    for (i = 0; i < array_size; i++) {
        if (array_2[i] != 0) (*result_2)++;
    }
    for (i = 0; i < array_size; i++) {
        if (array_3[i] != 0) (*result_3)++;
    }
}

// TIME FUNCTION
double get_time_diff(struct timeval start, struct timeval end) {
    return (end.tv_sec - start.tv_sec) + 
           (end.tv_usec - start.tv_usec) / 1000000.0;
}