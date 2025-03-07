#define _USE_MATH_DEFINES
#include <cmath>
#include "../include/simd_examples.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <random>
#include <omp.h>
#include <limits>

// Sequential operation (no parallelism, no SIMD)
void sequentialOperation(const std::vector<double>& a, const std::vector<double>& b, 
                        std::vector<double>& c) {
    const size_t size = a.size();
    for (size_t i = 0; i < size; ++i) {
        double x = a[i];
        double y = b[i];
        
        // Perform a relatively expensive operation to show the benefits of parallelism
        for (int j = 0; j < 20; ++j) {
            x = std::sin(x) + std::cos(y);
            y = std::cos(x) + std::sin(y);
        }
        
        c[i] = x * y;
    }
}

// SIMD-only operation (vectorization but no thread parallelism)
void simdOnlyOperation(const std::vector<double>& a, const std::vector<double>& b, 
                      std::vector<double>& c) {
    const size_t size = a.size();
    
    #pragma omp simd
    for (int i = 0; i < static_cast<int>(size); ++i) {
        double x = a[i];
        double y = b[i];
        
        for (int j = 0; j < 20; ++j) {
            x = std::sin(x) + std::cos(y);
            y = std::cos(x) + std::sin(y);
        }
        
        c[i] = x * y;
    }
}

// Thread-parallel-only operation (no explicit vectorization)
void parallelOnlyOperation(const std::vector<double>& a, const std::vector<double>& b, 
                          std::vector<double>& c, int numThreads) {
    const size_t size = a.size();
    
    #pragma omp parallel for num_threads(numThreads)
    for (int i = 0; i < static_cast<int>(size); ++i) {
        double x = a[i];
        double y = b[i];
        
        for (int j = 0; j < 20; ++j) {
            x = std::sin(x) + std::cos(y);
            y = std::cos(x) + std::sin(y);
        }
        
        c[i] = x * y;
    }
}

// Combined SIMD and thread parallelism (separate directives for OpenMP 2.0 compatibility)
void simdParallelOperation(const std::vector<double>& a, const std::vector<double>& b, 
                          std::vector<double>& c, int numThreads) {
    const size_t size = a.size();
    
    // In OpenMP 2.0 we can't use combined "parallel for simd", so we use nested directives
    #pragma omp parallel num_threads(numThreads)
    {
        #pragma omp for
        for (int i = 0; i < static_cast<int>(size); ++i) {
            double x = a[i];
            double y = b[i];
            
            for (int j = 0; j < 20; ++j) {
                x = std::sin(x) + std::cos(y);
                y = std::cos(x) + std::sin(y);
            }
            
            c[i] = x * y;
        }
    }
}

// SIMD with threads function exposed to other modules
void simdWithThreads(int numThreads, size_t vectorSize) {
    // Initialize vectors
    std::vector<double> a(vectorSize);
    std::vector<double> b(vectorSize);
    std::vector<double> c(vectorSize);
    
    // Initialize with random data
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis(0.0, 1.0);
    
    for (size_t i = 0; i < vectorSize; ++i) {
        a[i] = dis(gen);
        b[i] = dis(gen);
    }
    
    // Warm up
    for (size_t i = 0; i < 1000; ++i) {
        c[i] = a[i] * b[i];
    }
    
    // Measure sequential time
    auto start = std::chrono::high_resolution_clock::now();
    sequentialOperation(a, b, c);
    auto end = std::chrono::high_resolution_clock::now();
    double timeSequential = std::chrono::duration<double, std::milli>(end - start).count();
    
    // Measure SIMD-only time
    start = std::chrono::high_resolution_clock::now();
    simdOnlyOperation(a, b, c);
    end = std::chrono::high_resolution_clock::now();
    double timeSimdOnly = std::chrono::duration<double, std::milli>(end - start).count();
    
    // Measure parallel-only time
    start = std::chrono::high_resolution_clock::now();
    parallelOnlyOperation(a, b, c, numThreads);
    end = std::chrono::high_resolution_clock::now();
    double timeParallelOnly = std::chrono::duration<double, std::milli>(end - start).count();
    
    // Measure combined SIMD+parallel time
    start = std::chrono::high_resolution_clock::now();
    simdParallelOperation(a, b, c, numThreads);
    end = std::chrono::high_resolution_clock::now();
    double timeSimdParallel = std::chrono::duration<double, std::milli>(end - start).count();
    
    // Calculate speedups
    double speedupSimdOnly = timeSequential / timeSimdOnly;
    double speedupParallelOnly = timeSequential / timeParallelOnly;
    double speedupSimdParallel = timeSequential / timeSimdParallel;
    
    // Print results
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Vector size: " << vectorSize << ", Threads: " << numThreads << std::endl;
    std::cout << "Sequential time: " << timeSequential << " ms" << std::endl;
    std::cout << "SIMD-only time: " << timeSimdOnly << " ms (Speedup: " << speedupSimdOnly << "x)" << std::endl;
    std::cout << "Parallel-only time: " << timeParallelOnly << " ms (Speedup: " << speedupParallelOnly << "x)" << std::endl;
    std::cout << "SIMD+parallel time: " << timeSimdParallel << " ms (Speedup: " << speedupSimdParallel << "x)" << std::endl;
    
    // Calculate efficiency
    double theoreticalMaxSpeedup = static_cast<double>(numThreads) * (speedupSimdOnly);
    double actualEfficiency = (speedupSimdParallel / theoreticalMaxSpeedup) * 100.0;
    
    std::cout << "Theoretical max combined speedup: " << theoreticalMaxSpeedup << "x" << std::endl;
    std::cout << "Actual combined efficiency: " << actualEfficiency << "%" << std::endl;
}

