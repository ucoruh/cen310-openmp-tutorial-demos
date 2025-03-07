#include <iostream>
#include <vector>
#include <string>
#include <omp.h>
#include "../include/synchronization_demos.h"
#include "../include/utils.h"

// Implementation of the performance analysis function
void run_performance_analysis(int num_threads, int workload) {
    utils::print_header("OpenMP Synchronization Performance Analysis");
    std::cout << "Running performance analysis with " << num_threads << " threads and workload size " << workload << "\n\n";
    
    // Run various performance tests that are available in the codebase
    benchmark_critical_sections(num_threads, workload);
    demo_locks(num_threads, workload);
    
    std::cout << "\nPerformance analysis complete.\n";
}

// Implementation of the benchmarks function
void run_benchmarks(int num_threads, int workload) {
    utils::print_header("OpenMP Synchronization Benchmarks");
    std::cout << "Running all benchmarks with " << num_threads << " threads and workload size " << workload << "\n\n";
    
    // Run all benchmarks that are available in the codebase
    benchmark_critical_sections(num_threads, workload);
    demo_simple_locks(num_threads, workload);
    demo_nested_locks(num_threads, workload);
    
    std::cout << "\nAll benchmarks complete.\n";
} 