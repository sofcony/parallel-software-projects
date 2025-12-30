#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <math.h>
#ifdef _OPENMP
#include <omp.h>
#endif

#define DEBUG 0
#define SEED 12
#define WRITE_FILE 1

// Functions
void omp_build_csr(int **A, int rows, int cols, int nz, int *row_ptr, int *col_ind, int *values);     // Create the CSR format of the sparse array using parallel computation OpenMP
void omp_mult_dense(int iters, int *result_vector, int **inp_matrix, int* mult_vector, int rows, int cols);     // Perform matrix multiplication using using parallel computation OpenMP
void omp_mult_csr(int iters, int *result_vector, int* values, int* col_ind, int* row_ptr, int* vector, int rows);        // Perform CSR matrix multiplication using using parallel computation OpenMP
void print_csr(const int *row_ptr, const int *col_ind, const int *values, int rows);
double get_running_time(struct timeval time_final, struct timeval time_init);

// Global variables
int thread_count = 1;
int VALUES_MAX = 100;

int main (int argc, char *argv[])
{
	if (argc != 5) {
		printf("\nError: Incorrect execution!");
		printf("\nUsage: %s <num_row_values> <zeros_percent> <num_mult> <thread_count> \n", argv[0]);
		printf("    num_row_values: Number of values in each row/col of the matrix\n");
		printf("    zeros_percent: Percentage of zeros in the matrix\n");
		printf("    num_mult: Number of multiplications to perform\n");
		printf("    thread_count: Number of threads to use\n");
		printf("Example: %s 100 20 10 4\n", argv[0]);
		return 1;
	}

	// Check the arguments are valid
	if (atoi(argv[1]) <= 0 ) {
		printf("Error: num_row_values must be a positive integer.\n");
		return 1;
	}
	if (atoi(argv[2]) < 0 || atoi(argv[2]) > 100 ) {
		printf("Error: zeros_percent must be between 0 and 100.\n");
		return 1;
	}
	if (atoi(argv[3]) <= 0 ) {
		printf("Error: num_mult must be a positive integer.\n");
		return 1;
	}
	if (atoi(argv[4]) <= 0 ) {
		printf("Error: thread_count must be a positive integer.\n");
		return 1;
	}

	// Parse arguments
	int num_row_values = atoi(argv[1]);
	float zeros_percentage = atof(argv[2]) / 100.0;
	int num_mult = atoi(argv[3]);
	thread_count = atoi(argv[4]);

	// Print the Setup 
	printf("\n---- Sparse Matrix Multiplication (OpenMP) ----\n");
	printf("Number of row/column values: %d\n", num_row_values);
	printf("Percentage of zeros: %.0f %%\n", zeros_percentage*100.0);
	printf("Number of multiplications: %d\n", num_mult);
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
	sprintf(file_name, "%s_%srows_%sper_%siter_%sthr.csv"
			, argv[0], argv[1], argv[2], argv[3], argv[4]); 
	FILE* results_file = fopen(file_name, "a");
	if (!WRITE_FILE) { //Don't create file if not needed.
		remove(file_name);
		fclose(results_file);
	}
	//If file is empty (just created), fill first line with column names.
	else if (ftell(results_file) == 0) {
		fprintf(results_file, "CSR creation time;CSR mult time(sec);Dense mult time (sec)");
		fprintf(results_file, "(%d threads)\n", thread_count);
		}

		
	// Step 1.1: Square matrix dynamic allocation and initilization
	// -- Initilization: First we initiliaze all the matrix cells with values from 1 to VALUES_MAX
	int rows = num_row_values;
 	int cols = num_row_values;
	int **dense_matrix = malloc(rows * sizeof(int*));
	for (int i = 0; i < rows; i++) {
		dense_matrix[i] = malloc(cols * sizeof(int));
		for (int j = 0; j < cols; j++) {
			dense_matrix[i][j] = (rand() % VALUES_MAX) + 1;
		}
	}

    // Step 1.2 : Place the zero elements randomly in the sparse matrix according to the zeros percentage
	int num_all_values = rows * cols;
	int zeros_num = (int)(num_all_values * zeros_percentage);
	int values_num = num_all_values - zeros_num;
	if (DEBUG)
		printf("\n DEBUG: All number: %d, zeros number: %d, Values number: %d \n ", num_all_values, zeros_num, values_num);

	for (int i = 0; i < zeros_num; i++) {
		int row_ind, col_ind;
		do {
			row_ind = rand() % rows;  // pick indices
			col_ind = rand() % cols;
			// repeat if this cell is already zero
		} while (dense_matrix[row_ind][col_ind] == 0);

		dense_matrix[row_ind][col_ind] = 0;
	}

	// DEBUG: Validate zero/non-zero counts and percentage
	if (DEBUG) {
		int zero_count = 0;
		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < cols; j++) {
				if (dense_matrix[i][j] == 0)
					zero_count++;
			}
		}
		int nonzero_count = (rows * cols) - zero_count;
		double actual_zero_pct = (double)zero_count / (rows * cols);
		double target_zero_pct = zeros_percentage;
		double diff = fabs(actual_zero_pct - target_zero_pct);

		printf("Validation:\n");
		printf("  zeros: %d, non-zeros: %d\n", zero_count, nonzero_count);
		printf("  target zeros %%: %.4f, actual zeros %%: %.4f\n", target_zero_pct, actual_zero_pct);

        // Allow tiny floating error tolerance (e.g., 0.5%%)
		if (diff <= 0.005)
			printf("  OK: percentage within tolerance.\n");
		else
			printf("  WARN: percentage deviates by %.4f.\n", diff);
	}

	// Step 1.3 : Initialize the CSR arrays
	int *row_ptr = malloc((rows + 1) * sizeof(int));
	int *col_ind = malloc(values_num * sizeof(int));
	int *values  = malloc(values_num * sizeof(int));
	int non_zeros = values_num;

	gettimeofday(&time_init, NULL);
	omp_build_csr(dense_matrix, rows, cols, non_zeros, row_ptr, col_ind, values);
	gettimeofday(&time_final, NULL);
	running_time = get_running_time(time_final, time_init);
	printf("Parallel CSR array creation time: %f seconds.\n", running_time);
	if (WRITE_FILE)
		fprintf(results_file, "%lf;", running_time);

	// Step 1.4: Vector allocation and initialization with values from 1 to VALUES_MAX.
	int* vector = (int*) malloc(rows * sizeof(int));
	for (int i = 0; i < rows; ++i) {
		vector[i] = (rand() % VALUES_MAX) + 1;
	}


	// Step 2: Multiply CSR form with vector.
	// -- CSR Mult result vector allocation and initialization.
	// -- Multiplication between m x m, m x 1 matrices.
	// -- Resulting matrix is m x 1.
	int* csr_mult_res = (int*) malloc(rows * sizeof(int));
	for (int i = 0; i < rows; ++i) {
		csr_mult_res[i] = 0;
	}
	gettimeofday(&time_init, NULL);
	omp_mult_csr(num_mult, csr_mult_res, values, col_ind, row_ptr, vector, rows);
	gettimeofday(&time_final, NULL);
	running_time = get_running_time(time_final, time_init);
	printf("CSR matrix-vector multiplication with %d multiplications took %f seconds.\n", num_mult, running_time);
	if (WRITE_FILE)
		fprintf(results_file, "%lf;", running_time);

	// Step 3: Multiply dense matrix form with vector.
	// -- Dense Mult result vector allocation and initialization.
	int* dense_mult_res = (int*) malloc(rows * sizeof(int));
	for (int i = 0; i < rows; ++i) {
		dense_mult_res[i] = 0;
	}
	gettimeofday(&time_init, NULL);
	omp_mult_dense(num_mult, dense_mult_res, dense_matrix, vector, rows, cols);
	gettimeofday(&time_final, NULL);
	running_time = get_running_time(time_final, time_init);
	printf("Dense matrix-vector multiplication with %d multiplications took %f seconds.\n", num_mult, running_time);
	if (WRITE_FILE)
		fprintf(results_file, "%lf\n", running_time);

	// Step 4: Free memory and close file
	if (WRITE_FILE)
		fclose(results_file);
	for (int i = 0; i < rows; i++) {
		free(dense_matrix[i]);
	}
	free(dense_matrix);
	free(row_ptr);
	free(col_ind);
	free(values);
	free(vector);
	free(csr_mult_res);

	return 0;
}



