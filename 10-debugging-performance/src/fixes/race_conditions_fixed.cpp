#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <omp.h>
#include <atomic>
#include <mutex>
#include <fstream>
#include "../../include/cli_parser.h"
#include "../../include/profiler.h"
#include "../../include/debug_utils.h"

/**
 * @file race_conditions_fixed.cpp
 * @brief Example demonstrating fixes for various race conditions in OpenMP programs
 * 
 * This example shows different solutions to race conditions:
 * 1. Simple counter race - Fixed with critical sections, atomics, and reduction
 * 2. Read-modify-write race - Fixed with atomic operations
 * 3. Complex data structure race - Fixed with proper synchronization
 */

// Global variables for demonstration
int g_counter = 0;
std::vector<int> g_vector(1000, 0);
std::mutex g_mutex;
std::atomic<int> g_atomicCounter(0);
omp_lock_t g_ompLock;

// FIXED: Simple counter race using critical sections
void demonstrateSimpleRaceFixed_Critical(int numThreads, int iterations, bool verbose) {
    g_counter = 0;
    
    // Start profiling
    PROFILE_SCOPE("SimpleRaceFixed_Critical");
    
    std::cout << "Running fixed simple counter race (critical section) with " << numThreads 
              << " threads and " << iterations << " iterations..." << std::endl;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Fixed parallel increment using critical section
    #pragma omp parallel num_threads(numThreads)
    {
        int threadId = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < iterations; i++) {
            // Fix: Use critical section to synchronize access
            #pragma omp critical
            {
                g_counter++;
            }
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
    
    std::cout << "Fixed counter race (critical) completed in " << duration << " ms" << std::endl;
    std::cout << "Final counter value: " << g_counter 
              << " (expected: " << iterations << ")" << std::endl;
    
    if (g_counter != iterations) {
        std::cout << "Error: Counter value is still incorrect!" << std::endl;
    } else {
        std::cout << "Success: Counter value is correct." << std::endl;
    }
    
    std::cout << std::endl;
}

// FIXED: Simple counter race using atomic operations
void demonstrateSimpleRaceFixed_Atomic(int numThreads, int iterations, bool verbose) {
    g_atomicCounter = 0;
    
    // Start profiling
    PROFILE_SCOPE("SimpleRaceFixed_Atomic");
    
    std::cout << "Running fixed simple counter race (atomic) with " << numThreads 
              << " threads and " << iterations << " iterations..." << std::endl;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Fixed parallel increment using atomic operations
    #pragma omp parallel num_threads(numThreads)
    {
        int threadId = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < iterations; i++) {
            // Fix: Use atomic counter
            g_atomicCounter.fetch_add(1, std::memory_order_relaxed);
        }
        
        if (verbose) {
            #pragma omp critical
            {
                std::cout << "Thread " << threadId << ": Current atomic counter value = " 
                          << g_atomicCounter << std::endl;
            }
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime).count();
    
    std::cout << "Fixed counter race (atomic) completed in " << duration << " ms" << std::endl;
    std::cout << "Final atomic counter value: " << g_atomicCounter 
              << " (expected: " << iterations << ")" << std::endl;
    
    if (g_atomicCounter != iterations) {
        std::cout << "Error: Atomic counter value is incorrect!" << std::endl;
    } else {
        std::cout << "Success: Atomic counter value is correct." << std::endl;
    }
    
    std::cout << std::endl;
}

// FIXED: Simple counter race using OpenMP reduction
void demonstrateSimpleRaceFixed_Reduction(int numThreads, int iterations, bool verbose) {
    g_counter = 0;
    
    // Start profiling
    PROFILE_SCOPE("SimpleRaceFixed_Reduction");
    
    std::cout << "Running fixed simple counter race (reduction) with " << numThreads 
              << " threads and " << iterations << " iterations..." << std::endl;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Fixed parallel increment using OpenMP reduction
    #pragma omp parallel num_threads(numThreads)
    {
        int threadId = omp_get_thread_num();
        
        // Fix: Use reduction clause to handle the counter increment
        #pragma omp for reduction(+:g_counter)
        for (int i = 0; i < iterations; i++) {
            g_counter++;
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
    
    std::cout << "Fixed counter race (reduction) completed in " << duration << " ms" << std::endl;
    std::cout << "Final counter value: " << g_counter 
              << " (expected: " << iterations << ")" << std::endl;
    
    if (g_counter != iterations) {
        std::cout << "Error: Counter value is incorrect!" << std::endl;
    } else {
        std::cout << "Success: Counter value is correct." << std::endl;
    }
    
    std::cout << std::endl;
}

// FIXED: Complex data structure race using mutex
void demonstrateComplexRaceFixed_Mutex(int numThreads, int iterations, bool verbose) {
    // Initialize vector
    for (size_t i = 0; i < g_vector.size(); i++) {
        g_vector[i] = 0;
    }
    
    // Start profiling
    PROFILE_SCOPE("ComplexRaceFixed_Mutex");
    
    std::cout << "Running fixed complex data structure race (mutex) with " << numThreads 
              << " threads and " << iterations << " iterations..." << std::endl;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Fixed parallel update of vector elements using mutex
    #pragma omp parallel num_threads(numThreads)
    {
        int threadId = omp_get_thread_num();
        
        // Each thread updates elements with mutex protection
        for (int i = 0; i < iterations; i++) {
            // Calculate index with potential overlap between threads
            int index = (threadId + i) % g_vector.size();
            
            // Fix: Use mutex to protect access to the vector element
            {
                std::lock_guard<std::mutex> lock(g_mutex);
                g_vector[index]++;
            }
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
    
    std::cout << "Fixed complex race (mutex) completed in " << duration << " ms" << std::endl;
    std::cout << "Total increments: " << totalIncrements 
              << " (expected: " << iterations * numThreads << ")" << std::endl;
    
    if (totalIncrements != iterations * numThreads) {
        std::cout << "Error: Total increments is incorrect!" << std::endl;
    } else {
        std::cout << "Success: Total increments is correct." << std::endl;
    }
    
    std::cout << std::endl;
}

// FIXED: Complex data structure race using OpenMP lock
void demonstrateComplexRaceFixed_OmpLock(int numThreads, int iterations, bool verbose) {
    // Initialize vector
    for (size_t i = 0; i < g_vector.size(); i++) {
        g_vector[i] = 0;
    }
    
    // Initialize OpenMP lock
    omp_init_lock(&g_ompLock);
    
    // Start profiling
    PROFILE_SCOPE("ComplexRaceFixed_OmpLock");
    
    std::cout << "Running fixed complex data structure race (OMP lock) with " << numThreads 
              << " threads and " << iterations << " iterations..." << std::endl;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Fixed parallel update of vector elements using OpenMP lock
    #pragma omp parallel num_threads(numThreads)
    {
        int threadId = omp_get_thread_num();
        
        // Each thread updates elements with OpenMP lock protection
        for (int i = 0; i < iterations; i++) {
            // Calculate index with potential overlap between threads
            int index = (threadId + i) % g_vector.size();
            
            // Fix: Use OpenMP lock to protect access to the vector element
            omp_set_lock(&g_ompLock);
            g_vector[index]++;
            omp_unset_lock(&g_ompLock);
        }
        
        if (verbose) {
            #pragma omp critical
            {
                std::cout << "Thread " << threadId << " completed updates" << std::endl;
            }
        }
    }
    
    // Destroy the lock when done
    omp_destroy_lock(&g_ompLock);
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime).count();
    
    // Check for race evidence by counting total increments
    int totalIncrements = 0;
    for (int value : g_vector) {
        totalIncrements += value;
    }
    
    std::cout << "Fixed complex race (OMP lock) completed in " << duration << " ms" << std::endl;
    std::cout << "Total increments: " << totalIncrements 
              << " (expected: " << iterations * numThreads << ")" << std::endl;
    
    if (totalIncrements != iterations * numThreads) {
        std::cout << "Error: Total increments is incorrect!" << std::endl;
    } else {
        std::cout << "Success: Total increments is correct." << std::endl;
    }
    
    std::cout << std::endl;
}

// FIXED: Read-modify-write race using atomic operations
void demonstrateRMWRaceFixed(int numThreads, int iterations, bool verbose) {
    g_atomicCounter = 0;
    
    // Start profiling
    PROFILE_SCOPE("RMWRaceFixed");
    
    std::cout << "Running fixed read-modify-write race with " << numThreads 
              << " threads and " << iterations << " iterations..." << std::endl;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Fixed parallel increments using atomic operations
    #pragma omp parallel num_threads(numThreads)
    {
        int threadId = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < iterations; i++) {
            // Fix: Use atomic increment instead of regular increment
            g_atomicCounter.fetch_add(1, std::memory_order_relaxed);
        }
        
        if (verbose) {
            #pragma omp critical
            {
                std::cout << "Thread " << threadId << ": Current atomic counter value = " 
                          << g_atomicCounter << std::endl;
            }
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime).count();
    
    std::cout << "Fixed RMW race completed in " << duration << " ms" << std::endl;
    std::cout << "Final atomic counter value: " << g_atomicCounter 
              << " (expected: " << iterations << ")" << std::endl;
    
    if (g_atomicCounter != iterations) {
        std::cout << "Error: Atomic counter value is incorrect!" << std::endl;
    } else {
        std::cout << "Success: Atomic counter value is correct." << std::endl;
    }
    
    std::cout << std::endl;
}

// Demonstrates all fixed versions and generates performance metrics
void runAllFixedTests(int numThreads, int iterations, bool verbose, const std::string& reportFile) {
    // Reset profiler
    Profiler::getInstance().reset();
    
    // Run all fixed race condition tests
    demonstrateSimpleRaceFixed_Critical(numThreads, iterations, verbose);
    demonstrateSimpleRaceFixed_Atomic(numThreads, iterations, verbose);
    demonstrateSimpleRaceFixed_Reduction(numThreads, iterations, verbose);
    demonstrateRMWRaceFixed(numThreads, iterations, verbose);
    demonstrateComplexRaceFixed_Mutex(numThreads, iterations / 100, verbose);
    demonstrateComplexRaceFixed_OmpLock(numThreads, iterations / 100, verbose);
    
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
    
    // Enable profiling in profile build
    #ifdef PROFILE
    Profiler::getInstance().setEnabled(true);
    #endif
    
    if (!quiet) {
        std::cout << "=== OpenMP Race Conditions Fixed Demo ===" << std::endl;
        std::cout << "Threads: " << threads << std::endl;
        std::cout << "Iterations: " << iterations << std::endl;
        std::cout << std::endl;
    }
    
    // Run selected demo mode
    if (mode == "all") {
        runAllFixedTests(threads, iterations, verbose, reportFile);
    }
    else if (mode == "critical") {
        demonstrateSimpleRaceFixed_Critical(threads, iterations, verbose);
    }
    else if (mode == "atomic") {
        demonstrateSimpleRaceFixed_Atomic(threads, iterations, verbose);
    }
    else if (mode == "reduction") {
        demonstrateSimpleRaceFixed_Reduction(threads, iterations, verbose);
    }
    else if (mode == "rmw") {
        demonstrateRMWRaceFixed(threads, iterations, verbose);
    }
    else if (mode == "mutex") {
        demonstrateComplexRaceFixed_Mutex(threads, iterations / 100, verbose);
    }
    else if (mode == "omplock") {
        demonstrateComplexRaceFixed_OmpLock(threads, iterations / 100, verbose);
    }
    else {
        if (!quiet) {
            std::cout << "Unknown mode: " << mode << ". Running all tests." << std::endl;
        }
        runAllFixedTests(threads, iterations, verbose, reportFile);
    }
    
    // Save profiling report
    #ifdef PROFILE
    if (!reportFile.empty()) {
        Profiler::getInstance().saveToCSV(reportFile);
        std::cout << "Performance report saved to " << reportFile << std::endl;
    }
    else if (!quiet) {
        Profiler::getInstance().printSummary();
    }
    #endif
    
    return 0;
}