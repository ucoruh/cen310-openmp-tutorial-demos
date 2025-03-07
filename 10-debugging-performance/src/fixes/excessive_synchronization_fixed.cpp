#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <omp.h>
#include <mutex>
#include <atomic>
#include <fstream>
#include <random>
#include <algorithm>
#include <numeric>
#include "../../include/cli_parser.h"
#include "../../include/profiler.h"
#include "../../include/debug_utils.h"

/**
 * @file excessive_synchronization_fixed.cpp
 * @brief Example demonstrating solutions to excessive synchronization in OpenMP programs
 * 
 * This example shows how to improve performance by reducing synchronization overhead:
 * 1. Using thread-local storage to eliminate critical sections
 * 2. Using atomic operations for simple counter updates
 * 3. Minimizing the use of barriers
 * 4. Batching updates to reduce synchronization frequency
 */

// Global variables
std::vector<double> g_results;
std::atomic<int> g_atomicCounter(0);

// Structure to store synchronization overhead data
struct SynchronizationStats {
    std::vector<double> totalTime;       // Total time per thread
    std::vector<double> syncTime;        // Time spent in synchronization
    std::vector<int> criticalSections;   // Number of critical sections entered
    
    void initialize(int numThreads) {
        totalTime.resize(numThreads, 0.0);
        syncTime.resize(numThreads, 0.0);
        criticalSections.resize(numThreads, 0);
    }
    
    void reset() {
        std::fill(totalTime.begin(), totalTime.end(), 0.0);
        std::fill(syncTime.begin(), syncTime.end(), 0.0);
        std::fill(criticalSections.begin(), criticalSections.end(), 0);
    }
};

// Global statistics
SynchronizationStats g_stats;

// Function to perform a small amount of work
double doSmallWork() {
    double result = 0.0;
    for (int i = 0; i < 1000; i++) {
        result += std::sin(i) * std::cos(i);
    }
    return result;
}

// Function to perform a medium amount of work
double doMediumWork() {
    double result = 0.0;
    for (int i = 0; i < 10000; i++) {
        result += std::sin(i) * std::cos(i);
    }
    return result;
}

// Function to perform a large amount of work
double doLargeWork() {
    double result = 0.0;
    for (int i = 0; i < 100000; i++) {
        result += std::sin(i) * std::cos(i);
    }
    return result;
}

// Fixed implementation: Thread-local storage instead of fine-grained critical sections
double demoThreadLocalStorage(int numThreads, int elements, bool verbose) {
    // Reset global results vector
    g_results.clear();
    g_results.resize(elements, 0.0);
    
    // Reset statistics
    g_stats.reset();
    
    std::cout << "Running thread-local storage demo with " << numThreads 
              << " threads and " << elements << " elements..." << std::endl;
    
    // Start profiling
    PROFILE_SCOPE("ThreadLocalStorage");
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Process elements using thread-local storage to avoid synchronization
    #pragma omp parallel num_threads(numThreads)
    {
        int threadId = omp_get_thread_num();
        auto threadStartTime = std::chrono::high_resolution_clock::now();
        double threadSyncTime = 0.0;
        int criticalCount = 0;
        
        // Each thread gets its own chunk of the result array
        #pragma omp for
        for (int i = 0; i < elements; i++) {
            // Perform work without critical section needed
            double result = doMediumWork();
            
            // Directly update the corresponding element
            g_results[i] = result;
            
            if (verbose && i % 1000 == 0) {
                #pragma omp critical
                {
                    std::cout << "Thread " << threadId << " processed element " << i << std::endl;
                }
            }
        }
        
        auto threadEndTime = std::chrono::high_resolution_clock::now();
        g_stats.totalTime[threadId] = std::chrono::duration_cast<std::chrono::milliseconds>(
            threadEndTime - threadStartTime).count();
        g_stats.syncTime[threadId] = threadSyncTime;
        g_stats.criticalSections[threadId] = criticalCount;
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime).count();
    
    std::cout << "Thread-local storage approach completed in " << duration << " ms" << std::endl;
    
    return duration;
}

