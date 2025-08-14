/************************************************************
Program        : sha_ni.c
Description    : Micro benchmark for measuring time taken for SHA-1 instruction
Author         : Shruthi Sriram
Date           : 13-March-2025
Version        : 003
***************************************************************/

#include <immintrin.h>
#include <wmmintrin.h>
#include <stdio.h>
#include <windows.h>

// Function to print 128-bit data as hexadecimal
void print_text(const char* label, const __m128i* text) {
    printf("%s: ", label);
    printf("%08X", _mm_extract_epi32(*text, 0));
    printf("%08X", _mm_extract_epi32(*text, 1));
    printf("%08X", _mm_extract_epi32(*text, 2));
    printf("%08X", _mm_extract_epi32(*text, 3));
    printf("\n");
}

// Function to compute the SHA-1 hash for 4 rounds
void sha1_hash(const __m128i* data, __m128i* hash) {
    __m128i state = _mm_loadu_si128(hash);
    state = _mm_sha1rnds4_epu32(state, _mm_loadu_si128(data), 0); // Perform 4 rounds
    _mm_storeu_si128(hash, state);
}

// Function to get the current time in seconds
double get_time() {
    LARGE_INTEGER frequency, counter;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart / frequency.QuadPart;
}

int main() {
    __m128i data = _mm_setr_epi32(0x61626380, 0x00000000, 0x00000000, 0x00000018); // "abc" padded
    __m128i hash = _mm_setr_epi32(0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476); // Initial hash value

    // Print input data
    print_text("Input Data", &data);

    // Measure time before the SHA-1 hashing
    double start_time = get_time();

    // Perform one iteration of SHA-1 hashing
    sha1_hash(&data, &hash);

    // Measure time after the SHA-1 hashing
    double end_time = get_time();

    // Print the resulting hash
    print_text("SHA-1 Hash (After 1 Iteration)", &hash);

    // Print elapsed time
    printf("Elapsed Time: %f seconds\n", end_time - start_time);

    return 0;
}
