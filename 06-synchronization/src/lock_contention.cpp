#include <iostream>
#include <vector>
#include <string>
#include <omp.h>
#include "../include/synchronization_demos.h"
#include "../include/utils.h"

// Note: The actual implementation of the lock contention visualization
// is in thread_timeline.cpp for code organization purposes.
// This file serves as a wrapper to maintain the API consistency.

// This is just a stub since the actual implementation is in thread_timeline.cpp
// to avoid duplication. The header expects this function to exist.
void visualize_lock_contention(int num_threads, int workload) {
    utils::print_section("Lock Contention Visualization");
    std::cout << "For lock contention visualization, please use the Thread Timeline visualization\n";
    std::cout << "which includes lock contention analysis as part of its demonstration.\n\n";
    
    std::cout << "Running the Thread Timeline visualization instead...\n";
    
    // Call the thread timeline visualization which includes lock contention
    visualize_thread_timeline(num_threads, workload);
} 