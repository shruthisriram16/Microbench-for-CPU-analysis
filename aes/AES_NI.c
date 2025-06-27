/************************************************************
Program        : aes-ni.c
Description    : Micro benchmark for measuring time taken for AES instruction
Author         : Shruthi Sriram
Date           : 13-March-2025
Version        : 002
***************************************************************/
#include <immintrin.h>
#include <smmintrin.h>
#include <stdio.h>
#include <windows.h>
#include <string.h> // For memcpy

void print_text(const char* label, const __m128i* text) {
    printf("%s: ", label);
    for (int i = 0; i < 4; ++i) {
        printf("%08X", _mm_extract_epi32(*text, i));
    }
    printf("\n");
}

void key_expansion(const unsigned char* key, __m128i* key_schedule) {
    key_schedule[0] = _mm_loadu_si128((__m128i*)key);
    for (int i = 1; i < 11; ++i) {
        key_schedule[i] = _mm_aeskeygenassist_si128(key_schedule[i - 1], i);
        key_schedule[i] = _mm_xor_si128(key_schedule[i], _mm_slli_si128(key_schedule[i - 1], 4));
        key_schedule[i] = _mm_xor_si128(key_schedule[i], _mm_slli_si128(key_schedule[i - 1], 8));
        key_schedule[i] = _mm_xor_si128(key_schedule[i], _mm_slli_si128(key_schedule[i - 1], 12));
    }
}

void aes_encrypt(const __m128i* key_schedule, const __m128i* plaintext, __m128i* ciphertext) {
    __m128i state = _mm_loadu_si128(plaintext);
    state = _mm_xor_si128(state, key_schedule[0]); // Initial round key addition

    for (int i = 1; i < 10; ++i) {
        state = _mm_aesenc_si128(state, key_schedule[i]); // 9 main rounds
    }

    state = _mm_aesenclast_si128(state, key_schedule[10]); // Final round
    _mm_storeu_si128(ciphertext, state);
}

// Function to get the current time in seconds
double get_time() {
    LARGE_INTEGER frequency, counter;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart / frequency.QuadPart;
}

int main() {
    __m128i key_schedule[11];
    unsigned char key[16] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                               0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F }; // Example key
    char plain_bytes[16] = { 0 }; // Initialize 16 bytes to zero
    memcpy(plain_bytes, "hello", 5); // Copy "hello" (5 bytes)

    __m128i plaintext = _mm_loadu_si128((__m128i*)plain_bytes);
    __m128i ciphertext;

    // Initialize key schedule with a valid key
    key_expansion(key, key_schedule);

    // Print the plaintext
    print_text("Input Text", &plaintext);

    // Measure time before the AES encryption
    double start_time = get_time();

    // Encrypt the plaintext
    aes_encrypt(key_schedule, &plaintext, &ciphertext);

    // Measure time after the AES encryption
    double end_time = get_time();

    // Print the ciphertext
    print_text("Ciphertext", &ciphertext);

    // Print elapsed time
    printf("Elapsed Time: %f seconds\n", end_time - start_time);

    return 0;
}
