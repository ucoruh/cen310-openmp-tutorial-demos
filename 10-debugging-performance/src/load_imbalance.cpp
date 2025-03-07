#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <omp.h>
#include <random>
#include <fstream>
#include <algorithm>
#include <cmath>
#include "../include/cli_parser.h"
#include "../include/profiler.h"
#include "../include/debug_utils.h"

/**
 * @file load_imbalance.cpp
 * @brief Example demonstrating load imbalance issues in OpenMP programs
 * 
 * This example shows how load imbalance can impact parallel performance
 * when workloads vary significantly across loop iterations. It demonstrates
 * the default static scheduling and its limitations.
 */

// Structure to hold work item data
struct WorkItem {
    int id;
    int complexity;  // Higher value means more work
    double result;
    
    WorkItem(int id, int complexity) : id(id), complexity(complexity), result(0.0) {}
};

// Global vector to hold thread work statistics
std::vector<int> g_threadWork;
std::vector<double> g_threadTime;

// Generate a workload with imbalanced complexity
std::vector<WorkItem> generateImbalancedWorkload(int size, int maxComplexity, int pattern) {
    std::vector<WorkItem> workload;
    workload.reserve(size);
    
    std::mt19937 rng(42);  // Fixed seed for reproducibility
    
    if (pattern == 1) {
        // Pattern 1: Ascending complexity (1, 2, 3, ...)
        for (int i = 0; i < size; i++) {
            int complexity = (i % maxComplexity) + 1;
            workload.emplace_back(i, complexity);
        }
    }
    else if (pattern == 2) {
        // Pattern 2: Descending complexity (N, N-1, N-2, ...)
        for (int i = 0; i < size; i++) {
            int complexity = maxComplexity - (i % maxComplexity);
            workload.emplace_back(i, complexity);
        }
    }
    else if (pattern == 3) {
        // Pattern 3: First half is more complex than second half
        for (int i = 0; i < size; i++) {
            int complexity = (i < size / 2) ? maxComplexity : maxComplexity / 4;
            workload.emplace_back(i, complexity);
        }
    }
    else if (pattern == 4) {
        // Pattern 4: Random complexity
        std::uniform_int_distribution<int> dist(1, maxComplexity);
        for (int i = 0; i < size; i++) {
            int complexity = dist(rng);
            workload.emplace_back(i, complexity);
        }
    }
    else if (pattern == 5) {
        // Pattern 5: Periodic spikes (simulates periodic heavy tasks)
        for (int i = 0; i < size; i++) {
            int complexity = (i % 10 == 0) ? maxComplexity : 1;
            workload.emplace_back(i, complexity);
        }
    }
    else {
        // Default: Exponential distribution
        std::exponential_distribution<double> dist(1.5);
        for (int i = 0; i < size; i++) {
            int complexity = 1 + static_cast<int>((dist(rng) * maxComplexity));
            complexity = std::min(complexity, maxComplexity);
            workload.emplace_back(i, complexity);
        }
    }
    
    return workload;
}

// Function to perform work for a given complexity
double doWork(int complexity) {
    double result = 0.0;
    
    // Simulate work by performing varying amounts of calculation
    for (int i = 0; i < complexity * 10000; i++) {
        result += std::sin(i) * std::cos(i) / (1.0 + std::abs(std::sin(i)));
    }
    
    return result;
}

// Process workload with static scheduling (default)
double processWorkloadStatic(std::vector<WorkItem>& workload, int numThreads, bool verbose) {
    // Reset thread statistics
    g_threadWork.clear();
    g_threadWork.resize(numThreads, 0);
    
    g_threadTime.clear();
    g_threadTime.resize(numThreads, 0.0);
    
    std::cout << "Processing workload with " << workload.size() << " items using "
              << numThreads << " threads and STATIC scheduling..." << std::endl;
    
    // Start profiling
    PROFILE_SCOPE("StaticScheduling");
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Process workload with static scheduling
    #pragma omp parallel num_threads(numThreads)
    {
        int threadId = omp_get_thread_num();
        auto threadStartTime = std::chrono::high_resolution_clock::now();
        
        // Static schedule (default)
        #pragma omp for schedule(static)
        for (int i = 0; i < static_cast<int>(workload.size()); i++) {
            // Track work processed by this thread
            g_threadWork[threadId]++;
            
            // Do the work based on item complexity
            workload[i].result = doWork(workload[i].complexity);
            
            if (verbose && i % 1000 == 0) {
                #pragma omp critical
                {
                    std::cout << "Thread " << threadId << " processed item " 
                              << workload[i].id << " with complexity " 
                              << workload[i].complexity << std::endl;
                }
            }
        }
        
        auto threadEndTime = std::chrono::high_resolution_clock::now();
        g_threadTime[threadId] = std::chrono::duration_cast<std::chrono::milliseconds>(
            threadEndTime - threadStartTime).count();
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime).count();
    
    std::cout << "Static scheduling completed in " << duration << " ms" << std::endl;
    
    // Display thread statistics
    std::cout << "\nThread Work Distribution:" << std::endl;
    std::cout << std::left << std::setw(10) << "Thread" 
              << std::right << std::setw(15) << "Items Processed" 
              << std::right << std::setw(15) << "Time (ms)"
              << std::right << std::setw(15) << "Efficiency" << std::endl;
    std::cout << std::string(55, '-') << std::endl;
    
    double totalItems = 0;
    double maxThreadTime = 0;
    
    for (int i = 0; i < numThreads; i++) {
        totalItems += g_threadWork[i];
        maxThreadTime = std::max(maxThreadTime, g_threadTime[i]);
    }
    
    for (int i = 0; i < numThreads; i++) {
        double efficiency = (g_threadTime[i] / maxThreadTime) * 100.0;
        std::cout << std::left << std::setw(10) << i 
                  << std::right << std::setw(15) << g_threadWork[i]
                  << std::right << std::setw(15) << std::fixed << std::setprecision(1) << g_threadTime[i]
                  << std::right << std::setw(14) << std::fixed << std::setprecision(1) << efficiency << "%" << std::endl;
    }
    
    // Calculate imbalance metrics
    double avgItems = totalItems / numThreads;
    double maxItems = *std::max_element(g_threadWork.begin(), g_threadWork.end());
    double minItems = *std::min_element(g_threadWork.begin(), g_threadWork.end());
    
    std::cout << "\nLoad Imbalance Metrics:" << std::endl;
    std::cout << "Average items per thread: " << avgItems << std::endl;
    std::cout << "Max/Min ratio: " << maxItems / minItems << std::endl;
    std::cout << "Time imbalance: " << maxThreadTime / (*std::min_element(g_threadTime.begin(), g_threadTime.end())) << "x" << std::endl;
    
    return duration;
}

