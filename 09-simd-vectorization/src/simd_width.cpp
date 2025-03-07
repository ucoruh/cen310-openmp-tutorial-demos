#include "../include/simd_examples.h"
#include "../include/cpu_features.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <random>
#include <omp.h>

// Default vector operation without explicit SIMD width
void defaultWidthVectorOperation(const float* __restrict a, const float* __restrict b, float* __restrict c, int size) {
    #pragma omp simd
    for (int i = 0; i < size; ++i) {
        c[i] = a[i] * b[i] + a[i] / (b[i] + 0.01f); // Add small value to avoid division by zero
    }
}

// Vector operation with specific SIMD width hint
void explicitWidthVectorOperation(const float* __restrict a, const float* __restrict b, float* __restrict c, int size, int simdWidth) {
    int simdLength = simdWidth / (8 * sizeof(float)); // Number of floats per SIMD register
    
    // Use safelen clause to hint about SIMD width
    #pragma omp simd safelen(simdLength)
    for (int i = 0; i < size; ++i) {
        c[i] = a[i] * b[i] + a[i] / (b[i] + 0.01f);
    }
}

// Vector operation with remainder handling for non-multiple sizes
void remainderHandlingVectorOperation(const float* __restrict a, const float* __restrict b, float* __restrict c, int size, int simdWidth) {
    int simdLength = simdWidth / (8 * sizeof(float)); // Number of floats per SIMD register
    
    // Calculate the largest multiple of simdLength less than or equal to size
    int mainPortionSize = (size / simdLength) * simdLength;
    
    // Process the main portion that's a multiple of SIMD width
    #pragma omp simd safelen(simdLength)
    for (int i = 0; i < mainPortionSize; ++i) {
        c[i] = a[i] * b[i] + a[i] / (b[i] + 0.01f);
    }
    
    // Handle the remainder portion sequentially
    for (int i = mainPortionSize; i < size; ++i) {
        c[i] = a[i] * b[i] + a[i] / (b[i] + 0.01f);
    }
}

// Function to run vector operation with different SIMD widths
void vectorizeWithDifferentWidths(int simdWidth) {
    const int size = 50000000; // 50 million elements
    
    // Allocate and initialize arrays
    std::vector<float> a(size);
    std::vector<float> b(size);
    std::vector<float> c(size);
    
    // Initialize with random data
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(1.0f, 2.0f); // Avoid division by zero
    
    for (int i = 0; i < size; ++i) {
        a[i] = dis(gen);
        b[i] = dis(gen);
    }
    
    // Warm up
    for (int i = 0; i < 1000; ++i) {
        c[i] = a[i] * b[i] + a[i] / b[i];
    }
    
    // Measure execution time with default width
    auto start = std::chrono::high_resolution_clock::now();
    defaultWidthVectorOperation(a.data(), b.data(), c.data(), size);
    auto end = std::chrono::high_resolution_clock::now();
    double defaultTime = std::chrono::duration<double, std::milli>(end - start).count();
    
    // Measure execution time with explicit width
    start = std::chrono::high_resolution_clock::now();
    explicitWidthVectorOperation(a.data(), b.data(), c.data(), size, simdWidth);
    end = std::chrono::high_resolution_clock::now();
    double explicitTime = std::chrono::duration<double, std::milli>(end - start).count();
    
    // Measure execution time with remainder handling
    start = std::chrono::high_resolution_clock::now();
    remainderHandlingVectorOperation(a.data(), b.data(), c.data(), size, simdWidth);
    end = std::chrono::high_resolution_clock::now();
    double remainderTime = std::chrono::duration<double, std::milli>(end - start).count();
    
    // Print results
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "Default SIMD width execution time: " << defaultTime << " ms" << std::endl;
    std::cout << "Explicit SIMD width (" << simdWidth << " bits) execution time: " << explicitTime << " ms" << std::endl;
    std::cout << "SIMD with remainder handling execution time: " << remainderTime << " ms" << std::endl;
    
    // Calculate speedups
    double defaultVsExplicitSpeedup = defaultTime / explicitTime;
    double defaultVsRemainderSpeedup = defaultTime / remainderTime;
    std::cout << "Speedup with explicit width: " << defaultVsExplicitSpeedup << "x" << std::endl;
    std::cout << "Speedup with remainder handling: " << defaultVsRemainderSpeedup << "x" << std::endl;
}

// Run SIMD width adaptation demo
void runSIMDWidth() {
    std::cout << "\n=== SIMD Width Adaptation Demo ===" << std::endl;
    
    // Get the optimal SIMD width for the current CPU
    int simdWidth = getOptimalSIMDWidth();
    std::cout << "Detected optimal SIMD width: " << simdWidth << " bits" << std::endl;
    
    // Run tests with different SIMD widths
    std::cout << "\n--- Testing with 128-bit SIMD Width (SSE) ---" << std::endl;
    vectorizeWithDifferentWidths(128);
    
    if (simdWidth >= 256) {
        std::cout << "\n--- Testing with 256-bit SIMD Width (AVX) ---" << std::endl;
        vectorizeWithDifferentWidths(256);
    }
    
    if (simdWidth >= 512) {
        std::cout << "\n--- Testing with 512-bit SIMD Width (AVX-512) ---" << std::endl;
        vectorizeWithDifferentWidths(512);
    }
    
    // Run test with the optimal width for this CPU
    std::cout << "\n--- Testing with Optimal SIMD Width (" << simdWidth << " bits) ---" << std::endl;
    vectorizeWithDifferentWidths(simdWidth);
    
    // SIMD width explanation
    std::cout << "\n=== Understanding SIMD Width Adaptation ===" << std::endl;
    
    std::cout << "SIMD width refers to the size of vector registers used for parallel operations:" << std::endl;
    std::cout << "- 128-bit SIMD (SSE): 4 single-precision floats or 2 doubles per operation" << std::endl;
    std::cout << "- 256-bit SIMD (AVX): 8 single-precision floats or 4 doubles per operation" << std::endl;
    std::cout << "- 512-bit SIMD (AVX-512): 16 single-precision floats or 8 doubles per operation" << std::endl;
    
    std::cout << "\nAdapting to different SIMD widths:" << std::endl;
    std::cout << "1. Runtime detection allows using the widest supported instructions" << std::endl;
    std::cout << "2. The safelen clause hints the compiler about vector length" << std::endl;
    std::cout << "3. Remainder handling ensures correct results for arbitrary sizes" << std::endl;
    
    std::cout << "\nImportant considerations:" << std::endl;
    std::cout << "- Wider SIMD doesn't always mean better performance" << std::endl;
    std::cout << "- AVX/AVX2/AVX-512 can have different clock frequencies (frequency scaling)" << std::endl;
    std::cout << "- Memory bandwidth can become the bottleneck, not computation" << std::endl;
    std::cout << "- Algorithm complexity affects vectorization efficiency" << std::endl;
    
    std::cout << "\nBest practices:" << std::endl;
    std::cout << "- Always benchmark with different SIMD widths" << std::endl;
    std::cout << "- Consider writing multiple implementations for different CPU capabilities" << std::endl;
    std::cout << "- Use runtime detection to select the optimal implementation" << std::endl;
}