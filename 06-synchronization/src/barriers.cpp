#include <iostream>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <omp.h>
#include "../include/synchronization_demos.h"
#include "../include/utils.h"

// Demonstrate implicit barriers in OpenMP
void demo_implicit_barriers(int num_threads, int workload) {
    utils::print_subsection("Implicit Barriers Demonstration");
    std::cout << "Showing how implicit barriers work at the end of parallel regions\n\n";
    
    if (num_threads <= 0) {
        num_threads = omp_get_max_threads();
    }
    
    utils::print_result("Number of threads", num_threads);
    utils::print_result("Workload size", workload);
    
    std::vector<int> data(workload);
    std::iota(data.begin(), data.end(), 1); // Fill with 1 to workload
    
    std::vector<double> thread_times(num_threads, 0.0);
    
    utils::print_section("Stage 1: Computing squares");
    std::vector<int> squares(workload);
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        utils::Timer thread_timer;
        thread_timer.start();
        
        #pragma omp for
        for (int i = 0; i < workload; i++) {
            squares[i] = data[i] * data[i];
            
            // Simulate varying workloads
            if (i % num_threads == tid) {
                // Make some threads take longer
                for (volatile int j = 0; j < 1000 * (tid + 1); j++) { }
            }
        }
        
        thread_timer.stop();
        thread_times[tid] = thread_timer.elapsed_ms();
        
        #pragma omp critical
        {
            std::cout << "Thread " << tid << " completed Stage 1 in " 
                      << std::fixed << std::setprecision(2) << thread_times[tid] << " ms\n";
        }
    } // Implicit barrier here
    
    utils::print_section("Stage 2: Computing cubes");
    std::vector<int> cubes(workload);
    
    // Reset timing information
    std::fill(thread_times.begin(), thread_times.end(), 0.0);
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        utils::Timer thread_timer;
        thread_timer.start();
        
        #pragma omp for
        for (int i = 0; i < workload; i++) {
            cubes[i] = squares[i] * data[i];
            
            // Simulate different workload variations
            if (i % num_threads == (num_threads - tid - 1)) {
                // Different delay pattern from stage 1
                for (volatile int j = 0; j < 1000 * (num_threads - tid); j++) { }
            }
        }
        
        thread_timer.stop();
        thread_times[tid] = thread_timer.elapsed_ms();
        
        #pragma omp critical
        {
            std::cout << "Thread " << tid << " completed Stage 2 in " 
                      << std::fixed << std::setprecision(2) << thread_times[tid] << " ms\n";
        }
    } // Implicit barrier here
    
    std::cout << "\nAll threads synchronized at implicit barriers between stages\n";
    std::cout << "This ensured that all of Stage 1 was complete before Stage 2 began\n";
}

// Demonstrate explicit barriers in OpenMP
void demo_explicit_barriers(int num_threads, int workload) {
    utils::print_subsection("Explicit Barriers Demonstration");
    std::cout << "Showing how explicit barriers can be used within parallel regions\n\n";
    
    if (num_threads <= 0) {
        num_threads = omp_get_max_threads();
    }
    
    utils::print_result("Number of threads", num_threads);
    utils::print_result("Workload size", workload);
    
    // Create a smaller workload for this demo
    int reduced_workload = std::min(workload, 100);
    std::vector<std::vector<int>> thread_progress(num_threads, std::vector<int>(reduced_workload, 0));
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        
        // Step 1: Initialize data
        for (int i = 0; i < reduced_workload; i++) {
            if (i % num_threads == tid) {
                thread_progress[tid][i] = 1;
                
                // Simulate varying initialization times
                for (volatile int j = 0; j < 1000 * (tid + 1); j++) { }
            }
        }
        
        #pragma omp barrier // Explicit barrier - wait for all threads to finish initialization
        #pragma omp critical
        {
            std::cout << "Thread " << tid << " crossed barrier 1 (initialization complete)\n";
        }
        
        // Step 2: Process data
        for (int i = 0; i < reduced_workload; i++) {
            for (int t = 0; t < num_threads; t++) {
                if (i % num_threads == (tid + t) % num_threads) {
                    thread_progress[t][i] = 2;
                    
                    // Simulate varying processing times
                    for (volatile int j = 0; j < 2000 * ((tid + 1) % num_threads + 1); j++) { }
                }
            }
        }
        
        #pragma omp barrier // Explicit barrier - wait for all threads to finish processing
        #pragma omp critical
        {
            std::cout << "Thread " << tid << " crossed barrier 2 (processing complete)\n";
        }
        
        // Step 3: Finalize data
        for (int i = 0; i < reduced_workload; i++) {
            for (int t = 0; t < num_threads; t++) {
                if (i % num_threads == (tid + 2*t) % num_threads) {
                    thread_progress[t][i] = 3;
                    
                    // Simulate varying finalization times
                    for (volatile int j = 0; j < 500 * ((num_threads - tid) % num_threads + 1); j++) { }
                }
            }
        }
        
        #pragma omp barrier // Explicit barrier - wait for all threads to finish finalization
        #pragma omp critical
        {
            std::cout << "Thread " << tid << " crossed barrier 3 (finalization complete)\n";
        }
    }
    
    std::cout << "\nAll threads synchronized at explicit barriers during the parallel region\n";
    std::cout << "This ensures all threads complete each phase before moving to the next\n";
}

