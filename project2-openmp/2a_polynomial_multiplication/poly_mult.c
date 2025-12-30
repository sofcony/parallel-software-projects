#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#ifdef _OPENMP
#include <omp.h>
#endif

//DEBUG 1: lite debugging. 2: full debugging
#define DEBUG 0 
#define SEED 12
#define WRITE_FILE 1

// Functions
void serial_poly_multiply();
void omp_poly_multiply();
double get_running_time(struct timeval time_final, struct timeval time_init);

// Global variables
int poly_order = 0;
int thread_count = 0;
int* poly_coeff0; 
int* poly_coeff1; 
int* serial_multiply_coeffs;
int* omp_multiply_coeffs;

int main(int argc, char *argv[])
{
	if(argc != 3) {
		printf("\nError: Incorrect execution!");
		printf("\nUsage: %s <polynomial_order> <thread_count>\n", argv[0]);
		printf("    polynomial_order: Order of the polynomial (positive integer)\n");
		printf("    thread_count: Number of threads to use (positive integer)\n");
		printf("Example: %s 1000 4\n", argv[0]);
		return 1;
	}

	// Check the arguments are valid
	if (atoi(argv[1]) <=0) {
		printf("Error: polynomial_order must be a positive integer.\n");
		return 1;
	} 
	if (atoi(argv[3]) <=0) {
		printf("Error: thread_count must be positive integer.\n");
		return 1;
	}
	
	// Parse arguments
	poly_order = strtol(argv[1], NULL, 10);
	thread_count = strtol(argv[2], NULL, 10);
	serial_multiply_coeffs = (int*) malloc(poly_order*sizeof(int));
	omp_multiply_coeffs = (int*) malloc(poly_order*sizeof(int));

	// Print the Setup 
	printf("\n---- Polynomial Multiplication (OpenMP) ----\n");
	printf("Polynomial order: %d\n", poly_order);
	printf("Number of threads: %d\n\n", thread_count);
	
	// Initialize random seed
	if (DEBUG)
		srand(SEED); // For reproducibility
	else
		srand(time(NULL));
	// Time arguments
	struct timeval time_init;
	struct timeval time_final;
	double running_time = 0;

	// File arguments (if necessary)
	char file_name[100];
	sprintf(file_name, "%s_%sthr_%sorder.csv", argv[0], argv[2], argv[1]); 
	FILE* results_file = fopen(file_name, "a");
	if (!WRITE_FILE) { //Don't create file if not needed.
		remove(file_name);
		fclose(results_file);
	}
	//If file is empty (just created), fill first line with column names.
	else if (ftell(results_file) == 0) {
		fprintf(results_file, "Serial results (sec);Parallel results (sec) (%d threads)\n", thread_count);
	}


	// Step 1: Randomly generate polynomial coefficients (serially).
	gettimeofday(&time_init, NULL);
	srand(time(0));
	poly_coeff0 = (int*) malloc(poly_order*sizeof(int));
	poly_coeff1 = (int*) malloc(poly_order*sizeof(int));
	for (int i = 0; i < poly_order; ++i)
	{

		do {
			poly_coeff0[i] = rand() % 1000;
			poly_coeff1[i] = rand() % 1000;
		} while (poly_coeff0[i] == 0 || poly_coeff1[i] == 0);
		if (DEBUG == 2) {
			printf("poly_coeff0[%d] = %d\n", i, poly_coeff0[i]);
			printf("poly_coeff1[%d] = %d\n", i, poly_coeff1[i]);
		}
	}
	gettimeofday(&time_final, NULL);
	//Calculate running time for polynomial generation.
	running_time = get_running_time(time_final, time_init);
	printf("Polynomial coefficient generation of order %d polynomials took %lf seconds.\n",
			poly_order, running_time);


	//Step 2: Serial polynomial multiplication.
	gettimeofday(&time_init, NULL);
	serial_poly_multiply();
	gettimeofday(&time_final, NULL);
	//Calculate running time for serial execution.
	running_time = get_running_time(time_final, time_init);
	printf("Serial polynomial multiplication of order %d polynomials took %lf seconds.\n"
			, poly_order, running_time);
	if (WRITE_FILE)
		fprintf(results_file, "%lf;", running_time);
	
		
	//Step 3: Parallel polynomial multiplication.
	gettimeofday(&time_init, NULL);
	omp_poly_multiply();
	gettimeofday(&time_final, NULL);
	//Calculate running time for parallel execution.
	running_time = get_running_time(time_final, time_init);
	printf("Parallel polynomial multiplication of order %d polynomials with %d threads took %lf seconds.\n"
			, poly_order,thread_count, running_time);
	if (WRITE_FILE) 
		fprintf(results_file, "%lf\n", running_time);

	if (DEBUG == 2) {
		for (int i = 0; i < poly_order; ++i) {
			printf("serial_multiply_coeffs[%d] = %d\n", i, serial_multiply_coeffs[i]);
			printf("omp_multiply_coeffs[%d] = %d\n", i, omp_multiply_coeffs[i]);
		}
	}

	//Check that results are the same.
	for (int i = 0; i < poly_order; ++i) {
		if (serial_multiply_coeffs[i] != omp_multiply_coeffs[i]) {
			printf("WARNING: serial_multiply_coeffs[%d] = %d and omp_multiply_coeffs[%d] = %d are different!\n"
					, i, serial_multiply_coeffs[i], i, omp_multiply_coeffs[i]);
		}
	}

	if (WRITE_FILE)
		fclose(results_file);

	//Free memory.
	free(poly_coeff0);
	free(poly_coeff1);
	free(serial_multiply_coeffs);
	free(omp_multiply_coeffs);

	return 0;
}

void serial_poly_multiply()
{
	if (DEBUG >= 1) {
		printf("In serial_poly_multiply.\n");
		printf("\npoly_order = %d\n", poly_order);
	}
	for (int i = 0; i < poly_order; ++i) {
		for (int j = 0; j < poly_order; ++j) {
			serial_multiply_coeffs[i] += poly_coeff0[i]*poly_coeff1[j];
		}
	}
	return;
}

void omp_poly_multiply()
{
	if (DEBUG >= 1) {
		printf("In omp_poly_multiply.\n");
		printf("\npoly_order = %d\n", poly_order);
	}
	#pragma omp parallel for num_threads(thread_count)
	for (int i = 0; i < poly_order; ++i) {
		for (int j = 0; j < poly_order; ++j) {
			omp_multiply_coeffs[i] += poly_coeff0[i]*poly_coeff1[j];
		}
		if (DEBUG == 2) {
			printf("omp_multiply_coeffs[%d] = %d\n", i, omp_multiply_coeffs[i]);
		}
	}
	return;
}

double get_running_time(struct timeval time_final, struct timeval time_init)
{
	return (time_final.tv_sec - time_init.tv_sec) + (time_final.tv_usec - time_init.tv_usec) / 1000000.0;
}
