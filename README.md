# Microbench - CPU Instruction Performance Benchmarking Suite

## Overview

Microbench is a collection of microbenchmarks designed to measure and analyze the performance characteristics of modern x86 CPU instructions on Windows platforms. The suite focuses on SIMD (Single Instruction, Multiple Data) operations, cryptographic hardware acceleration, and memory access patterns.

**Author:** Shruthi Sriram  
**Platform:** Windows (x86/x64)  
**Language:** C with Intel Intrinsics  
**Development Period:** March 2025 - Present

---

## Project Architecture

### Directory Structure

```
Microbench/
├── aes/              # AES-NI encryption benchmarks
├── avx_add/          # AVX addition operations
├── avx_div/          # AVX division operations
├── avx_mul/          # AVX multiplication (not implemented)
├── avx_sub/          # AVX subtraction (not implemented)
├── cpu_bound/        # CPU-intensive workload benchmarks
├── fma_add/          # Fused Multiply-Add addition
├── fma_sub/          # Fused Multiply-Add subtraction
├── memory_bound/     # Memory-intensive workload benchmarks
└── sha_ni/           # SHA-1 hardware hashing benchmarks
```

### Common Design Patterns

All benchmarks follow a consistent architecture:

1. **High-Resolution Timing**
   - Uses Windows `QueryPerformanceCounter` API
   - Provides microsecond-level precision
   - Common `get_time()` function across all benchmarks

2. **Memory Management**
   - 32-byte aligned memory allocation using `_aligned_malloc()`
   - Required for optimal SIMD performance
   - Proper cleanup with `_aligned_free()`

3. **Array Processing**
   - Float arrays for computational workloads
   - Random initialization with `init_array()`
   - Chunked processing (typically 8 elements per SIMD operation)

4. **Performance Measurement**
   - Start/end time capture around critical sections
   - Multiple iterations (typically 100) for averaging
   - Comparison between scalar and vectorized implementations

---

## Benchmark Categories

### 1. AVX (Advanced Vector Extensions) Benchmarks

AVX enables parallel processing of multiple data elements in a single instruction using 256-bit registers (8 floats).

#### **AVX Addition** ([avx_add/avx_add.c](avx_add/avx_add.c))
- **Array Size:** 4 MB (4×1024×1024 floats)
- **Chunk Size:** 8 floats
- **Iterations:** 100

**Logic:**
```
Scalar:  c[i] = a[i] + b[i]  (one element at a time)
AVX:     c[i:i+7] = a[i:i+7] + b[i:i+7]  (8 elements simultaneously)
```

**Key Intrinsics:**
- `_mm256_loadu_ps()` - Load 8 floats from unaligned memory
- `_mm256_add_ps()` - Add 8 floats in parallel
- `_mm256_storeu_ps()` - Store 8 floats to unaligned memory

#### **AVX Division** ([avx_div/avx_div.c](avx_div/avx_div.c))
- **Array Size:** 4 MB (4×1024×1024 floats)
- **Purpose:** Measure division performance (typically slower than addition/multiplication)

**Logic:**
```
Scalar:  c[i] = a[i] / b[i]  (with zero-check)
AVX:     c[i:i+7] = a[i:i+7] / b[i:i+7]  (8 parallel divisions)
```

**Key Intrinsics:**
- `_mm256_div_ps()` - Divide 8 floats in parallel

---

### 2. FMA (Fused Multiply-Add) Benchmarks

FMA combines multiplication and addition into a single instruction with higher accuracy and performance.

#### **FMA Addition** ([fma_add/fma_add.c](fma_add/fma_add.c))
- **Array Size:** 4 MB
- **Operation:** `d[i] = a[i] × b[i] + c[i]`

**Logic:**
- Performs multiply-add in one atomic operation
- Reduces rounding errors compared to separate mul + add
- Single instruction instead of two

**Key Intrinsics:**
- `_mm256_fmadd_ps()` - Fused multiply-add: (a × b) + c

#### **FMA Subtraction** ([fma_sub/fma_sub.c](fma_sub/fma_sub.c))
- **Array Size:** 1 KB (1024 floats)
- **Operation:** `d[i] = a[i] × b[i] - c[i]`

**Key Intrinsics:**
- `_mm256_fmsub_ps()` - Fused multiply-subtract: (a × b) - c

---

### 3. CPU vs Memory Bound Benchmarks

These benchmarks demonstrate the distinction between computation-limited and memory-limited workloads.

