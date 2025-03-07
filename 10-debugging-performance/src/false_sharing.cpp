#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <omp.h>
#include <fstream>
#include <algorithm>
#include <cstring>
#include "../include/cli_parser.h"
#include "../include/profiler.h"
#include "../include/debug_utils.h"

/**
 * @file false_sharing.cpp
 * @brief Example demonstrating false sharing issues in OpenMP programs
 * 
 * False sharing occurs when multiple threads access different variables
 * that happen to be located in the same cache line, causing cache coherency
 * overhead. This example demonstrates the performance impact of false sharing
 * and illustrates the memory layout that causes it.
 */

// Default cache line size (in bytes) - typically 64 bytes on modern CPUs
constexpr int DEFAULT_CACHE_LINE_SIZE = 64;

// Structure demonstrating false sharing with adjacent data
struct FalseSharing {
    int counter[8]; // 8 integers (32 bytes) - two counters fit in a cache line
};

// Structure with padding to avoid false sharing
struct PaddedData {
    alignas(DEFAULT_CACHE_LINE_SIZE) int counter;
    char padding[DEFAULT_CACHE_LINE_SIZE - sizeof(int)]; // Pad to cache line size
};

// Function to visualize the memory layout for educational purposes
void visualizeMemoryLayout(const FalseSharing& data, int cacheLineSize) {
    std::cout << "Memory Layout Visualization (Cache line size: " << cacheLineSize << " bytes)\n";
    std::cout << "Each symbol represents 4 bytes (sizeof(int)):\n";
    std::cout << "  [0-7] - Counter elements\n";
    std::cout << "  | - Cache line boundary\n\n";
    
    // Calculate how many integers fit in a cache line
    int intsPerCacheLine = cacheLineSize / sizeof(int);
    
    std::cout << "FalseSharing struct layout:\n";
    std::cout << "|";
    
    for (int i = 0; i < 8; i++) {
        // Print the element number
        std::cout << " " << i << " ";
        
        // Print cache line boundary if needed
        if ((i + 1) % intsPerCacheLine == 0 && i < 7) {
            std::cout << "|";
        }
    }
    
    std::cout << "|\n\n";
    
    // Show which threads access which elements
    std::cout << "In a 4-thread scenario:\n";
    std::cout << "Thread 0 accesses element 0\n";
    std::cout << "Thread 1 accesses element 1\n";
    std::cout << "Thread 2 accesses element 2\n";
    std::cout << "Thread 3 accesses element 3\n";
    std::cout << std::endl;
    
    if (intsPerCacheLine >= 2) {
        std::cout << "Since " << intsPerCacheLine << " integers fit in a cache line,\n";
        std::cout << "threads 0 and 1 share a cache line, causing false sharing.\n";
        std::cout << "Threads 2 and 3 also share a different cache line.\n";
    }
    
    std::cout << std::endl;
}

