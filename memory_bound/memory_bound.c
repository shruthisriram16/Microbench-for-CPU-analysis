#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <immintrin.h>  // AVX intrinsics
#include <windows.h>    // For high-resolution timers

#define ARRAY_SIZE (50 * 1024 * 1024)  // Array size for 50 MB of data
#define CHUNK_SIZE 8                   // Chunk size for AVX operations

// Initialize the arrays with random values
void init_array(float* a, float* b, float* c, int size) {
    srand((int)time(0));
    for (int i = 0; i < size; ++i) {
        a[i] = (float)rand();
        b[i] = (float)rand();
        c[i] = 0;
    }
}

// Get the current time in high resolution
double get_time() {
    LARGE_INTEGER frequency, counter;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart / frequency.QuadPart;
}

// Perform AVX addition on chunks of data
void avx_add_chunk(float* a, float* b, float* c, int array_size, int chunk_size) {
    for (int i = 0; i < array_size; i += chunk_size) {
        __m256 avx_a = _mm256_loadu_ps(&a[i]);
        __m256 avx_b = _mm256_loadu_ps(&b[i]);
        __m256 avx_c = _mm256_add_ps(avx_a, avx_b);
        _mm256_storeu_ps(&c[i], avx_c);
    }
}

int main() {
    double start_time, end_time, total_time, cpu_time, memory_time;

    // Allocate aligned memory for arrays
    float* a = (float*)_aligned_malloc(ARRAY_SIZE * sizeof(float), 32);
    float* b = (float*)_aligned_malloc(ARRAY_SIZE * sizeof(float), 32);
    float* c = (float*)_aligned_malloc(ARRAY_SIZE * sizeof(float), 32);

    if (a == NULL || b == NULL || c == NULL) {
        printf("Memory allocation failed!\n");
        return 1;
    }

    // Measure the total time
    start_time = get_time();

    // Perform memory operations (initialization)
    init_array(a, b, c, ARRAY_SIZE);

    // Perform AVX addition
    avx_add_chunk(a, b, c, ARRAY_SIZE, CHUNK_SIZE);

    end_time = get_time();
    total_time = end_time - start_time;  // Total time for initialization + computation
    printf("Total time: %lf seconds\n", total_time);

    // Measure CPU time for AVX addition only (computation)
    start_time = get_time();
    avx_add_chunk(a, b, c, ARRAY_SIZE, CHUNK_SIZE);
    end_time = get_time();
    cpu_time = end_time - start_time;  // CPU time for computation only
    printf("CPU time: %lf seconds\n", cpu_time);

    // Compute memory time
    memory_time = total_time - cpu_time;
    printf("Memory time: %lf seconds\n", memory_time);

    // Free allocated memory
    _aligned_free(a);
    _aligned_free(b);
    _aligned_free(c);

    return 0;
}