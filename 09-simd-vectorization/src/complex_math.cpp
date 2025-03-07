#include "../include/simd_examples.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <cmath>
#include <random>
#include <omp.h>

// Define M_PI if not already defined
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Transcendental math operations without SIMD
void scalarTranscendental(const std::vector<double>& input, std::vector<double>& output) {
    const size_t size = input.size();
    for (size_t i = 0; i < size; ++i) {
        // Combine several transcendental functions: sin(x) + cos(x*2) / exp(x/4)
        double x = input[i];
        output[i] = std::sin(x) + std::cos(x * 2.0) / std::exp(x / 4.0);
    }
}

// Transcendental math operations with SIMD
void simdTranscendental(const std::vector<double>& input, std::vector<double>& output) {
    const size_t size = input.size();
    
    #pragma omp simd
    for (size_t i = 0; i < size; ++i) {
        // Same operations as above, but with SIMD directive
        double x = input[i];
        output[i] = std::sin(x) + std::cos(x * 2.0) / std::exp(x / 4.0);
    }
}

// Custom polynomial approximation of sin function - used to show custom math with SIMD
// This is a simplified Taylor series approximation
double customSin(double x) {
    // Normalize to -PI..PI range
    while (x > M_PI) x -= 2 * M_PI;
    while (x < -M_PI) x += 2 * M_PI;
    
    // Taylor series approximation: x - x^3/3! + x^5/5! - x^7/7!
    double x2 = x * x;
    double x3 = x2 * x;
    double x5 = x3 * x2;
    double x7 = x5 * x2;
    
    return x - (x3 / 6.0) + (x5 / 120.0) - (x7 / 5040.0);
}

// Custom math without SIMD
void scalarCustomMath(const std::vector<double>& input, std::vector<double>& output) {
    const size_t size = input.size();
    for (size_t i = 0; i < size; ++i) {
        // Use our custom sin function
        double x = input[i];
        output[i] = customSin(x);
    }
}

// Custom math with SIMD
void simdCustomMath(const std::vector<double>& input, std::vector<double>& output) {
    const size_t size = input.size();
    
    #pragma omp simd
    for (size_t i = 0; i < size; ++i) {
        // Use our custom sin function with SIMD
        double x = input[i];
        output[i] = customSin(x);
    }
}

// Compute transcendental functions (exposed to other modules)
void computeTranscendental(std::vector<double>& input, std::vector<double>& output, bool useSimd) {
    if (useSimd) {
        simdTranscendental(input, output);
    } else {
        scalarTranscendental(input, output);
    }
}

