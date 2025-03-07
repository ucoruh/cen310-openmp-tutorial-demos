#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <omp.h>
#include <random>
#include <fstream>
#include <algorithm>
#include <cstring>
#include <numeric>
#include "../include/cli_parser.h"
#include "../include/profiler.h"
#include "../include/debug_utils.h"

/**
 * @file memory_issues.cpp
 * @brief Example demonstrating memory access patterns and issues in OpenMP programs
 * 
 * This example shows how memory access patterns affect performance:
 * 1. Cache-unfriendly access patterns (strided, random)
 * 2. Memory bandwidth saturation
 * 3. NUMA effects on Windows
 * 4. Cache thrashing from multiple threads
 */

// Constants for memory tests
constexpr int CACHE_LINE_SIZE = 64;    // Typical cache line size in bytes
constexpr int KB = 1024;
constexpr int MB = 1024 * KB;

// Simple array structure for demonstration
struct SimpleArray {
    int* data;
    size_t size;
    
    SimpleArray(size_t size_) : size(size_) {
        data = new int[size];
        std::memset(data, 0, size * sizeof(int));
    }
    
    ~SimpleArray() {
        delete[] data;
    }
};

// Memory statistics structure
struct MemoryStats {
    double totalTime;           // Total execution time
    double bandwidthMBps;       // Memory bandwidth in MB/s
    double elementsPerSecond;   // Elements processed per second
    double missRate;            // Estimated cache miss rate (0-1)
};

// Function to generate random indices for access patterns
std::vector<int> generateRandomIndices(int size) {
    std::vector<int> indices(size);
    std::iota(indices.begin(), indices.end(), 0);  // Fill with 0, 1, 2, ...
    
    std::mt19937 rng(42);  // Fixed seed for reproducibility
    std::shuffle(indices.begin(), indices.end(), rng);
    
    return indices;
}

