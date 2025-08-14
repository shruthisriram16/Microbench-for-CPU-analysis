/************************************************************
Program		: CPUbound.c
Description	: Micro benchmark for measuring time taken for CPU-bound operations
Author		: Shruthi Sriram
Date		: 19-March-2025
Version		: 001
***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <immintrin.h>  // AVX intrinsics
#include <windows.h>    // For high-resolution timers

#define ARRAY_SIZE 1024    // Array size for computation
#define CHUNK_SIZE 8       // Chunk size for AVX operations

// Initialize the arrays with random values
void init_array(float* a, float* b, float* c, int size) {
    srand((int)time(0));  // Seed the random number generator
    for (int i = 0; i < size; ++i) {
        a[i] = (float)rand();  // Initialize random values for array a
        b[i] = (float)rand();  // Initialize random values for array b
        c[i] = 0;  // Initialize result array c to zeros
    }
}

// Get the current time in high resolution
double get_time() {
    LARGE_INTEGER frequency, counter;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart / frequency.QuadPart;
}

// Perform scalar addition of two arrays
void scalar_addition(float* a, float* b, float* c, int size) {
    for (int i = 0; i < size; i++) {
        c[i] = a[i] + b[i];  // Perform addition element by element
    }
}

// Perform AVX addition on chunks of data
void avx_add_chunk(float* a, float* b, float* c, int array_size, int chunk_size) {
    for (int i = 0; i < array_size; i += chunk_size) {
        __m256 avx_a = _mm256_loadu_ps(&a[i]);  // Load 8 floats from array a
        __m256 avx_b = _mm256_loadu_ps(&b[i]);  // Load 8 floats from array b
        __m256 avx_c = _mm256_add_ps(avx_a, avx_b);  // Perform AVX addition
        _mm256_storeu_ps(&c[i], avx_c);  // Store the result back into array c
    }
}

int main() {
    double start_time, end_time, cpu_time_scalar, cpu_time_avx;

    // Allocate aligned memory for arrays
    float* a = (float*)_aligned_malloc(ARRAY_SIZE * sizeof(float), 32);
    float* b = (float*)_aligned_malloc(ARRAY_SIZE * sizeof(float), 32);
    float* c = (float*)_aligned_malloc(ARRAY_SIZE * sizeof(float), 32);

    if (a == NULL || b == NULL || c == NULL) {
        printf("Memory allocation failed!\n");
        return 1;
    }

    // Initialize arrays
    init_array(a, b, c, ARRAY_SIZE);

    // Measure CPU time for scalar addition
    start_time = get_time();
    scalar_addition(a, b, c, ARRAY_SIZE);
    end_time = get_time();
    cpu_time_scalar = end_time - start_time;  // Calculate CPU time for scalar addition
    printf("Time taken for scalar addition (CPU time): %lf seconds\n", cpu_time_scalar);

    // Measure CPU time for AVX addition 
    start_time = get_time();
    avx_add_chunk(a, b, c, ARRAY_SIZE, CHUNK_SIZE);
    end_time = get_time();
    cpu_time_avx = end_time - start_time;  // Calculate CPU time for AVX addition
    printf("Time taken for AVX addition (CPU time): %lf seconds\n", cpu_time_avx);

    // Free allocated memory
    _aligned_free(a);
    _aligned_free(b);
    _aligned_free(c);

    return 0;
}