// Benchmark function for the false sharing case
double benchmarkFalseSharing(int numThreads, int iterations, int cacheLineSize, bool verbose) {
    // Allocate the data structure with counters
    FalseSharing data;
    std::memset(&data, 0, sizeof(data));
    
    std::cout << "Running false sharing benchmark with " << numThreads 
              << " threads and " << iterations << " iterations..." << std::endl;
    
    // Start profiling
    PROFILE_SCOPE("FalseSharing");
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Each thread increments its own counter, but counters are adjacent
    #pragma omp parallel num_threads(numThreads)
    {
        int threadId = omp_get_thread_num();
        
        // Ensure we don't go out of bounds
        if (threadId < 8) {
            // Each thread repeatedly increments its own counter
            for (int i = 0; i < iterations; i++) {
                data.counter[threadId]++;
            }
        }
        
        if (verbose) {
            #pragma omp critical
            {
                std::cout << "Thread " << threadId << " counter value: " 
                          << data.counter[threadId] << std::endl;
            }
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime).count();
    
    std::cout << "False sharing benchmark completed in " << duration << " ms" << std::endl;
    
    // Verify results
    bool correct = true;
    for (int i = 0; i < numThreads && i < 8; i++) {
        if (data.counter[i] != iterations) {
            correct = false;
            std::cout << "Error: Counter " << i << " value is " 
                      << data.counter[i] << ", expected " << iterations << std::endl;
        }
    }
    
    if (correct) {
        std::cout << "All counter values are correct." << std::endl;
    }
    
    return duration;
}

// Benchmark function for a slightly improved case (with some distance)
double benchmarkReducedSharing(int numThreads, int iterations, int cacheLineSize, bool verbose) {
    // Allocate the data structure with counters
    FalseSharing data;
    std::memset(&data, 0, sizeof(data));
    
    std::cout << "Running reduced sharing benchmark with " << numThreads 
              << " threads and " << iterations << " iterations..." << std::endl;
    
    // Start profiling
    PROFILE_SCOPE("ReducedSharing");
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Each thread increments a counter with more space between them
    #pragma omp parallel num_threads(numThreads)
    {
        int threadId = omp_get_thread_num();
        
        // Introduce spacing - thread 0 uses counter 0, thread 1 uses counter 2, etc.
        int counterIndex = (threadId * 2) % 8;
        
        // Each thread repeatedly increments its own counter
        for (int i = 0; i < iterations; i++) {
            data.counter[counterIndex]++;
        }
        
        if (verbose) {
            #pragma omp critical
            {
                std::cout << "Thread " << threadId << " using counter " << counterIndex 
                          << ", value: " << data.counter[counterIndex] << std::endl;
            }
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime).count();
    
    std::cout << "Reduced sharing benchmark completed in " << duration << " ms" << std::endl;
    
    return duration;
}

// Benchmark function for the padded case (no false sharing)
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

// Run all benchmarks and compare results
void compareApproaches(int numThreads, int iterations, int cacheLineSize, bool verbose, const std::string& reportFile) {
    // Reset profiler
    Profiler::getInstance().reset();
    
    // Visualize memory layout (educational)
    FalseSharing dummyData;
    visualizeMemoryLayout(dummyData, cacheLineSize);
    
    // Run benchmarks
    std::cout << "\n=== False Sharing Performance Comparison ===\n" << std::endl;
    
    double falseTime = benchmarkFalseSharing(numThreads, iterations, cacheLineSize, verbose);
    std::cout << std::endl;
    
    double reducedTime = benchmarkReducedSharing(numThreads, iterations, cacheLineSize, verbose);
    std::cout << std::endl;
    
    double paddedTime = benchmarkPadded(numThreads, iterations, cacheLineSize, verbose);
    
    // Calculate speedups
    double speedupReduced = falseTime / reducedTime;
    double speedupPadded = falseTime / paddedTime;
    
    // Print comparison
    std::cout << "\n=== Performance Summary ===\n";
    std::cout << std::left << std::setw(20) << "Approach" 
              << std::right << std::setw(15) << "Time (ms)" 
              << std::right << std::setw(15) << "Speedup" << std::endl;
    std::cout << std::string(50, '-') << std::endl;
    
    std::cout << std::left << std::setw(20) << "False Sharing" 
              << std::right << std::setw(15) << falseTime 
              << std::right << std::setw(15) << "1.00x" << std::endl;
    
    std::cout << std::left << std::setw(20) << "Reduced Sharing" 
              << std::right << std::setw(15) << reducedTime 
              << std::right << std::setw(15) << std::fixed << std::setprecision(2) 
              << speedupReduced << "x" << std::endl;
    
    std::cout << std::left << std::setw(20) << "Padded (No Sharing)" 
              << std::right << std::setw(15) << paddedTime 
              << std::right << std::setw(15) << std::fixed << std::setprecision(2) 
              << speedupPadded << "x" << std::endl;
    
    // Save report to CSV if specified
    if (!reportFile.empty()) {
        std::ofstream file(reportFile);
        if (file.is_open()) {
            file << "Approach,Time (ms),Speedup" << std::endl;
            file << "False Sharing," << falseTime << ",1.00" << std::endl;
            file << "Reduced Sharing," << reducedTime << "," << std::fixed << std::setprecision(2) << speedupReduced << std::endl;
            file << "Padded (No Sharing)," << paddedTime << "," << std::fixed << std::setprecision(2) << speedupPadded << std::endl;
            
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
    int padding = parser.getIntOption("padding", 0);
    bool verbose = parser.getBoolOption("verbose", false);
    bool quiet = parser.getBoolOption("quiet", false);
    std::string reportFile = parser.getStringOption("report", "");
    std::string mode = parser.getStringOption("mode", "all");
    
    // Enable profiling in profile build
    #ifdef PROFILE
    Profiler::getInstance().setEnabled(true);
    #endif
    
    if (!quiet) {
        std::cout << "=== OpenMP False Sharing Demo ===" << std::endl;
        std::cout << "Threads: " << threads << std::endl;
        std::cout << "Iterations: " << iterations << std::endl;
        std::cout << "Cache Line Size: " << cacheLineSize << " bytes" << std::endl;
        std::cout << std::endl;
    }
    
    // Run selected demo mode
    if (mode == "all") {
        compareApproaches(threads, iterations, cacheLineSize, verbose, reportFile);
    }
    else if (mode == "false") {
        benchmarkFalseSharing(threads, iterations, cacheLineSize, verbose);
    }
    else if (mode == "reduced") {
        benchmarkReducedSharing(threads, iterations, cacheLineSize, verbose);
    }
    else if (mode == "padded") {
        benchmarkPadded(threads, iterations, cacheLineSize, verbose);
    }
    else {
        if (!quiet) {
            std::cout << "Unknown mode: " << mode << ". Running all benchmarks." << std::endl;
        }
        compareApproaches(threads, iterations, cacheLineSize, verbose, reportFile);
    }
    
    return 0;
}