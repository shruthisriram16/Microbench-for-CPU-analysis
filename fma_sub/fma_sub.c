/************************************************************
Program		: fma_sub .c
Description	: Micro bench mark for measuring time taken for fma sub instruction
Author		: Shruthi Sriram
Date		: 12-March-2025
Version		: 001
***************************************************************/
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<Windows.h> // For high-resolution timers
#include<immintrin.h> // AVX intrinsics
#define ARRAY_SIZE 1024
#define CHUNK_SIZE 8
#define ITERATIONS 100

void init_array(float* a, float* b, float* c, float* d, int array_size)
{
	// Seed the random number with current time
	srand((int)time(0));

	// Initialize arrays with some values
	for (int i = 0; i < ARRAY_SIZE; ++i) {
		a[i] = (float)(rand() % 100);
		b[i] = (float)(rand() % 100);
		c[i] = (float)(rand() % 100);
		d[i] = 0;
	}
}

void print_array(float* a, float* b, float* c, float* d, int size)
{
	int i = 0;
	for (i = 0; i < size; i++)
	{
		printf("a[%d] = %f, b[%d]=%f, c[%d] = %f,d[%d] = %f \n", i, a[i], i, b[i], i, c[i], i, d[i]);
	}
}

// Function to get high-resolution time
double get_time()
{
	LARGE_INTEGER frequency, start;
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&start);
	return (double)start.QuadPart / frequency.QuadPart;
}

// Function to perform fma on a chunk
void fma_sub_chunk(float* a, float* b, float* c, float* d, int array_size, int chunk_size)
{
	for (int i = 0; i < array_size; i += chunk_size) {
		__m256 avx_a = _mm256_loadu_ps(&a[i]);  // Load 8 floats from array a
		__m256 avx_b = _mm256_loadu_ps(&b[i]); // Load 8 floats from array b
		__m256 avx_c = _mm256_loadu_ps(&c[i]);
		__m256 avx_d = _mm256_fmsub_ps(avx_a, avx_b, avx_c);  // Perform 
		_mm256_storeu_ps(&d[i], avx_d);  // Store the result back to array d
	}
}

int main()
{
	double start_time, end_time, elapsed_time;
	double average_time[ITERATIONS] = { 0, };
	int i = 0;


	// Allocate memory for arrays
	float* a = (float*)_aligned_malloc(ARRAY_SIZE * sizeof(float), 32);
	float* b = (float*)_aligned_malloc(ARRAY_SIZE * sizeof(float), 32);
	float* c = (float*)_aligned_malloc(ARRAY_SIZE * sizeof(float), 32);
	float* d = (float*)_aligned_malloc(ARRAY_SIZE * sizeof(float), 32);


	if (a == NULL || b == NULL || c == NULL || d == NULL) {
		printf("Memory allocation failed!\n");
		return 1;
	}

	// Initialize arrays with some values
	init_array(a, b, c, d, ARRAY_SIZE);

	// fma operation 
	for (i = 0; i < ITERATIONS; i++) {

		start_time = get_time();
		fma_sub_chunk(a, b, c, d, ARRAY_SIZE, CHUNK_SIZE);
		end_time = get_time();
		//Compute elapsed time	
		elapsed_time = end_time - start_time;
		average_time[i] = elapsed_time;
		printf("Start time = %lf and End time = %lf \n", start_time, end_time);
		printf("Iteration = %4d, Array size= %d. Elapsed Time = %lf seconds\n", i + 1, ARRAY_SIZE, elapsed_time);
	}

	//Compute average time
	elapsed_time = 0;
	for (i = 0; i < ITERATIONS; i++) {
		elapsed_time += average_time[i];
	}
	elapsed_time = elapsed_time / ITERATIONS;
	printf("-------------------------------------------------------------------------\n");
	printf("Average Time taken for fm subtractation for array size %d is : %lf seconds\n", ARRAY_SIZE, elapsed_time);
	printf("-------------------------------------------------------------------------\n");

	//print_array(a, b, c, d, ARRAY_SIZE);
	// Free allocated memory
	_aligned_free(a);
	_aligned_free(b);
	_aligned_free(c);

	return 0;
}
