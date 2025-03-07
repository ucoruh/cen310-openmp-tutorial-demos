#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <omp.h>
#include <atomic>
#include <mutex>
#include <fstream>
#include "../include/cli_parser.h"
#include "../include/profiler.h"
#include "../include/debug_utils.h"

/**
 * @file race_conditions.cpp
 * @brief Example demonstrating various race conditions in OpenMP programs
 * 
 * This example shows different types of race conditions:
 * 1. Simple counter race
 * 2. Read-modify-write race
 * 3. Complex data structure race
 * 
 * It also demonstrates how to use the race detection techniques and
 * OpenMP synchronization mechanisms to fix these issues.
 */

// Global variables for demonstration
int g_counter = 0;
std::vector<int> g_vector(1000, 0);
std::mutex g_mutex;
std::atomic<int> g_atomicCounter(0);

// Race conditions with simple counter
void demonstrateSimpleRace(int numThreads, int iterations, bool verbose) {
    g_counter = 0;
    g_atomicCounter = 0;
    
    // Watch the counter for race detection
    RaceDetector::getInstance().watchLocation(&g_counter, "g_counter");
    
    // Start profiling
    PROFILE_SCOPE("SimpleRace");
    
    std::cout << "Running simple counter race with " << numThreads 
              << " threads and " << iterations << " iterations..." << std::endl;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Problematic parallel increment (race condition)
    #pragma omp parallel num_threads(numThreads)
    {
        int threadId = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < iterations; i++) {
            // Race condition here - multiple threads updating g_counter
            RECORD_READ(g_counter); // Race detection instrumentation
            int temp = g_counter;
            
            // Simulate some work to make race more likely
            if (i % 100 == 0) {
                for (volatile int j = 0; j < 1000; j++) {}
            }
            
            temp++;
            
            RECORD_WRITE(g_counter); // Race detection instrumentation
            g_counter = temp;
            
            // Atomic counter for comparison (no race)
            g_atomicCounter.fetch_add(1, std::memory_order_relaxed);
        }
        
        if (verbose) {
            #pragma omp critical
            {
                std::cout << "Thread " << threadId << ": Current counter value = " 
                          << g_counter << std::endl;
            }
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime).count();
    
    std::cout << "Race completed in " << duration << " ms" << std::endl;
    std::cout << "Final counter value: " << g_counter 
              << " (expected: " << iterations << ")" << std::endl;
    std::cout << "Atomic counter value: " << g_atomicCounter 
              << " (expected: " << iterations << ")" << std::endl;
    
    if (g_counter != iterations) {
        std::cout << "Race condition detected! Counter value is incorrect." << std::endl;
    }
    
    std::cout << std::endl;
}