// Fixed implementation: Atomic operations instead of locks
double demoAtomicOperations(int numThreads, int elements, bool verbose) {
    // Reset global counter
    g_atomicCounter = 0;
    
    // Reset statistics
    g_stats.reset();
    
    std::cout << "Running atomic operations demo with " << numThreads 
              << " threads and " << elements << " elements..." << std::endl;
    
    // Start profiling
    PROFILE_SCOPE("AtomicOperations");
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Process elements using atomic operations instead of locks
    #pragma omp parallel num_threads(numThreads)
    {
        int threadId = omp_get_thread_num();
        auto threadStartTime = std::chrono::high_resolution_clock::now();
        double threadSyncTime = 0.0;
        int criticalCount = 0;
        
        #pragma omp for
        for (int i = 0; i < elements; i++) {
            // Perform very small work
            double result = doSmallWork();
            
            // Use atomic operation instead of lock - much faster
            auto syncStart = std::chrono::high_resolution_clock::now();
            g_atomicCounter.fetch_add(1, std::memory_order_relaxed);
            auto syncEnd = std::chrono::high_resolution_clock::now();
            
            // Track time spent in synchronization (much less than with locks)
            threadSyncTime += std::chrono::duration_cast<std::chrono::microseconds>(
                syncEnd - syncStart).count() / 1000.0;
            criticalCount++;
            
            // Use the result to avoid optimization
            if (result < 0 && verbose && i % 1000 == 0) {
                #pragma omp critical
                {
                    std::cout << "Thread " << threadId << " processed element " << i << std::endl;
                }
            }
        }
        
        auto threadEndTime = std::chrono::high_resolution_clock::now();
        g_stats.totalTime[threadId] = std::chrono::duration_cast<std::chrono::milliseconds>(
            threadEndTime - threadStartTime).count();
        g_stats.syncTime[threadId] = threadSyncTime;
        g_stats.criticalSections[threadId] = criticalCount;
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime).count();
    
    std::cout << "Atomic operations completed in " << duration << " ms" << std::endl;
    std::cout << "Final counter value: " << g_atomicCounter << " (expected: " << elements << ")" << std::endl;
    
    return duration;
}

// Fixed implementation: Reduced barriers
double demoReducedBarriers(int numThreads, int elements, bool verbose) {
    // Reset global results vector
    g_results.clear();
    g_results.resize(elements, 0.0);
    
    // Reset statistics
    g_stats.reset();
    
    std::cout << "Running reduced barriers demo with " << numThreads 
              << " threads and " << elements << " iterations..." << std::endl;
    
    // Start profiling
    PROFILE_SCOPE("ReducedBarriers");
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Create array with different workloads
    std::vector<int> workSizes(elements);
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(1, 3);
    
    for (int i = 0; i < elements; i++) {
        workSizes[i] = dist(rng);
    }
    
    // Process with minimal barriers - only one at the end
    #pragma omp parallel num_threads(numThreads)
    {
        int threadId = omp_get_thread_num();
        auto threadStartTime = std::chrono::high_resolution_clock::now();
        double threadSyncTime = 0.0;
        int criticalCount = 0;
        
        // Process all elements with a single parallel for - no intermediate barriers
        #pragma omp for schedule(dynamic, 100)
        for (int i = 0; i < elements; i++) {
            // Perform varying amounts of work
            double result = 0.0;
            
            switch (workSizes[i]) {
                case 1: result = doSmallWork(); break;
                case 2: result = doMediumWork(); break;
                case 3: result = doLargeWork(); break;
            }
            
            g_results[i] = result;
            
            if (verbose && i % 1000 == 0) {
                #pragma omp critical
                {
                    std::cout << "Thread " << threadId << " processed element " << i << std::endl;
                }
            }
        }
        
        // Just one barrier at the end (implicit)
        auto syncStart = std::chrono::high_resolution_clock::now();
        // No explicit barrier needed - end of parallel region has implicit barrier
        auto syncEnd = std::chrono::high_resolution_clock::now();
        
        // Track time spent in synchronization
        threadSyncTime += std::chrono::duration_cast<std::chrono::microseconds>(
            syncEnd - syncStart).count() / 1000.0;
        criticalCount++;
        
        auto threadEndTime = std::chrono::high_resolution_clock::now();
        g_stats.totalTime[threadId] = std::chrono::duration_cast<std::chrono::milliseconds>(
            threadEndTime - threadStartTime).count();
        g_stats.syncTime[threadId] = threadSyncTime;
        g_stats.criticalSections[threadId] = criticalCount;
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime).count();
    
    std::cout << "Reduced barriers completed in " << duration << " ms" << std::endl;
    
    return duration;
}