// Benchmark barrier performance
void benchmark_barrier_performance(int num_threads, int /*workload*/) {
    utils::print_subsection("Barrier Performance Analysis");
    std::cout << "Measuring the overhead of different barrier types\n\n";
    
    if (num_threads <= 0) {
        num_threads = omp_get_max_threads();
    }
    
    utils::print_result("Number of threads", num_threads);
    
    const int iterations = 10000;
    std::vector<std::pair<std::string, double>> results;
    
    // Benchmark implicit barriers
    auto benchmark_implicit = [&]() {
        for (int i = 0; i < iterations; i++) {
            #pragma omp parallel num_threads(num_threads)
            {
                // Empty parallel region - only measuring barrier overhead
            } // Implicit barrier
        }
    };
    
    double time_implicit = utils::benchmark_function(benchmark_implicit);
    results.push_back({"Implicit barrier (per iteration)", time_implicit / iterations});
    
    // Benchmark explicit barriers
    auto benchmark_explicit = [&]() {
        #pragma omp parallel num_threads(num_threads)
        {
            for (int i = 0; i < iterations; i++) {
                #pragma omp barrier
            }
        }
    };
    
    double time_explicit = utils::benchmark_function(benchmark_explicit);
    results.push_back({"Explicit barrier (per iteration)", time_explicit / iterations});
    
    // No barrier (for comparison)
    auto benchmark_no_barrier = [&]() {
        #pragma omp parallel num_threads(num_threads)
        {
            for (int i = 0; i < iterations; i++) {
                // Do nothing - no synchronization
            }
        }
    };
    
    double time_no_barrier = utils::benchmark_function(benchmark_no_barrier);
    results.push_back({"No barrier loop (per iteration)", time_no_barrier / iterations});
    
    // Display results
    utils::print_section("Barrier Performance Results");
    for (const auto& result : results) {
        utils::print_result(result.first, result.second, "ms");
    }
    
    // Calculate overhead
    double implicit_overhead = (time_implicit - time_no_barrier) / iterations;
    double explicit_overhead = (time_explicit - time_no_barrier) / iterations;
    
    utils::print_section("Barrier Overhead");
    utils::print_result("Implicit barrier overhead", implicit_overhead, "ms");
    utils::print_result("Explicit barrier overhead", explicit_overhead, "ms");
    
    // Draw chart
    utils::print_section("Performance Comparison");
    std::vector<std::pair<std::string, double>> chart_data = {
        {"Implicit", implicit_overhead},
        {"Explicit", explicit_overhead}
    };
    utils::draw_bar_chart(chart_data);
}

// Main barriers demo function
void demo_barriers(int num_threads, int /*workload*/) {
    utils::print_section("Barrier Synchronization Mechanisms");
    std::cout << "OpenMP provides barrier synchronization to coordinate threads\n";
    std::cout << "This demo shows both implicit and explicit barriers\n\n";
    
    // Reduce workload size for this demo to make it more interactive
    int reduced_workload = 10000; // Sabit bir değer kullanıyorum
    
    // Show implicit barriers
    demo_implicit_barriers(num_threads, reduced_workload);
    utils::pause_console();
    
    // Show explicit barriers
    demo_explicit_barriers(num_threads, reduced_workload / 100);
    utils::pause_console();
    
    // Benchmark performance
    benchmark_barrier_performance(num_threads, reduced_workload);
}
