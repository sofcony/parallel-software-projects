#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#ifdef _OPENMP
#include <omp.h>
#endif

// # ifdef _OPENMP
// int my_rank = omp_get_thread_num ( );
// int thread_count = omp_get_num_threads ( );
// # else
// int my_rank = 0;
// int thread_count = 1;
// # endif

void merge(int A[],int B[],int min,int max);
void mergeSort(int A[], int B[], int min, int max);
void omp_mergeSort(int A[], int B[], int min, int max, int thread_count);
void omp_mergeSort_entry(int A[], int B[], int min, int max, int thread_count);
int main (int argc, char *argv[])
{
	if(argc != 4) {
		printf("\nIncorrect execution! Please insert proper arguments as follows.");    
		printf("\n./mergesort <matrix size> <(s)erial/(p)arallel mode> <thread count>\n");
		return 1;
	}

	int msize = strtol(argv[1], NULL, 10);
	int thread_count = strtol(argv[3], NULL, 10);
	int* A; //Unsorted matrix.
	int* B; //Sorted matrix (starting value is 0).
	struct timeval time_init;
	struct timeval time_final;
	double running_time = 0;

	//Generate unsorted matrix.
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
	
	//Begin mergesort.
	if (strcmp(argv[2], "parallel") == 0 || strcmp(argv[2], "p") == 0) 
	{
		printf("Parallel mode selected.\n");

		gettimeofday(&time_init, NULL);
		#pragma omp parallel num_threads(thread_count)
		{
			#pragma omp single
			{
				omp_mergeSort(A, B, 0, msize-1, thread_count); //Starting matrix is A, resulting matrix is B.
				// min is 0 because our array always starts from 0.
				// max is msize-1 cause that's the max index value.
			}
		}
		gettimeofday(&time_final, NULL);

		running_time = (time_final.tv_sec - time_init.tv_sec)
			+ (time_final.tv_usec - time_init.tv_usec) / 1000000.0;
		printf("Parallel mergesort time: %f seconds.\n", running_time);
	}
	else if (strcmp(argv[2], "serial") == 0 || strcmp(argv[2], "s") == 0)
	{
		printf("Serial mode selected.\n");
		gettimeofday(&time_init, NULL);
		mergeSort(A, B, 0, msize-1); //Starting matrix is A, resulting matrix is B.
		// min is 0 because our array always starts from 0.
		// max is msize-1 cause that's the max index value.
		gettimeofday(&time_final, NULL);
		running_time = (time_final.tv_sec - time_init.tv_sec)
			+ (time_final.tv_usec - time_init.tv_usec) / 1000000.0;
		printf("Serial mergesort time: %f seconds.\n", running_time);
	}
	else
	{
		printf("Invalid mode! Please select either (p)arallel or (s)erial mode.\n");
		return 1;
	}
	
	// printf("\nSorted matrix B is:");
	// for(int i = 0; i < msize; ++i)
	// {
	// 	printf("\nB[%d] = %d", i, B[i]);
	// }

	free(A);
	free(B);
	printf("\n");
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
	if (max - min > 1) // If the size of the array is greater than 1
	{
		// if ((max - min + 1) <= CUTOFF || thread_count <= 1)
		if (thread_count <= 1)
		{
			mergeSort(A, B, min, max);
			return;
		}

		#pragma omp task shared(A, B)
		omp_mergeSort(B, A, min, mid, thread_count / 2);

		#pragma omp task shared(A, B)
		omp_mergeSort(B, A, mid + 1, max, thread_count - thread_count / 2);

		#pragma omp taskwait
		merge(A, B, min, max);
	}
	else
	{
		merge(A, B, min, max);
	}
	
}

// void omp_mergeSort_old(int A[], int B[], int min, int max, int thread_count)
// {	
// 	int mid = (min + max)/2;
//     if (max - min > 1)
//     {
//         #pragma omp parallel num_threads(thread_count)
//         {
//             #pragma omp single
//             {
//                 #pragma omp task shared(A, B) if(thread_count > 1)
//                 omp_mergeSort(B, A, min, mid, thread_count / 2);

//                 #pragma omp task shared(A, B) if(thread_count > 1)
//                 omp_mergeSort(B, A, mid + 1, max, thread_count - thread_count / 2);

//                 #pragma omp taskwait
//                 merge(A, B, min, max);
//             }
//         }
//     }
//     else
//     {
//         merge(A, B, min, max);
//     }
	
// }