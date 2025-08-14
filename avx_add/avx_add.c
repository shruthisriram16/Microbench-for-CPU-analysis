/************************************************************
Program		: avxadd.c
Description	: Micro bench mark for measuring time taken for AVX Add instruction
Author		: Shruthi Sriram
Date		: 8-March-2025
Version		: 001
***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <immintrin.h>  // AVX intrinsics
#include <windows.h>    // For high-resolution timers

#define ARRAY_SIZE (4*1024*1024)	// Array size
#define CHUNK_SIZE 8	// Chunk size 
#define ITERATIONS 100				// Number of iterations

void init_array(float* a, float* b, float* c, int size)
{

	// Seed the random number with current time
	srand((int)time(0));

	// Initialize arrays with some values
	for (int i = 0; i < ARRAY_SIZE; ++i) {
		a[i] = (float)rand();
		b[i] = (float)rand();
		c[i] = 0;
	}
}

void print_array(float* a, float* b, float* c, int size)
{
	int i = 0;
	for (i = 0; i < size; i++)
	{
		printf("a[%d] = %f, b[%d]=%f, c[%d] = %f \n", i, a[i], i, b[i], i, c[i]);
	}
}

// Function to get high-resolution time
double get_time() {
	LARGE_INTEGER frequency, start;
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&start);
	return (double)start.QuadPart / frequency.QuadPart;
}


void scalar_addition(float* a, float* b, float* c, int size)
{
	int i = 0;
	for (i = 0; i < size; i++) {
		c[i] = a[i] + b[i];
	}
}

// Function to perform AVX addition on a chunk
void avx_add_chunk(float* a, float* b, float* c, int array_size, int chunk_size)
{
	for (int i = 0; i < array_size; i += chunk_size) {
		__m256 avx_a = _mm256_loadu_ps(&a[i]);  // Load 8 floats from array a
		__m256 avx_b = _mm256_loadu_ps(&b[i]);  // Load 8 floats from array b
		__m256 avx_c = _mm256_add_ps(avx_a, avx_b);  // Perform AVX addition
		_mm256_storeu_ps(&c[i], avx_c);  // Store the result back to array c
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

	if (a == NULL || b == NULL || c == NULL) {
		printf("Memory allocation failed!\n");
		return 1;
	}

	// Initialize arrays with some values
	init_array(a, b, c, ARRAY_SIZE);


	//Scalar addition
	start_time = get_time();
	scalar_addition(a, b, c, ARRAY_SIZE);
	end_time = get_time();
	//Compute elapsed time	
	elapsed_time = end_time - start_time;
	printf("Start time = %lf and End time = %lf \n", start_time, end_time);
	printf("Time taken for scalar addition for array size %d is : %lf seconds\n\n", ARRAY_SIZE, elapsed_time);


	// Vector addition using AVX 
	for (i = 0; i < ITERATIONS; i++) {
		//SIMD addition
		start_time = get_time();
		avx_add_chunk(a, b, c, ARRAY_SIZE, CHUNK_SIZE);
		end_time = get_time();
		//Compute elapsed time	
		elapsed_time = end_time - start_time;
		average_time[i] = elapsed_time;
		//printf("Start time = %lf and End time = %lf \n", start_time, end_time);
		printf("Iteration = %4d, Array size= %d. Elapsed Time = %lf seconds\n", i + 1, ARRAY_SIZE, elapsed_time);
	}

	//Compute average time
	elapsed_time = 0;
	for (i = 0; i < ITERATIONS; i++) {
		elapsed_time += average_time[i];
	}
	elapsed_time = elapsed_time / ITERATIONS;
	printf("-------------------------------------------------------------------------\n");
	printf("Average Time taken for SIMD addition for array size %d is : %lf seconds\n", ARRAY_SIZE, elapsed_time);
	printf("-------------------------------------------------------------------------\n");

	//print_array(a, b, c, ARRAY_SIZE);
	// Free allocated memory
	_aligned_free(a);
	_aligned_free(b);
	_aligned_free(c);
}