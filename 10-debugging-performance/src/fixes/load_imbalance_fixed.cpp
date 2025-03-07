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
#include <numeric>
#include "../../include/cli_parser.h"
#include "../../include/profiler.h"
#include "../../include/debug_utils.h"

/**
 * @file load_imbalance_fixed.cpp
 * @brief Example demonstrating solutions to load imbalance in OpenMP programs
 * 
 * This example shows different scheduling strategies to address load imbalance:
 * 1. Dynamic scheduling - assigns iterations to threads on demand
 * 2. Guided scheduling - starts with large chunks, decreases over time
 * 3. Auto scheduling - lets OpenMP runtime choose the best strategy
 * 4. Custom scheduling with manual work distribution
 */

// Structure to hold work item data
struct WorkItem {
    int id;
    int complexity;  // Higher value means more work
    double result;
    
    WorkItem(int id, int complexity) : id(id), complexity(complexity), result(0.0) {}
};

// Thread statistics
struct ThreadStats {
    std::vector<int> itemsProcessed;
    std::vector<double> timeSpent;
    std::vector<int> totalComplexity;
    
    void initialize(int numThreads) {
        itemsProcessed.resize(numThreads, 0);
        timeSpent.resize(numThreads, 0.0);
        totalComplexity.resize(numThreads, 0);
    }
    
    void reset() {
        std::fill(itemsProcessed.begin(), itemsProcessed.end(), 0);
        std::fill(timeSpent.begin(), timeSpent.end(), 0.0);
        std::fill(totalComplexity.begin(), totalComplexity.end(), 0);
    }
};

// Global statistics
ThreadStats g_stats;

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

