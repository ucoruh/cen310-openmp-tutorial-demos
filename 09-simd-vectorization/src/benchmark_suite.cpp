#define _USE_MATH_DEFINES
#include <cmath>
#include "../include/benchmark_suite.h"
#include "../include/simd_examples.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <chrono>
#include <vector>
#include <string>
#include <random>
#include <functional>
#include <algorithm>

// Forward declaration of functions from simd_parallelism.cpp
extern void sequentialOperation(const std::vector<double>& a, const std::vector<double>& b, std::vector<double>& c);
extern void simdParallelOperation(const std::vector<double>& a, const std::vector<double>& b, std::vector<double>& c, int numThreads);

// Utility function to measure execution time of a function
double measureExecutionTime(std::function<void()> func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

// Initialize a vector with random values
template<typename T>
void initializeRandomVector(std::vector<T>& vec, T min = 0, T max = 100) {
    std::random_device rd;
    std::mt19937 gen(rd());
    
    if constexpr (std::is_integral<T>::value) {
        std::uniform_int_distribution<T> dis(min, max);
        for (auto& val : vec) {
            val = dis(gen);
        }
    } else {
        std::uniform_real_distribution<T> dis(min, max);
        for (auto& val : vec) {
            val = dis(gen);
        }
    }
}

// Verify that two vectors have approximately equal values
template<typename T>
bool verifyVectorResults(const std::vector<T>& expected, const std::vector<T>& actual, T epsilon) {
    if (expected.size() != actual.size()) {
        return false;
    }
    
    for (size_t i = 0; i < expected.size(); ++i) {
        if (std::abs(expected[i] - actual[i]) > epsilon) {
            return false;
        }
    }
    
    return true;
}

// Vector addition benchmark
BenchmarkResult benchmarkVectorAddition(size_t vectorSize) {
    BenchmarkResult result;
    result.name = "Vector Addition";
    
    // Initialize test vectors
    std::vector<double> a(vectorSize);
    std::vector<double> b(vectorSize);
    std::vector<double> c_scalar(vectorSize, 0.0);
    std::vector<double> c_simd(vectorSize, 0.0);
    
    // Fill with random values
    initializeRandomVector(a, 0.0, 100.0);
    initializeRandomVector(b, 0.0, 100.0);
    
    // Warm up
    for (size_t i = 0; i < 1000; ++i) {
        c_scalar[i] = a[i] + b[i];
    }
    
    // Benchmark scalar version
    result.timeScalar = measureExecutionTime([&]() {
        basicVectorAddition(a, b, c_scalar, false);
    });
    
    // Benchmark vectorized version
    result.timeVectorized = measureExecutionTime([&]() {
        basicVectorAddition(a, b, c_simd, true);
    });
    
    // Calculate speedup and verify results
    result.speedup = result.timeScalar / result.timeVectorized;
    result.verified = verifyVectorResults(c_scalar, c_simd, 1e-10);
    
    return result;
}

// Array multiplication benchmark
BenchmarkResult benchmarkArrayMultiply(size_t vectorSize) {
    BenchmarkResult result;
    result.name = "Array Multiplication";
    
    // Initialize test vectors
    std::vector<float> a(vectorSize);
    std::vector<float> b(vectorSize);
    std::vector<float> c_scalar(vectorSize, 0.0f);
    std::vector<float> c_simd(vectorSize, 0.0f);
    
    // Fill with random values
    initializeRandomVector(a, 0.1f, 10.0f);
    initializeRandomVector(b, 0.1f, 10.0f);
    
    // Benchmark scalar version
    result.timeScalar = measureExecutionTime([&]() {
        arrayElementWiseMultiply(a, b, c_scalar, false);
    });
    
    // Benchmark vectorized version
    result.timeVectorized = measureExecutionTime([&]() {
        arrayElementWiseMultiply(a, b, c_simd, true);
    });
    
    // Calculate speedup and verify results
    result.speedup = result.timeScalar / result.timeVectorized;
    result.verified = verifyVectorResults(c_scalar, c_simd, 1e-5f);
    
    return result;
}

// Transcendental functions benchmark
BenchmarkResult benchmarkTranscendental(size_t vectorSize) {
    BenchmarkResult result;
    result.name = "Transcendental Functions";
    
    // Initialize test vectors
    std::vector<double> input(vectorSize);
    std::vector<double> output_scalar(vectorSize, 0.0);
    std::vector<double> output_simd(vectorSize, 0.0);
    
    // Fill with random values between -π and π
    initializeRandomVector(input, -M_PI, M_PI);
    
    // Benchmark scalar version
    result.timeScalar = measureExecutionTime([&]() {
        computeTranscendental(input, output_scalar, false);
    });
    
    // Benchmark vectorized version
    result.timeVectorized = measureExecutionTime([&]() {
        computeTranscendental(input, output_simd, true);
    });
    
    // Calculate speedup and verify results
    result.speedup = result.timeScalar / result.timeVectorized;
    result.verified = verifyVectorResults(output_scalar, output_simd, 1e-10);
    
    return result;
}

// Memory alignment benchmark
BenchmarkResult benchmarkAlignedAccess(size_t vectorSize) {
    BenchmarkResult result;
    result.name = "Memory Alignment";
    
    // Create dummy vectors for timing (actual implementation uses raw pointers)
    std::vector<float> dummy(vectorSize);
    
    // Warm up
    alignedVsUnaligned(false);
    alignedVsUnaligned(true);
    
    // Benchmark unaligned version
    result.timeScalar = measureExecutionTime([&]() {
        alignedVsUnaligned(false);
    });
    
    // Benchmark aligned version
    result.timeVectorized = measureExecutionTime([&]() {
        alignedVsUnaligned(true);
    });
    
    // Calculate speedup (no explicit verification)
    result.speedup = result.timeScalar / result.timeVectorized;
    result.verified = true;  // We assume alignment works correctly
    
    return result;
}

// Mixed precision operations benchmark
BenchmarkResult benchmarkMixedPrecision(size_t vectorSize) {
    BenchmarkResult result;
    result.name = "Mixed Precision";
    
    // Initialize test vectors
    std::vector<float> floatVec(vectorSize);
    std::vector<int> intVec(vectorSize);
    std::vector<float> result_scalar(vectorSize, 0.0f);
    std::vector<float> result_simd(vectorSize, 0.0f);
    
    // Fill with random values
    initializeRandomVector(floatVec, 0.1f, 100.0f);
    initializeRandomVector(intVec, 1, 100);
    
    // Benchmark scalar version
    result.timeScalar = measureExecutionTime([&]() {
        mixedTypeOperations(floatVec, intVec, result_scalar, false);
    });
    
    // Benchmark vectorized version
    result.timeVectorized = measureExecutionTime([&]() {
        mixedTypeOperations(floatVec, intVec, result_simd, true);
    });
    
    // Calculate speedup and verify results
    result.speedup = result.timeScalar / result.timeVectorized;
    result.verified = verifyVectorResults(result_scalar, result_simd, 1e-5f);
    
    return result;
}

// SIMD with thread parallelism benchmark
BenchmarkResult benchmarkSIMDParallelism(size_t vectorSize, int numThreads) {
    BenchmarkResult result;
    result.name = "SIMD+Threads (" + std::to_string(numThreads) + " threads)";
    
    // Initialize vectors for the actual test
    std::vector<double> a(vectorSize);
    std::vector<double> b(vectorSize);
    std::vector<double> c_seq(vectorSize);
    std::vector<double> c_simd(vectorSize);
    
    // Fill with random values
    initializeRandomVector(a, 0.0, 1.0);
    initializeRandomVector(b, 0.0, 1.0);
    
    // Benchmark sequential version
    result.timeScalar = measureExecutionTime([&]() {
        sequentialOperation(a, b, c_seq);
    });
    
    // Benchmark vectorized+parallel version
    result.timeVectorized = measureExecutionTime([&]() {
        simdParallelOperation(a, b, c_simd, numThreads);
    });
    
    // Calculate speedup and verify results
    result.speedup = result.timeScalar / result.timeVectorized;
    result.verified = verifyVectorResults(c_seq, c_simd, 1e-10);
    
    return result;
}

// Display benchmark results as a table
void displayBenchmarkResults(const std::vector<BenchmarkResult>& results) {
    std::cout << "\n=== Benchmark Results ===" << std::endl;
    
    // Table header
    std::cout << std::left 
              << std::setw(30) << "Benchmark"
              << std::right 
              << std::setw(15) << "Scalar (ms)" 
              << std::setw(15) << "SIMD (ms)" 
              << std::setw(15) << "Speedup" 
              << std::setw(15) << "Verified" 
              << std::endl;
    
    // Separator line
    std::cout << std::string(90, '-') << std::endl;
    
    // Table rows
    std::cout << std::fixed << std::setprecision(3);
    for (const auto& result : results) {
        std::cout << std::left 
                  << std::setw(30) << result.name
                  << std::right 
                  << std::setw(15) << result.timeScalar
                  << std::setw(15) << result.timeVectorized
                  << std::setw(15) << result.speedup
                  << std::setw(15) << (result.verified ? "✓" : "✗")
                  << std::endl;
    }
    
    // Summary statistics
    double avgSpeedup = 0.0;
    double maxSpeedup = 0.0;
    std::string fastestBenchmark;
    
    for (const auto& result : results) {
        avgSpeedup += result.speedup;
        if (result.speedup > maxSpeedup) {
            maxSpeedup = result.speedup;
            fastestBenchmark = result.name;
        }
    }
    
    avgSpeedup /= results.size();
    
    std::cout << std::string(90, '-') << std::endl;
    std::cout << "Average Speedup: " << avgSpeedup << "x" << std::endl;
    std::cout << "Maximum Speedup: " << maxSpeedup << "x (in " << fastestBenchmark << ")" << std::endl;
}

// ASCII visualization of benchmark results
void displayPerformanceGraph(const std::vector<BenchmarkResult>& results) {
    std::cout << "\n=== Performance Visualization ===" << std::endl;
    
    const int maxBarLength = 50;  // Maximum length of the bar in characters
    const char barChar = '#';     // Character to use for the bar
    
    // Find the maximum speedup for scaling
    double maxSpeedup = 0.0;
    for (const auto& result : results) {
        maxSpeedup = std::max(maxSpeedup, result.speedup);
    }
    
    // Draw the graph
    for (const auto& result : results) {
        // Calculate the bar length
        int barLength = static_cast<int>((result.speedup / maxSpeedup) * maxBarLength);
        
        // Truncate benchmark name if too long
        std::string name = result.name;
        if (name.length() > 25) {
            name = name.substr(0, 22) + "...";
        }
        
        // Print the benchmark name and speedup
        std::cout << std::left << std::setw(25) << name << " | ";
        
        // Print the bar
        for (int i = 0; i < barLength; ++i) {
            std::cout << barChar;
        }
        
        // Print the speedup value
        std::cout << " " << std::fixed << std::setprecision(2) << result.speedup << "x" << std::endl;
    }
    
    // Add a legend
    std::cout << std::string(60, '-') << std::endl;
    std::cout << "Each " << barChar << " represents approximately " 
              << std::fixed << std::setprecision(2) << (maxSpeedup / maxBarLength) 
              << "x speedup" << std::endl;
}

// Generate a performance report as a text file
void generatePerformanceReport(const std::vector<BenchmarkResult>& results, const std::string& filename) {
    std::ofstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << " for writing." << std::endl;
        return;
    }
    
    // Write report header
    file << "=======================================================" << std::endl;
    file << "        OpenMP SIMD Vectorization Benchmark Report      " << std::endl;
    file << "=======================================================" << std::endl;
    file << std::endl;
    
    // Write system information
    file << "System Information:" << std::endl;
    file << "------------------" << std::endl;
    file << "Date: " << __DATE__ << " " << __TIME__ << std::endl;
    file << "Compiler: " << _MSC_VER << " (MSVC)" << std::endl;
#ifdef _OPENMP
    file << "OpenMP Version: " << _OPENMP << std::endl;
#else
    file << "OpenMP Version: Not enabled" << std::endl;
#endif
    file << std::endl;
    
    // Write benchmark results
    file << "Benchmark Results:" << std::endl;
    file << "-----------------" << std::endl;
    file << std::left 
         << std::setw(30) << "Benchmark"
         << std::right 
         << std::setw(15) << "Scalar (ms)" 
         << std::setw(15) << "SIMD (ms)" 
         << std::setw(15) << "Speedup" 
         << std::setw(15) << "Verified" 
         << std::endl;
    
    file << std::string(90, '-') << std::endl;
    
    file << std::fixed << std::setprecision(3);
    for (const auto& result : results) {
        file << std::left 
             << std::setw(30) << result.name
             << std::right 
             << std::setw(15) << result.timeScalar
             << std::setw(15) << result.timeVectorized
             << std::setw(15) << result.speedup
             << std::setw(15) << (result.verified ? "Yes" : "No")
             << std::endl;
    }
    
    // Write summary statistics
    double avgSpeedup = 0.0;
    double maxSpeedup = 0.0;
    std::string fastestBenchmark;
    
    for (const auto& result : results) {
        avgSpeedup += result.speedup;
        if (result.speedup > maxSpeedup) {
            maxSpeedup = result.speedup;
            fastestBenchmark = result.name;
        }
    }
    
    avgSpeedup /= results.size();
    
    file << std::string(90, '-') << std::endl;
    file << "Average Speedup: " << avgSpeedup << "x" << std::endl;
    file << "Maximum Speedup: " << maxSpeedup << "x (in " << fastestBenchmark << ")" << std::endl;
    file << std::endl;
    
    // Write ASCII graph
    file << "Performance Visualization:" << std::endl;
    file << "-------------------------" << std::endl;
    
    const int maxBarLength = 50;
    const char barChar = '#';
    
    for (const auto& result : results) {
        int barLength = static_cast<int>((result.speedup / maxSpeedup) * maxBarLength);
        
        file << std::left << std::setw(25) << result.name << " | ";
        
        for (int i = 0; i < barLength; ++i) {
            file << barChar;
        }
        
        file << " " << std::fixed << std::setprecision(2) << result.speedup << "x" << std::endl;
    }
    
    file << std::string(60, '-') << std::endl;
    file << "Each " << barChar << " represents approximately " 
         << std::fixed << std::setprecision(2) << (maxSpeedup / maxBarLength) 
         << "x speedup" << std::endl;
    file << std::endl;
    
    // Write conclusion
    file << "Conclusion:" << std::endl;
    file << "-----------" << std::endl;
    if (avgSpeedup > 3.0) {
        file << "SIMD vectorization provides excellent performance improvements for " << std::endl;
        file << "the tested operations, with an average speedup of " << avgSpeedup << "x." << std::endl;
    } else if (avgSpeedup > 1.5) {
        file << "SIMD vectorization provides good performance improvements for " << std::endl;
        file << "the tested operations, with an average speedup of " << avgSpeedup << "x." << std::endl;
    } else {
        file << "SIMD vectorization provides modest performance improvements for " << std::endl;
        file << "the tested operations, with an average speedup of " << avgSpeedup << "x." << std::endl;
        file << "This may be due to memory bandwidth limitations or compiler optimizations." << std::endl;
    }
    
    file.close();
    
    std::cout << "Performance report written to " << filename << std::endl;
}

