#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>

#define SEED 2

int *poly1, *poly2;
int *serial_mult_result, *parallel_mult_result;
int n; // Degree of the polynomials
int num_threads;


// Function to get the current time in seconds
double get_time() {
   struct timespec ts;
   clock_gettime(CLOCK_MONOTONIC, &ts);
   return ts.tv_sec + ts.tv_nsec * 1e-9;
}

// Function to compare serial and parallel results
static int compare_results(void) {
    int mismatches = 0;
    for (int k = 0; k <= 2*n; ++k) {
        if (serial_mult_result[k] != parallel_mult_result[k]) {
            ++mismatches;
        }
    }
    return mismatches;
}

// Initialize the polynomials with random coefficients
void initialize_polynomials(int rand_max){

    for(int i = 0; i <= n; i++){
        // Get a random coefficient from -rand_max to rand_max, excluding the 0
        do {
            poly1[i] = (rand() % ((2*rand_max)+1)) - rand_max;
        } while (poly1[i] == 0);
        do {
            poly2[i] = (rand() % ((2*rand_max)+1)) - rand_max;
        } while (poly2[i] == 0);
    }    

    return;
}

// Serial Polynomial Multiplication
void serial_polynomial_multiplication() {
    for (int i=0; i<=n; i++){
        for (int j=0; j<=n; j++){
            serial_mult_result[i+j] += poly1[i] * poly2[j];
        }
    }
}


// TODO: Check
void serial_polynomial_multiplication_k() {
    for (int k = 0; k <= 2*n; ++k) {
        int i_min = (k - n > 0) ? (k - n) : 0;
        int i_max = (k < n) ? k : n;
        int sum = 0;
        for (int i = i_min; i <= i_max; ++i) {
            sum += poly1[i] * poly2[k - i];
        }
        serial_mult_result[k] = sum; // single store
    }
}




// Thread function for polynomial multiplication
void *thread_multiply(void *rank) {
    long t = (long)rank;

    int total = 2*n + 1;
    int chunk = (total + num_threads - 1) / num_threads; // ceil
    int start_k = t * chunk;
    int end_k   = start_k + chunk;
    if (end_k > total) end_k = total;

    for (int k = start_k; k < end_k; ++k) {
        int i_min = (k - n > 0) ? (k - n) : 0;
        int i_max = (k < n) ? k : n;
        int sum = 0;
        for (int i = i_min; i <= i_max; ++i) {
            sum += poly1[i] * poly2[k - i];
        }
        parallel_mult_result[k] = sum; 
    }

    return NULL;
}

// Parallel Polynomial Multiplication (main thread function)
void parallel_multiply() {

    pthread_t* threads = malloc(num_threads * sizeof(pthread_t));

    long thread;

    // Create threads
    for (thread = 0; thread < num_threads; thread++) {
        pthread_create(&threads[thread], NULL, thread_multiply, (void*) thread);
    }
    // Wait for all threads to complete
    for (thread = 0; thread < num_threads; thread++) {
        pthread_join(threads[thread], NULL);
    }

    free(threads);
}


int main(int argc, char *argv[]) {

    if (argc != 3) {
        printf("Error: Please use %s <polynomial_degree> <threads_number> \n", argv[0]);
    }

    srand(SEED);
    // srand(time(NULL));

    n = atoi(argv[1]);
    num_threads = atoi(argv[2]);

    printf("\n === Threads multiplication with use of Pthreads === \n");
    printf("Degree of the polynomials: %d", n);
    printf("\nNumber of threads: %d \n", num_threads);


    // --------- Initialization ---------
    double start_time = get_time();

    // Allocate memory
    poly1 = (int *)malloc((n+1)* sizeof(int));
    poly2 = (int *)malloc((n+1)* sizeof(int));

    serial_mult_result = (int *)malloc((2*n+1)* sizeof(int));
    parallel_mult_result = (int *)malloc((2*n+1)* sizeof(int));
    memset(serial_mult_result, 0, (2*n+1)*sizeof(int));
    memset(parallel_mult_result, 0, (2*n+1)*sizeof(int));

    int coeff_max = 10;
    initialize_polynomials(coeff_max);

    double init_time = get_time() - start_time;
    

    // --------- Serial Polynomial Multiplication ---------
    start_time = get_time();
    // serial_polynomial_multiplication();
    serial_polynomial_multiplication_k();
    double serial_time = get_time() - start_time;
    

    // --------- Parallel Polynomial Multiplication ---------
    start_time = get_time();
    parallel_multiply();
    double parallel_time = get_time() - start_time;
    

    // --------- Timing Results ---------
    printf("\nInitialization time: %.10f seconds", init_time);
    printf("\nSerial multiplication time: %.10f seconds", serial_time);
    printf("\nParallel multiplication time: %.10f seconds \n", parallel_time);


    // --------- Verification ---------
    int mism = compare_results();
    if (mism == 0) {
        printf("\nResults match (serial vs parallel).\n");
    } else {
        printf("Mismatch in %d coefficients!\n", mism);
    }

    // Print the polynomials
    // printf("\n");
    // printf("\nPolynomial 1: ");
    // for(int i = 0; i <= n; i++){
    //     printf("%dx^%d ", poly1[i], i);
    // }

    // printf("\nPolynomial 2: ");
    // for(int i = 0; i <= n; i++){
    //     printf("%dx^%d ", poly2[i], i);
    // }

    // printf("\n Serial Multiplication Polynomial: ");
    // for(int i = 0; i <= 2*n; i++){
    //     printf("%dx^%d", serial_mult_result[i], i);
    // }




    // Free the allocated memory
    free(poly1);
    free(poly2);
    free(serial_mult_result);
    free(parallel_mult_result);


    printf("\n");
    return 0;

}