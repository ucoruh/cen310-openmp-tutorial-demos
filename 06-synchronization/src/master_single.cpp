#include <iostream>
#include <vector>
#include <iomanip>
#include <string>
#include <omp.h>
#include "../include/synchronization_demos.h"
#include "../include/utils.h"

// Demonstrate the master construct
void demo_master_construct(int num_threads, int /*workload*/) {
    utils::print_subsection("OpenMP Master Construct Demonstration");
    std::cout << "The master construct specifies that a section of code should be executed\n";
    std::cout << "only by the master thread (thread 0)\n\n";
    
    if (num_threads <= 0) {
        num_threads = omp_get_max_threads();
    }
    
    utils::print_result("Number of threads", static_cast<double>(num_threads));
    
    utils::print_section("Master Construct Example");
    
    // Counter to track which threads executed certain sections
    std::vector<int> executed_master(num_threads, 0);
    std::vector<int> executed_parallel(num_threads, 0);
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        
        // All threads execute this section
        executed_parallel[tid]++;
        
        #pragma omp critical
        {
            std::cout << "Thread " << tid << " executing parallel section\n";
        }
        
        // Only master thread (thread 0) executes this section
        #pragma omp master
        {
            executed_master[tid]++;
            std::cout << "\nMASTER SECTION: Only thread " << tid << " (master) executes this code\n\n";
            
            // Simulate some work that only master should do
            for (volatile int i = 0; i < 10000000; i++) { }
        }
        // No implicit barrier after master
        
        // All threads execute this again
        #pragma omp critical
        {
            std::cout << "Thread " << tid << " continues execution after master section\n";
        }
    }
    
    std::cout << "\nExecution statistics:\n";
    for (int i = 0; i < num_threads; i++) {
        std::cout << "Thread " << i << ": executed parallel sections " << executed_parallel[i] 
                  << " times, master sections " << executed_master[i] << " times\n";
    }
    
    std::cout << "\nKey points about the master construct:\n";
    std::cout << "1. Only thread 0 executes the master section\n";
    std::cout << "2. There is NO implicit barrier at the end of a master construct\n";
    std::cout << "3. Non-master threads SKIP the master section and continue execution\n";
}

// Demonstrate the single construct
void demo_single_construct(int num_threads, int /*workload*/) {
    utils::print_subsection("OpenMP Single Construct Demonstration");
    std::cout << "The single construct specifies that a section of code should be executed\n";
    std::cout << "by only one thread (not necessarily the master thread)\n\n";
    
    if (num_threads <= 0) {
        num_threads = omp_get_max_threads();
    }
    
    utils::print_result("Number of threads", num_threads);
    
    utils::print_section("Single Construct Example");
    
    // Counter to track which thread executed the single section
    std::vector<int> executed_single(num_threads, 0);
    std::vector<int> executed_parallel(num_threads, 0);
    
    // Run multiple times to show that different threads might execute the single section
    for (int run = 0; run < 5; run++) {
        std::cout << "\nRun " << run + 1 << ":\n";
        
        #pragma omp parallel num_threads(num_threads)
        {
            int tid = omp_get_thread_num();
            
            // All threads execute this section
            executed_parallel[tid]++;
            
            #pragma omp critical
            {
                std::cout << "Thread " << tid << " executing parallel section\n";
            }
            
            // Only one thread executes this section (any thread can be chosen)
            #pragma omp single
            {
                executed_single[tid]++;
                std::cout << "\nSINGLE SECTION: Thread " << tid << " was chosen to execute this code\n\n";
                
                // Simulate some work that only one thread should do
                for (volatile int i = 0; i < 5000000; i++) { }
            }
            // Implicit barrier here by default
            
            #pragma omp critical
            {
                std::cout << "Thread " << tid << " continues after single section and barrier\n";
            }
        }
    }
    
    std::cout << "\nExecution statistics:\n";
    for (int i = 0; i < num_threads; i++) {
        std::cout << "Thread " << i << ": executed parallel sections " << executed_parallel[i] 
                  << " times, single sections " << executed_single[i] << " times\n";
    }
    
    std::cout << "\nKey points about the single construct:\n";
    std::cout << "1. Exactly one thread executes the single section (can be any thread)\n";
    std::cout << "2. There IS an implicit barrier at the end of a single construct (unless nowait is specified)\n";
    std::cout << "3. All other threads wait at the barrier\n";
}

