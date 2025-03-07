#include "../include/simd_examples.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <omp.h>

// Basic vector addition without SIMD explicitly enabled
double scalarVectorAddition(const std::vector<double>& a, const std::vector<double>& b, std::vector<double>& c) {
    auto start = std::chrono::high_resolution_clock::now();
    
    const size_t size = a.size();
    for (size_t i = 0; i < size; ++i) {
        c[i] = a[i] + b[i];
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

// Vector addition with OpenMP SIMD directive
double simdVectorAddition(const std::vector<double>& a, const std::vector<double>& b, std::vector<double>& c) {
    auto start = std::chrono::high_resolution_clock::now();
    
    const size_t size = a.size();
    
    // Using OpenMP SIMD directive to vectorize the loop
    #pragma omp simd
    for (size_t i = 0; i < size; ++i) {
        c[i] = a[i] + b[i];
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

// Basic vector addition entry point
double basicVectorAddition(const std::vector<double>& a, const std::vector<double>& b, std::vector<double>& c, bool useSimd) {
    if (useSimd) {
        return simdVectorAddition(a, b, c);
    } else {
        return scalarVectorAddition(a, b, c);
    }
}

// Run a comprehensive demo of basic SIMD operations
void runBasicSIMD() {
    std::cout << "\n=== Basic SIMD Vectorization Demo ===" << std::endl;
    
    // Vector size
    const size_t size = 10000000; // 10 million elements
    
    std::cout << "Initializing vectors with " << size << " elements..." << std::endl;
    
    // Initialize input vectors
    std::vector<double> a(size, 1.0);
    std::vector<double> b(size, 2.0);
    std::vector<double> c_scalar(size, 0.0);
    std::vector<double> c_simd(size, 0.0);
    
    // Warm up the cache
    for (size_t i = 0; i < 100; ++i) {
        c_scalar[i] = a[i] + b[i];
        c_simd[i] = a[i] + b[i];
    }
    
    std::cout << "\nRunning scalar (non-vectorized) addition..." << std::endl;
    double timeScalar = scalarVectorAddition(a, b, c_scalar);
    
    std::cout << "Running SIMD (vectorized) addition..." << std::endl;
    double timeSimd = simdVectorAddition(a, b, c_simd);
    
    // Verify results
    bool resultsMatch = true;
    for (size_t i = 0; i < size; ++i) {
        if (std::abs(c_scalar[i] - c_simd[i]) > 1e-10) {
            resultsMatch = false;
            std::cout << "Mismatch at index " << i << ": "
                      << c_scalar[i] << " vs " << c_simd[i] << std::endl;
            break;
        }
    }
    
    // Calculate speedup
    double speedup = timeScalar / timeSimd;
    
    // Print results
    std::cout << "\n=== Vector Addition Performance Results ===" << std::endl;
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "Vector size: " << size << " elements" << std::endl;
    std::cout << "Scalar execution time: " << timeScalar << " ms" << std::endl;
    std::cout << "SIMD execution time: " << timeSimd << " ms" << std::endl;
    std::cout << "Speedup: " << speedup << "x" << std::endl;
    std::cout << "Results verification: " << (resultsMatch ? "PASSED" : "FAILED") << std::endl;
    
    // Check the first and last elements for visual verification
    std::cout << "\nSample values:" << std::endl;
    std::cout << "First element: a[0]=" << a[0] << ", b[0]=" << b[0] 
              << ", c_scalar[0]=" << c_scalar[0] << ", c_simd[0]=" << c_simd[0] << std::endl;
    std::cout << "Last element: a[" << size-1 << "]=" << a[size-1] << ", b[" << size-1 << "]=" << b[size-1]
              << ", c_scalar[" << size-1 << "]=" << c_scalar[size-1] 
              << ", c_simd[" << size-1 << "]=" << c_simd[size-1] << std::endl;
    
    // SIMD vectorization explanation
    std::cout << "\n=== Understanding SIMD Vectorization ===" << std::endl;
    std::cout << "What happened behind the scenes:" << std::endl;
    std::cout << "1. Scalar version: Processes one element at a time" << std::endl;
    std::cout << "   a[0] + b[0], then a[1] + b[1], then a[2] + b[2], ..." << std::endl;
    std::cout << "2. SIMD version: Processes multiple elements in parallel" << std::endl;
    
    // Explain what happened based on the CPU's SIMD width
    if (speedup > 1.5) {
        std::cout << "   The compiler successfully vectorized the loop!" << std::endl;
        std::cout << "   Using 128-bit registers: 2 doubles processed per instruction" << std::endl;
        std::cout << "   or 256-bit registers: 4 doubles processed per instruction (AVX)" << std::endl;
    } else {
        std::cout << "   The speedup was minimal, which could indicate:" << std::endl;
        std::cout << "   - The compiler may not have vectorized the loop effectively" << std::endl;
        std::cout << "   - There might be memory bandwidth limitations" << std::endl;
        std::cout << "   - The operation is too simple, so overhead dominates" << std::endl;
    }
    
    std::cout << "\nNote: The actual SIMD instructions used depend on your CPU's" << std::endl;
    std::cout << "      supported instruction sets (SSE, AVX, AVX2, etc.)" << std::endl;
    std::cout << "      For modern CPUs, you might see even better performance" << std::endl;
    std::cout << "      with larger vectors or more complex computations." << std::endl;
}