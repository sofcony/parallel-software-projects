#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#ifdef _OPENMP
#include <omp.h>
#endif

#define DEBUG 0
#define SEED 12
#define WRITE_FILE 1

// Functions
void merge(int A[],int B[],int min,int max);
void mergeSort(int A[], int B[], int min, int max);
void omp_mergeSort(int A[], int B[], int min, int max, int thread_count);
double get_running_time(struct timeval time_final, struct timeval time_init);
void serial_or_parallel(char* s_or_p, char* cmd_arg);

// Global variables
int x,y;
int thread_count = 0;

int main (int argc, char *argv[])
{
	if(argc != 4) {
		printf("\nError: Incorrect execution!");
		printf("\nUsage: %s <matrix_size> <(s)erial/(p)arallel mode> <thread_count>\n", argv[0]);
		printf("    matrix_size: Number of elements in the matrix\n");
		printf("    mode: 's' for serial or 'p' for parallel execution\n");
		printf("    thread_count: Number of threads to use (only for parallel mode)\n");
		printf("Example: %s 1000000 p 4\n", argv[0]);
		return 1;
	}

	// Check the arguments are valid
	if (atoi(argv[1]) <=0) {
		printf("Error: matrix_size must be a positive integer.\n");
		return 1;
	} 
	if (atoi(argv[3]) <=0) {
		printf("Error: thread_count must be positive integer.\n");
		return 1;
	}

	// Parse arguments
	int msize = strtol(argv[1], NULL, 10);
	int thread_count = strtol(argv[3], NULL, 10);
	char merge_mode[9];
	serial_or_parallel(merge_mode, argv[2]);
	int* A; //Unsorted matrix.
	int* B; //Sorted matrix (starting value is 0).

	// Print the setup
	printf("\n---- Mergesort via Serial or Parallel (OpenMP) ----\n");
	printf("Mode: %s\n", merge_mode);
	printf("Matrix size: %d\n", msize);
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
	sprintf(file_name, "%s_%s_%sthr_%selem.csv", argv[0], merge_mode, argv[3], argv[1]); 
	FILE* results_file = fopen(file_name, "a");
	//Don't create file if not needed.
	if (!WRITE_FILE) {
		remove(file_name);
		fclose(results_file);
	}
	//If file is empty (just created), fill first line with column names.
	else if (ftell(results_file) == 0) {
		fprintf(results_file, "Mergesort time (sec) (%d threads) (%s mode)\n"
				, thread_count, merge_mode);
		}


	// Step 1: Generate unsorted matrix.
	if (msize < 2)
	{
		printf("ERROR: Matrix size must be greater than 2!\n");
		return 1;
	}
	srand(time(0));
	A = (int*) malloc(msize*sizeof(int));
	B = (int*) malloc(msize*sizeof(int));
	for (int i = 0; i < msize; ++i)	
	{
		A[i] = rand();
	}
	
	//Step 2: Begin mergesort.
	if (DEBUG)
		printf("merge_mode is: %s\n", merge_mode);

	if (strcmp(merge_mode, "parallel") == 0)
	{
		printf("Parallel mode selected.\n");
		gettimeofday(&time_init, NULL);
		omp_mergeSort(A, B, 0, msize - 1, thread_count);
		gettimeofday(&time_final, NULL);
		running_time = get_running_time(time_final, time_init);
		printf("Mergesort of matrix with %d elements and %d threads took %lf seconds.\n"
				, msize, thread_count, running_time);
		if (WRITE_FILE)
			fprintf(results_file, "%lf\n", running_time);
	}
	else if (strcmp(merge_mode, "serial") == 0)
	{
		printf("Serial mode selected.\n");
		gettimeofday(&time_init, NULL);
		mergeSort(A, B, 0, msize - 1); //Starting matrix is A, resulting matrix is B.
		// min is 0 because our array always starts from 0.
		// max is msize - 1 cause that's the max index value.
		gettimeofday(&time_final, NULL);
		running_time = get_running_time(time_final, time_init);
		printf("Mergesort of matrix with %d elements took %lf seconds.\n"
				, msize, running_time);
		if (WRITE_FILE)
			fprintf(results_file, "%lf\n", running_time);
	}
	else
	{
		printf("Invalid mode! Please select either (p)arallel or (s)erial mode.\n");
		return 1;
	}
	
	if (DEBUG >= 1)
	{
		printf("\nSorted matrix B is:");
		for(int i = 0; i < msize; ++i)
		{
			printf("B[%d] = %d\n", i, B[i]);
		}
	}

	if (WRITE_FILE)
		fclose(results_file);

	//Free memory
	free(A);
	free(B);
	return 0;
}


void merge(int A[],int B[],int min,int max)
{
	int mid = (min+max)/2;
	int i = min;
	int j = mid+1;
	int p = min;
	int k = 0;
	
	//Sorting the "messed up" parts of the big matrix (the big matrix is comprised of two sorted sub-matrices).
	while ((i<=mid) && (j<=max))
	{
		if (A[i] < A[j])
		{
			B[p] = A[i];
			++i;
			++p;
		}
		else
		{
			B[p] = A[j];
			++j;
			++p;
		}
	}
	
	if (i == mid)
	{
		for (k = j; k <= max; ++k)
		{
			B[k] = A[k];
		}
	}
	
	if (j == max+1)
	{
		for (k = p; k <= max; ++k)
		{
			B[k] = A[i];
			++i;
		}
	}
}

void mergeSort(int A[], int B[], int min, int max)
{
	int mid = (min + max)/2;
	if (max - min > 1) //Splits the B matrix (which is the same as the A matrix at the start)
			   //in two until it's a matrix with size 1.
	{
		mergeSort(B, A, min, mid); //Splits the first half into matrices with size 1.
		mergeSort(B, A, mid+1, max); //Splits the second half into matrices with size 1.
	}
	merge(A, B, min, max); //Here we merge the split B matrices back together into the A matrix. 
	//On the final run, we merge the two halves of A into B,
	//but on the other runs we merge parts of B into A
	//(to create two sorted sub-arrays on the A matrix for the final run).
}

void omp_mergeSort(int A[], int B[], int min, int max, int thread_count)
{
	int mid = (min + max)/2;
	#pragma omp parallel num_threads(thread_count)
	#pragma omp single
	{
		if (max - min > 1) //Splits the B matrix (which is the same as the A matrix at the start)
			   	//in two until it's a matrix with size 1.
		{
			#pragma omp task
			mergeSort(B, A, min, mid); //Splits the first half into matrices with size 1.
			mergeSort(B, A, mid+1, max); //Splits the second half into matrices with size 1.
		}
		merge(A, B, min, max); //Here we merge the split B matrices back together into the A matrix. 
		//On the final run, we merge the two halves of A into B,
		//but on the other runs we merge parts of B into A
		//(to create two sorted sub-arrays on the A matrix for the final run).
	}
}

double get_running_time(struct timeval time_final, struct timeval time_init)
{
	return (time_final.tv_sec - time_init.tv_sec) + (time_final.tv_usec - time_init.tv_usec) / 1000000.0;
}

void serial_or_parallel(char* s_or_p, char* cmd_arg)
{
	if (strcmp(cmd_arg, "parallel") == 0 || strcmp(cmd_arg, "p") == 0)
		strcpy(s_or_p, "parallel");
	else if (strcmp(cmd_arg, "serial") == 0 || strcmp(cmd_arg, "s") == 0)
		strcpy(s_or_p, "serial");
	return;
}