// Visualize work distribution for educational purposes
void visualizeWorkDistribution(const std::vector<WorkItem>& workload, int numThreads) {
    int size = workload.size();
    int itemsPerThread = (size + numThreads - 1) / numThreads; // Ceiling division
    
    std::cout << "\nWork Distribution Visualization (Static Scheduling):" << std::endl;
    std::cout << "Each character represents one work item, value indicates complexity." << std::endl;
    
    for (int t = 0; t < numThreads; t++) {
        std::cout << "Thread " << t << ": ";
        
        int start = t * itemsPerThread;
        int end = std::min(start + itemsPerThread, size);
        
        for (int i = start; i < end; i++) {
            // Show complexity as a character (1=A, 2=B, etc.)
            char c;
            if (workload[i].complexity <= 9) {
                c = '0' + workload[i].complexity;
            } else {
                c = 'A' + (workload[i].complexity - 10);
            }
            
            std::cout << c;
        }
        
        // Calculate total work for this thread
        int totalWork = 0;
        for (int i = start; i < end; i++) {
            totalWork += workload[i].complexity;
        }
        
        std::cout << " (Total complexity: " << totalWork << ")" << std::endl;
    }
    
    std::cout << std::endl;
}

// Entry point
int main(int argc, char* argv[]) {
    // Parse command line arguments
    CliParser parser(argc, argv);
    
    // Get parameters
    int threads = parser.getIntOption("threads", std::min(4, omp_get_max_threads()));
    int workItems = parser.getIntOption("size", 10000);
    int maxComplexity = parser.getIntOption("complexity", 25);
    int pattern = parser.getIntOption("pattern", 0);
    bool verbose = parser.getBoolOption("verbose", false);
    bool quiet = parser.getBoolOption("quiet", false);
    std::string reportFile = parser.getStringOption("report", "");
    std::string mode = parser.getStringOption("mode", "static");
    
    // Enable profiling in profile build
    #ifdef PROFILE
    Profiler::getInstance().setEnabled(true);
    #endif
    
    if (!quiet) {
        std::cout << "=== OpenMP Load Imbalance Demo ===" << std::endl;
        std::cout << "Threads: " << threads << std::endl;
        std::cout << "Work Items: " << workItems << std::endl;
        std::cout << "Max Complexity: " << maxComplexity << std::endl;
        std::cout << "Pattern: " << pattern << std::endl;
        std::cout << std::endl;
    }
    
    // Generate workload
    auto workload = generateImbalancedWorkload(workItems, maxComplexity, pattern);
    
    // Visualize the static work distribution
    if (!quiet && workItems <= 200) {
        visualizeWorkDistribution(workload, threads);
    }
    
    // Process workload with static scheduling
    double staticTime = 0.0;
    
    if (mode == "static" || mode == "all") {
        staticTime = processWorkloadStatic(workload, threads, verbose);
    }
    
    // Save report to CSV if specified
    if (!reportFile.empty()) {
        std::ofstream file(reportFile);
        if (file.is_open()) {
            file << "Scheduling,Time (ms)" << std::endl;
            file << "Static," << staticTime << std::endl;
            
            // Add thread work statistics
            file << "\nThread,Items,Time (ms)" << std::endl;
            for (int i = 0; i < threads; i++) {
                file << i << "," << g_threadWork[i] << "," << g_threadTime[i] << std::endl;
            }
            
            std::cout << "Performance report saved to: " << reportFile << std::endl;
        }
    }
    
    // Save profiling data
    #ifdef PROFILE
    if (!reportFile.empty()) {
        std::string profileFile = reportFile + ".profile.csv";
        Profiler::getInstance().saveToCSV(profileFile);
        std::cout << "Profiling data saved to: " << profileFile << std::endl;
    } else {
        Profiler::getInstance().printSummary();
    }
    #endif
    
    return 0;
}