#### **CPU Bound** ([cpu_bound/cpu_bound.c](cpu_bound/cpu_bound.c))
- **Array Size:** 1 KB (1024 floats)
- **Purpose:** Small dataset fits in CPU cache, emphasizing computation speed

**Logic:**
```
Small arrays → Data resides in L1/L2 cache
Bottleneck: CPU instruction throughput
Speedup from AVX: ~8x theoretical (8 floats/instruction)
```

**Measurements:**
- Scalar addition time
- AVX addition time
- Speedup ratio

#### **Memory Bound** ([memory_bound/memory_bound.c](memory_bound/memory_bound.c))
- **Array Size:** 50 MB (50×1024×1024 floats)
- **Purpose:** Large dataset exceeds cache, memory bandwidth becomes bottleneck

**Logic:**
```
Large arrays → Cannot fit in cache
Bottleneck: Memory bandwidth (RAM access)
Speedup from AVX: < 8x (memory bandwidth limited)
```

**Measurements:**
1. **Total Time:** Initialization + computation
2. **CPU Time:** Computation only (second run, data in cache)
3. **Memory Time:** Total - CPU (time spent on memory operations)

**Key Insight:** Demonstrates that SIMD benefits diminish when memory bandwidth is the limiting factor.

---

### 4. Cryptographic Hardware Acceleration

Modern CPUs include dedicated instructions for cryptographic operations, providing significant speedup over software implementations.

#### **AES-NI (Advanced Encryption Standard)** ([aes/AES_NI.c](aes/AES_NI.c))
- **Key Size:** 128-bit (16 bytes)
- **Block Size:** 128-bit (16 bytes)
- **Rounds:** 10 (for 128-bit keys)

**Architecture:**
```
Input: Plaintext (16 bytes) + Key (16 bytes)
↓
Key Expansion (generates 11 round keys)
↓
Initial Round Key Addition
↓
9 Main Rounds (AES encryption)
↓
Final Round
↓
Output: Ciphertext (16 bytes)
```

**Key Intrinsics:**
- `_mm_aeskeygenassist_si128()` - Generate round keys
- `_mm_aesenc_si128()` - Perform one encryption round
- `_mm_aesenclast_si128()` - Final encryption round

**Logic:**
1. **Key Expansion:** Derives 11 round keys from the initial key
2. **Encryption:** Applies 10 rounds of AES transformation
3. **Timing:** Measures hardware-accelerated encryption speed

#### **SHA-NI (Secure Hash Algorithm)** ([sha_ni/sha-ni.c](sha_ni/sha-ni.c))
- **Algorithm:** SHA-1
- **Input:** Message block (16 bytes)
- **Output:** Hash value (20 bytes, partial in this example)

**Key Intrinsics:**
- `_mm_sha1rnds4_epu32()` - Perform 4 rounds of SHA-1 compression

**Logic:**
1. Input data is prepared (padded message "abc")
2. Initial hash value loaded (SHA-1 constants)
3. One iteration of SHA-1 compression executed
4. Timing measures hardware hash acceleration

---

## Technical Details

### Memory Alignment

All benchmarks use 32-byte aligned memory allocation:

```c
float* a = (float*)_aligned_malloc(SIZE * sizeof(float), 32);
```

**Rationale:**
- AVX instructions require/benefit from 32-byte alignment
- Aligned loads (`_mm256_load_ps`) are faster than unaligned (`_mm256_loadu_ps`)
- Prevents cache line splits

### Timing Methodology

```c
double get_time() {
    LARGE_INTEGER frequency, counter;
    QueryPerformanceFrequency(&frequency);  // Get timer frequency
    QueryPerformanceCounter(&counter);      // Get current counter
    return (double)counter.QuadPart / frequency.QuadPart;
}
```