void omp_build_csr(int **A, int rows, int cols, int nnz, int *row_ptr, int *col_ind, int *values)
{
	int k = 0;
	row_ptr[0] = 0;

	# pragma omp parallel for num_threads(thread_count)
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			int v = A[i][j];
			if (v != 0) {
				col_ind[k] = j;
				values[k] = v;
				k++;
			}
		}
		row_ptr[i + 1] = k;
	}
}

void omp_mult_csr(int iters, int *result_vector, int* values, int* col_ind, int* row_ptr, int* mult_vector, int rows)
{	
	// Copy of the multiplication vector
	int* func_vector = (int*)malloc(rows * sizeof(int));
	for (int i = 0; i < rows; i++) {
		func_vector[i] = mult_vector[i];
	}

	for (int n = 0; n < iters; n++) {

		# pragma omp parallel for num_threads(thread_count)
		for (int i = 0; i < rows; i++) {
			int sum = 0;

			for (int j = row_ptr[i]; j < row_ptr[i+1]; j++) {
				sum += values[j] * func_vector[col_ind[j]];
			}
			result_vector[i] = sum;
		}

		// Update func_vector for the next iteration
		if (n < iters-1) {
			for (int i = 0; i< rows; i++ ) {
				func_vector[i] = result_vector[i];
			}
		}	
	}

	free(func_vector);
}


void omp_mult_dense(int iters, int *result_vector, int **inp_matrix, int* mult_vector, int rows, int cols) 
{
	// Copy of the multiplication vector
	int* func_vector = (int*)malloc(rows * sizeof(int));
	for (int i = 0; i < rows; i++) {
		func_vector[i] = mult_vector[i];
	}

	for (int n = 0; n < iters; n++) {

		# pragma omp parallel for num_threads(thread_count)
		for (int i = 0; i < rows; i++) {
			int sum = 0;

			for (int j = 0; j < cols; j++) {
                // sum += A[i][j] × func_vector[j]
                sum += inp_matrix[i][j] * func_vector[j];
            }
			result_vector[i] = sum;
		}

		// Update func_vector for the next iteration
		if (n < iters-1) {
			for (int i = 0; i< rows; i++ ) {
				func_vector[i] = result_vector[i];
			}
		}	
	}

	free(func_vector);
}

double get_running_time(struct timeval time_final, struct timeval time_init)
{
	return (time_final.tv_sec - time_init.tv_sec) + (time_final.tv_usec - time_init.tv_usec) / 1000000.0;
}
