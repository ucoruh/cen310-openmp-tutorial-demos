#include <iostream>
#include <omp.h>
#include <vector>
#include <string>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include <memory>
#include "../include/system_topology.h"
#include "../include/thread_utils.h"
#include "../include/cli_parser.h"

// Demo selection menu
void displayMenu() {
    std::cout << "\n=== OpenMP Nested Parallelism & Thread Affinity Demos ===" << std::endl;
    std::cout << "1. System Topology Information" << std::endl;
    std::cout << "2. Basic Nested Parallelism Demo" << std::endl;
    std::cout << "3. Thread Affinity Demo" << std::endl;
    std::cout << "4. Custom Thread Placement Demo" << std::endl;
    std::cout << "5. Matrix Multiplication Performance" << std::endl;
    std::cout << "0. Exit" << std::endl;
    std::cout << "\nEnter your choice: ";
}

// System topology info demo
void systemTopologyDemo(const SystemTopology& topology) {
    std::cout << "\n=== System Topology Information ===" << std::endl;
    std::cout << "Total logical processors: " << topology.getLogicalProcessorCount() << std::endl;
    std::cout << "Physical packages: " << topology.getPhysicalPackageCount() << std::endl;
    std::cout << "Cores per package: " << topology.getCoresPerPackage() << std::endl;
    std::cout << "NUMA nodes: " << topology.getNumaNodeCount() << std::endl;
    
    // Display detailed processor mapping
    topology.displayProcessorMap();
    
    // Display NUMA node info if available
    if (topology.getNumaNodeCount() > 1) {
        topology.displayNumaInfo();
    }
}

// Basic nested parallelism demo
void basicNestedDemo(int outerThreads, int innerThreads) {
    std::cout << "\n=== Basic Nested Parallelism Demo ===" << std::endl;
    std::cout << "Outer threads: " << outerThreads << ", Inner threads: " << innerThreads << std::endl;
    
    // Enable nested parallelism
    omp_set_nested(1);
    
    // Current active levels info
    std::cout << "Nested enabled: " << omp_get_nested() << std::endl;
    
    // Outer parallel region
    #pragma omp parallel num_threads(outerThreads) default(none) shared(std::cout, innerThreads)
    {
        int outerID = omp_get_thread_num();
        int outerNumThreads = omp_get_num_threads();
        int coreID = GetThreadCoreID();
        
        #pragma omp critical
        {
            std::cout << "Outer region - Thread " << outerID << "/" << outerNumThreads
                      << " running on core " << coreID << std::endl;
        }
        
        // Inner parallel region
        #pragma omp parallel num_threads(innerThreads) default(none) shared(std::cout, outerID, outerNumThreads)
        {
            int innerID = omp_get_thread_num();
            int innerNumThreads = omp_get_num_threads();
            int innerCoreID = GetThreadCoreID();
            
            #pragma omp critical
            {
                std::cout << "  Inner region - Outer thread " << outerID 
                          << ", inner thread " << innerID << "/" << innerNumThreads
                          << " running on core " << innerCoreID << std::endl;
            }
            
            // Simulate some work
            #pragma omp barrier
        }
    }
}

// Thread affinity demo
void threadAffinityDemo() {
    std::cout << "\n=== Thread Affinity Demo ===" << std::endl;
    
    const int num_threads = std::min(4, omp_get_num_procs());
    
    // In OpenMP 2.0, we can't directly control thread placement with proc_bind
    // but we can show the natural placement
    std::cout << "\nObserving default thread placement with " << num_threads << " threads:" << std::endl;
    
    #pragma omp parallel num_threads(num_threads) default(none) shared(std::cout)
    {
        int thread_id = omp_get_thread_num();
        int core_id = GetThreadCoreID();
        
        #pragma omp critical
        {
            std::cout << "Thread " << thread_id << " running on core " << core_id << std::endl;
        }
    }
}

