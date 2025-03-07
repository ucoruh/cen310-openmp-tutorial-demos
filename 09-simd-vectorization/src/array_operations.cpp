#include "../include/simd_examples.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <random>
#include <omp.h>
#include <cassert>

// Element-wise array multiplication without SIMD
void scalarArrayMultiply(const std::vector<float>& a, const std::vector<float>& b, std::vector<float>& c) {
    const int size = static_cast<int>(a.size());
    for (int i = 0; i < size; ++i) {
        c[i] = a[i] * b[i];
    }
}

// Element-wise array multiplication with SIMD
void simdArrayMultiply(const std::vector<float>& a, const std::vector<float>& b, std::vector<float>& c) {
    const int size = static_cast<int>(a.size());
    
    // Ensure we tell the compiler the arrays don't alias
    const float* __restrict a_ptr = a.data();
    const float* __restrict b_ptr = b.data();
    float* __restrict c_ptr = c.data();
    
    #pragma omp simd
    for (int i = 0; i < size; ++i) {
        c_ptr[i] = a_ptr[i] * b_ptr[i];
    }
}

// Array reduction without SIMD
float scalarArrayReduction(const std::vector<float>& a) {
    float sum = 0.0f;
    const int size = static_cast<int>(a.size());
    
    for (int i = 0; i < size; ++i) {
        sum += a[i];
    }
    
    return sum;
}

// Array reduction with SIMD
float simdArrayReduction(const std::vector<float>& a) {
    float sum = 0.0f;
    const int size = static_cast<int>(a.size());
    
    // Use const pointer to help vectorization
    const float* __restrict a_ptr = a.data();
    
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < size; ++i) {
        sum += a_ptr[i];
    }
    
    return sum;
}

// Element-wise array multiply function exposed to other modules
void arrayElementWiseMultiply(const std::vector<float>& a, const std::vector<float>& b, std::vector<float>& c, bool useSimd) {
    // Make sure arrays are the same size
    assert(a.size() == b.size() && a.size() == c.size());
    
    if (useSimd) {
        simdArrayMultiply(a, b, c);
    } else {
        scalarArrayMultiply(a, b, c);
    }
}

// Array reduction function exposed to other modules
float arrayReduction(const std::vector<float>& a, bool useSimd) {
    if (useSimd) {
        return simdArrayReduction(a);
    } else {
        return scalarArrayReduction(a);
    }
}

// Run array operations demo
void runArrayOperations() {
    std::cout << "\n=== Array Operations with SIMD Demo ===" << std::endl;
    
    // Vector size
    const int size = 50000000; // 50 million elements
    
    std::cout << "Initializing arrays with " << size << " elements..." << std::endl;
    
    // Initialize input vectors with random data
    std::vector<float> a(size);
    std::vector<float> b(size);
    std::vector<float> c_scalar(size, 0.0f);
    std::vector<float> c_simd(size, 0.0f);
    
    // Initialize random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    
    // Fill arrays with random values
    for (int i = 0; i < size; ++i) {
        a[i] = dis(gen);
        b[i] = dis(gen);
    }
    
    // Measure element-wise multiplication performance
    std::cout << "\n--- Element-wise Multiplication ---" << std::endl;
    
    // Scalar multiplication
    auto start = std::chrono::high_resolution_clock::now();
    scalarArrayMultiply(a, b, c_scalar);
    auto end = std::chrono::high_resolution_clock::now();
    double timeScalarMultiply = std::chrono::duration<double, std::milli>(end - start).count();
    
    // SIMD multiplication
    start = std::chrono::high_resolution_clock::now();
    simdArrayMultiply(a, b, c_simd);
    end = std::chrono::high_resolution_clock::now();
    double timeSimdMultiply = std::chrono::duration<double, std::milli>(end - start).count();
    
    // Calculate speedup
    double speedupMultiply = timeScalarMultiply / timeSimdMultiply;
    
    // Verify results
    bool multiplyResultsMatch = true;
    for (int i = 0; i < size; ++i) {
        if (std::abs(c_scalar[i] - c_simd[i]) > 1e-5f) {
            multiplyResultsMatch = false;
            std::cout << "Multiplication mismatch at index " << i << ": "
                      << c_scalar[i] << " vs " << c_simd[i] << std::endl;
            break;
        }
    }
    
    // Print multiplication results
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "Scalar multiply time: " << timeScalarMultiply << " ms" << std::endl;
    std::cout << "SIMD multiply time: " << timeSimdMultiply << " ms" << std::endl;
    std::cout << "Speedup: " << speedupMultiply << "x" << std::endl;
    std::cout << "Results verification: " << (multiplyResultsMatch ? "PASSED" : "FAILED") << std::endl;
    
    // Measure reduction performance
    std::cout << "\n--- Array Reduction (Sum) ---" << std::endl;
    
    // Scalar reduction
    start = std::chrono::high_resolution_clock::now();
    float sumScalar = scalarArrayReduction(a);
    end = std::chrono::high_resolution_clock::now();
    double timeScalarReduction = std::chrono::duration<double, std::milli>(end - start).count();
    
    // SIMD reduction
    start = std::chrono::high_resolution_clock::now();
    float sumSimd = simdArrayReduction(a);
    end = std::chrono::high_resolution_clock::now();
    double timeSimdReduction = std::chrono::duration<double, std::milli>(end - start).count();
    
    // Calculate speedup
    double speedupReduction = timeScalarReduction / timeSimdReduction;
    
    // Verify results
    bool reductionResultsMatch = (std::abs(sumScalar - sumSimd) < 1e-2f);
    
    // Print reduction results
    std::cout << "Scalar reduction time: " << timeScalarReduction << " ms" << std::endl;
    std::cout << "SIMD reduction time: " << timeSimdReduction << " ms" << std::endl;
    std::cout << "Speedup: " << speedupReduction << "x" << std::endl;
    std::cout << "Results verification: " << (reductionResultsMatch ? "PASSED" : "FAILED") << std::endl;
    std::cout << "Scalar sum: " << sumScalar << ", SIMD sum: " << sumSimd << std::endl;
    std::cout << "Difference: " << std::abs(sumScalar - sumSimd) << std::endl;
    
    // SIMD explanation
    std::cout << "\n=== SIMD Array Operations Explained ===" << std::endl;
    
    std::cout << "1. Element-wise Multiplication:" << std::endl;
    std::cout << "   Scalar: Processes one pair of elements at a time" << std::endl;
    std::cout << "   SIMD: Processes multiple pairs in parallel (4 or 8 pairs per instruction)" << std::endl;
    
    std::cout << "\n2. Array Reduction:" << std::endl;
    std::cout << "   Scalar: Accumulates one value at a time into sum" << std::endl;
    std::cout << "   SIMD: Uses parallel reduction techniques:" << std::endl;
    std::cout << "     - First accumulates in multiple partial sums" << std::endl;
    std::cout << "     - Then combines these partial sums" << std::endl;
    
    std::cout << "\nNote: Small differences in reduction results are expected due to" << std::endl;
    std::cout << "      different summation order affecting floating-point precision." << std::endl;
    std::cout << "      This is normal and not an indication of incorrect calculation." << std::endl;
}