// Race conditions with complex data structure
void demonstrateComplexRace(int numThreads, int iterations, bool verbose) {
    // Initialize vector
    for (size_t i = 0; i < g_vector.size(); i++) {
        g_vector[i] = 0;
    }
    
    // Watch the vector for race detection
    RaceDetector::getInstance().watchLocation(&g_vector[0], "g_vector", g_vector.size() * sizeof(int));
    
    // Start profiling
    PROFILE_SCOPE("ComplexRace");
    
    std::cout << "Running complex data structure race with " << numThreads 
              << " threads and " << iterations << " iterations..." << std::endl;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Problematic parallel update of vector elements
    #pragma omp parallel num_threads(numThreads)
    {
        int threadId = omp_get_thread_num();
        
        // Each thread updates elements - potential overlap causing races
        for (int i = 0; i < iterations; i++) {
            // Calculate index with potential overlap between threads
            int index = (threadId + i) % g_vector.size();
            
            // Read-modify-write operation with race
            RECORD_READ(g_vector[index]);
            int value = g_vector[index];
            
            // Simulate some work
            if (i % 100 == 0) {
                for (volatile int j = 0; j < 1000; j++) {}
            }
            
            value++;
            
            RECORD_WRITE(g_vector[index]);
            g_vector[index] = value;
        }
        
        if (verbose) {
            #pragma omp critical
            {
                std::cout << "Thread " << threadId << " completed updates" << std::endl;
            }
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime).count();
    
    // Check for race evidence by counting total increments
    int totalIncrements = 0;
    for (int value : g_vector) {
        totalIncrements += value;
    }
    
    std::cout << "Complex race completed in " << duration << " ms" << std::endl;
    std::cout << "Total increments: " << totalIncrements 
              << " (expected: " << iterations * numThreads << ")" << std::endl;
    
    if (totalIncrements != iterations * numThreads) {
        std::cout << "Race condition detected! Total increments is incorrect." << std::endl;
    }
    
    std::cout << std::endl;
}

// Race conditions with read-modify-write
void demonstrateRMWRace(int numThreads, int iterations, bool verbose) {
    g_counter = 0;
    
    // Watch the counter for race detection
    RaceDetector::getInstance().watchLocation(&g_counter, "g_counter");
    
    // Start profiling
    PROFILE_SCOPE("RMWRace");
    
    std::cout << "Running read-modify-write race with " << numThreads 
              << " threads and " << iterations << " iterations..." << std::endl;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Problematic parallel increments using shorthand (still a race)
    #pragma omp parallel num_threads(numThreads)
    {
        int threadId = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < iterations; i++) {
            // Race condition with shorthand increment
            // This is still a read-modify-write operation internally
            RECORD_READ(g_counter);
            RECORD_WRITE(g_counter);
            g_counter++;  // This is not atomic!
        }
        
        if (verbose) {
            #pragma omp critical
            {
                std::cout << "Thread " << threadId << ": Current counter value = " 
                          << g_counter << std::endl;
            }
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime).count();
    
    std::cout << "RMW race completed in " << duration << " ms" << std::endl;
    std::cout << "Final counter value: " << g_counter 
              << " (expected: " << iterations << ")" << std::endl;
    
    if (g_counter != iterations) {
        std::cout << "Race condition detected! Counter value is incorrect." << std::endl;
    }
    
    std::cout << std::endl;
}

// Demonstrates all race conditions and generates performance metrics
void runAllRaceTests(int numThreads, int iterations, bool verbose, const std::string& reportFile) {
    // Reset profiler
    Profiler::getInstance().reset();
    
    // Run all race condition tests
    demonstrateSimpleRace(numThreads, iterations, verbose);
    demonstrateRMWRace(numThreads, iterations, verbose);
    demonstrateComplexRace(numThreads, iterations / 100, verbose);  // Fewer iterations for complex
    
    // Generate performance report
    if (!reportFile.empty()) {
        Profiler::getInstance().saveToCSV(reportFile);
        std::cout << "Performance report saved to: " << reportFile << std::endl;
    } else {
        std::cout << "\nPerformance Summary:" << std::endl;
        Profiler::getInstance().printSummary();
    }
}

// Entry point
int main(int argc, char* argv[]) {
    // Parse command line arguments
    CliParser parser(argc, argv);
    
    // Get parameters
    int threads = parser.getIntOption("threads", omp_get_max_threads());
    int iterations = parser.getIntOption("iterations", 1000000);
    bool verbose = parser.getBoolOption("verbose", false);
    bool quiet = parser.getBoolOption("quiet", false);
    std::string reportFile = parser.getStringOption("report", "");
    std::string mode = parser.getStringOption("mode", "all");
    
    // Enable race detection in debug mode
    #ifdef DEBUG_RACE_DETECTION
    RaceDetector::getInstance().setEnabled(true);
    #endif
    
    // Enable profiling in profile build
    #ifdef PROFILE
    Profiler::getInstance().setEnabled(true);
    #endif
    
    if (!quiet) {
        std::cout << "=== OpenMP Race Conditions Debugging Demo ===" << std::endl;
        std::cout << "Threads: " << threads << std::endl;
        std::cout << "Iterations: " << iterations << std::endl;
        std::cout << std::endl;
    }
    
    // Run selected demo mode
    if (mode == "simple" || mode == "all") {
        demonstrateSimpleRace(threads, iterations, verbose);
    }
    
    if (mode == "rmw" || mode == "all") {
        demonstrateRMWRace(threads, iterations, verbose);
    }
    
    if (mode == "complex" || mode == "all") {
        demonstrateComplexRace(threads, iterations / 100, verbose);  // Fewer iterations for complex
    }
    
    // Generate race detection report
    #ifdef DEBUG_RACE_DETECTION
    if (!reportFile.empty()) {
        std::string raceReportFile = reportFile + ".race_report";
        RaceDetector::getInstance().generateReport(raceReportFile);
        std::cout << "Race detection report saved to " << raceReportFile << std::endl;
    }
    #endif
    
    // Save profiling report
    #ifdef PROFILE
    if (!reportFile.empty()) {
        Profiler::getInstance().saveToCSV(reportFile);
        std::cout << "Performance report saved to " << reportFile << std::endl;
    } else if (!quiet) {
        Profiler::getInstance().printSummary();
    }
    #endif
    
    return 0;
}