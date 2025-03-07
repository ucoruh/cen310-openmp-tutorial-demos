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
#include "../../include/cli_parser.h"
#include "../../include/profiler.h"
#include "../../include/debug_utils.h"

/**
 * @file memory_issues_fixed.cpp
 * @brief Example demonstrating solutions to memory issues in OpenMP programs
 * 
 * This example shows optimized memory access patterns:
 * 1. Cache-friendly access patterns
 * 2. Memory bandwidth optimization
 * 3. NUMA-aware allocation and access
 * 4. Cache-oblivious algorithms
 */

// Constants for memory tests
constexpr int CACHE_LINE_SIZE = 64;    // Typical cache line size in bytes
constexpr int KB = 1024;
constexpr int MB = 1024 * KB;

// Cache line aligned array structure
struct AlignedArray {
    int* data;
    size_t size;
    
    AlignedArray(size_t size_) : size(size_) {
        // Allocate cache-aligned memory
        #ifdef _WIN32
        data = (int*)_aligned_malloc(size * sizeof(int), CACHE_LINE_SIZE);
        #else
        posix_memalign((void**)&data, CACHE_LINE_SIZE, size * sizeof(int));
        #endif
        
        std::memset(data, 0, size * sizeof(int));
    }
    
    ~AlignedArray() {
        #ifdef _WIN32
        _aligned_free(data);
        #else
        free(data);
        #endif
    }
};

// Structure for memory performance statistics
struct MemoryStats {
    double totalTime;           // Total execution time
    double bandwidthMBps;       // Memory bandwidth in MB/s
    double elementsPerSecond;   // Elements processed per second
    double missRate;            // Estimated cache miss rate (0-1)
};

// Cache-friendly block structure for 2D operations
template<typename T>
class BlockedArray {
private:
    std::vector<T> data;
    size_t rows;
    size_t cols;
    size_t blockSize;

public:
    BlockedArray(size_t rows_, size_t cols_, size_t blockSize_ = 64) 
        : rows(rows_), cols(cols_), blockSize(blockSize_) {
        data.resize(rows * cols, 0);
    }
    
    // Access with row-major layout
    T& at(size_t row, size_t col) {
        return data[row * cols + col];
    }
    
    // Access with blocked layout
    T& blockAt(size_t row, size_t col) {
        // Calculate block coordinates
        size_t blockRow = row / blockSize;
        size_t blockCol = col / blockSize;
        
        // Calculate position within block
        size_t inBlockRow = row % blockSize;
        size_t inBlockCol = col % blockSize;
        
        // Calculate overall index
        size_t blocksPerRow = (cols + blockSize - 1) / blockSize;
        size_t blockIndex = blockRow * blocksPerRow + blockCol;
        size_t inBlockIndex = inBlockRow * blockSize + inBlockCol;
        
        return data[blockIndex * (blockSize * blockSize) + inBlockIndex];
    }
    
    size_t getRows() const { return rows; }
    size_t getCols() const { return cols; }
    size_t getBlockSize() const { return blockSize; }
};

// Function to generate cache-friendly sequential indices
std::vector<int> generateSequentialIndices(int size) {
    std::vector<int> indices(size);
    std::iota(indices.begin(), indices.end(), 0);  // Fill with 0, 1, 2, ...
    return indices;
}