// Process workload with dynamic scheduling
double processWorkloadDynamic(std::vector<WorkItem>& workload, int numThreads, int chunkSize, bool verbose) {
    // Reset thread statistics
    g_stats.reset();
    
    std::cout << "Processing workload with " << workload.size() << " items using "
              << numThreads << " threads and DYNAMIC scheduling (chunk size: " 
              << chunkSize << ")..." << std::endl;
    
    // Start profiling
    PROFILE_SCOPE("DynamicScheduling");
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Process workload with dynamic scheduling
    #pragma omp parallel num_threads(numThreads)
    {
        int threadId = omp_get_thread_num();
        auto threadStartTime = std::chrono::high_resolution_clock::now();
        
        // Dynamic schedule - assigns chunks of iterations on demand
        #pragma omp for schedule(dynamic, chunkSize)
        for (int i = 0; i < static_cast<int>(workload.size()); i++) {
            // Track work processed by this thread
            g_stats.itemsProcessed[threadId]++;
            g_stats.totalComplexity[threadId] += workload[i].complexity;
            
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
        g_stats.timeSpent[threadId] = std::chrono::duration_cast<std::chrono::milliseconds>(
            threadEndTime - threadStartTime).count();
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime).count();
    
    std::cout << "Dynamic scheduling completed in " << duration << " ms" << std::endl;
    
    return duration;
}

// Process workload with guided scheduling
double processWorkloadGuided(std::vector<WorkItem>& workload, int numThreads, int minChunkSize, bool verbose) {
    // Reset thread statistics
    g_stats.reset();
    
    std::cout << "Processing workload with " << workload.size() << " items using "
              << numThreads << " threads and GUIDED scheduling (min chunk size: " 
              << minChunkSize << ")..." << std::endl;
    
    // Start profiling
    PROFILE_SCOPE("GuidedScheduling");
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Process workload with guided scheduling
    #pragma omp parallel num_threads(numThreads)
    {
        int threadId = omp_get_thread_num();
        auto threadStartTime = std::chrono::high_resolution_clock::now();
        
        // Guided schedule - starts with large chunks, decreases over time
        #pragma omp for schedule(guided, minChunkSize)
        for (int i = 0; i < static_cast<int>(workload.size()); i++) {
            // Track work processed by this thread
            g_stats.itemsProcessed[threadId]++;
            g_stats.totalComplexity[threadId] += workload[i].complexity;
            
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
        g_stats.timeSpent[threadId] = std::chrono::duration_cast<std::chrono::milliseconds>(
            threadEndTime - threadStartTime).count();
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime).count();
    
    std::cout << "Guided scheduling completed in " << duration << " ms" << std::endl;
    
    return duration;
}

// Process workload with auto scheduling
double processWorkloadAuto(std::vector<WorkItem>& workload, int numThreads, bool verbose) {
    // Reset thread statistics
    g_stats.reset();
    
    std::cout << "Processing workload with " << workload.size() << " items using "
              << numThreads << " threads and AUTO scheduling..." << std::endl;
    
    // Start profiling
    PROFILE_SCOPE("AutoScheduling");
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Process workload with auto scheduling
    #pragma omp parallel num_threads(numThreads)
    {
        int threadId = omp_get_thread_num();
        auto threadStartTime = std::chrono::high_resolution_clock::now();
        
        // Auto schedule - lets OpenMP runtime choose the best strategy
        #pragma omp for schedule(static)
        for (int i = 0; i < static_cast<int>(workload.size()); i++) {
            // Track work processed by this thread
            g_stats.itemsProcessed[threadId]++;
            g_stats.totalComplexity[threadId] += workload[i].complexity;
            
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
        g_stats.timeSpent[threadId] = std::chrono::duration_cast<std::chrono::milliseconds>(
            threadEndTime - threadStartTime).count();
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime).count();
    
    std::cout << "Auto scheduling completed in " << duration << " ms" << std::endl;
    
    return duration;
}

// Process workload with complexity-aware scheduling
double processWorkloadComplexityAware(std::vector<WorkItem>& workload, int numThreads, bool verbose) {
    // Reset thread statistics
    g_stats.reset();
    
    std::cout << "Processing workload with " << workload.size() << " items using "
              << numThreads << " threads and COMPLEXITY-AWARE scheduling..." << std::endl;
    
    // Start profiling
    PROFILE_SCOPE("ComplexityAwareScheduling");
    
    // First, calculate the total complexity
    int totalComplexity = 0;
    for (const auto& item : workload) {
        totalComplexity += item.complexity;
    }
    
    // Calculate target complexity per thread
    int targetComplexity = totalComplexity / numThreads;
    
    if (verbose) {
        std::cout << "Total workload complexity: " << totalComplexity << std::endl;
        std::cout << "Target complexity per thread: " << targetComplexity << std::endl;
    }
    
    // Sort workload by complexity (descending)
    std::sort(workload.begin(), workload.end(), 
              [](const WorkItem& a, const WorkItem& b) {
                  return a.complexity > b.complexity;
              });
    
    // Assign start and end indices for each thread
    std::vector<std::pair<int, int>> threadRanges(numThreads);
    
    // Greedy partitioning algorithm
    std::vector<int> threadComplexity(numThreads, 0);
    int currentThread = 0;
    threadRanges[0].first = 0;  // First thread starts at index 0
    
    for (size_t i = 0; i < workload.size(); i++) {
        // Add current item to current thread
        threadComplexity[currentThread] += workload[i].complexity;
        
        // Check if we've reached target complexity for this thread
        if (threadComplexity[currentThread] >= targetComplexity && currentThread < numThreads - 1) {
            // Set end index for current thread
            threadRanges[currentThread].second = i + 1;
            
            // Move to next thread
            currentThread++;
            threadRanges[currentThread].first = i + 1;
        }
    }
    
    // Set end index for last thread
    threadRanges[currentThread].second = workload.size();
    
    // Fill in any empty threads (might happen with small workloads)
    for (int t = currentThread + 1; t < numThreads; t++) {
        threadRanges[t].first = workload.size();
        threadRanges[t].second = workload.size();
    }
    
    if (verbose) {
        for (int t = 0; t < numThreads; t++) {
            std::cout << "Thread " << t << " assigned items " 
                      << threadRanges[t].first << " to " << threadRanges[t].second - 1
                      << " (complexity: " << threadComplexity[t] << ")" << std::endl;
        }
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Process workload with custom complexity-aware scheduling
    #pragma omp parallel num_threads(numThreads)
    {
        int threadId = omp_get_thread_num();
        auto threadStartTime = std::chrono::high_resolution_clock::now();
        
        // Get assigned range for this thread
        int start = threadRanges[threadId].first;
        int end = threadRanges[threadId].second;
        
        // Process assigned items
        for (int i = start; i < end; i++) {
            // Track work processed by this thread
            g_stats.itemsProcessed[threadId]++;
            g_stats.totalComplexity[threadId] += workload[i].complexity;
            
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
        g_stats.timeSpent[threadId] = std::chrono::duration_cast<std::chrono::milliseconds>(
            threadEndTime - threadStartTime).count();
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime).count();
    
    std::cout << "Complexity-aware scheduling completed in " << duration << " ms" << std::endl;
    
    return duration;
}

// Display thread statistics
void displayThreadStats(int numThreads) {
    // Calculate efficiency metrics
    double maxThreadTime = *std::max_element(g_stats.timeSpent.begin(), g_stats.timeSpent.end());
    
    std::cout << "\nThread Work Distribution:" << std::endl;
    std::cout << std::left << std::setw(10) << "Thread" 
              << std::right << std::setw(15) << "Items" 
              << std::right << std::setw(15) << "Complexity"
              << std::right << std::setw(15) << "Time (ms)"
              << std::right << std::setw(15) << "Efficiency" << std::endl;
    std::cout << std::string(70, '-') << std::endl;
    
    for (int i = 0; i < numThreads; i++) {
        double efficiency = (g_stats.timeSpent[i] / maxThreadTime) * 100.0;
        std::cout << std::left << std::setw(10) << i 
                  << std::right << std::setw(15) << g_stats.itemsProcessed[i]
                  << std::right << std::setw(15) << g_stats.totalComplexity[i]
                  << std::right << std::setw(15) << std::fixed << std::setprecision(1) << g_stats.timeSpent[i]
                  << std::right << std::setw(14) << std::fixed << std::setprecision(1) << efficiency << "%" << std::endl;
    }
    
    // Calculate overall imbalance metrics
    double avgItems = std::accumulate(g_stats.itemsProcessed.begin(), g_stats.itemsProcessed.end(), 0.0) / numThreads;
    double avgComplexity = std::accumulate(g_stats.totalComplexity.begin(), g_stats.totalComplexity.end(), 0.0) / numThreads;
    double minTime = *std::min_element(g_stats.timeSpent.begin(), g_stats.timeSpent.end());
    
    std::cout << "\nLoad Balance Metrics:" << std::endl;
    std::cout << "Average items per thread: " << avgItems << std::endl;
    std::cout << "Average complexity per thread: " << avgComplexity << std::endl;
    std::cout << "Time spread: " << maxThreadTime / minTime << "x" << std::endl;
    
    // Calculate standard deviation of thread times
    double sumSq = 0.0;
    double meanTime = std::accumulate(g_stats.timeSpent.begin(), g_stats.timeSpent.end(), 0.0) / numThreads;
    
    for (int i = 0; i < numThreads; i++) {
        sumSq += std::pow(g_stats.timeSpent[i] - meanTime, 2);
    }
    
    double stdDev = std::sqrt(sumSq / numThreads);
    double cv = stdDev / meanTime * 100.0;  // Coefficient of variation in percent
    
    std::cout << "Time coefficient of variation: " << std::fixed << std::setprecision(1) << cv << "%" << std::endl;
    std::cout << "(Lower CV means better load balance)" << std::endl;
    
    std::cout << std::endl;
}

// Run all scheduling strategies and compare
void compareSchedulingStrategies(std::vector<WorkItem>& workload, int numThreads, bool verbose, const std::string& reportFile) {
    // Initialize thread statistics
    g_stats.initialize(numThreads);
    
    // Reset profiler
    Profiler::getInstance().reset();
    
    std::cout << "=== OpenMP Scheduling Strategies Comparison ===\n" << std::endl;
    
    // Test dynamic scheduling
    double dynamicTime = processWorkloadDynamic(workload, numThreads, 10, verbose);
    std::vector<int> dynamicItems = g_stats.itemsProcessed;
    std::vector<double> dynamicTimes = g_stats.timeSpent;
    displayThreadStats(numThreads);
    std::cout << std::endl;
    
    // Test guided scheduling
    double guidedTime = processWorkloadGuided(workload, numThreads, 4, verbose);
    std::vector<int> guidedItems = g_stats.itemsProcessed;
    std::vector<double> guidedTimes = g_stats.timeSpent;
    displayThreadStats(numThreads);
    std::cout << std::endl;
    
    // Test auto scheduling
    double autoTime = processWorkloadAuto(workload, numThreads, verbose);
    std::vector<int> autoItems = g_stats.itemsProcessed;
    std::vector<double> autoTimes = g_stats.timeSpent;
    displayThreadStats(numThreads);
    std::cout << std::endl;
    
    // Test complexity-aware scheduling
    double complexityAwareTime = processWorkloadComplexityAware(workload, numThreads, verbose);
    std::vector<int> complexityAwareItems = g_stats.itemsProcessed;
    std::vector<double> complexityAwareTimes = g_stats.timeSpent;
    displayThreadStats(numThreads);
    
    // Print comparison
    std::cout << "\n=== Performance Summary ===\n";
    std::cout << std::left << std::setw(25) << "Scheduling Strategy" 
              << std::right << std::setw(15) << "Time (ms)" 
              << std::right << std::setw(15) << "Speedup" << std::endl;
    std::cout << std::string(55, '-') << std::endl;
    
    // Calculate the best time for relative comparison
    double bestTime = std::min({dynamicTime, guidedTime, autoTime, complexityAwareTime});
    
    std::cout << std::left << std::setw(25) << "Dynamic" 
              << std::right << std::setw(15) << dynamicTime 
              << std::right << std::setw(15) << std::fixed << std::setprecision(2) 
              << (bestTime == 0 ? 1.0 : bestTime / dynamicTime) << "x" << std::endl;
    
    std::cout << std::left << std::setw(25) << "Guided" 
              << std::right << std::setw(15) << guidedTime
              << std::right << std::setw(15) << std::fixed << std::setprecision(2) 
              << (bestTime == 0 ? 1.0 : bestTime / guidedTime) << "x" << std::endl;
    
    std::cout << std::left << std::setw(25) << "Auto" 
              << std::right << std::setw(15) << autoTime
              << std::right << std::setw(15) << std::fixed << std::setprecision(2) 
              << (bestTime == 0 ? 1.0 : bestTime / autoTime) << "x" << std::endl;
    
    std::cout << std::left << std::setw(25) << "Complexity-Aware" 
              << std::right << std::setw(15) << complexityAwareTime
              << std::right << std::setw(15) << std::fixed << std::setprecision(2) 
              << (bestTime == 0 ? 1.0 : bestTime / complexityAwareTime) << "x" << std::endl;
    
    // Save report to CSV if specified
    if (!reportFile.empty()) {
        std::ofstream file(reportFile);
        if (file.is_open()) {
            file << "Scheduling,Time (ms),Speedup" << std::endl;
            file << "Dynamic," << dynamicTime << "," << std::fixed << std::setprecision(2) 
                 << (bestTime == 0 ? 1.0 : bestTime / dynamicTime) << std::endl;
            file << "Guided," << guidedTime << "," << std::fixed << std::setprecision(2) 
                 << (bestTime == 0 ? 1.0 : bestTime / guidedTime) << std::endl;
            file << "Auto," << autoTime << "," << std::fixed << std::setprecision(2) 
                 << (bestTime == 0 ? 1.0 : bestTime / autoTime) << std::endl;
            file << "Complexity-Aware," << complexityAwareTime << "," << std::fixed << std::setprecision(2) 
                 << (bestTime == 0 ? 1.0 : bestTime / complexityAwareTime) << std::endl;
            
            // Add thread work statistics
            file << "\nDynamic Thread Statistics" << std::endl;
            file << "Thread,Items,Time (ms)" << std::endl;
            for (int i = 0; i < numThreads; i++) {
                file << i << "," << dynamicItems[i] << "," << dynamicTimes[i] << std::endl;
            }
            
            file << "\nGuided Thread Statistics" << std::endl;
            file << "Thread,Items,Time (ms)" << std::endl;
            for (int i = 0; i < numThreads; i++) {
                file << i << "," << guidedItems[i] << "," << guidedTimes[i] << std::endl;
            }
            
            file << "\nAuto Thread Statistics" << std::endl;
            file << "Thread,Items,Time (ms)" << std::endl;
            for (int i = 0; i < numThreads; i++) {
                file << i << "," << autoItems[i] << "," << autoTimes[i] << std::endl;
            }
            
            file << "\nComplexity-Aware Thread Statistics" << std::endl;
            file << "Thread,Items,Time (ms)" << std::endl;
            for (int i = 0; i < numThreads; i++) {
                file << i << "," << complexityAwareItems[i] << "," << complexityAwareTimes[i] << std::endl;
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
    }
    #endif
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
    int chunkSize = parser.getIntOption("chunk-size", 10);
    std::string schedule = parser.getStringOption("schedule", "all");
    bool verbose = parser.getBoolOption("verbose", false);
    bool quiet = parser.getBoolOption("quiet", false);
    std::string reportFile = parser.getStringOption("report", "");
    
    // Enable profiling in profile build
    #ifdef PROFILE
    Profiler::getInstance().setEnabled(true);
    #endif
    
    if (!quiet) {
        std::cout << "=== OpenMP Load Imbalance Fixed Demo ===" << std::endl;
        std::cout << "Threads: " << threads << std::endl;
        std::cout << "Work Items: " << workItems << std::endl;
        std::cout << "Max Complexity: " << maxComplexity << std::endl;
        std::cout << "Pattern: " << pattern << std::endl;
        std::cout << std::endl;
    }
    
    // Generate workload
    auto workload = generateImbalancedWorkload(workItems, maxComplexity, pattern);
    
    // Initialize thread statistics
    g_stats.initialize(threads);
    
    // Run selected scheduling strategy
    if (schedule == "all") {
        compareSchedulingStrategies(workload, threads, verbose, reportFile);
    }
    else if (schedule == "dynamic") {
        double time = processWorkloadDynamic(workload, threads, chunkSize, verbose);
        displayThreadStats(threads);
        
        if (!reportFile.empty()) {
            std::ofstream file(reportFile);
            if (file.is_open()) {
                file << "Scheduling,Time (ms)" << std::endl;
                file << "Dynamic," << time << std::endl;
                
                // Add thread work statistics
                file << "\nThread,Items,Time (ms)" << std::endl;
                for (int i = 0; i < threads; i++) {
                    file << i << "," << g_stats.itemsProcessed[i] << "," << g_stats.timeSpent[i] << std::endl;
                }
                
                std::cout << "Performance report saved to: " << reportFile << std::endl;
            }
        }
    }
    else if (schedule == "guided") {
        double time = processWorkloadGuided(workload, threads, chunkSize, verbose);
        displayThreadStats(threads);
        
        if (!reportFile.empty()) {
            std::ofstream file(reportFile);
            if (file.is_open()) {
                file << "Scheduling,Time (ms)" << std::endl;
                file << "Guided," << time << std::endl;
                
                // Add thread work statistics
                file << "\nThread,Items,Time (ms)" << std::endl;
                for (int i = 0; i < threads; i++) {
                    file << i << "," << g_stats.itemsProcessed[i] << "," << g_stats.timeSpent[i] << std::endl;
                }
                
                std::cout << "Performance report saved to: " << reportFile << std::endl;
            }
        }
    }
    else if (schedule == "auto") {
        double time = processWorkloadAuto(workload, threads, verbose);
        displayThreadStats(threads);
        
        if (!reportFile.empty()) {
            std::ofstream file(reportFile);
            if (file.is_open()) {
                file << "Scheduling,Time (ms)" << std::endl;
                file << "Auto," << time << std::endl;
                
                // Add thread work statistics
                file << "\nThread,Items,Time (ms)" << std::endl;
                for (int i = 0; i < threads; i++) {
                    file << i << "," << g_stats.itemsProcessed[i] << "," << g_stats.timeSpent[i] << std::endl;
                }
                
                std::cout << "Performance report saved to: " << reportFile << std::endl;
            }
        }
    }
    else if (schedule == "complexity" || schedule == "custom") {
        double time = processWorkloadComplexityAware(workload, threads, verbose);
        displayThreadStats(threads);
        
        if (!reportFile.empty()) {
            std::ofstream file(reportFile);
            if (file.is_open()) {
                file << "Scheduling,Time (ms)" << std::endl;
                file << "Complexity-Aware," << time << std::endl;
                
                // Add thread work statistics
                file << "\nThread,Items,Time (ms)" << std::endl;
                for (int i = 0; i < threads; i++) {
                    file << i << "," << g_stats.itemsProcessed[i] << "," << g_stats.timeSpent[i] << std::endl;
                }
                
                std::cout << "Performance report saved to: " << reportFile << std::endl;
            }
        }
    }
    else {
        if (!quiet) {
            std::cout << "Unknown scheduling strategy: " << schedule << ". Using 'all'." << std::endl;
        }
        compareSchedulingStrategies(workload, threads, verbose, reportFile);
    }
    
    return 0;
}