// Measure memory bandwidth using sequential access
MemoryStats measureSequentialMemory(int* data, size_t size, int numThreads, bool verbose) {
    const size_t iterations = 100;  // Number of iterations to average
    const size_t bytes = size * sizeof(int);
    
    std::cout << "Running sequential memory access test with " << numThreads 
              << " threads and " << (bytes / MB) << " MB of data..." << std::endl;
    
    // Start profiling
    PROFILE_SCOPE("SequentialMemory");
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Each thread accesses a contiguous chunk of memory sequentially
    #pragma omp parallel num_threads(numThreads)
    {
        int threadId = omp_get_thread_num();
        size_t perThread = size / numThreads;
        size_t startIdx = threadId * perThread;
        size_t endIdx = (threadId == numThreads - 1) ? size : startIdx + perThread;
        
        // Perform sequential memory access
        for (size_t iter = 0; iter < iterations; iter++) {
            // Read and write to memory sequentially (cache-friendly)
            for (size_t i = startIdx; i < endIdx; i++) {
                data[i] = data[i] + 1;  // Simple operation to force memory access
            }
            
            if (verbose && iter % 10 == 0) {
                #pragma omp critical
                {
                    std::cout << "Thread " << threadId << " completed iteration " << iter << std::endl;
                }
            }
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime).count() / 1000.0;  // Convert to seconds
    
    // Calculate memory statistics
    double totalBytes = static_cast<double>(bytes) * iterations * 2;  // Read + Write
    double bandwidthMBps = (totalBytes / MB) / duration;
    
    std::cout << "Sequential memory test completed in " << duration << " seconds" << std::endl;
    std::cout << "Memory bandwidth: " << std::fixed << std::setprecision(2) 
              << bandwidthMBps << " MB/s" << std::endl;
    
    MemoryStats stats;
    stats.totalTime = duration;
    stats.bandwidthMBps = bandwidthMBps;
    stats.elementsPerSecond = (size * iterations) / duration;
    stats.missRate = 0.0;  // Can't directly measure, estimated low for sequential
    
    return stats;
}

// Measure memory bandwidth using strided access
MemoryStats measureStridedMemory(int* data, size_t size, int stride, int numThreads, bool verbose) {
    const size_t iterations = 100;  // Number of iterations to average
    const size_t bytes = size * sizeof(int);
    
    std::cout << "Running strided memory access test (stride=" << stride << ") with " 
              << numThreads << " threads and " << (bytes / MB) << " MB of data..." << std::endl;
    
    // Start profiling
    PROFILE_SCOPE("StridedMemory");
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Each thread accesses memory with the specified stride
    #pragma omp parallel num_threads(numThreads)
    {
        int threadId = omp_get_thread_num();
        size_t perThread = size / numThreads;
        size_t startIdx = threadId * perThread;
        size_t endIdx = (threadId == numThreads - 1) ? size : startIdx + perThread;
        
        // Perform strided memory access
        for (size_t iter = 0; iter < iterations; iter++) {
            // Read and write to memory with stride (potentially cache-unfriendly)
            for (size_t i = startIdx; i < endIdx; i += stride) {
                data[i % size] = data[i % size] + 1;  // Use modulo to stay in bounds
            }
            
            if (verbose && iter % 10 == 0) {
                #pragma omp critical
                {
                    std::cout << "Thread " << threadId << " completed iteration " << iter << std::endl;
                }
            }
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime).count() / 1000.0;  // Convert to seconds
    
    // Calculate memory statistics
    double totalBytes = static_cast<double>(bytes) * iterations * 2;  // Read + Write
    double bandwidthMBps = (totalBytes / MB) / duration;
    
    std::cout << "Strided memory test completed in " << duration << " seconds" << std::endl;
    std::cout << "Memory bandwidth: " << std::fixed << std::setprecision(2) 
              << bandwidthMBps << " MB/s" << std::endl;
    
    MemoryStats stats;
    stats.totalTime = duration;
    stats.bandwidthMBps = bandwidthMBps;
    stats.elementsPerSecond = (size * iterations) / duration;
    stats.missRate = 0.5;  // Estimated high for strided access
    
    return stats;
}

// Measure memory bandwidth using random access
MemoryStats measureRandomMemory(int* data, size_t size, int numThreads, bool verbose) {
    const size_t iterations = 50;  // Fewer iterations for random access (slower)
    const size_t bytes = size * sizeof(int);
    
    std::cout << "Running random memory access test with " << numThreads 
              << " threads and " << (bytes / MB) << " MB of data..." << std::endl;
    
    // Generate random indices for each thread
    std::vector<std::vector<int>> threadIndices(numThreads);
    for (int t = 0; t < numThreads; t++) {
        threadIndices[t] = generateRandomIndices(size / numThreads);
        
        // Adjust indices for thread's portion of the array
        size_t startIdx = t * (size / numThreads);
        for (auto& idx : threadIndices[t]) {
            idx += startIdx;
        }
    }
    
    // Start profiling
    PROFILE_SCOPE("RandomMemory");
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Each thread accesses memory randomly
    #pragma omp parallel num_threads(numThreads)
    {
        int threadId = omp_get_thread_num();
        const auto& indices = threadIndices[threadId];
        
        // Perform random memory access
        for (size_t iter = 0; iter < iterations; iter++) {
            // Read and write to memory in random order (cache-unfriendly)
            for (size_t i = 0; i < indices.size(); i++) {
                data[indices[i]] = data[indices[i]] + 1;
            }
            
            if (verbose && iter % 10 == 0) {
                #pragma omp critical
                {
                    std::cout << "Thread " << threadId << " completed iteration " << iter << std::endl;
                }
            }
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime).count() / 1000.0;  // Convert to seconds
    
    // Calculate memory statistics
    double totalBytes = static_cast<double>(bytes) * iterations * 2;  // Read + Write
    double bandwidthMBps = (totalBytes / MB) / duration;
    
    std::cout << "Random memory test completed in " << duration << " seconds" << std::endl;
    std::cout << "Memory bandwidth: " << std::fixed << std::setprecision(2) 
              << bandwidthMBps << " MB/s" << std::endl;
    
    MemoryStats stats;
    stats.totalTime = duration;
    stats.bandwidthMBps = bandwidthMBps;
    stats.elementsPerSecond = (size * iterations) / duration;
    stats.missRate = 0.9;  // Estimated very high for random access
    
    return stats;
}

// Demonstrate cache thrashing with multiple threads
MemoryStats demonstrateCacheThrashing(int* data, size_t size, int numThreads, bool verbose) {
    const size_t iterations = 1000;  // More iterations to observe thrashing
    const size_t bytes = size * sizeof(int);
    
    std::cout << "Running cache thrashing demonstration with " << numThreads 
              << " threads and " << (bytes / KB) << " KB of data..." << std::endl;
    
    // Each thread will access the same small region of memory
    // This will cause cache line bouncing between processor caches
    const size_t regionSize = 16 * KB / sizeof(int);  // 16KB region - fits in L1 cache
    
    // Start profiling
    PROFILE_SCOPE("CacheThrashing");
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // All threads access the same memory region
    #pragma omp parallel num_threads(numThreads)
    {
        int threadId = omp_get_thread_num();
        
        // Perform repeated access to the same region
        for (size_t iter = 0; iter < iterations; iter++) {
            // Each thread accesses a different element in the same cache line
            for (size_t i = 0; i < regionSize; i++) {
                // Ensure threads access the same cache lines but different elements
                size_t idx = i + (threadId % 16);  // 16 ints per 64-byte cache line
                if (idx < size) {
                    data[idx] = data[idx] + threadId;
                }
            }
            
            if (verbose && iter % 100 == 0) {
                #pragma omp critical
                {
                    std::cout << "Thread " << threadId << " completed iteration " << iter << std::endl;
                }
            }
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime).count() / 1000.0;  // Convert to seconds
    
    // Calculate memory statistics (adjusted for the smaller working set)
    double totalBytes = static_cast<double>(regionSize * sizeof(int)) * iterations * numThreads * 2;  // Read + Write
    double bandwidthMBps = (totalBytes / MB) / duration;
    
    std::cout << "Cache thrashing test completed in " << duration << " seconds" << std::endl;
    std::cout << "Effective bandwidth: " << std::fixed << std::setprecision(2) 
              << bandwidthMBps << " MB/s" << std::endl;
    
    MemoryStats stats;
    stats.totalTime = duration;
    stats.bandwidthMBps = bandwidthMBps;
    stats.elementsPerSecond = (regionSize * iterations * numThreads) / duration;
    stats.missRate = 0.8;  // Estimated high due to thrashing
    
    return stats;
}

// Demonstrate NUMA effects (simplified for Windows)
MemoryStats demonstrateNUMAEffects(int* data, size_t size, int numThreads, bool verbose) {
    const size_t iterations = 100;
    const size_t bytes = size * sizeof(int);
    
    std::cout << "Running NUMA effects demonstration with " << numThreads 
              << " threads and " << (bytes / MB) << " MB of data..." << std::endl;
    
    // Start profiling
    PROFILE_SCOPE("NUMAEffects");
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // In this test, all memory is initialized by the master thread (node 0)
    // Then accessed by all threads - causing remote memory access for threads on other NUMA nodes
    
    // First, initialize the memory with the master thread
    std::memset(data, 0, bytes);
    
    // Now have all threads access the memory
    #pragma omp parallel num_threads(numThreads)
    {
        int threadId = omp_get_thread_num();
        size_t perThread = size / numThreads;
        
        // Intentionally access data across thread boundaries to demonstrate NUMA effects
        // Each thread accesses all parts of the array in a round-robin fashion
        for (size_t iter = 0; iter < iterations; iter++) {
            for (size_t i = 0; i < perThread; i++) {
                // Access pattern that causes cross-NUMA access
                size_t idx = (i * numThreads + threadId) % size;
                data[idx] = data[idx] + 1;
            }
            
            if (verbose && iter % 10 == 0) {
                #pragma omp critical
                {
                    std::cout << "Thread " << threadId << " completed iteration " << iter << std::endl;
                }
            }
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime).count() / 1000.0;  // Convert to seconds
    
    // Calculate memory statistics
    double totalBytes = static_cast<double>(bytes) * iterations * 2;  // Read + Write
    double bandwidthMBps = (totalBytes / MB) / duration;
    
    std::cout << "NUMA effects test completed in " << duration << " seconds" << std::endl;
    std::cout << "Memory bandwidth: " << std::fixed << std::setprecision(2) 
              << bandwidthMBps << " MB/s" << std::endl;
    
    MemoryStats stats;
    stats.totalTime = duration;
    stats.bandwidthMBps = bandwidthMBps;
    stats.elementsPerSecond = (size * iterations) / duration;
    stats.missRate = 0.6;  // Estimated moderately high for cross-NUMA access
    
    return stats;
}

// Run all memory tests and compare
void compareAllMemoryTests(int numThreads, size_t sizeInMB, bool verbose, const std::string& reportFile) {
    size_t sizeInElements = (sizeInMB * MB) / sizeof(int);
    
    std::cout << "=== Memory Access Pattern Comparison ===" << std::endl;
    std::cout << "Allocating " << sizeInMB << " MB of memory for tests..." << std::endl;
    
    // Allocate memory for tests
    SimpleArray array(sizeInElements);
    
    // Reset profiler
    Profiler::getInstance().reset();
    
    // Run memory tests
    auto seqStats = measureSequentialMemory(array.data, sizeInElements, numThreads, verbose);
    std::cout << std::endl;
    
    auto stridedStats = measureStridedMemory(array.data, sizeInElements, 16, numThreads, verbose);
    std::cout << std::endl;
    
    auto randomStats = measureRandomMemory(array.data, sizeInElements, numThreads, verbose);
    std::cout << std::endl;
    
    // Run cache thrashing test with a smaller dataset
    SimpleArray smallArray(16 * KB / sizeof(int));
    auto thrashingStats = demonstrateCacheThrashing(smallArray.data, smallArray.size, numThreads, verbose);
    std::cout << std::endl;
    
    // Run NUMA effects test
    auto numaStats = demonstrateNUMAEffects(array.data, sizeInElements, numThreads, verbose);
    
    // Print comparison
    std::cout << "\n=== Memory Performance Summary ===\n";
    std::cout << std::left << std::setw(25) << "Access Pattern" 
              << std::right << std::setw(15) << "Time (s)" 
              << std::right << std::setw(20) << "Bandwidth (MB/s)"
              << std::right << std::setw(20) << "Relative" << std::endl;
    std::cout << std::string(80, '-') << std::endl;
    
    // Calculate relative performance based on bandwidth
    double maxBandwidth = std::max({seqStats.bandwidthMBps, stridedStats.bandwidthMBps, 
                                   randomStats.bandwidthMBps, thrashingStats.bandwidthMBps, 
                                   numaStats.bandwidthMBps});
    
    std::cout << std::left << std::setw(25) << "Sequential Access" 
              << std::right << std::setw(15) << std::fixed << std::setprecision(2) << seqStats.totalTime
              << std::right << std::setw(20) << std::fixed << std::setprecision(2) << seqStats.bandwidthMBps
              << std::right << std::setw(20) << std::fixed << std::setprecision(2) 
              << seqStats.bandwidthMBps / maxBandwidth << "x" << std::endl;
    
    std::cout << std::left << std::setw(25) << "Strided Access" 
              << std::right << std::setw(15) << std::fixed << std::setprecision(2) << stridedStats.totalTime
              << std::right << std::setw(20) << std::fixed << std::setprecision(2) << stridedStats.bandwidthMBps
              << std::right << std::setw(20) << std::fixed << std::setprecision(2) 
              << stridedStats.bandwidthMBps / maxBandwidth << "x" << std::endl;
    
    std::cout << std::left << std::setw(25) << "Random Access" 
              << std::right << std::setw(15) << std::fixed << std::setprecision(2) << randomStats.totalTime
              << std::right << std::setw(20) << std::fixed << std::setprecision(2) << randomStats.bandwidthMBps
              << std::right << std::setw(20) << std::fixed << std::setprecision(2) 
              << randomStats.bandwidthMBps / maxBandwidth << "x" << std::endl;
    
    std::cout << std::left << std::setw(25) << "Cache Thrashing" 
              << std::right << std::setw(15) << std::fixed << std::setprecision(2) << thrashingStats.totalTime
              << std::right << std::setw(20) << std::fixed << std::setprecision(2) << thrashingStats.bandwidthMBps
              << std::right << std::setw(20) << std::fixed << std::setprecision(2) 
              << thrashingStats.bandwidthMBps / maxBandwidth << "x" << std::endl;
    
    std::cout << std::left << std::setw(25) << "NUMA Effects" 
              << std::right << std::setw(15) << std::fixed << std::setprecision(2) << numaStats.totalTime
              << std::right << std::setw(20) << std::fixed << std::setprecision(2) << numaStats.bandwidthMBps
              << std::right << std::setw(20) << std::fixed << std::setprecision(2) 
              << numaStats.bandwidthMBps / maxBandwidth << "x" << std::endl;
    
    // Save report to CSV if specified
    if (!reportFile.empty()) {
        std::ofstream file(reportFile);
        if (file.is_open()) {
            file << "Access Pattern,Time (s),Bandwidth (MB/s),Elements/s,Relative,Est. Miss Rate" << std::endl;
            file << "Sequential Access," << std::fixed << std::setprecision(2) << seqStats.totalTime << ","
                 << std::fixed << std::setprecision(2) << seqStats.bandwidthMBps << ","
                 << std::fixed << std::setprecision(2) << seqStats.elementsPerSecond << ","
                 << std::fixed << std::setprecision(2) << seqStats.bandwidthMBps / maxBandwidth << ","
                 << std::fixed << std::setprecision(2) << seqStats.missRate << std::endl;
            
            file << "Strided Access," << std::fixed << std::setprecision(2) << stridedStats.totalTime << ","
                 << std::fixed << std::setprecision(2) << stridedStats.bandwidthMBps << ","
                 << std::fixed << std::setprecision(2) << stridedStats.elementsPerSecond << ","
                 << std::fixed << std::setprecision(2) << stridedStats.bandwidthMBps / maxBandwidth << ","
                 << std::fixed << std::setprecision(2) << stridedStats.missRate << std::endl;
            
            file << "Random Access," << std::fixed << std::setprecision(2) << randomStats.totalTime << ","
                 << std::fixed << std::setprecision(2) << randomStats.bandwidthMBps << ","
                 << std::fixed << std::setprecision(2) << randomStats.elementsPerSecond << ","
                 << std::fixed << std::setprecision(2) << randomStats.bandwidthMBps / maxBandwidth << ","
                 << std::fixed << std::setprecision(2) << randomStats.missRate << std::endl;
            
            file << "Cache Thrashing," << std::fixed << std::setprecision(2) << thrashingStats.totalTime << ","
                 << std::fixed << std::setprecision(2) << thrashingStats.bandwidthMBps << ","
                 << std::fixed << std::setprecision(2) << thrashingStats.elementsPerSecond << ","
                 << std::fixed << std::setprecision(2) << thrashingStats.bandwidthMBps / maxBandwidth << ","
                 << std::fixed << std::setprecision(2) << thrashingStats.missRate << std::endl;
            
            file << "NUMA Effects," << std::fixed << std::setprecision(2) << numaStats.totalTime << ","
                 << std::fixed << std::setprecision(2) << numaStats.bandwidthMBps << ","
                 << std::fixed << std::setprecision(2) << numaStats.elementsPerSecond << ","
                 << std::fixed << std::setprecision(2) << numaStats.bandwidthMBps / maxBandwidth << ","
                 << std::fixed << std::setprecision(2) << numaStats.missRate << std::endl;
            
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
    int sizeInMB = parser.getIntOption("size", 256);  // Default to 256MB
    bool verbose = parser.getBoolOption("verbose", false);
    bool quiet = parser.getBoolOption("quiet", false);
    std::string reportFile = parser.getStringOption("report", "");
    std::string mode = parser.getStringOption("mode", "all");
    
    // Enable profiling in profile build
    #ifdef PROFILE
    Profiler::getInstance().setEnabled(true);
    #endif
    
    if (!quiet) {
        std::cout << "=== OpenMP Memory Issues Demo ===" << std::endl;
        std::cout << "Threads: " << threads << std::endl;
        std::cout << "Memory Size: " << sizeInMB << " MB" << std::endl;
        std::cout << std::endl;
    }
    
    // Calculate the number of elements based on size in MB
    size_t sizeInElements = (sizeInMB * MB) / sizeof(int);
    
    // Run selected memory test
    if (mode == "all") {
        compareAllMemoryTests(threads, sizeInMB, verbose, reportFile);
    }
    else if (mode == "sequential") {
        SimpleArray array(sizeInElements);
        auto stats = measureSequentialMemory(array.data, array.size, threads, verbose);
        
        if (!reportFile.empty()) {
            std::ofstream file(reportFile);
            if (file.is_open()) {
                file << "Access Pattern,Time (s),Bandwidth (MB/s),Elements/s,Est. Miss Rate" << std::endl;
                file << "Sequential Access," << std::fixed << std::setprecision(2) << stats.totalTime << ","
                     << std::fixed << std::setprecision(2) << stats.bandwidthMBps << ","
                     << std::fixed << std::setprecision(2) << stats.elementsPerSecond << ","
                     << std::fixed << std::setprecision(2) << stats.missRate << std::endl;
                
                std::cout << "Performance report saved to: " << reportFile << std::endl;
            }
        }
    }
    else if (mode == "strided") {
        SimpleArray array(sizeInElements);
        auto stats = measureStridedMemory(array.data, array.size, 16, threads, verbose);
        
        if (!reportFile.empty()) {
            std::ofstream file(reportFile);
            if (file.is_open()) {
                file << "Access Pattern,Time (s),Bandwidth (MB/s),Elements/s,Est. Miss Rate" << std::endl;
                file << "Strided Access," << std::fixed << std::setprecision(2) << stats.totalTime << ","
                     << std::fixed << std::setprecision(2) << stats.bandwidthMBps << ","
                     << std::fixed << std::setprecision(2) << stats.elementsPerSecond << ","
                     << std::fixed << std::setprecision(2) << stats.missRate << std::endl;
                
                std::cout << "Performance report saved to: " << reportFile << std::endl;
            }
        }
    }
    else if (mode == "random") {
        SimpleArray array(sizeInElements);
        auto stats = measureRandomMemory(array.data, array.size, threads, verbose);
        
        if (!reportFile.empty()) {
            std::ofstream file(reportFile);
            if (file.is_open()) {
                file << "Access Pattern,Time (s),Bandwidth (MB/s),Elements/s,Est. Miss Rate" << std::endl;
                file << "Random Access," << std::fixed << std::setprecision(2) << stats.totalTime << ","
                     << std::fixed << std::setprecision(2) << stats.bandwidthMBps << ","
                     << std::fixed << std::setprecision(2) << stats.elementsPerSecond << ","
                     << std::fixed << std::setprecision(2) << stats.missRate << std::endl;
                
                std::cout << "Performance report saved to: " << reportFile << std::endl;
            }
        }
    }
    else if (mode == "thrashing") {
        SimpleArray array(16 * KB / sizeof(int));
        auto stats = demonstrateCacheThrashing(array.data, array.size, threads, verbose);
        
        if (!reportFile.empty()) {
            std::ofstream file(reportFile);
            if (file.is_open()) {
                file << "Access Pattern,Time (s),Bandwidth (MB/s),Elements/s,Est. Miss Rate" << std::endl;
                file << "Cache Thrashing," << std::fixed << std::setprecision(2) << stats.totalTime << ","
                     << std::fixed << std::setprecision(2) << stats.bandwidthMBps << ","
                     << std::fixed << std::setprecision(2) << stats.elementsPerSecond << ","
                     << std::fixed << std::setprecision(2) << stats.missRate << std::endl;
                
                std::cout << "Performance report saved to: " << reportFile << std::endl;
            }
        }
    }
    else if (mode == "numa") {
        SimpleArray array(sizeInElements);
        auto stats = demonstrateNUMAEffects(array.data, array.size, threads, verbose);
        
        if (!reportFile.empty()) {
            std::ofstream file(reportFile);
            if (file.is_open()) {
                file << "Access Pattern,Time (s),Bandwidth (MB/s),Elements/s,Est. Miss Rate" << std::endl;
                file << "NUMA Effects," << std::fixed << std::setprecision(2) << stats.totalTime << ","
                     << std::fixed << std::setprecision(2) << stats.bandwidthMBps << ","
                     << std::fixed << std::setprecision(2) << stats.elementsPerSecond << ","
                     << std::fixed << std::setprecision(2) << stats.missRate << std::endl;
                
                std::cout << "Performance report saved to: " << reportFile << std::endl;
            }
        }
    }
    else {
        if (!quiet) {
            std::cout << "Unknown memory test: " << mode << ". Using 'all'." << std::endl;
        }
        compareAllMemoryTests(threads, sizeInMB, verbose, reportFile);
    }
    
    return 0;
}