// Custom thread placement demo
void customThreadPlacementDemo(const SystemTopology& topology) {
    std::cout << "\n=== Custom Thread Placement Demo ===" << std::endl;
    
    int numCores = topology.getLogicalProcessorCount();
    if (numCores < 4) {
        std::cout << "This demo requires at least 4 logical processors." << std::endl;
        return;
    }
    
    // Create a custom affinity mask to bind threads to specific cores
    std::cout << "Setting custom affinity masks for manual thread placement..." << std::endl;
    
    // Example: Place threads on alternating cores
    std::vector<int> core_list;
    for (int i = 0; i < std::min(8, numCores); i += 2) {
        core_list.push_back(i);
    }
    
    std::cout << "Selected cores for threads: ";
    for (auto core : core_list) {
        std::cout << core << " ";
    }
    std::cout << std::endl;
    
    // Run a parallel region with manual affinity control
    #pragma omp parallel num_threads(core_list.size()) default(none) shared(std::cout, core_list)
    {
        int thread_id = omp_get_thread_num();
        
        // Set affinity for this thread to a specific core
        if (thread_id < core_list.size()) {
            SetThreadAffinity(core_list[thread_id]);
            
            // Get the actual core ID after setting affinity
            int actual_core = GetThreadCoreID();
            
            #pragma omp critical
            {
                std::cout << "Thread " << thread_id << " assigned to core " 
                          << core_list[thread_id] << ", actually running on core " 
                          << actual_core << std::endl;
            }
        }
        
        // Do some work to test the affinity
        double sum = 0.0;
        for (int i = 0; i < 10000000; i++) {
            sum += i * 0.01;
        }
        
        #pragma omp barrier
    }
}

// Matrix multiplication demo
void matrixMultiplicationDemo() {
    std::cout << "\n=== Matrix Multiplication Performance Demo ===" << std::endl;
    std::cout << "Please run the 'matrix_multiplication' executable for this demo." << std::endl;
    std::cout << "Example: build\\Release\\matrix_multiplication.exe --matrix_size=1000" << std::endl;
}

// Entry point
int main(int argc, char* argv[]) {
    // Parse command line arguments
    CliParser parser(argc, argv);
    
    // Check if OpenMP is available
    #ifdef _OPENMP
        std::cout << "OpenMP is supported! Version: " << _OPENMP << std::endl;
    #else
        std::cerr << "OpenMP is not supported!" << std::endl;
        return 1;
    #endif
    
    // Initialize system topology detector
    SystemTopology topology;
    topology.detectTopology();
    
    // Print basic system info
    std::cout << "Detected " << topology.getLogicalProcessorCount() << " logical processors" << std::endl;
    
    // Default thread configurations
    int outer_threads = std::min(4, topology.getLogicalProcessorCount());
    int inner_threads = 2;
    
    // Override with command line parameters if provided
    if (parser.hasOption("outer_threads")) {
        outer_threads = parser.getIntOption("outer_threads");
    }
    
    if (parser.hasOption("inner_threads")) {
        inner_threads = parser.getIntOption("inner_threads");
    }
    
    // Check for specific demo request from command line
    int choice = -1;
    if (parser.hasOption("demo")) {
        choice = parser.getIntOption("demo");
    }
    
    // Main menu loop
    while (choice != 0) {
        if (choice == -1) {
            displayMenu();
            std::cin >> choice;
        }
        
        switch (choice) {
            case 0:
                std::cout << "\nExiting program." << std::endl;
                break;
                
            case 1:
                systemTopologyDemo(topology);
                break;
                
            case 2:
                basicNestedDemo(outer_threads, inner_threads);
                break;
                
            case 3:
                threadAffinityDemo();
                break;
                
            case 4:
                customThreadPlacementDemo(topology);
                break;
                
            case 5:
                matrixMultiplicationDemo();
                break;
                
            default:
                std::cout << "Invalid choice. Please try again." << std::endl;
        }
        
        if (choice != 0 && !parser.hasOption("demo")) {
            std::cout << "\nPress Enter to return to the menu...";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cin.get();
            choice = -1;  // Go back to menu
        } else {
            // If launched with a specific demo, exit after running
            if (parser.hasOption("demo")) {
                choice = 0;
            }
        }
    }
    
    return 0;
}