// Demonstrate the single construct with nowait
void demo_single_nowait(int num_threads, int /*workload*/) {
    utils::print_subsection("OpenMP Single Construct with nowait");
    std::cout << "The nowait clause removes the implicit barrier at the end of a single construct\n\n";
    
    if (num_threads <= 0) {
        num_threads = omp_get_max_threads();
    }
    
    utils::print_result("Number of threads", num_threads);
    
    utils::print_section("Single Construct with nowait Example");
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        
        #pragma omp critical
        {
            std::cout << "Thread " << tid << " started\n";
        }
        
        // Only one thread executes this, others continue immediately
        #pragma omp single nowait
        {
            std::cout << "\nSINGLE (nowait): Thread " << tid << " doing initialization work\n";
            // Simulate some initialization work
            for (volatile int i = 0; i < 50000000; i++) { }
            std::cout << "SINGLE (nowait): Thread " << tid << " finished initialization\n\n";
        }
        // NO barrier here due to nowait
        
        #pragma omp critical
        {
            std::cout << "Thread " << tid << " continuing without waiting for single to complete\n";
        }
    }
    
    std::cout << "\nKey points about the single construct with nowait:\n";
    std::cout << "1. Exactly one thread executes the single section\n";
    std::cout << "2. The nowait clause removes the implicit barrier\n";
    std::cout << "3. Other threads continue execution immediately without waiting\n";
}

// Compare master and single constructs
void compare_master_single(int num_threads, int /*workload*/) {
    utils::print_subsection("Master vs Single: Performance Comparison");
    std::cout << "Comparing performance characteristics of master and single constructs\n\n";
    
    if (num_threads <= 0) {
        num_threads = omp_get_max_threads();
    }
    
    utils::print_result("Number of threads", num_threads);
    
    const int iterations = 10000;
    std::vector<std::pair<std::string, double>> results;
    
    // Benchmark master construct
    auto benchmark_master = [&]() {
        for (int i = 0; i < iterations; i++) {
            #pragma omp parallel num_threads(num_threads)
            {
                #pragma omp master
                {
                    // Small amount of work in master section
                    volatile int dummy = 0;
                    for (int j = 0; j < 10; j++) {
                        dummy += j;
                    }
                }
            }
        }
    };
    
    double time_master = utils::benchmark_function(benchmark_master);
    results.push_back({"Master construct", time_master / iterations});
    
    // Benchmark single construct
    auto benchmark_single = [&]() {
        for (int i = 0; i < iterations; i++) {
            #pragma omp parallel num_threads(num_threads)
            {
                #pragma omp single
                {
                    // Same work as in master section
                    volatile int dummy = 0;
                    for (int j = 0; j < 10; j++) {
                        dummy += j;
                    }
                }
            }
        }
    };
    
    double time_single = utils::benchmark_function(benchmark_single);
    results.push_back({"Single construct", time_single / iterations});
    
    // Benchmark single construct with nowait
    auto benchmark_single_nowait = [&]() {
        for (int i = 0; i < iterations; i++) {
            #pragma omp parallel num_threads(num_threads)
            {
                #pragma omp single nowait
                {
                    // Same work as before
                    volatile int dummy = 0;
                    for (int j = 0; j < 10; j++) {
                        dummy += j;
                    }
                }
            }
        }
    };
    
    double time_single_nowait = utils::benchmark_function(benchmark_single_nowait);
    results.push_back({"Single with nowait", time_single_nowait / iterations});
    
    // Display results
    utils::print_section("Performance Results");
    for (const auto& result : results) {
        utils::print_result(result.first, result.second, "ms");
    }
    
    // Draw chart
    utils::print_section("Performance Comparison");
    utils::draw_bar_chart(results);
    
    // Analysis
    std::cout << "\nAnalysis:\n";
    std::cout << "1. Master is typically faster because:\n";
    std::cout << "   - It doesn't require thread coordination (always thread 0)\n";
    std::cout << "   - It doesn't have an implicit barrier\n\n";
    
    std::cout << "2. Single with nowait is faster than regular single because:\n";
    std::cout << "   - It eliminates the barrier synchronization cost\n";
    std::cout << "   - But it still has overhead to determine which thread executes the section\n\n";
    
    std::cout << "3. Use the appropriate construct based on your needs:\n";
    std::cout << "   - Master: When you specifically need thread 0 to do the work\n";
    std::cout << "   - Single: When any thread can do the work but others must wait for completion\n";
    std::cout << "   - Single nowait: When any thread can do the work and others can continue\n";
}

// Main function for master and single demos
void demo_master_single(int /*num_threads*/, int /*workload*/) {
    utils::print_section("OpenMP Master and Single Constructs");
    std::cout << "These constructs control which threads execute certain blocks of code\n";
    std::cout << "within a parallel region\n\n";
    
    // Demo of master construct
    demo_master_construct(4, 1000);
    utils::pause_console();
    
    // Demo of single construct
    demo_single_construct(4, 1000);
    utils::pause_console();
    
    // Demo of single construct with nowait
    demo_single_nowait(4, 1000);
    utils::pause_console();
    
    // Compare master and single constructs
    compare_master_single(4, 1000);
} 