// Optimized sequential memory access
MemoryStats measureCacheOptimizedMemory(int* data, size_t size, int numThreads, bool verbose) {
    const size_t iterations = 100;
    const size_t bytes = size * sizeof(int);
    
    std::cout << "Running cache-optimized memory access test with " << numThreads 
              << " threads and " << (bytes / MB) << " MB of data..." << std::endl;
    
    // Start profiling
    PROFILE_SCOPE("CacheOptimizedMemory");
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Each thread accesses a contiguous chunk of memory sequentially
    #pragma omp parallel num_threads(numThreads)
    {
        int threadId = omp_get_thread_num();
        size_t perThread = size / numThreads;
        size_t startIdx = threadId * perThread;
        size_t endIdx = (threadId == numThreads - 1) ? size : startIdx + perThread;
        
        // Prefetch hint (supported by most compilers)
        #if defined(__GNUC__) || defined(__clang__)
        __builtin_prefetch(&data[startIdx], 0, 3);
        #endif
        
        // Perform sequential memory access with cache optimization
        for (size_t iter = 0; iter < iterations; iter++) {
            // Read and write to memory sequentially (cache-friendly)
            for (size_t i = startIdx; i < endIdx; i++) {
                // Prefetch next cache line
                #if defined(__GNUC__) || defined(__clang__)
                if (i + 16 < endIdx) {
                    __builtin_prefetch(&data[i + 16], 1, 3);
                }
                #endif
                
                data[i] = data[i] + 1;
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
    
    std::cout << "Cache-optimized memory test completed in " << duration << " seconds" << std::endl;
    std::cout << "Memory bandwidth: " << std::fixed << std::setprecision(2) 
              << bandwidthMBps << " MB/s" << std::endl;
    
    MemoryStats stats;
    stats.totalTime = duration;
    stats.bandwidthMBps = bandwidthMBps;
    stats.elementsPerSecond = (size * iterations) / duration;
    stats.missRate = 0.0;  // Estimated very low for optimized sequential
    
    return stats;
}

// Optimized blocked access for 2D data
MemoryStats measureBlockedAccessMatrix(int numThreads, int matrixSize, int blockSize, bool verbose) {
    const size_t iterations = 10;  // Fewer iterations for matrix operations
    
    std::cout << "Running blocked matrix access test with " << numThreads 
              << " threads, matrix size " << matrixSize << "x" << matrixSize
              << ", and block size " << blockSize << "..." << std::endl;
    
    // Create matrices
    BlockedArray<int> matrixA(matrixSize, matrixSize, blockSize);
    BlockedArray<int> matrixB(matrixSize, matrixSize, blockSize);
    BlockedArray<int> matrixC(matrixSize, matrixSize, blockSize);
    
    // Start profiling
    PROFILE_SCOPE("BlockedMatrixAccess");
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Perform blocked matrix multiplication (cache-friendly)
    for (size_t iter = 0; iter < iterations; iter++) {
        #pragma omp parallel num_threads(numThreads)
        {
            int threadId = omp_get_thread_num();
            
            // Blocked matrix multiplication
            #pragma omp for
            for (int i = 0; i < matrixSize; i += blockSize) {
                for (int j = 0; j < matrixSize; j += blockSize) {
                    for (int k = 0; k < matrixSize; k += blockSize) {
                        // Multiply blocks
                        for (int ii = i; ii < std::min(i + blockSize, matrixSize); ii++) {
                            for (int jj = j; jj < std::min(j + blockSize, matrixSize); jj++) {
                                int sum = matrixC.at(ii, jj);
                                for (int kk = k; kk < std::min(k + blockSize, matrixSize); kk++) {
                                    sum += matrixA.at(ii, kk) * matrixB.at(kk, jj);
                                }
                                matrixC.at(ii, jj) = sum;
                            }
                        }
                    }
                }
            }
            
            if (verbose && iter % 2 == 0) {
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
    double matrixBytes = static_cast<double>(matrixSize * matrixSize * sizeof(int));
    double totalBytes = matrixBytes * 3 * iterations;  // Read A, B, Read/Write C
    double bandwidthMBps = (totalBytes / MB) / duration;
    
    std::cout << "Blocked matrix test completed in " << duration << " seconds" << std::endl;
    std::cout << "Memory bandwidth: " << std::fixed << std::setprecision(2) 
              << bandwidthMBps << " MB/s" << std::endl;
    
    MemoryStats stats;
    stats.totalTime = duration;
    stats.bandwidthMBps = bandwidthMBps;
    stats.elementsPerSecond = (matrixSize * matrixSize * iterations) / duration;
    stats.missRate = 0.1;  // Estimated low for blocked access
    
    return stats;
}

// NUMA-aware allocation and access
MemoryStats measureNUMAAwareAccess(int numThreads, size_t sizePerThread, bool verbose) {
    const size_t iterations = 100;
    const size_t totalSize = sizePerThread * numThreads;
    const size_t bytes = totalSize * sizeof(int);
    
    std::cout << "Running NUMA-aware access test with " << numThreads 
              << " threads and " << (bytes / MB) << " MB of data..." << std::endl;
    
    // Allocate memory for per-thread arrays (will be allocated on first-touch NUMA node)
    std::vector<AlignedArray> threadArrays;
    for (int t = 0; t < numThreads; t++) {
        threadArrays.emplace_back(sizePerThread);
    }
    
    // Start profiling
    PROFILE_SCOPE("NUMAAwareAccess");
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // First touch initialization to ensure proper NUMA placement
    #pragma omp parallel num_threads(numThreads)
    {
        int threadId = omp_get_thread_num();
        
        // Initialize thread's own memory (first-touch policy)
        for (size_t i = 0; i < sizePerThread; i++) {
            threadArrays[threadId].data[i] = threadId;
        }
    }
    
    // Now process with locality-aware access
    #pragma omp parallel num_threads(numThreads)
    {
        int threadId = omp_get_thread_num();
        
        // Process own memory (NUMA-local)
        for (size_t iter = 0; iter < iterations; iter++) {
            for (size_t i = 0; i < sizePerThread; i++) {
                threadArrays[threadId].data[i]++;
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
    
    std::cout << "NUMA-aware access test completed in " << duration << " seconds" << std::endl;
    std::cout << "Memory bandwidth: " << std::fixed << std::setprecision(2) 
              << bandwidthMBps << " MB/s" << std::endl;
    
    MemoryStats stats;
    stats.totalTime = duration;
    stats.bandwidthMBps = bandwidthMBps;
    stats.elementsPerSecond = (totalSize * iterations) / duration;
    stats.missRate = 0.05;  // Estimated very low for NUMA-local access
    
    return stats;
}

// Cache-oblivious matrix transpose
MemoryStats measureCacheObliviousTranspose(int numThreads, int matrixSize, bool verbose) {
    const size_t iterations = 20;
    
    std::cout << "Running cache-oblivious matrix transpose with " << numThreads 
              << " threads and matrix size " << matrixSize << "x" << matrixSize
              << "..." << std::endl;
    
    // Allocate matrices
    std::vector<int> matrixA(matrixSize * matrixSize, 1);
    std::vector<int> matrixB(matrixSize * matrixSize, 0);
    
    // Helper function for recursive transpose
    auto transposeRecursive = [&](auto&& self, int startRow, int startCol, 
                                  int numRows, int numCols, 
                                  const std::vector<int>& src, std::vector<int>& dst) -> void {
        // Base case: small enough to transpose directly
        if (numRows <= 32 && numCols <= 32) {
            for (int i = 0; i < numRows; i++) {
                for (int j = 0; j < numCols; j++) {
                    dst[(startCol + j) * matrixSize + (startRow + i)] = 
                        src[(startRow + i) * matrixSize + (startCol + j)];
                }
            }
            return;
        }
        
        // Recursive case: split into quadrants
        if (numRows >= numCols) {
            int halfRows = numRows / 2;
            self(self, startRow, startCol, halfRows, numCols, src, dst);
            self(self, startRow + halfRows, startCol, numRows - halfRows, numCols, src, dst);
        } else {
            int halfCols = numCols / 2;
            self(self, startRow, startCol, numRows, halfCols, src, dst);
            self(self, startRow, startCol + halfCols, numRows, numCols - halfCols, src, dst);
        }
    };
    
    // Start profiling
    PROFILE_SCOPE("CacheObliviousTranspose");
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Perform cache-oblivious transpose
    for (size_t iter = 0; iter < iterations; iter++) {
        #pragma omp parallel num_threads(numThreads)
        {
            #pragma omp single
            {
                // Start the recursive transpose
                transposeRecursive(transposeRecursive, 0, 0, matrixSize, matrixSize, matrixA, matrixB);
            }
        }
        
        // Swap matrices for next iteration
        if (iter < iterations - 1) {
            std::swap(matrixA, matrixB);
        }
        
        if (verbose && iter % 5 == 0) {
            std::cout << "Completed iteration " << iter << std::endl;
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime).count() / 1000.0;  // Convert to seconds
    
    // Calculate memory statistics
    double matrixBytes = static_cast<double>(matrixSize * matrixSize * sizeof(int));
    double totalBytes = matrixBytes * 2 * iterations;  // Read from A, Write to B
    double bandwidthMBps = (totalBytes / MB) / duration;
    
    std::cout << "Cache-oblivious transpose completed in " << duration << " seconds" << std::endl;
    std::cout << "Memory bandwidth: " << std::fixed << std::setprecision(2) 
              << bandwidthMBps << " MB/s" << std::endl;
    
    MemoryStats stats;
    stats.totalTime = duration;
    stats.bandwidthMBps = bandwidthMBps;
    stats.elementsPerSecond = (matrixSize * matrixSize * iterations) / duration;
    stats.missRate = 0.2;  // Estimated moderate for transpose (better than naive)
    
    return stats;
}

// Software prefetching for stream operations
MemoryStats measureSoftwarePrefetching(int* data, size_t size, int numThreads, bool verbose) {
    const size_t iterations = 100;
    const size_t bytes = size * sizeof(int);
    const int prefetchDistance = 16;  // Prefetch 16 elements ahead
    
    std::cout << "Running software prefetching test with " << numThreads 
              << " threads and " << (bytes / MB) << " MB of data..." << std::endl;
    
    // Start profiling
    PROFILE_SCOPE("SoftwarePrefetching");
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Stream operation with software prefetching
    #pragma omp parallel num_threads(numThreads)
    {
        int threadId = omp_get_thread_num();
        size_t perThread = size / numThreads;
        size_t startIdx = threadId * perThread;
        size_t endIdx = (threadId == numThreads - 1) ? size : startIdx + perThread;
        
        // Perform optimized stream operation
        for (size_t iter = 0; iter < iterations; iter++) {
            for (size_t i = startIdx; i < endIdx; i++) {
                // Software prefetching
                #if defined(__GNUC__) || defined(__clang__)
                if (i + prefetchDistance < endIdx) {
                    __builtin_prefetch(&data[i + prefetchDistance], 1, 3);
                }
                #endif
                
                // Stream operation (increment)
                data[i]++;
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
    
    std::cout << "Software prefetching test completed in " << duration << " seconds" << std::endl;
    std::cout << "Memory bandwidth: " << std::fixed << std::setprecision(2) 
              << bandwidthMBps << " MB/s" << std::endl;
    
    MemoryStats stats;
    stats.totalTime = duration;
    stats.bandwidthMBps = bandwidthMBps;
    stats.elementsPerSecond = (size * iterations) / duration;
    stats.missRate = 0.01;  // Estimated very low with prefetching
    
    return stats;
}

// Run all optimized memory tests and compare
void compareOptimizedMemoryTests(int numThreads, size_t sizeInMB, bool verbose, const std::string& reportFile) {
    size_t sizeInElements = (sizeInMB * MB) / sizeof(int);
    
    std::cout << "=== Optimized Memory Access Pattern Comparison ===" << std::endl;
    std::cout << "Allocating " << sizeInMB << " MB of memory for tests..." << std::endl;
    
    // Allocate aligned memory for tests
    AlignedArray array(sizeInElements);
    
    // Reset profiler
    Profiler::getInstance().reset();
    
    // Run optimized memory tests
    auto cacheOptStats = measureCacheOptimizedMemory(array.data, sizeInElements, numThreads, verbose);
    std::cout << std::endl;
    
    // Use a smaller size for matrix operations
    int matrixSize = std::min(2048, static_cast<int>(std::sqrt(sizeInElements / 4)));
    int blockSize = 32;  // Typical L1 cache-friendly block size
    
    auto blockedStats = measureBlockedAccessMatrix(numThreads, matrixSize, blockSize, verbose);
    std::cout << std::endl;
    
    auto numaStats = measureNUMAAwareAccess(numThreads, sizeInElements / numThreads, verbose);
    std::cout << std::endl;
    
    auto obliviousStats = measureCacheObliviousTranspose(numThreads, matrixSize, verbose);
    std::cout << std::endl;
    
    auto prefetchStats = measureSoftwarePrefetching(array.data, sizeInElements, numThreads, verbose);
    
    // Print comparison
    std::cout << "\n=== Optimized Memory Performance Summary ===\n";
    std::cout << std::left << std::setw(30) << "Optimization Technique" 
              << std::right << std::setw(15) << "Time (s)" 
              << std::right << std::setw(20) << "Bandwidth (MB/s)"
              << std::right << std::setw(20) << "Relative" << std::endl;
    std::cout << std::string(85, '-') << std::endl;
    
    // Calculate relative performance based on bandwidth
    double maxBandwidth = std::max({cacheOptStats.bandwidthMBps, blockedStats.bandwidthMBps, 
                                   numaStats.bandwidthMBps, obliviousStats.bandwidthMBps,
                                   prefetchStats.bandwidthMBps});
    
    std::cout << std::left << std::setw(30) << "Cache-Optimized Sequential" 
              << std::right << std::setw(15) << std::fixed << std::setprecision(2) << cacheOptStats.totalTime
              << std::right << std::setw(20) << std::fixed << std::setprecision(2) << cacheOptStats.bandwidthMBps
              << std::right << std::setw(20) << std::fixed << std::setprecision(2) 
              << cacheOptStats.bandwidthMBps / maxBandwidth << "x" << std::endl;
    
    std::cout << std::left << std::setw(30) << "Blocked Matrix Access" 
              << std::right << std::setw(15) << std::fixed << std::setprecision(2) << blockedStats.totalTime
              << std::right << std::setw(20) << std::fixed << std::setprecision(2) << blockedStats.bandwidthMBps
              << std::right << std::setw(20) << std::fixed << std::setprecision(2) 
              << blockedStats.bandwidthMBps / maxBandwidth << "x" << std::endl;
    
    std::cout << std::left << std::setw(30) << "NUMA-Aware Access" 
              << std::right << std::setw(15) << std::fixed << std::setprecision(2) << numaStats.totalTime
              << std::right << std::setw(20) << std::fixed << std::setprecision(2) << numaStats.bandwidthMBps
              << std::right << std::setw(20) << std::fixed << std::setprecision(2) 
              << numaStats.bandwidthMBps / maxBandwidth << "x" << std::endl;
    
    std::cout << std::left << std::setw(30) << "Cache-Oblivious Algorithm" 
              << std::right << std::setw(15) << std::fixed << std::setprecision(2) << obliviousStats.totalTime
              << std::right << std::setw(20) << std::fixed << std::setprecision(2) << obliviousStats.bandwidthMBps
              << std::right << std::setw(20) << std::fixed << std::setprecision(2) 
              << obliviousStats.bandwidthMBps / maxBandwidth << "x" << std::endl;
    
    std::cout << std::left << std::setw(30) << "Software Prefetching" 
              << std::right << std::setw(15) << std::fixed << std::setprecision(2) << prefetchStats.totalTime
              << std::right << std::setw(20) << std::fixed << std::setprecision(2) << prefetchStats.bandwidthMBps
              << std::right << std::setw(20) << std::fixed << std::setprecision(2) 
              << prefetchStats.bandwidthMBps / maxBandwidth << "x" << std::endl;
    
    // Save report to CSV if specified
    if (!reportFile.empty()) {
        std::ofstream file(reportFile);
        if (file.is_open()) {
            file << "Optimization Technique,Time (s),Bandwidth (MB/s),Elements/s,Relative,Est. Miss Rate" << std::endl;
            
            file << "Cache-Optimized Sequential," << std::fixed << std::setprecision(2) << cacheOptStats.totalTime << ","
                 << std::fixed << std::setprecision(2) << cacheOptStats.bandwidthMBps << ","
                 << std::fixed << std::setprecision(2) << cacheOptStats.elementsPerSecond << ","
                 << std::fixed << std::setprecision(2) << cacheOptStats.bandwidthMBps / maxBandwidth << ","
                 << std::fixed << std::setprecision(2) << cacheOptStats.missRate << std::endl;
            
            file << "Blocked Matrix Access," << std::fixed << std::setprecision(2) << blockedStats.totalTime << ","
                 << std::fixed << std::setprecision(2) << blockedStats.bandwidthMBps << ","
                 << std::fixed << std::setprecision(2) << blockedStats.elementsPerSecond << ","
                 << std::fixed << std::setprecision(2) << blockedStats.bandwidthMBps / maxBandwidth << ","
                 << std::fixed << std::setprecision(2) << blockedStats.missRate << std::endl;
            
            file << "NUMA-Aware Access," << std::fixed << std::setprecision(2) << numaStats.totalTime << ","
                 << std::fixed << std::setprecision(2) << numaStats.bandwidthMBps << ","
                 << std::fixed << std::setprecision(2) << numaStats.elementsPerSecond << ","
                 << std::fixed << std::setprecision(2) << numaStats.bandwidthMBps / maxBandwidth << ","
                 << std::fixed << std::setprecision(2) << numaStats.missRate << std::endl;
            
            file << "Cache-Oblivious Algorithm," << std::fixed << std::setprecision(2) << obliviousStats.totalTime << ","
                 << std::fixed << std::setprecision(2) << obliviousStats.bandwidthMBps << ","
                 << std::fixed << std::setprecision(2) << obliviousStats.elementsPerSecond << ","
                 << std::fixed << std::setprecision(2) << obliviousStats.bandwidthMBps / maxBandwidth << ","
                 << std::fixed << std::setprecision(2) << obliviousStats.missRate << std::endl;
            
            file << "Software Prefetching," << std::fixed << std::setprecision(2) << prefetchStats.totalTime << ","
                 << std::fixed << std::setprecision(2) << prefetchStats.bandwidthMBps << ","
                 << std::fixed << std::setprecision(2) << prefetchStats.elementsPerSecond << ","
                 << std::fixed << std::setprecision(2) << prefetchStats.bandwidthMBps / maxBandwidth << ","
                 << std::fixed << std::setprecision(2) << prefetchStats.missRate << std::endl;
            
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
        std::cout << "=== OpenMP Memory Optimization Demo ===" << std::endl;
        std::cout << "Threads: " << threads << std::endl;
        std::cout << "Memory Size: " << sizeInMB << " MB" << std::endl;
        std::cout << std::endl;
    }
    
    // Calculate the number of elements based on size in MB
    size_t sizeInElements = (sizeInMB * MB) / sizeof(int);
    
    // Run selected optimization
    if (mode == "all") {
        compareOptimizedMemoryTests(threads, sizeInMB, verbose, reportFile);
    }
    else if (mode == "cache" || mode == "sequential") {
        AlignedArray array(sizeInElements);
        auto stats = measureCacheOptimizedMemory(array.data, array.size, threads, verbose);
        
        if (!reportFile.empty()) {
            std::ofstream file(reportFile);
            if (file.is_open()) {
                file << "Optimization,Time (s),Bandwidth (MB/s),Elements/s,Est. Miss Rate" << std::endl;
                file << "Cache-Optimized Sequential," << std::fixed << std::setprecision(2) << stats.totalTime << ","
                     << std::fixed << std::setprecision(2) << stats.bandwidthMBps << ","
                     << std::fixed << std::setprecision(2) << stats.elementsPerSecond << ","
                     << std::fixed << std::setprecision(2) << stats.missRate << std::endl;
                
                std::cout << "Performance report saved to: " << reportFile << std::endl;
            }
        }
    }
    else if (mode == "blocked" || mode == "matrix") {
        // Use a smaller size for matrix operations
        int matrixSize = std::min(2048, static_cast<int>(std::sqrt(sizeInElements / 4)));
        int blockSize = parser.getIntOption("block-size", 32);
        
        auto stats = measureBlockedAccessMatrix(threads, matrixSize, blockSize, verbose);
        
        if (!reportFile.empty()) {
            std::ofstream file(reportFile);
            if (file.is_open()) {
                file << "Optimization,Time (s),Bandwidth (MB/s),Elements/s,Est. Miss Rate" << std::endl;
                file << "Blocked Matrix Access," << std::fixed << std::setprecision(2) << stats.totalTime << ","
                     << std::fixed << std::setprecision(2) << stats.bandwidthMBps << ","
                     << std::fixed << std::setprecision(2) << stats.elementsPerSecond << ","
                     << std::fixed << std::setprecision(2) << stats.missRate << std::endl;
                
                std::cout << "Performance report saved to: " << reportFile << std::endl;
            }
        }
    }
    else if (mode == "numa") {
        auto stats = measureNUMAAwareAccess(threads, sizeInElements / threads, verbose);
        
        if (!reportFile.empty()) {
            std::ofstream file(reportFile);
            if (file.is_open()) {
                file << "Optimization,Time (s),Bandwidth (MB/s),Elements/s,Est. Miss Rate" << std::endl;
                file << "NUMA-Aware Access," << std::fixed << std::setprecision(2) << stats.totalTime << ","
                     << std::fixed << std::setprecision(2) << stats.bandwidthMBps << ","
                     << std::fixed << std::setprecision(2) << stats.elementsPerSecond << ","
                     << std::fixed << std::setprecision(2) << stats.missRate << std::endl;
                
                std::cout << "Performance report saved to: " << reportFile << std::endl;
            }
        }
    }
    else if (mode == "oblivious" || mode == "transpose") {
        // Use a smaller size for matrix operations
        int matrixSize = std::min(2048, static_cast<int>(std::sqrt(sizeInElements / 4)));
        
        auto stats = measureCacheObliviousTranspose(threads, matrixSize, verbose);
        
        if (!reportFile.empty()) {
            std::ofstream file(reportFile);
            if (file.is_open()) {
                file << "Optimization,Time (s),Bandwidth (MB/s),Elements/s,Est. Miss Rate" << std::endl;
                file << "Cache-Oblivious Algorithm," << std::fixed << std::setprecision(2) << stats.totalTime << ","
                     << std::fixed << std::setprecision(2) << stats.bandwidthMBps << ","
                     << std::fixed << std::setprecision(2) << stats.elementsPerSecond << ","
                     << std::fixed << std::setprecision(2) << stats.missRate << std::endl;
                
                std::cout << "Performance report saved to: " << reportFile << std::endl;
            }
        }
    }
    else if (mode == "prefetch") {
        AlignedArray array(sizeInElements);
        auto stats = measureSoftwarePrefetching(array.data, array.size, threads, verbose);
        
        if (!reportFile.empty()) {
            std::ofstream file(reportFile);
            if (file.is_open()) {
                file << "Optimization,Time (s),Bandwidth (MB/s),Elements/s,Est. Miss Rate" << std::endl;
                file << "Software Prefetching," << std::fixed << std::setprecision(2) << stats.totalTime << ","
                     << std::fixed << std::setprecision(2) << stats.bandwidthMBps << ","
                     << std::fixed << std::setprecision(2) << stats.elementsPerSecond << ","
                     << std::fixed << std::setprecision(2) << stats.missRate << std::endl;
                
                std::cout << "Performance report saved to: " << reportFile << std::endl;
            }
        }
    }
    else {
        if (!quiet) {
            std::cout << "Unknown optimization: " << mode << ". Using 'all'." << std::endl;
        }
        compareOptimizedMemoryTests(threads, sizeInMB, verbose, reportFile);
    }
    
    return 0;
}