# SIMD Vectorization Guide

This guide provides a comprehensive explanation of SIMD vectorization concepts, with a focus on using OpenMP directives in C++.

## Table of Contents

1. [Introduction to SIMD](#introduction-to-simd)
2. [SIMD Instruction Sets](#simd-instruction-sets)
3. [OpenMP SIMD Directives](#openmp-simd-directives)
4. [Vectorization Requirements](#vectorization-requirements)
5. [Performance Optimization Strategies](#performance-optimization-strategies)
6. [Common Pitfalls](#common-pitfalls)
7. [Advanced Techniques](#advanced-techniques)

---

## Introduction to SIMD

SIMD (Single Instruction, Multiple Data) is a parallel execution model that allows a single instruction to operate on multiple data elements simultaneously. This is a form of data parallelism that can significantly accelerate computational tasks.

### Why SIMD Matters

In modern computing, SIMD vectorization is crucial for achieving high performance because:

- It utilizes specialized CPU registers and instructions to process multiple data elements at once
- It can provide 2-16x speedup for properly vectorized code
- It complements multi-threading by adding another level of parallelism
- It's available on virtually all modern CPUs

### Conceptual Model

Without SIMD (scalar processing):
```
operation: a + b = c
execution: a[0] + b[0] = c[0], then a[1] + b[1] = c[1], ...
```

With SIMD (vector processing):
```
operation: a + b = c
execution: [a[0],a[1],a[2],a[3]] + [b[0],b[1],b[2],b[3]] = [c[0],c[1],c[2],c[3]]
```

---

## SIMD Instruction Sets

Modern CPUs support various SIMD instruction sets, each with different capabilities and register widths:

### SSE (Streaming SIMD Extensions)
- 128-bit registers (XMM0-XMM7)
- Can process 4 floats or 2 doubles at once
- Available on virtually all x86-64 CPUs

### AVX (Advanced Vector Extensions)
- 256-bit registers (YMM0-YMM15)
- Can process 8 floats or 4 doubles at once
- Available on most CPUs from 2011 onwards

### AVX2
- Extends AVX with more instructions
- Better support for integer operations
- Available on most CPUs from 2013 onwards

### AVX-512
- 512-bit registers (ZMM0-ZMM31)
- Can process 16 floats or 8 doubles at once
- Available on some high-end CPUs from 2016 onwards

---

## OpenMP SIMD Directives

OpenMP provides simple pragmas to enable SIMD vectorization without having to write intrinsics or assembly code.

### Basic SIMD Directive

```cpp
#pragma omp simd
for (int i = 0; i < n; i++) {
    c[i] = a[i] + b[i];
}
```

### SIMD with Reduction

```cpp
float sum = 0.0f;
#pragma omp simd reduction(+:sum)
for (int i = 0; i < n; i++) {
    sum += a[i];
}
```

### SIMD with Alignment Information

```cpp
#pragma omp simd aligned(a,b,c:32)
for (int i = 0; i < n; i++) {
    c[i] = a[i] + b[i];
}
```

### SIMD with Safelen

```cpp
#pragma omp simd safelen(16)
for (int i = 0; i < n; i++) {
    // Operations that work correctly up to 16 elements at once
}
```

### SIMD with Linear Clause

```cpp
#pragma omp simd linear(p:1)
for (int i = 0; i < n; i++) {
    *p = a[i];
    p++;  // Linear pointer increment
}
```

### Combining SIMD with Parallelism

```cpp
#pragma omp parallel for simd
for (int i = 0; i < n; i++) {
    c[i] = a[i] + b[i];
}
```

---

## Vectorization Requirements

Not all loops can be automatically vectorized. For successful vectorization:

### 1. Loop Structure

- Fixed, countable number of iterations
- Single entry and exit points
- No (or minimal) control flow inside loop
- No early exits or breaks

### 2. Memory Access

- Sequential/strided memory access patterns
- No (or minimal) indirect memory accesses
- Aligned memory access when possible
- No (or minimal) data dependencies between iterations

### 3. Operation Types

- Simple arithmetic operations vectorize best
- Some math functions (sin, cos, exp) have vectorized implementations
- Avoid complex control flow (if/else conditions)
- Avoid early exits or returns within the loop

---

## Performance Optimization Strategies

To maximize SIMD performance:

### 1. Memory Alignment

- Align data to SIMD register width boundaries (16, 32, or 64 bytes)
- Use aligned allocation functions:
  ```cpp
  float* data = (float*)_aligned_malloc(size * sizeof(float), 32);
  ```
- Inform the compiler about alignment:
  ```cpp
  #pragma omp simd aligned(data:32)
  ```

### 2. Data Layout

- Use Structure of Arrays (SoA) instead of Array of Structures (AoS)
- Poor layout:
  ```cpp
  struct Particle { float x, y, z, vx, vy, vz; };
  Particle particles[1000];
  ```
- Better layout:
  ```cpp
  struct ParticleSystem {
      float x[1000], y[1000], z[1000];
      float vx[1000], vy[1000], vz[1000];
  };
  ```

### 3. Minimize Dependencies

- Avoid loop-carried dependencies
- Bad (dependency on previous iteration):
  ```cpp
  for (int i = 1; i < n; i++) {
      a[i] = a[i-1] * 0.5f;
  }
  ```
- Good (no dependencies):
  ```cpp
  for (int i = 0; i < n; i++) {
      c[i] = a[i] * b[i];
  }
  ```

### 4. Minimize Control Flow

- Avoid branches inside loops
- Bad:
  ```cpp
  for (int i = 0; i < n; i++) {
      if (a[i] > 0)
          c[i] = a[i] + b[i];
      else
          c[i] = a[i] - b[i];
  }
  ```
- Better (using predication):
  ```cpp
  for (int i = 0; i < n; i++) {
      float add = a[i] + b[i];
      float sub = a[i] - b[i];
      c[i] = (a[i] > 0) ? add : sub;
  }
  ```

### 5. Loop Splitting

Split complex loops into simpler ones that can be vectorized:

```cpp
// Original complex loop
for (int i = 0; i < n; i++) {
    if (a[i] > 0) {
        b[i] = a[i] * 2;
    }
    c[i] = a[i] + 5;
}

// Split into two loops
#pragma omp simd
for (int i = 0; i < n; i++) {
    if (a[i] > 0) {
        b[i] = a[i] * 2;
    }
}

#pragma omp simd
for (int i = 0; i < n; i++) {
    c[i] = a[i] + 5;
}
```

---

## Common Pitfalls

### 1. Aliasing

Pointers that might reference overlapping memory can prevent vectorization:

```cpp
void add(float* a, float* b, float* c, int n) {
    // Compiler might not vectorize this if a, b, and c could overlap
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}
```

Solution: Use the `restrict` keyword (C) or `__restrict` (C++) to indicate non-overlapping pointers:

```cpp
void add(float* __restrict a, float* __restrict b, float* __restrict c, int n) {
    // Now the compiler knows the pointers don't overlap
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}
```

### 2. Function Calls

Function calls inside loops can prevent vectorization:

```cpp
// May not vectorize due to function call
for (int i = 0; i < n; i++) {
    c[i] = some_function(a[i], b[i]);
}
```

Solutions:
- Inline the function
- Use the `inline` keyword
- Use compiler-specific pragmas to force inlining

### 3. Complex Loop Exit Conditions

```cpp
// Hard to vectorize due to early exit
for (int i = 0; i < n; i++) {
    if (a[i] < 0) break;
    c[i] = a[i] + b[i];
}
```

Solution: Restructure the loop or use masks to handle the condition.

### 4. Mixed Precision

```cpp
// Mixed types can reduce vectorization efficiency
for (int i = 0; i < n; i++) {
    float_array[i] = int_array[i] * 2.5f;
}
```

Solution: Use consistent data types or handle conversions carefully.

---

## Advanced Techniques

### 1. Loop Blocking/Tiling

Break large loops into smaller chunks to improve cache utilization:

```cpp
// Original loop
for (int i = 0; i < n; i++) {
    c[i] = a[i] + b[i];
}

// Blocked version
const int block_size = 256;
for (int i = 0; i < n; i += block_size) {
    int end = min(i + block_size, n);
    #pragma omp simd
    for (int j = i; j < end; j++) {
        c[j] = a[j] + b[j];
    }
}
```

### 2. Explicit Vector Length

Specify the vector length based on hardware capabilities:

```cpp
#pragma omp simd simdlen(8)
for (int i = 0; i < n; i++) {
    c[i] = a[i] + b[i];
}
```

### 3. Runtime SIMD Width Adaptation

Detect and adapt to different SIMD capabilities at runtime:

```cpp
void process_vector(float* a, float* b, float* c, int n) {
    // Detect CPU features and choose optimal implementation
    if (cpu_supports_avx512()) {
        process_vector_avx512(a, b, c, n);
    } else if (cpu_supports_avx2()) {
        process_vector_avx2(a, b, c, n);
    } else if (cpu_supports_sse2()) {
        process_vector_sse2(a, b, c, n);
    } else {
        process_vector_scalar(a, b, c, n);
    }
}
```

### 4. Manual Vectorization

In performance-critical code, you might need to use intrinsics for full control:

```cpp
#include <immintrin.h>

void add_avx(float* a, float* b, float* c, int n) {
    for (int i = 0; i < n; i += 8) {
        __m256 va = _mm256_load_ps(&a[i]);
        __m256 vb = _mm256_load_ps(&b[i]);
        __m256 vc = _mm256_add_ps(va, vb);
        _mm256_store_ps(&c[i], vc);
    }
}
```

This approach gives maximum control but is less portable and harder to maintain.

---

By following these guidelines and understanding the principles of SIMD vectorization, you can significantly improve the performance of computational code on modern CPUs.