// Fixed implementation: Batched updates
double demoBatchedUpdates(int numThreads, int elements, bool verbose) {
    // Reset global counter
    g_atomicCounter = 0;
    
    // Reset statistics
    g_stats.reset();
    
    std::cout << "Running batched updates demo with " << numThreads 
              << " threads and " << elements << " elements..." << std::endl;
    
    // Start profiling
    PROFILE_SCOPE("BatchedUpdates");
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Use batch size - adjust for optimal performance
    const int batchSize = 1000;
    
    // Process elements with batched updates to reduce synchronization
    #pragma omp parallel num_threads(numThreads)
    {
        int threadId = omp_get_thread_num();
        auto threadStartTime = std::chrono::high_resolution_clock::now();
        double threadSyncTime = 0.0;
        int criticalCount = 0;
        
        // Local counter for batching updates
        int localCounter = 0;
        
        #pragma omp for
        for (int i = 0; i < elements; i++) {
            // Perform very small work
            double result = doSmallWork();
            
            // Accumulate locally without synchronization
            localCounter++;
            
            // Batch update to reduce synchronization frequency
            if (localCounter >= batchSize || i == elements - 1) {
                auto syncStart = std::chrono::high_resolution_clock::now();
                g_atomicCounter.fetch_add(localCounter, std::memory_order_relaxed);
                auto syncEnd = std::chrono::high_resolution_clock::now();
                
                // Track time spent in synchronization
                threadSyncTime += std::chrono::duration_cast<std::chrono::microseconds>(
                    syncEnd - syncStart).count() / 1000.0;
                criticalCount++;
                
                // Reset local counter after batch update
                localCounter = 0;
            }
            
            // Use the result to avoid optimization
            if (result < 0 && verbose && i % 1000 == 0) {
                #pragma omp critical
                {
                    std::cout << "Thread " << threadId << " processed element " << i << std::endl;
                }
            }
        }
        
        auto threadEndTime = std::chrono::high_resolution_clock::now();
        g_stats.totalTime[threadId] = std::chrono::duration_cast<std::chrono::milliseconds>(
            threadEndTime - threadStartTime).count();
        g_stats.syncTime[threadId] = threadSyncTime;
        g_stats.criticalSections[threadId] = criticalCount;
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime).count();
    
    std::cout << "Batched updates completed in " << duration << " ms" << std::endl;
    std::cout << "Final counter value: " << g_atomicCounter << " (expected: " << elements << ")" << std::endl;
    
    return duration;
}

