#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <omp.h>
#include <fstream>
#include <algorithm>
#include <cstring>
#include "../../include/cli_parser.h"
#include "../../include/profiler.h"
#include "../../include/debug_utils.h"

/**
 * @file false_sharing_fixed.cpp
 * @brief Example demonstrating solutions to false sharing issues in OpenMP programs
 * 
 * This file shows several techniques to fix false sharing:
 * 1. Padding arrays to ensure each thread uses data in separate cache lines
 * 2. Using thread-local variables to avoid shared data completely
 * 3. Using array-of-structs vs. struct-of-arrays approach
 */

// Default cache line size (in bytes) - typically 64 bytes on modern CPUs
constexpr int DEFAULT_CACHE_LINE_SIZE = 64;

// Structure with padding to avoid false sharing
struct PaddedData {
    alignas(DEFAULT_CACHE_LINE_SIZE) int counter;
    char padding[DEFAULT_CACHE_LINE_SIZE - sizeof(int)]; // Pad to cache line size
};

// Structure with explicit alignment - another approach
struct alignas(DEFAULT_CACHE_LINE_SIZE) AlignedData {
    int counter;
    // No explicit padding, but align the whole struct
};

// Benchmark function for the padded solution
double benchmarkPadded(int numThreads, int iterations, int cacheLineSize, bool verbose) {
    // Allocate array of padded data structures
    std::vector<PaddedData> data(numThreads);
    for (int i = 0; i < numThreads; i++) {
        data[i].counter = 0;
    }
    
    std::cout << "Running padded benchmark with " << numThreads 
              << " threads and " << iterations << " iterations..." << std::endl;
    
    // Show padded data structure details
    if (verbose) {
        std::cout << "PaddedData size: " << sizeof(PaddedData) << " bytes" << std::endl;
        std::cout << "Each counter is aligned to " << cacheLineSize << " bytes" << std::endl;
    }
    
    // Start profiling
    PROFILE_SCOPE("Padded");
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Each thread increments its own padded counter
    #pragma omp parallel num_threads(numThreads)
    {
        int threadId = omp_get_thread_num();
        
        // Each thread repeatedly increments its own counter
        for (int i = 0; i < iterations; i++) {
            data[threadId].counter++;
        }
        
        if (verbose) {
            #pragma omp critical
            {
                std::cout << "Thread " << threadId << " counter value: " 
                          << data[threadId].counter << std::endl;
            }
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime).count();
    
    std::cout << "Padded benchmark completed in " << duration << " ms" << std::endl;
    
    // Verify results
    bool correct = true;
    for (int i = 0; i < numThreads; i++) {
        if (data[i].counter != iterations) {
            correct = false;
            std::cout << "Error: Counter " << i << " value is " 
                      << data[i].counter << ", expected " << iterations << std::endl;
        }
    }
    
    if (correct) {
        std::cout << "All counter values are correct." << std::endl;
    }
    
    return duration;
}

// Benchmark function for the aligned solution
double benchmarkAligned(int numThreads, int iterations, int cacheLineSize, bool verbose) {
    // Allocate array of aligned data structures
    std::vector<AlignedData> data(numThreads);
    for (int i = 0; i < numThreads; i++) {
        data[i].counter = 0;
    }
    
    std::cout << "Running aligned benchmark with " << numThreads 
              << " threads and " << iterations << " iterations..." << std::endl;
    
    // Show aligned data structure details
    if (verbose) {
        std::cout << "AlignedData size: " << sizeof(AlignedData) << " bytes" << std::endl;
        std::cout << "AlignedData alignment: " << alignof(AlignedData) << " bytes" << std::endl;
    }
    
    // Start profiling
    PROFILE_SCOPE("Aligned");
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Each thread increments its own aligned counter
    #pragma omp parallel num_threads(numThreads)
    {
        int threadId = omp_get_thread_num();
        
        // Each thread repeatedly increments its own counter
        for (int i = 0; i < iterations; i++) {
            data[threadId].counter++;
        }
        
        if (verbose) {
            #pragma omp critical
            {
                std::cout << "Thread " << threadId << " counter value: " 
                          << data[threadId].counter << std::endl;
            }
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime).count();
    
    std::cout << "Aligned benchmark completed in " << duration << " ms" << std::endl;
    
    // Verify results
    bool correct = true;
    for (int i = 0; i < numThreads; i++) {
        if (data[i].counter != iterations) {
            correct = false;
            std::cout << "Error: Counter " << i << " value is " 
                      << data[i].counter << ", expected " << iterations << std::endl;
        }
    }
    
    if (correct) {
        std::cout << "All counter values are correct." << std::endl;
    }
    
    return duration;
}

// Benchmark function for the thread-local approach
double benchmarkThreadLocal(int numThreads, int iterations, bool verbose) {
    // Array to store final results
    std::vector<int> results(numThreads, 0);
    
    std::cout << "Running thread-local benchmark with " << numThreads 
              << " threads and " << iterations << " iterations..." << std::endl;
    
    // Start profiling
    PROFILE_SCOPE("ThreadLocal");
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Each thread increments its own thread-local counter
    #pragma omp parallel num_threads(numThreads)
    {
        int threadId = omp_get_thread_num();
        int localCounter = 0;  // Thread-local variable
        
        // Each thread repeatedly increments its local counter
        for (int i = 0; i < iterations; i++) {
            localCounter++;
        }
        
        // Store the final value
        results[threadId] = localCounter;
        
        if (verbose) {
            #pragma omp critical
            {
                std::cout << "Thread " << threadId << " counter value: " 
                          << localCounter << std::endl;
            }
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime).count();
    
    std::cout << "Thread-local benchmark completed in " << duration << " ms" << std::endl;
    
    // Verify results
    bool correct = true;
    for (int i = 0; i < numThreads; i++) {
        if (results[i] != iterations) {
            correct = false;
            std::cout << "Error: Counter " << i << " value is " 
                      << results[i] << ", expected " << iterations << std::endl;
        }
    }
    
    if (correct) {
        std::cout << "All counter values are correct." << std::endl;
    }
    
    return duration;
}

// Benchmark function for manual array indexing
double benchmarkArrayIndexing(int numThreads, int iterations, int cacheLineSize, bool verbose) {
    // Calculate how many integers we can fit in a cache line
    int intsPerCacheLine = cacheLineSize / sizeof(int);
    
    // Allocate array with space for each thread in a separate cache line
    std::vector<int> data(numThreads * intsPerCacheLine, 0);
    
    std::cout << "Running array indexing benchmark with " << numThreads 
              << " threads and " << iterations << " iterations..." << std::endl;
    
    // Show array details
    if (verbose) {
        std::cout << "Integers per cache line: " << intsPerCacheLine << std::endl;
        std::cout << "Array size: " << data.size() << " integers (" 
                  << data.size() * sizeof(int) << " bytes)" << std::endl;
    }
    
    // Start profiling
    PROFILE_SCOPE("ArrayIndexing");
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Each thread increments its own counter with spacing to avoid sharing
    #pragma omp parallel num_threads(numThreads)
    {
        int threadId = omp_get_thread_num();
        int index = threadId * intsPerCacheLine;  // Space counters by cache line size
        
        // Each thread repeatedly increments its own counter
        for (int i = 0; i < iterations; i++) {
            data[index]++;
        }
        
        if (verbose) {
            #pragma omp critical
            {
                std::cout << "Thread " << threadId << " using index " << index 
                          << ", value: " << data[index] << std::endl;
            }
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime).count();
    
    std::cout << "Array indexing benchmark completed in " << duration << " ms" << std::endl;
    
    // Verify results
    bool correct = true;
    for (int i = 0; i < numThreads; i++) {
        int index = i * intsPerCacheLine;
        if (data[index] != iterations) {
            correct = false;
            std::cout << "Error: Counter at index " << index << " value is " 
                      << data[index] << ", expected " << iterations << std::endl;
        }
    }
    
    if (correct) {
        std::cout << "All counter values are correct." << std::endl;
    }
    
    return duration;
}

// Visualize memory layout for educational purposes
void visualizeFixedLayouts(int numThreads, int cacheLineSize) {
    std::cout << "=== Memory Layout Visualization for Fixed Approaches ===\n";
    std::cout << "Cache line size: " << cacheLineSize << " bytes\n";
    
    // Calculate how many integers fit in a cache line
    int intsPerCacheLine = cacheLineSize / sizeof(int);
    
    // Padded approach
    std::cout << "\n1. Padded Approach:\n";
    std::cout << "Each thread's counter is in a separate struct with padding:\n";
    
    for (int t = 0; t < std::min(numThreads, 4); t++) {
        std::cout << "Thread " << t << " data: |";
        std::cout << " C" << t << " ";
        for (int i = 1; i < intsPerCacheLine; i++) {
            std::cout << " - ";  // Padding
        }
        std::cout << "|\n";
    }
    
    // Array indexing approach
    std::cout << "\n2. Array Indexing Approach:\n";
    std::cout << "Counters are spaced out by cache line size:\n";
    
    std::cout << "|";
    for (int t = 0; t < std::min(numThreads, 4); t++) {
        std::cout << " C" << t << " ";
        for (int i = 1; i < intsPerCacheLine; i++) {
            std::cout << " - ";  // Unused space
        }
        std::cout << "|";
    }
    std::cout << "\n";
    
    // Thread-local approach
    std::cout << "\n3. Thread-local Approach:\n";
    std::cout << "Each thread's counter is in its own CPU register or stack,\n";
    std::cout << "completely avoiding shared memory access during computation.\n";
    
    std::cout << std::endl;
}

// Run all benchmarks and compare results
void compareApproaches(int numThreads, int iterations, int cacheLineSize, bool verbose, const std::string& reportFile) {
    // Reset profiler
    Profiler::getInstance().reset();
    
    // Show memory layouts
    visualizeFixedLayouts(numThreads, cacheLineSize);
    
    // Run benchmarks
    std::cout << "\n=== False Sharing Solutions Performance Comparison ===\n" << std::endl;
    
    double paddedTime = benchmarkPadded(numThreads, iterations, cacheLineSize, verbose);
    std::cout << std::endl;
    
    double alignedTime = benchmarkAligned(numThreads, iterations, cacheLineSize, verbose);
    std::cout << std::endl;
    
    double threadLocalTime = benchmarkThreadLocal(numThreads, iterations, verbose);
    std::cout << std::endl;
    
    double arrayIndexingTime = benchmarkArrayIndexing(numThreads, iterations, cacheLineSize, verbose);
    
    // Print comparison
    std::cout << "\n=== Performance Summary ===\n";
    std::cout << std::left << std::setw(20) << "Approach" 
              << std::right << std::setw(15) << "Time (ms)" 
              << std::right << std::setw(15) << "Relative" << std::endl;
    std::cout << std::string(50, '-') << std::endl;
    
    // Find the best time for relative comparison
    double bestTime = std::min({paddedTime, alignedTime, threadLocalTime, arrayIndexingTime});
    
    std::cout << std::left << std::setw(20) << "Padded Structs" 
              << std::right << std::setw(15) << paddedTime 
              << std::right << std::setw(15) << std::fixed << std::setprecision(2) 
              << bestTime / paddedTime << "x" << std::endl;
    
    std::cout << std::left << std::setw(20) << "Aligned Structs" 
              << std::right << std::setw(15) << alignedTime
              << std::right << std::setw(15) << std::fixed << std::setprecision(2) 
              << bestTime / alignedTime << "x" << std::endl;
    
    std::cout << std::left << std::setw(20) << "Thread-local" 
              << std::right << std::setw(15) << threadLocalTime
              << std::right << std::setw(15) << std::fixed << std::setprecision(2) 
              << bestTime / threadLocalTime << "x" << std::endl;
    
    std::cout << std::left << std::setw(20) << "Array Indexing" 
              << std::right << std::setw(15) << arrayIndexingTime
              << std::right << std::setw(15) << std::fixed << std::setprecision(2) 
              << bestTime / arrayIndexingTime << "x" << std::endl;
    
    // Save report to CSV if specified
    if (!reportFile.empty()) {
        std::ofstream file(reportFile);
        if (file.is_open()) {
            file << "Approach,Time (ms),Relative" << std::endl;
            file << "Padded Structs," << paddedTime << "," << std::fixed << std::setprecision(2) << bestTime / paddedTime << std::endl;
            file << "Aligned Structs," << alignedTime << "," << std::fixed << std::setprecision(2) << bestTime / alignedTime << std::endl;
            file << "Thread-local," << threadLocalTime << "," << std::fixed << std::setprecision(2) << bestTime / threadLocalTime << std::endl;
            file << "Array Indexing," << arrayIndexingTime << "," << std::fixed << std::setprecision(2) << bestTime / arrayIndexingTime << std::endl;
            
            std::cout << "Performance report saved to: " << reportFile << std::endl;
        }
    }
    
    // Save profiling data
    #ifdef PROFILE
    if (!reportFile.empty()) {
        std::string profileFile = reportFile + ".profile.csv";
        Profiler::getInstance().saveToCSV(profileFile);
        std::cout << "Profiling data saved to: " << profileFile << std::endl;
    }
    #endif
}

// Entry point
int main(int argc, char* argv[]) {
    // Parse command line arguments
    CliParser parser(argc, argv);
    
    // Get parameters
    int threads = parser.getIntOption("threads", std::min(4, omp_get_max_threads()));
    int iterations = parser.getIntOption("iterations", 100000000);
    int cacheLineSize = parser.getIntOption("cache-line", DEFAULT_CACHE_LINE_SIZE);
    bool verbose = parser.getBoolOption("verbose", false);
    bool quiet = parser.getBoolOption("quiet", false);
    std::string reportFile = parser.getStringOption("report", "");
    std::string mode = parser.getStringOption("mode", "all");
    
    // Enable profiling in profile build
    #ifdef PROFILE
    Profiler::getInstance().setEnabled(true);
    #endif
    
    if (!quiet) {
        std::cout << "=== OpenMP False Sharing Fixed Demo ===" << std::endl;
        std::cout << "Threads: " << threads << std::endl;
        std::cout << "Iterations: " << iterations << std::endl;
        std::cout << "Cache Line Size: " << cacheLineSize << " bytes" << std::endl;
        std::cout << std::endl;
    }
    
    // Run selected demo mode
    if (mode == "all") {
        compareApproaches(threads, iterations, cacheLineSize, verbose, reportFile);
    }
    else if (mode == "padded") {
        benchmarkPadded(threads, iterations, cacheLineSize, verbose);
    }
    else if (mode == "aligned") {
        benchmarkAligned(threads, iterations, cacheLineSize, verbose);
    }
    else if (mode == "threadlocal") {
        benchmarkThreadLocal(threads, iterations, verbose);
    }
    else if (mode == "array") {
        benchmarkArrayIndexing(threads, iterations, cacheLineSize, verbose);
    }
    else {
        if (!quiet) {
            std::cout << "Unknown mode: " << mode << ". Running all benchmarks." << std::endl;
        }
        compareApproaches(threads, iterations, cacheLineSize, verbose, reportFile);
    }
    
    return 0;
}