// Run complex math demo
void runComplexMath() {
    std::cout << "\n=== Complex Math Functions with SIMD Demo ===" << std::endl;
    
    // Vector size
    const size_t size = 20000000; // 20 million elements
    
    std::cout << "Initializing arrays with " << size << " elements..." << std::endl;
    
    // Initialize random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis(-M_PI, M_PI);
    
    // Initialize input vector with random angles
    std::vector<double> input(size);
    for (auto& val : input) {
        val = dis(gen);
    }
    
    // Output vectors
    std::vector<double> output_scalar_trans(size, 0.0);
    std::vector<double> output_simd_trans(size, 0.0);
    std::vector<double> output_scalar_custom(size, 0.0);
    std::vector<double> output_simd_custom(size, 0.0);
    
    // Measure standard transcendental performance
    std::cout << "\n--- Standard Transcendental Functions ---" << std::endl;
    
    // Scalar transcendental
    auto start = std::chrono::high_resolution_clock::now();
    scalarTranscendental(input, output_scalar_trans);
    auto end = std::chrono::high_resolution_clock::now();
    double timeScalarTrans = std::chrono::duration<double, std::milli>(end - start).count();
    
    // SIMD transcendental
    start = std::chrono::high_resolution_clock::now();
    simdTranscendental(input, output_simd_trans);
    end = std::chrono::high_resolution_clock::now();
    double timeSimdTrans = std::chrono::duration<double, std::milli>(end - start).count();
    
    // Calculate speedup
    double speedupTrans = timeScalarTrans / timeSimdTrans;
    
    // Verify transcendental results
    bool transResultsMatch = true;
    for (size_t i = 0; i < size; ++i) {
        if (std::abs(output_scalar_trans[i] - output_simd_trans[i]) > 1e-10) {
            transResultsMatch = false;
            std::cout << "Transcendental mismatch at index " << i << ": "
                      << output_scalar_trans[i] << " vs " << output_simd_trans[i] << std::endl;
            break;
        }
    }
    
    // Print transcendental results
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "Scalar transcendental time: " << timeScalarTrans << " ms" << std::endl;
    std::cout << "SIMD transcendental time: " << timeSimdTrans << " ms" << std::endl;
    std::cout << "Speedup: " << speedupTrans << "x" << std::endl;
    std::cout << "Results verification: " << (transResultsMatch ? "PASSED" : "FAILED") << std::endl;
    
    // Measure custom math performance
    std::cout << "\n--- Custom Math Functions ---" << std::endl;
    
    // Scalar custom math
    start = std::chrono::high_resolution_clock::now();
    scalarCustomMath(input, output_scalar_custom);
    end = std::chrono::high_resolution_clock::now();
    double timeScalarCustom = std::chrono::duration<double, std::milli>(end - start).count();
    
    // SIMD custom math
    start = std::chrono::high_resolution_clock::now();
    simdCustomMath(input, output_simd_custom);
    end = std::chrono::high_resolution_clock::now();
    double timeSimdCustom = std::chrono::duration<double, std::milli>(end - start).count();
    
    // Calculate speedup
    double speedupCustom = timeScalarCustom / timeSimdCustom;
    
    // Verify custom math results
    bool customResultsMatch = true;
    for (size_t i = 0; i < size; ++i) {
        if (std::abs(output_scalar_custom[i] - output_simd_custom[i]) > 1e-10) {
            customResultsMatch = false;
            std::cout << "Custom math mismatch at index " << i << ": "
                      << output_scalar_custom[i] << " vs " << output_simd_custom[i] << std::endl;
            break;
        }
    }
    
    // Print custom math results
    std::cout << "Scalar custom math time: " << timeScalarCustom << " ms" << std::endl;
    std::cout << "SIMD custom math time: " << timeSimdCustom << " ms" << std::endl;
    std::cout << "Speedup: " << speedupCustom << "x" << std::endl;
    std::cout << "Results verification: " << (customResultsMatch ? "PASSED" : "FAILED") << std::endl;
    
    // Compare standard vs custom sin implementation
    std::cout << "\n--- Standard vs Custom Sin Implementation ---" << std::endl;
    double standardSin = std::sin(0.5);
    double customSinResult = customSin(0.5);
    double sinDifference = std::abs(standardSin - customSinResult);
    
    std::cout << "Standard sin(0.5): " << standardSin << std::endl;
    std::cout << "Custom sin(0.5): " << customSinResult << std::endl;
    std::cout << "Absolute difference: " << sinDifference << std::endl;
    std::cout << "Relative error: " << (sinDifference / std::abs(standardSin)) * 100.0 << "%" << std::endl;
    
    // Vectorization explanation
    std::cout << "\n=== SIMD Math Vectorization Explained ===" << std::endl;
    
    std::cout << "1. Standard Transcendental Functions:" << std::endl;
    std::cout << "   - SIMD libraries implement optimized versions of math functions" << std::endl;
    std::cout << "   - Functions like sin, cos, exp are vectorized in hardware/library" << std::endl;
    
    if (speedupTrans > 1.5) {
        std::cout << "   - Your CPU successfully vectorized these operations!" << std::endl;
    } else {
        std::cout << "   - The speedup was modest, possibly because:" << std::endl;
        std::cout << "     * The math library already uses SIMD internally" << std::endl;
        std::cout << "     * Transcendental functions are complex, limiting parallelism" << std::endl;
    }
    
    std::cout << "\n2. Custom Math Functions:" << std::endl;
    std::cout << "   - Polynomial approximations are well-suited for SIMD" << std::endl;
    std::cout << "   - Our custom sin uses basic arithmetic operations:" << std::endl;
    std::cout << "     * Addition/subtraction" << std::endl;
    std::cout << "     * Multiplication/division" << std::endl;
    std::cout << "     * No branching in the core calculation" << std::endl;
    
    if (speedupCustom > speedupTrans) {
        std::cout << "   - Custom math showed better speedup than standard functions!" << std::endl;
        std::cout << "   - This demonstrates how simple arithmetic is efficient with SIMD" << std::endl;
    }
    
    std::cout << "\nNote: When writing custom math functions for SIMD:" << std::endl;
    std::cout << "      - Minimize branches (if/else) inside loop bodies" << std::endl;
    std::cout << "      - Use polynomial approximations where possible" << std::endl;
    std::cout << "      - Consider the accuracy-performance tradeoff" << std::endl;
}