// Display synchronization statistics
void displaySyncStats(int numThreads) {
    // Calculate totals and averages
    double totalTime = std::accumulate(g_stats.totalTime.begin(), g_stats.totalTime.end(), 0.0);
    double totalSyncTime = std::accumulate(g_stats.syncTime.begin(), g_stats.syncTime.end(), 0.0);
    int totalCritical = std::accumulate(g_stats.criticalSections.begin(), g_stats.criticalSections.end(), 0);
    
    double avgTime = totalTime / numThreads;
    double avgSyncTime = totalSyncTime / numThreads;
    double avgCritical = static_cast<double>(totalCritical) / numThreads;
    
    // Calculate overall synchronization overhead
    double overallOverhead = (totalSyncTime / totalTime) * 100.0;
    
    std::cout << "\nSynchronization Statistics:" << std::endl;
    std::cout << std::left << std::setw(10) << "Thread" 
              << std::right << std::setw(15) << "Total (ms)" 
              << std::right << std::setw(15) << "Sync (ms)"
              << std::right << std::setw(15) << "Sync (%)"
              << std::right << std::setw(15) << "Critical Secs" << std::endl;
    std::cout << std::string(70, '-') << std::endl;
    
    for (int i = 0; i < numThreads; i++) {
        double syncPercentage = (g_stats.totalTime[i] > 0) ? 
            (g_stats.syncTime[i] / g_stats.totalTime[i]) * 100.0 : 0.0;
        
        std::cout << std::left << std::setw(10) << i 
                  << std::right << std::setw(15) << std::fixed << std::setprecision(1) << g_stats.totalTime[i]
                  << std::right << std::setw(15) << std::fixed << std::setprecision(1) << g_stats.syncTime[i]
                  << std::right << std::setw(14) << std::fixed << std::setprecision(1) << syncPercentage << "%"
                  << std::right << std::setw(15) << g_stats.criticalSections[i] << std::endl;
    }
    
    std::cout << std::string(70, '-') << std::endl;
    std::cout << std::left << std::setw(10) << "Average" 
              << std::right << std::setw(15) << std::fixed << std::setprecision(1) << avgTime
              << std::right << std::setw(15) << std::fixed << std::setprecision(1) << avgSyncTime
              << std::right << std::setw(14) << std::fixed << std::setprecision(1) << overallOverhead << "%"
              << std::right << std::setw(15) << std::fixed << std::setprecision(1) << avgCritical << std::endl;
    
    std::cout << "\nOverall synchronization overhead: " << std::fixed << std::setprecision(1) 
              << overallOverhead << "%" << std::endl;
    
    std::cout << std::endl;
}