**Characteristics:**
- High resolution (~100 nanosecond precision on modern systems)
- Monotonic (doesn't go backwards)
- Not affected by system time changes

### Iteration Strategy

Most benchmarks use 100 iterations to:
1. Warm up CPU caches
2. Allow CPU frequency scaling to stabilize
3. Average out measurement noise
4. Detect performance variability

---

## Performance Considerations

### Expected Speedup Factors

| Benchmark Type | Expected AVX Speedup | Limiting Factor |
|---------------|---------------------|-----------------|
| CPU Bound (small) | 6-8x | CPU throughput |
| Memory Bound (large) | 2-4x | Memory bandwidth |
| AVX Addition | 6-8x | CPU ALU |
| AVX Division | 2-4x | Division latency |
| FMA Operations | 8-10x | Combined ops |
| AES-NI | 10-50x | Dedicated hardware |
| SHA-NI | 8-20x | Dedicated hardware |

### Factors Affecting Performance

1. **CPU Frequency:** Turbo boost can affect single-iteration measurements
2. **Cache State:** Cold vs warm cache significantly impacts results
3. **Memory Bandwidth:** Limits performance for large datasets
4. **Instruction Latency:** Division slower than addition
5. **Throughput vs Latency:** Multiple parallel operations can hide latency

---

## Compilation Requirements

### Required CPU Features
- **AVX/AVX2 support** (Intel Sandy Bridge+, AMD Bulldozer+)
- **FMA3 support** (Intel Haswell+, AMD Piledriver+)
- **AES-NI** (Intel Westmere+, AMD Bulldozer+)
- **SHA Extensions** (Intel Goldmont+, AMD Zen+)

### Compiler Flags (MSVC)
```
/arch:AVX2    - Enable AVX2 instructions
/O2           - Optimize for speed
/fp:fast      - Fast floating-point model
```

### Compiler Flags (GCC/Clang)
```
-mavx2        - Enable AVX2
-mfma         - Enable FMA
-maes         - Enable AES-NI
-msha         - Enable SHA extensions
-O3           - Maximum optimization
```

---

## Usage

### Compiling Individual Benchmarks

**Using MSVC (Visual Studio):**
```cmd
cl /O2 /arch:AVX2 /Fe:avx_add.exe avx_add\avx_add.c
cl /O2 /arch:AVX2 /Fe:cpu_bound.exe cpu_bound\cpu_bound.c
cl /O2 /arch:AVX2 /Fe:aes_ni.exe aes\AES_NI.c
```

**Using GCC/Clang:**
```bash
gcc -O3 -mavx2 -mfma avx_add/avx_add.c -o avx_add.exe
gcc -O3 -mavx2 cpu_bound/cpu_bound.c -o cpu_bound.exe
gcc -O3 -maes -msse4.1 aes/AES_NI.c -o aes_ni.exe
```

### Running Benchmarks

```cmd
cd avx_add
.\avx_add.exe

cd ..\cpu_bound
.\cpu_bound.exe
```

### Interpreting Results

Example output from **cpu_bound.c**:
```
Time taken for scalar addition (CPU time): 0.000015 seconds
Time taken for AVX addition (CPU time): 0.000002 seconds
```

**Analysis:**
- Speedup = 0.000015 / 0.000002 = 7.5x
- Close to theoretical 8x speedup for 8-wide AVX
- Indicates CPU-bound, cache-resident workload

---

## Development Status

| Benchmark | Status | Notes |
|-----------|--------|-------|
| AVX Addition | ✅ Complete | |
| AVX Division | ✅ Complete | |
| AVX Multiplication | ❌ Not Implemented | Directory exists, no code |
| AVX Subtraction | ❌ Not Implemented | Directory exists, no code |
| FMA Addition | ✅ Complete | |
| FMA Subtraction | ✅ Complete | Smaller array (1KB) |
| CPU Bound | ✅ Complete | |
| Memory Bound | ✅ Complete | |
| AES-NI | ✅ Complete | |
| SHA-NI | ✅ Complete | |

---

## Future Enhancements

### Potential Additions
1. **AVX-512 Support** (if available on target CPUs)
2. **AVX Multiplication and Subtraction** benchmarks
3. **SHA-256 benchmarks** (in addition to SHA-1)
4. **Cross-platform support** (Linux, macOS)
5. **Automated test harness** (batch compilation, execution, reporting)
6. **CSV output** for analysis in Excel/Python
7. **Statistical analysis** (min, max, median, standard deviation)
8. **Power consumption measurements** (if hardware supports RAPL)
9. **Multi-threaded benchmarks** (parallel SIMD operations)
10. **Comparison with standard library implementations**

---

## References

### Intel Intrinsics Guide
- https://www.intel.com/content/www/us/en/docs/intrinsics-guide/

### Instruction Set References
- **AVX:** Advanced Vector Extensions (256-bit SIMD)
- **FMA:** Fused Multiply-Add (combined multiply + add)
- **AES-NI:** Hardware AES encryption/decryption
- **SHA-NI:** Hardware SHA hashing

### Performance Optimization Resources
- Intel® 64 and IA-32 Architectures Optimization Reference Manual
- AMD Software Optimization Guide for AMD Family Processors
- Agner Fog's Optimization Manuals

---

## License

Not specified in source files.

---

## Contact

**Author:** Shruthi Sriram  
**Project Name:** Microbench  
**Last Updated:** March 2025
