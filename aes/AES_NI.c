#include <stdio.h>
#include <windows.h>
#include <wmmintrin.h>
#include <immintrin.h>

// Print 128-bit block
void print_block(const char* label, __m128i block) {
    unsigned char bytes[16];
    _mm_storeu_si128((__m128i*)bytes, block);

    printf("%s: ", label);
    for (int i = 0; i < 16; i++)
        printf("%02X", bytes[i]);
    printf("\n");
}

// Macro for key expansion
#define AES_KEY_EXPAND(round, rcon)                     \
    assist = _mm_aeskeygenassist_si128(temp, rcon);     \
    assist = _mm_shuffle_epi32(assist, 0xff);           \
    temp = _mm_xor_si128(temp, _mm_slli_si128(temp, 4));\
    temp = _mm_xor_si128(temp, _mm_slli_si128(temp, 4));\
    temp = _mm_xor_si128(temp, _mm_slli_si128(temp, 4));\
    temp = _mm_xor_si128(temp, assist);                 \
    round_keys[round] = temp;

// Key expansion
void key_expansion(unsigned char* key, __m128i* round_keys) {
    round_keys[0] = _mm_loadu_si128((__m128i*)key);
    __m128i temp = round_keys[0];
    __m128i assist;

    AES_KEY_EXPAND(1, 0x01);
    AES_KEY_EXPAND(2, 0x02);
    AES_KEY_EXPAND(3, 0x04);
    AES_KEY_EXPAND(4, 0x08);
    AES_KEY_EXPAND(5, 0x10);
    AES_KEY_EXPAND(6, 0x20);
    AES_KEY_EXPAND(7, 0x40);
    AES_KEY_EXPAND(8, 0x80);
    AES_KEY_EXPAND(9, 0x1B);
    AES_KEY_EXPAND(10, 0x36);
}

// Encrypt one block
__m128i aes_encrypt(__m128i block, __m128i* round_keys) {
    block = _mm_xor_si128(block, round_keys[0]);

    for (int i = 1; i < 10; i++)
        block = _mm_aesenc_si128(block, round_keys[i]);

    block = _mm_aesenclast_si128(block, round_keys[10]);

    return block;
}

int main() {

    unsigned char key[16] = {
        0x00,0x01,0x02,0x03,
        0x04,0x05,0x06,0x07,
        0x08,0x09,0x0A,0x0B,
        0x0C,0x0D,0x0E,0x0F
    };

    unsigned char plaintext_bytes[16] = {
        0x00,0x11,0x22,0x33,
        0x44,0x55,0x66,0x77,
        0x88,0x99,0xAA,0xBB,
        0xCC,0xDD,0xEE,0xFF
    };

    __m128i round_keys[11];
    key_expansion(key, round_keys);

    __m128i plaintext = _mm_loadu_si128((__m128i*)plaintext_bytes);

    print_block("Plaintext ", plaintext);

    const int N = 1000000;   // 1 million encryptions
    __m128i ciphertext;

    LARGE_INTEGER freq, start, end;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);

    for (int i = 0; i < N; i++) {
        ciphertext = aes_encrypt(plaintext, round_keys);
    }

    QueryPerformanceCounter(&end);

    print_block("Ciphertext", ciphertext);

    double time_taken = (double)(end.QuadPart - start.QuadPart) / freq.QuadPart;
    printf("Total Time: %.6f seconds\n", time_taken);
    printf("Average Time per encryption: %.9f seconds\n", time_taken / N);

    return 0;
}
