#include <iostream>
#include <vector>
#include <iomanip>
#include <string>
#include <algorithm>
#include <omp.h>
#include "../include/synchronization_demos.h"
#include "../include/utils.h"

// Demonstrate basic ordered execution
void demo_ordered_execution(int num_threads, int workload) {
    utils::print_subsection("OpenMP Ordered Construct Demonstration");
    std::cout << "The ordered construct ensures serial execution of a section within a parallel loop\n\n";
    
    if (num_threads <= 0) {
        num_threads = omp_get_max_threads();
    }
    
    // Use a reduced workload for this demo to keep output manageable
    int ordered_workload = std::min(workload, 20);
    
    utils::print_result("Number of threads", num_threads);
    utils::print_result("Workload size", ordered_workload);
    
    std::cout << "\nRunning ordered execution...\n\n";
    
    utils::print_section("Parallel Loop with Ordered Section");
    
    #pragma omp parallel for num_threads(num_threads) ordered schedule(dynamic, 1)
    for (int i = 0; i < ordered_workload; i++) {
        int tid = omp_get_thread_num();
        
        // This is executed in parallel and out of order
        #pragma omp critical
        {
            std::cout << "Thread " << tid << " processing item " << i << " (parallel section)\n";
        }
        
        // Simulate work
        for (volatile int j = 0; j < 1000 * (tid + 1); j++) { }
        
        // This is executed in sequential order (by the iteration number)
        #pragma omp ordered
        {
            std::cout << "  --> ORDERED: Processing item " << i 
                      << " in sequence by thread " << tid << "\n";
        }
    }
    
    std::cout << "\nNote that the 'ordered' sections were executed in sequential order (by iteration number),\n";
    std::cout << "even though the threads processed the iterations in a potentially different order.\n";
}

// Compare ordered vs unordered execution
void ordered_vs_unordered(int num_threads, int workload) {
    utils::print_subsection("Ordered vs. Unordered Execution Performance Comparison");
    std::cout << "Comparing performance of loops with and without the ordered construct\n\n";
    
    if (num_threads <= 0) {
        num_threads = omp_get_max_threads();
    }
    
    // Use a larger workload for meaningful performance measurement
    int performance_workload = std::max(workload, 10000);
    std::vector<int> data(performance_workload);
    std::vector<int> result_ordered(performance_workload);
    std::vector<int> result_unordered(performance_workload);
    
    // Initialize data
    for (int i = 0; i < performance_workload; i++) {
        data[i] = i % 1000;
    }
    
    utils::print_result("Number of threads", num_threads);
    utils::print_result("Workload size", performance_workload);
    
    // Measure unordered execution
    utils::Timer timer_unordered;
    timer_unordered.start();
    
    #pragma omp parallel for num_threads(num_threads) schedule(dynamic, 100)
    for (int i = 0; i < performance_workload; i++) {
        // Simulate some work - compute a complex operation
        int value = data[i];
        for (int j = 0; j < 100; j++) {
            value = (value * value) % 7919; // Some arbitrary computation
        }
        result_unordered[i] = value;
    }
    
    timer_unordered.stop();
    double time_unordered = timer_unordered.elapsed_ms();
    
    // Measure ordered execution
    utils::Timer timer_ordered;
    timer_ordered.start();
    
    #pragma omp parallel for num_threads(num_threads) schedule(dynamic, 100) ordered
    for (int i = 0; i < performance_workload; i++) {
        // Same work as above
        int value = data[i];
        for (int j = 0; j < 100; j++) {
            value = (value * value) % 7919;
        }
        
        // This section will be executed in order of iteration number
        #pragma omp ordered
        {
            result_ordered[i] = value;
        }
    }
    
    timer_ordered.stop();
    double time_ordered = timer_ordered.elapsed_ms();
    
    // Results
    utils::print_section("Performance Results");
    utils::print_result("Unordered execution time", time_unordered, "ms");
    utils::print_result("Ordered execution time", time_ordered, "ms");
    
    double slowdown = (time_ordered / time_unordered);
    utils::print_result("Slowdown factor with ordered", slowdown, "x");
    
    // Additional explanation
    std::cout << "\nThe ordered construct introduces overhead because:\n";
    std::cout << "1. It requires tracking the next iteration that should execute the ordered section\n";
    std::cout << "2. It forces threads to wait if they're ahead of the sequence\n";
    std::cout << "3. It adds synchronization overhead between threads\n\n";
    
    std::cout << "Use ordered only when maintaining the original sequence is essential for correctness.\n";
    
    // Draw chart
    utils::print_section("Performance Comparison");
    std::vector<std::pair<std::string, double>> chart_data = {
        {"Unordered", time_unordered},
        {"Ordered", time_ordered}
    };
    utils::draw_bar_chart(chart_data);
    
    // Verify results are the same
    bool results_match = true;
    for (int i = 0; i < performance_workload; i++) {
        if (result_ordered[i] != result_unordered[i]) {
            results_match = false;
            break;
        }
    }
    
    std::cout << "\nResults match: " << (results_match ? "Yes" : "No") << "\n";
    std::cout << "(This is expected since we're doing the same computation in both cases)\n";
}

// Main ordered demo function
void demo_ordered(int num_threads, int workload) {
    utils::print_section("OpenMP Ordered Construct");
    std::cout << "The ordered construct forces a section of a parallel loop\n";
    std::cout << "to execute in the original sequential order\n\n";
    
    // Basic ordered demo
    demo_ordered_execution(num_threads, workload);
    utils::pause_console();
    
    // Performance comparison
    ordered_vs_unordered(num_threads, workload);
} 