// Run SIMD with thread parallelism demo
void runSIMDParallelism() {
    std::cout << "\n=== SIMD with Thread Parallelism Demo ===" << std::endl;
    
    // Vector size
    const size_t vectorSize = 1000000; // 1 million elements
    
    // Get available hardware threads
    int maxThreads = omp_get_max_threads();
    std::cout << "System has " << maxThreads << " available hardware threads" << std::endl;
    
    // Test scaling with different thread counts
    std::cout << "\n--- Scaling Analysis ---" << std::endl;
    std::cout << "Testing performance with different thread counts:" << std::endl;
    
    // Test with 1 thread (essentially sequential but with parallel overhead)
    std::cout << "\nTest with 1 thread:" << std::endl;
    simdWithThreads(1, vectorSize);
    
    // Test with 2 threads
    if (maxThreads >= 2) {
        std::cout << "\nTest with 2 threads:" << std::endl;
        simdWithThreads(2, vectorSize);
    }
    
    // Test with 4 threads
    if (maxThreads >= 4) {
        std::cout << "\nTest with 4 threads:" << std::endl;
        simdWithThreads(4, vectorSize);
    }
    
    // Test with max threads
    if (maxThreads > 4) {
        std::cout << "\nTest with max threads (" << maxThreads << "):" << std::endl;
        simdWithThreads(maxThreads, vectorSize);
    }
    
    // Test with thread count optimization
    std::cout << "\n--- Thread Count Optimization ---" << std::endl;
    std::cout << "Finding the optimal thread count for this workload..." << std::endl;
    
    // Try different thread counts to find the optimal value
    double bestTime = std::numeric_limits<double>::max();
    int optimalThreads = 1;
    
    for (int threads = 1; threads <= maxThreads; threads++) {
        // Skip some thread counts for faster testing
        if (threads > 4 && threads < maxThreads && threads % 2 != 0) {
            continue;
        }
        
        // Create temporary vectors for this test
        std::vector<double> a(vectorSize);
        std::vector<double> b(vectorSize);
        std::vector<double> c(vectorSize);
        
        // Initialize with random data
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<double> dis(0.0, 1.0);
        
        for (size_t i = 0; i < vectorSize; ++i) {
            a[i] = dis(gen);
            b[i] = dis(gen);
        }
        
        // Time the combined operation
        auto start = std::chrono::high_resolution_clock::now();
        simdParallelOperation(a, b, c, threads);
        auto end = std::chrono::high_resolution_clock::now();
        double time = std::chrono::duration<double, std::milli>(end - start).count();
        
        std::cout << "Threads: " << threads << ", Time: " << time << " ms" << std::endl;
        
        if (time < bestTime) {
            bestTime = time;
            optimalThreads = threads;
        }
    }
    
    std::cout << "\nOptimal thread count for this workload: " << optimalThreads << " threads" << std::endl;
    std::cout << "Best execution time: " << bestTime << " ms" << std::endl;
    
    // SIMD + parallelism explanation
    std::cout << "\n=== SIMD + Thread Parallelism Explained ===" << std::endl;
    
    std::cout << "1. Different Levels of Parallelism:" << std::endl;
    std::cout << "   - SIMD (Data Parallelism): Multiple data elements per CPU instruction" << std::endl;
    std::cout << "   - Thread Parallelism: Multiple CPU cores working simultaneously" << std::endl;
    
    std::cout << "\n2. Combined Benefits:" << std::endl;
    std::cout << "   - SIMD: Typically 2-8x speedup on a single core" << std::endl;
    std::cout << "   - Thread Parallelism: Linear speedup with core count (ideally)" << std::endl;
    std::cout << "   - Combined: Multiplicative effect of both parallelism types" << std::endl;
    
    std::cout << "\n3. Practical Considerations:" << std::endl;
    std::cout << "   - Thread overhead can reduce efficiency at high thread counts" << std::endl;
    std::cout << "   - Memory bandwidth often becomes the limiting factor" << std::endl;
    std::cout << "   - Optimal thread count may be less than maximum available" << std::endl;
    std::cout << "   - NUMA effects can impact performance on multi-socket systems" << std::endl;
    
    std::cout << "\n4. OpenMP Directives:" << std::endl;
    std::cout << "   - #pragma omp simd - Vectorization only" << std::endl;
    std::cout << "   - #pragma omp parallel for - Thread parallelism only" << std::endl;
    std::cout << "   - Nested directives used for combined parallelism in OpenMP 2.0" << std::endl;
    
    std::cout << "\nNote: In newer OpenMP versions, you can use the combined directive" << std::endl;
    std::cout << "      #pragma omp parallel for simd for maximum performance." << std::endl;
}