// Run all optimized synchronization approaches and compare
void compareOptimizedApproaches(int numThreads, int elements, bool verbose, const std::string& reportFile) {
    // Initialize statistics
    g_stats.initialize(numThreads);
    
    // Reset profiler
    Profiler::getInstance().reset();
    
    std::cout << "=== Optimized Synchronization Approaches Comparison ===\n" << std::endl;
    
    // Test thread-local storage
    double localTime = demoThreadLocalStorage(numThreads, elements, verbose);
    double localOverhead = (std::accumulate(g_stats.syncTime.begin(), g_stats.syncTime.end(), 0.0) / 
                           std::max(1.0, std::accumulate(g_stats.totalTime.begin(), g_stats.totalTime.end(), 0.0))) * 100.0;
    std::vector<double> localTimes = g_stats.totalTime;
    std::vector<double> localSyncTimes = g_stats.syncTime;
    displaySyncStats(numThreads);
    std::cout << std::endl;
    
    // Test atomic operations
    double atomicTime = demoAtomicOperations(numThreads, elements, verbose);
    double atomicOverhead = (std::accumulate(g_stats.syncTime.begin(), g_stats.syncTime.end(), 0.0) / 
                            std::max(1.0, std::accumulate(g_stats.totalTime.begin(), g_stats.totalTime.end(), 0.0))) * 100.0;
    std::vector<double> atomicTimes = g_stats.totalTime;
    std::vector<double> atomicSyncTimes = g_stats.syncTime;
    displaySyncStats(numThreads);
    std::cout << std::endl;
    
    // Test reduced barriers
    double barrierTime = demoReducedBarriers(numThreads, elements, verbose);
    double barrierOverhead = (std::accumulate(g_stats.syncTime.begin(), g_stats.syncTime.end(), 0.0) / 
                             std::max(1.0, std::accumulate(g_stats.totalTime.begin(), g_stats.totalTime.end(), 0.0))) * 100.0;
    std::vector<double> barrierTimes = g_stats.totalTime;
    std::vector<double> barrierSyncTimes = g_stats.syncTime;
    displaySyncStats(numThreads);
    std::cout << std::endl;
    
    // Test batched updates
    double batchedTime = demoBatchedUpdates(numThreads, elements, verbose);
    double batchedOverhead = (std::accumulate(g_stats.syncTime.begin(), g_stats.syncTime.end(), 0.0) / 
                             std::max(1.0, std::accumulate(g_stats.totalTime.begin(), g_stats.totalTime.end(), 0.0))) * 100.0;
    std::vector<double> batchedTimes = g_stats.totalTime;
    std::vector<double> batchedSyncTimes = g_stats.syncTime;
    displaySyncStats(numThreads);
    
    // Print comparison
    std::cout << "\n=== Performance Summary ===\n";
    std::cout << std::left << std::setw(25) << "Optimization Approach" 
              << std::right << std::setw(15) << "Time (ms)" 
              << std::right << std::setw(20) << "Sync Overhead (%)"
              << std::right << std::setw(15) << "Relative" << std::endl;
    std::cout << std::string(75, '-') << std::endl;
    
    // Find the fastest approach for relative comparison
    double bestTime = std::min({localTime, atomicTime, barrierTime, batchedTime});
    
    std::cout << std::left << std::setw(25) << "Thread-local Storage" 
              << std::right << std::setw(15) << localTime 
              << std::right << std::setw(19) << std::fixed << std::setprecision(1) << localOverhead << "%"
              << std::right << std::setw(15) << std::fixed << std::setprecision(2) 
              << localTime / bestTime << "x" << std::endl;
    
    std::cout << std::left << std::setw(25) << "Atomic Operations" 
              << std::right << std::setw(15) << atomicTime
              << std::right << std::setw(19) << std::fixed << std::setprecision(1) << atomicOverhead << "%"
              << std::right << std::setw(15) << std::fixed << std::setprecision(2) 
              << atomicTime / bestTime << "x" << std::endl;
    
    std::cout << std::left << std::setw(25) << "Reduced Barriers" 
              << std::right << std::setw(15) << barrierTime
              << std::right << std::setw(19) << std::fixed << std::setprecision(1) << barrierOverhead << "%"
              << std::right << std::setw(15) << std::fixed << std::setprecision(2) 
              << barrierTime / bestTime << "x" << std::endl;
    
    std::cout << std::left << std::setw(25) << "Batched Updates" 
              << std::right << std::setw(15) << batchedTime
              << std::right << std::setw(19) << std::fixed << std::setprecision(1) << batchedOverhead << "%"
              << std::right << std::setw(15) << std::fixed << std::setprecision(2) 
              << batchedTime / bestTime << "x" << std::endl;
    
    // Save report to CSV if specified
    if (!reportFile.empty()) {
        std::ofstream file(reportFile);
        if (file.is_open()) {
            file << "Approach,Time (ms),Sync Overhead (%),Relative" << std::endl;
            file << "Thread-local Storage," << localTime << "," << std::fixed << std::setprecision(1) 
                 << localOverhead << "," << std::fixed << std::setprecision(2) 
                 << localTime / bestTime << std::endl;
            file << "Atomic Operations," << atomicTime << "," << std::fixed << std::setprecision(1) 
                 << atomicOverhead << "," << std::fixed << std::setprecision(2) 
                 << atomicTime / bestTime << std::endl;
            file << "Reduced Barriers," << barrierTime << "," << std::fixed << std::setprecision(1) 
                 << barrierOverhead << "," << std::fixed << std::setprecision(2) 
                 << barrierTime / bestTime << std::endl;
            file << "Batched Updates," << batchedTime << "," << std::fixed << std::setprecision(1) 
                 << batchedOverhead << "," << std::fixed << std::setprecision(2) 
                 << batchedTime / bestTime << std::endl;
            
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
    int elements = parser.getIntOption("elements", 100000);
    bool verbose = parser.getBoolOption("verbose", false);
    bool quiet = parser.getBoolOption("quiet", false);
    std::string reportFile = parser.getStringOption("report", "");
    std::string mode = parser.getStringOption("mode", "all");
    
    // Enable profiling in profile build
    #ifdef PROFILE
    Profiler::getInstance().setEnabled(true);
    #endif
    
    if (!quiet) {
        std::cout << "=== OpenMP Excessive Synchronization Fixed Demo ===" << std::endl;
        std::cout << "Threads: " << threads << std::endl;
        std::cout << "Elements: " << elements << std::endl;
        std::cout << std::endl;
    }
    
    // Initialize thread statistics
    g_stats.initialize(threads);
    
    // Run selected optimization approach
    if (mode == "all") {
        compareOptimizedApproaches(threads, elements, verbose, reportFile);
    }
    else if (mode == "local" || mode == "thread-local") {
        double time = demoThreadLocalStorage(threads, elements, verbose);
        displaySyncStats(threads);
        
        if (!reportFile.empty()) {
            std::ofstream file(reportFile);
            if (file.is_open()) {
                file << "Approach,Time (ms),Sync Overhead (%)" << std::endl;
                
                double overhead = (std::accumulate(g_stats.syncTime.begin(), g_stats.syncTime.end(), 0.0) / 
                                  std::max(1.0, std::accumulate(g_stats.totalTime.begin(), g_stats.totalTime.end(), 0.0))) * 100.0;
                
                file << "Thread-local Storage," << time << "," << std::fixed << std::setprecision(1) 
                     << overhead << std::endl;
                
                std::cout << "Performance report saved to: " << reportFile << std::endl;
            }
        }
    }
    else if (mode == "atomic") {
        double time = demoAtomicOperations(threads, elements, verbose);
        displaySyncStats(threads);
        
        if (!reportFile.empty()) {
            std::ofstream file(reportFile);
            if (file.is_open()) {
                file << "Approach,Time (ms),Sync Overhead (%)" << std::endl;
                
                double overhead = (std::accumulate(g_stats.syncTime.begin(), g_stats.syncTime.end(), 0.0) / 
                                  std::max(1.0, std::accumulate(g_stats.totalTime.begin(), g_stats.totalTime.end(), 0.0))) * 100.0;
                
                file << "Atomic Operations," << time << "," << std::fixed << std::setprecision(1) 
                     << overhead << std::endl;
                
                std::cout << "Performance report saved to: " << reportFile << std::endl;
            }
        }
    }
    else if (mode == "barriers" || mode == "reduced-barriers") {
        double time = demoReducedBarriers(threads, elements, verbose);
        displaySyncStats(threads);
        
        if (!reportFile.empty()) {
            std::ofstream file(reportFile);
            if (file.is_open()) {
                file << "Approach,Time (ms),Sync Overhead (%)" << std::endl;
                
                double overhead = (std::accumulate(g_stats.syncTime.begin(), g_stats.syncTime.end(), 0.0) / 
                                  std::max(1.0, std::accumulate(g_stats.totalTime.begin(), g_stats.totalTime.end(), 0.0))) * 100.0;
                
                file << "Reduced Barriers," << time << "," << std::fixed << std::setprecision(1) 
                     << overhead << std::endl;
                
                std::cout << "Performance report saved to: " << reportFile << std::endl;
            }
        }
    }
    else if (mode == "batched" || mode == "batched-updates") {
        double time = demoBatchedUpdates(threads, elements, verbose);
        displaySyncStats(threads);
        
        if (!reportFile.empty()) {
            std::ofstream file(reportFile);
            if (file.is_open()) {
                file << "Approach,Time (ms),Sync Overhead (%)" << std::endl;
                
                double overhead = (std::accumulate(g_stats.syncTime.begin(), g_stats.syncTime.end(), 0.0) / 
                                  std::max(1.0, std::accumulate(g_stats.totalTime.begin(), g_stats.totalTime.end(), 0.0))) * 100.0;
                
                file << "Batched Updates," << time << "," << std::fixed << std::setprecision(1) 
                     << overhead << std::endl;
                
                std::cout << "Performance report saved to: " << reportFile << std::endl;
            }
        }
    }
    else {
        if (!quiet) {
            std::cout << "Unknown optimization approach: " << mode << ". Using 'all'." << std::endl;
        }
        compareOptimizedApproaches(threads, elements, verbose, reportFile);
    }
    
    return 0;
}