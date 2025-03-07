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
#include "../include/cli_parser.h"
#include "../include/profiler.h"
#include "../include/debug_utils.h"

/**
 * @file excessive_synchronization.cpp
 * @brief Example demonstrating excessive synchronization issues in OpenMP programs
 * 
 * This example shows how excessive synchronization can limit parallel performance
 * by causing threads to wait for each other unnecessarily. It demonstrates several
 * common synchronization patterns and their performance impact.
 */

// Global mutex for synchronization
std::mutex g_mutex;

// Global variables for critical section demonstration
std::vector<double> g_results;
std::atomic<int> g_atomicCounter(0);
omp_lock_t g_ompLock;

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

// Demonstrate fine-grained critical sections
double demoFineCritical(int numThreads, int elements, bool verbose) {
    // Reset global results vector
    g_results.clear();
    g_results.resize(elements, 0.0);
    
    // Reset statistics
    g_stats.reset();
    
    std::cout << "Running fine-grained critical section demo with " << numThreads 
              << " threads and " << elements << " elements..." << std::endl;
    
    // Start profiling
    PROFILE_SCOPE("FineCritical");
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Process elements with excessive synchronization
    #pragma omp parallel num_threads(numThreads)
    {
        int threadId = omp_get_thread_num();
        auto threadStartTime = std::chrono::high_resolution_clock::now();
        double threadSyncTime = 0.0;
        int criticalCount = 0;
        
        #pragma omp for
        for (int i = 0; i < elements; i++) {
            // Perform work outside critical section
            double result = doMediumWork();
            
            // Enter critical section for every result update
            auto syncStart = std::chrono::high_resolution_clock::now();
            #pragma omp critical
            {
                g_results[i] = result;
                criticalCount++;
            }
            auto syncEnd = std::chrono::high_resolution_clock::now();
            
            // Track time spent in synchronization
            threadSyncTime += std::chrono::duration_cast<std::chrono::microseconds>(
                syncEnd - syncStart).count() / 1000.0;
            
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
    
    std::cout << "Fine-grained critical sections completed in " << duration << " ms" << std::endl;
    
    return duration;
}

// Demonstrate coarse-grained critical sections
double demoCoarseCritical(int numThreads, int elements, bool verbose) {
    // Reset global results vector
    g_results.clear();
    g_results.resize(elements, 0.0);
    
    // Reset statistics
    g_stats.reset();
    
    std::cout << "Running coarse-grained critical section demo with " << numThreads 
              << " threads and " << elements << " elements..." << std::endl;
    
    // Start profiling
    PROFILE_SCOPE("CoarseCritical");
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Process elements with a single large critical section per thread
    #pragma omp parallel num_threads(numThreads)
    {
        int threadId = omp_get_thread_num();
        auto threadStartTime = std::chrono::high_resolution_clock::now();
        double threadSyncTime = 0.0;
        int criticalCount = 0;
        
        // Local storage for results
        std::vector<double> localResults;
        
        #pragma omp for
        for (int i = 0; i < elements; i++) {
            // Perform work outside critical section
            double result = doMediumWork();
            
            // Store result locally
            localResults.push_back(result);
        }
        
        // Enter critical section once to update all results
        auto syncStart = std::chrono::high_resolution_clock::now();
        #pragma omp critical
        {
            int localIndex = 0;
            for (int i = threadId * (elements / numThreads); 
                 i < (threadId + 1) * (elements / numThreads) && i < elements; 
                 i++) {
                if (localIndex < static_cast<int>(localResults.size())) {
                    g_results[i] = localResults[localIndex++];
                }
            }
            criticalCount++;
        }
        auto syncEnd = std::chrono::high_resolution_clock::now();
        
        // Track time spent in synchronization
        threadSyncTime += std::chrono::duration_cast<std::chrono::microseconds>(
            syncEnd - syncStart).count() / 1000.0;
        
        if (verbose) {
            #pragma omp critical
            {
                std::cout << "Thread " << threadId << " processed " 
                          << localResults.size() << " elements" << std::endl;
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
    
    std::cout << "Coarse-grained critical sections completed in " << duration << " ms" << std::endl;
    
    return duration;
}

// Demonstrate excessive locking
double demoExcessiveLocking(int numThreads, int elements, bool verbose) {
    // Reset global counter
    g_atomicCounter = 0;
    
    // Initialize OpenMP lock
    omp_init_lock(&g_ompLock);
    
    // Reset statistics
    g_stats.reset();
    
    std::cout << "Running excessive locking demo with " << numThreads 
              << " threads and " << elements << " elements..." << std::endl;
    
    // Start profiling
    PROFILE_SCOPE("ExcessiveLocking");
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Process elements with excessive lock/unlock cycles
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
            
            // Take a lock for a tiny operation (excessive synchronization)
            auto syncStart = std::chrono::high_resolution_clock::now();
            omp_set_lock(&g_ompLock);
            g_atomicCounter++;
            omp_unset_lock(&g_ompLock);
            auto syncEnd = std::chrono::high_resolution_clock::now();
            
            // Track time spent in synchronization
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
    
    // Destroy the lock when done
    omp_destroy_lock(&g_ompLock);
    
    auto endTime = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime).count();
    
    std::cout << "Excessive locking completed in " << duration << " ms" << std::endl;
    std::cout << "Final counter value: " << g_atomicCounter << " (expected: " << elements << ")" << std::endl;
    
    return duration;
}

// Demonstrate excessive barriers
double demoExcessiveBarriers(int numThreads, int elements, bool verbose) {
    // Reset global results vector
    g_results.clear();
    g_results.resize(elements, 0.0);
    
    // Reset statistics
    g_stats.reset();
    
    std::cout << "Running excessive barriers demo with " << numThreads 
              << " threads and " << elements << " iterations..." << std::endl;
    
    // Start profiling
    PROFILE_SCOPE("ExcessiveBarriers");
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Create array with different workloads
    std::vector<int> workSizes(elements);
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(1, 3);
    
    for (int i = 0; i < elements; i++) {
        workSizes[i] = dist(rng);
    }
    
    // Process with excessive barriers
    #pragma omp parallel num_threads(numThreads)
    {
        int threadId = omp_get_thread_num();
        auto threadStartTime = std::chrono::high_resolution_clock::now();
        double threadSyncTime = 0.0;
        int criticalCount = 0;
        
        for (int i = 0; i < elements / 10; i++) {  // Divide by 10 to reduce iteration count
            // Each thread processes its portion
            #pragma omp for nowait
            for (int j = 0; j < 10; j++) {
                int index = i * 10 + j;
                if (index < elements) {
                    // Perform varying amounts of work
                    double result = 0.0;
                    
                    switch (workSizes[index]) {
                        case 1: result = doSmallWork(); break;
                        case 2: result = doMediumWork(); break;
                        case 3: result = doLargeWork(); break;
                    }
                    
                    g_results[index] = result;
                }
            }
            
            // Excessive barrier after every small batch (problematic)
            auto syncStart = std::chrono::high_resolution_clock::now();
            #pragma omp barrier
            auto syncEnd = std::chrono::high_resolution_clock::now();
            
            // Track time spent in synchronization
            threadSyncTime += std::chrono::duration_cast<std::chrono::microseconds>(
                syncEnd - syncStart).count() / 1000.0;
            criticalCount++;
            
            if (verbose && i % 10 == 0) {
                #pragma omp master
                {
                    std::cout << "Completed batch " << i << std::endl;
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
    
    std::cout << "Excessive barriers completed in " << duration << " ms" << std::endl;
    
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
        double syncPercentage = (g_stats.syncTime[i] / g_stats.totalTime[i]) * 100.0;
        
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

// Run all synchronization demos and compare
void compareAllSynchronizationPatterns(int numThreads, int elements, bool verbose, const std::string& reportFile) {
    // Initialize statistics
    g_stats.initialize(numThreads);
    
    // Reset profiler
    Profiler::getInstance().reset();
    
    std::cout << "=== Excessive Synchronization Patterns Comparison ===\n" << std::endl;
    
    // Test fine-grained critical sections
    double fineTime = demoFineCritical(numThreads, elements, verbose);
    double fineOverhead = (std::accumulate(g_stats.syncTime.begin(), g_stats.syncTime.end(), 0.0) / 
                          std::accumulate(g_stats.totalTime.begin(), g_stats.totalTime.end(), 0.0)) * 100.0;
    std::vector<double> fineTimes = g_stats.totalTime;
    std::vector<double> fineSyncTimes = g_stats.syncTime;
    displaySyncStats(numThreads);
    std::cout << std::endl;
    
    // Test coarse-grained critical sections
    double coarseTime = demoCoarseCritical(numThreads, elements, verbose);
    double coarseOverhead = (std::accumulate(g_stats.syncTime.begin(), g_stats.syncTime.end(), 0.0) / 
                            std::accumulate(g_stats.totalTime.begin(), g_stats.totalTime.end(), 0.0)) * 100.0;
    std::vector<double> coarseTimes = g_stats.totalTime;
    std::vector<double> coarseSyncTimes = g_stats.syncTime;
    displaySyncStats(numThreads);
    std::cout << std::endl;
    
    // Test excessive locking
    double lockingTime = demoExcessiveLocking(numThreads, elements, verbose);
    double lockingOverhead = (std::accumulate(g_stats.syncTime.begin(), g_stats.syncTime.end(), 0.0) / 
                             std::accumulate(g_stats.totalTime.begin(), g_stats.totalTime.end(), 0.0)) * 100.0;
    std::vector<double> lockingTimes = g_stats.totalTime;
    std::vector<double> lockingSyncTimes = g_stats.syncTime;
    displaySyncStats(numThreads);
    std::cout << std::endl;
    
    // Test excessive barriers
    double barrierTime = demoExcessiveBarriers(numThreads, elements, verbose);
    double barrierOverhead = (std::accumulate(g_stats.syncTime.begin(), g_stats.syncTime.end(), 0.0) / 
                             std::accumulate(g_stats.totalTime.begin(), g_stats.totalTime.end(), 0.0)) * 100.0;
    std::vector<double> barrierTimes = g_stats.totalTime;
    std::vector<double> barrierSyncTimes = g_stats.syncTime;
    displaySyncStats(numThreads);
    
    // Print comparison
    std::cout << "\n=== Performance Summary ===\n";
    std::cout << std::left << std::setw(25) << "Synchronization Pattern" 
              << std::right << std::setw(15) << "Time (ms)" 
              << std::right << std::setw(20) << "Sync Overhead (%)"
              << std::right << std::setw(15) << "Relative" << std::endl;
    std::cout << std::string(75, '-') << std::endl;
    
    // Find the fastest approach for relative comparison
    double bestTime = std::min({fineTime, coarseTime, lockingTime, barrierTime});
    
    std::cout << std::left << std::setw(25) << "Fine-grained Critical" 
              << std::right << std::setw(15) << fineTime 
              << std::right << std::setw(19) << std::fixed << std::setprecision(1) << fineOverhead << "%"
              << std::right << std::setw(15) << std::fixed << std::setprecision(2) 
              << fineTime / bestTime << "x" << std::endl;
    
    std::cout << std::left << std::setw(25) << "Coarse-grained Critical" 
              << std::right << std::setw(15) << coarseTime
              << std::right << std::setw(19) << std::fixed << std::setprecision(1) << coarseOverhead << "%"
              << std::right << std::setw(15) << std::fixed << std::setprecision(2) 
              << coarseTime / bestTime << "x" << std::endl;
    
    std::cout << std::left << std::setw(25) << "Excessive Locking" 
              << std::right << std::setw(15) << lockingTime
              << std::right << std::setw(19) << std::fixed << std::setprecision(1) << lockingOverhead << "%"
              << std::right << std::setw(15) << std::fixed << std::setprecision(2) 
              << lockingTime / bestTime << "x" << std::endl;
    
    std::cout << std::left << std::setw(25) << "Excessive Barriers" 
              << std::right << std::setw(15) << barrierTime
              << std::right << std::setw(19) << std::fixed << std::setprecision(1) << barrierOverhead << "%"
              << std::right << std::setw(15) << std::fixed << std::setprecision(2) 
              << barrierTime / bestTime << "x" << std::endl;
    
    // Save report to CSV if specified
    if (!reportFile.empty()) {
        std::ofstream file(reportFile);
        if (file.is_open()) {
            file << "Pattern,Time (ms),Sync Overhead (%),Relative" << std::endl;
            file << "Fine-grained Critical," << fineTime << "," << std::fixed << std::setprecision(1) 
                 << fineOverhead << "," << std::fixed << std::setprecision(2) 
                 << fineTime / bestTime << std::endl;
            file << "Coarse-grained Critical," << coarseTime << "," << std::fixed << std::setprecision(1) 
                 << coarseOverhead << "," << std::fixed << std::setprecision(2) 
                 << coarseTime / bestTime << std::endl;
            file << "Excessive Locking," << lockingTime << "," << std::fixed << std::setprecision(1) 
                 << lockingOverhead << "," << std::fixed << std::setprecision(2) 
                 << lockingTime / bestTime << std::endl;
            file << "Excessive Barriers," << barrierTime << "," << std::fixed << std::setprecision(1) 
                 << barrierOverhead << "," << std::fixed << std::setprecision(2) 
                 << barrierTime / bestTime << std::endl;
            
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
        std::cout << "=== OpenMP Excessive Synchronization Demo ===" << std::endl;
        std::cout << "Threads: " << threads << std::endl;
        std::cout << "Elements: " << elements << std::endl;
        std::cout << std::endl;
    }
    
    // Initialize thread statistics
    g_stats.initialize(threads);
    
    // Run selected synchronization pattern
    if (mode == "all") {
        compareAllSynchronizationPatterns(threads, elements, verbose, reportFile);
    }
    else if (mode == "fine" || mode == "fine-critical") {
        double time = demoFineCritical(threads, elements, verbose);
        displaySyncStats(threads);
        
        if (!reportFile.empty()) {
            std::ofstream file(reportFile);
            if (file.is_open()) {
                file << "Pattern,Time (ms),Sync Overhead (%)" << std::endl;
                
                double overhead = (std::accumulate(g_stats.syncTime.begin(), g_stats.syncTime.end(), 0.0) / 
                                  std::accumulate(g_stats.totalTime.begin(), g_stats.totalTime.end(), 0.0)) * 100.0;
                
                file << "Fine-grained Critical," << time << "," << std::fixed << std::setprecision(1) 
                     << overhead << std::endl;
                
                std::cout << "Performance report saved to: " << reportFile << std::endl;
            }
        }
    }
    else if (mode == "coarse" || mode == "coarse-critical") {
        double time = demoCoarseCritical(threads, elements, verbose);
        displaySyncStats(threads);
        
        if (!reportFile.empty()) {
            std::ofstream file(reportFile);
            if (file.is_open()) {
                file << "Pattern,Time (ms),Sync Overhead (%)" << std::endl;
                
                double overhead = (std::accumulate(g_stats.syncTime.begin(), g_stats.syncTime.end(), 0.0) / 
                                  std::accumulate(g_stats.totalTime.begin(), g_stats.totalTime.end(), 0.0)) * 100.0;
                
                file << "Coarse-grained Critical," << time << "," << std::fixed << std::setprecision(1) 
                     << overhead << std::endl;
                
                std::cout << "Performance report saved to: " << reportFile << std::endl;
            }
        }
    }
    else if (mode == "locking") {
        double time = demoExcessiveLocking(threads, elements, verbose);
        displaySyncStats(threads);
        
        if (!reportFile.empty()) {
            std::ofstream file(reportFile);
            if (file.is_open()) {
                file << "Pattern,Time (ms),Sync Overhead (%)" << std::endl;
                
                double overhead = (std::accumulate(g_stats.syncTime.begin(), g_stats.syncTime.end(), 0.0) / 
                                  std::accumulate(g_stats.totalTime.begin(), g_stats.totalTime.end(), 0.0)) * 100.0;
                
                file << "Excessive Locking," << time << "," << std::fixed << std::setprecision(1) 
                     << overhead << std::endl;
                
                std::cout << "Performance report saved to: " << reportFile << std::endl;
            }
        }
    }
    else if (mode == "barriers") {
        double time = demoExcessiveBarriers(threads, elements, verbose);
        displaySyncStats(threads);
        
        if (!reportFile.empty()) {
            std::ofstream file(reportFile);
            if (file.is_open()) {
                file << "Pattern,Time (ms),Sync Overhead (%)" << std::endl;
                
                double overhead = (std::accumulate(g_stats.syncTime.begin(), g_stats.syncTime.end(), 0.0) / 
                                  std::accumulate(g_stats.totalTime.begin(), g_stats.totalTime.end(), 0.0)) * 100.0;
                
                file << "Excessive Barriers," << time << "," << std::fixed << std::setprecision(1) 
                     << overhead << std::endl;
                
                std::cout << "Performance report saved to: " << reportFile << std::endl;
            }
        }
    }
    else {
        if (!quiet) {
            std::cout << "Unknown synchronization pattern: " << mode << ". Using 'all'." << std::endl;
        }
        compareAllSynchronizationPatterns(threads, elements, verbose, reportFile);
    }
    
    return 0;
}