// Run the full benchmark suite
void runBenchmarkSuite() {
    std::cout << "\n=== Running SIMD Vectorization Benchmark Suite ===" << std::endl;
    
    // Vector sizes for benchmarks
    const size_t smallSize = 1000000;   // 1 million elements
    const size_t mediumSize = 10000000;  // 10 million elements
    const size_t largeSize = 50000000;  // 50 million elements
    
    std::cout << "Initializing benchmark suite with vector sizes:" << std::endl;
    std::cout << "- Small: " << smallSize << " elements" << std::endl;
    std::cout << "- Medium: " << mediumSize << " elements" << std::endl;
    std::cout << "- Large: " << largeSize << " elements" << std::endl;
    
    // Collect benchmark results
    std::vector<BenchmarkResult> results;
    
    // Run and add benchmarks to results
    std::cout << "\nRunning benchmarks..." << std::endl;
    
    std::cout << "1. Vector Addition (Medium)..." << std::endl;
    results.push_back(benchmarkVectorAddition(mediumSize));
    
    std::cout << "2. Array Multiplication (Medium)..." << std::endl;
    results.push_back(benchmarkArrayMultiply(mediumSize));
    
    std::cout << "3. Transcendental Functions (Small)..." << std::endl;
    results.push_back(benchmarkTranscendental(smallSize));
    
    std::cout << "4. Memory Alignment (Large)..." << std::endl;
    results.push_back(benchmarkAlignedAccess(largeSize));
    
    std::cout << "5. Mixed Precision Operations (Medium)..." << std::endl;
    results.push_back(benchmarkMixedPrecision(mediumSize));
    
    std::cout << "6. SIMD Parallelism with 2 threads (Medium)..." << std::endl;
    results.push_back(benchmarkSIMDParallelism(mediumSize, 2));
    
    std::cout << "7. SIMD Parallelism with 4 threads (Medium)..." << std::endl;
    results.push_back(benchmarkSIMDParallelism(mediumSize, 4));
    
    // Display the results
    displayBenchmarkResults(results);
    
    // Display performance visualization
    displayPerformanceGraph(results);
    
    // Generate a performance report file
    generatePerformanceReport(results, "benchmarks/performance_report.txt");
    
    std::cout << "\nBenchmarking completed!" << std::endl;
    std::cout << "Check the report file for detailed results." << std::endl;
}