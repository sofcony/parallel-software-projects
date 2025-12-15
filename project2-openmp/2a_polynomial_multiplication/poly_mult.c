#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#ifdef _OPENMP
#include <omp.h>
#endif


#define DEBUG 0 //1: lite debugging 2: full debugging
#define WRITE_FILE 0

void serial_poly_multiply();
void omp_poly_multiply();

int poly_order = 0;
int thread_count = 0;
int* poly_coeff0; 
int* poly_coeff1; 
int* serial_multiply_coeffs;
int* omp_multiply_coeffs;

int main(int argc, char *argv[]) {
	if(argc != 3) {
		printf("\nIncorrect execution! Please insert proper arguments as follows.");    
		printf("\n./poly_mult <polynomial order> <thread count>\n");
		return 1;
	}

	struct timeval time_init;
	struct timeval time_final;
	double running_time = 0.0;
	FILE* results_file = fopen("poly_mult_res.csv", "a");
	//Don't create file if not needed.
	if (!WRITE_FILE) {
		remove("poly_mult_res.csv");
		fclose(results_file);
	}

	poly_order = strtol(argv[1], NULL, 10);
	thread_count = strtol(argv[2], NULL, 10);
	serial_multiply_coeffs = (int*) malloc(poly_order*sizeof(int));
	omp_multiply_coeffs = (int*) malloc(poly_order*sizeof(int));
	// Generate polynomial coefficients (serially).
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
		if (DEBUG == 1) {
			printf("poly_coeff0[%d] = %d\n", i, poly_coeff0[i]);
			printf("poly_coeff1[%d] = %d\n", i, poly_coeff1[i]);
		}
	}
	gettimeofday(&time_final, NULL);
	//Calculate running time for polynomial generation.
	running_time = (time_final.tv_sec - time_init.tv_sec) + (time_final.tv_usec - time_init.tv_usec) / 1000000.0;
	printf("Polynomial coefficient generation of order %d polynomials tool %lf seconds.\n",
			poly_order, running_time);

	//Serial polynomial multiplication.
	gettimeofday(&time_init, NULL);
	serial_poly_multiply();
	gettimeofday(&time_final, NULL);
	//Calculate running time for serial execution.
	running_time = (time_final.tv_sec - time_init.tv_sec) + (time_final.tv_usec - time_init.tv_usec) / 1000000.0;
	printf("Serial polynomial multiplication of order %d polynomials took %lf seconds.\n"
			, poly_order, running_time);
	if (WRITE_FILE) {
		//If file is empty (just created), fill first line with column names.
		if (ftell(results_file) == 0) {
			fprintf(results_file, "Serial results (sec);Parallel results (sec) (%d threads)\n", thread_count);
		}
		fprintf(results_file, "%lf;", running_time);
	}
	
	//Parallel polynomial multiplication.
	gettimeofday(&time_init, NULL);
	omp_poly_multiply();
	gettimeofday(&time_final, NULL);
	//Calculate running time for parallel execution.
	running_time = (time_final.tv_sec - time_init.tv_sec) +
		(time_final.tv_usec - time_init.tv_usec) / 1000000.0;
	printf("Parallel polynomial multiplication of order %d polynomials with %d threads took %lf seconds.\n"
			, poly_order,thread_count, running_time);
	if (WRITE_FILE) {
		fprintf(results_file, "%lf\n", running_time);
		fclose(results_file);
	}
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
	//Free memory.
	free(poly_coeff0);
	free(poly_coeff1);
	free(serial_multiply_coeffs);
	free(omp_multiply_coeffs);

	return 0;
}

void serial_poly_multiply() {
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

void omp_poly_multiply() {
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
