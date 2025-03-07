#include <iostream>
#include <vector>
#include <iomanip>
#include <string>
#include <omp.h>
#include "../include/synchronization_demos.h"
#include "../include/utils.h"

// Basic critical section demo
void basic_critical_section(int num_threads, int iterations) {
    utils::print_subsection("Basic Critical Section Demo");
    std::cout << "Using #pragma omp critical to protect shared counter\n";
    std::cout << "Incrementing a shared counter with " << num_threads << " threads\n";
    std::cout << "Each thread will increment " << iterations << " times\n\n";
    
    // Expected final count
    long long expected_count = static_cast<long long>(num_threads) * iterations;
    
    // Counter with critical section
    long long counter_critical = 0;
    
    // Counter with race condition for comparison
    long long counter_race = 0;
    
    // Set the number of threads
    if (num_threads > 0) {
        omp_set_num_threads(num_threads);
    } else {
        num_threads = omp_get_max_threads();
    }
    
    utils::print_result("Number of threads", num_threads);
    utils::print_result("Iterations per thread", iterations);
    utils::print_result("Expected final count", static_cast<double>(expected_count));
    
    // With race condition
    std::cout << "\nRunning with race condition...\n";
    utils::Timer timer;
    timer.start();
    
    #pragma omp parallel 
    {
        for (int i = 0; i < iterations; i++) {
            counter_race++; // Race condition here!
        }
    }
    
    timer.stop();
    double race_time = timer.elapsed_ms();
    
    // With critical section
    std::cout << "\nRunning with critical section...\n";
    timer.reset();
    timer.start();
    
    #pragma omp parallel 
    {
        for (int i = 0; i < iterations; i++) {
            #pragma omp critical
            {
                counter_critical++; // Protected by critical section
            }
        }
    }
    
    timer.stop();
    double critical_time = timer.elapsed_ms();
    
    // Calculate and print results
    utils::print_result("Final count (with race)", static_cast<double>(counter_race));
    utils::print_result("Final count (with critical)", static_cast<double>(counter_critical));
    utils::print_result("Expected count", static_cast<double>(expected_count));
    utils::print_result("Error (race)", static_cast<double>(expected_count - counter_race));
    utils::print_result("Error (critical)", static_cast<double>(expected_count - counter_critical));
    utils::print_result("Execution time (race)", race_time, "ms");
    utils::print_result("Execution time (critical)", critical_time, "ms");
    utils::print_result("Overhead factor", critical_time / race_time, "x");
    
    std::cout << "\nThe critical section ensures correct results but introduces overhead.\n";
    std::cout << "The race condition version is faster but produces incorrect results.\n";
}

// Named critical sections demo
void named_critical_sections(int num_threads, int iterations) {
    utils::print_subsection("Named Critical Sections Demo");
    std::cout << "Using named critical sections for fine-grained control\n";
    std::cout << "We'll maintain two separate counters with different critical sections\n\n";
    
    // Two separate counters
    long long counter1 = 0;
    long long counter2 = 0;
    
    // Expected counts
    long long expected_count1 = static_cast<long long>(num_threads) * iterations;
    long long expected_count2 = static_cast<long long>(num_threads) * iterations * 2;
    
    // Set the number of threads
    if (num_threads > 0) {
        omp_set_num_threads(num_threads);
    } else {
        num_threads = omp_get_max_threads();
    }
    
    utils::print_result("Number of threads", num_threads);
    utils::print_result("Iterations per thread", iterations);
    
    utils::Timer timer;
    timer.start();
    
    #pragma omp parallel 
    {
        for (int i = 0; i < iterations; i++) {
            // Use a named critical section for counter1
            #pragma omp critical(counter1_lock)
            {
                counter1++;
            }
            
            // Update counter2 twice per iteration
            #pragma omp critical(counter2_lock)
            {
                counter2++;
            }
            
            // Do some work between the critical sections
            for (int j = 0; j < 100; j++) {
                // Simulate work
            }
            
            #pragma omp critical(counter2_lock)
            {
                counter2++;
            }
        }
    }
    
    timer.stop();
    
    // Calculate and print results
    utils::print_result("Counter 1 final value", static_cast<double>(counter1));
    utils::print_result("Counter 1 expected value", static_cast<double>(expected_count1));
    utils::print_result("Counter 1 error", static_cast<double>(expected_count1 - counter1));
    
    utils::print_result("Counter 2 final value", static_cast<double>(counter2));
    utils::print_result("Counter 2 expected value", static_cast<double>(expected_count2));
    utils::print_result("Counter 2 error", static_cast<double>(expected_count2 - counter2));
    
    utils::print_result("Execution time", timer.elapsed_ms(), "ms");
    
    std::cout << "\nNamed critical sections allow different critical regions to execute in parallel.\n";
    std::cout << "This can improve performance when multiple independent shared resources need protection.\n";
}

// Nested critical sections demo
void nested_critical_sections(int num_threads, int iterations) {
    utils::print_subsection("Nested Critical Sections Demo");
    std::cout << "Demonstrating nested critical sections behavior\n\n";
    
    // Counters for different scenarios
    long long counter_same_name = 0;
    long long counter_diff_name = 0;
    long long counter_unnamed = 0;
    
    // Expected counts
    long long expected_count = static_cast<long long>(num_threads) * iterations;
    
    // Set the number of threads
    if (num_threads > 0) {
        omp_set_num_threads(num_threads);
    } else {
        num_threads = omp_get_max_threads();
    }
    
    utils::print_result("Number of threads", num_threads);
    utils::print_result("Iterations per thread", iterations);
    
    // Scenario 1: Same named critical section (can deadlock if not implemented correctly)
    utils::Timer timer;
    timer.start();
    
    #pragma omp parallel 
    {
        for (int i = 0; i < iterations; i++) {
            #pragma omp critical(same_name)
            {
                // Outer critical section
                counter_same_name++;
                
                // Nested critical section with same name
                // This should work because OpenMP implementations typically handle this correctly
                #pragma omp critical(same_name_inner)
                {
                    // Inner critical section
                    counter_same_name++;
                }
            }
        }
    }
    
    timer.stop();
    double same_name_time = timer.elapsed_ms();
    
    // Scenario 2: Different named critical sections
    timer.reset();
    timer.start();
    
    #pragma omp parallel 
    {
        for (int i = 0; i < iterations; i++) {
            #pragma omp critical(outer_lock)
            {
                // Outer critical section
                counter_diff_name++;
                
                // Nested critical section with different name
                #pragma omp critical(inner_lock)
                {
                    // Inner critical section
                    counter_diff_name++;
                }
            }
        }
    }
    
    timer.stop();
    double diff_name_time = timer.elapsed_ms();
    
    // Scenario 3: Unnamed critical sections
    timer.reset();
    timer.start();
    
    #pragma omp parallel 
    {
        for (int i = 0; i < iterations; i++) {
            #pragma omp critical
            {
                // Outer critical section
                counter_unnamed++;
                
                // Nested unnamed critical section - give it a name to avoid nesting error
                #pragma omp critical(inner_section)
                {
                    // Inner critical section
                    counter_unnamed++;
                }
            }
        }
    }
    
    timer.stop();
    double unnamed_time = timer.elapsed_ms();
    
    // Calculate and print results
    utils::print_result("Counter (same name)", static_cast<double>(counter_same_name));
    utils::print_result("Counter (diff name)", static_cast<double>(counter_diff_name));
    utils::print_result("Counter (unnamed)", static_cast<double>(counter_unnamed));
    utils::print_result("Expected value", static_cast<double>(expected_count * 2)); // Each iteration increments twice
    
    utils::print_result("Execution time (same name)", same_name_time, "ms");
    utils::print_result("Execution time (diff name)", diff_name_time, "ms");
    utils::print_result("Execution time (unnamed)", unnamed_time, "ms");
    
    std::cout << "\nNested critical sections work in OpenMP, but they may have performance implications.\n";
    std::cout << "Different critical section names allow for more concurrency between regions.\n";
    std::cout << "Unnamed critical sections all use the same internal lock.\n";
}

// Critical section performance benchmark
void critical_section_benchmark(int num_threads, int workload) {
    utils::print_subsection("Critical Section Performance Benchmark");
    std::cout << "Measuring performance impact of critical sections\n\n";
    
    // Set the number of threads
    if (num_threads > 0) {
        omp_set_num_threads(num_threads);
    } else {
        num_threads = omp_get_max_threads();
    }
    
    utils::print_result("Number of threads", num_threads);
    
    // Test various critical section granularities
    const int array_size = workload;
    std::vector<int> array = utils::generate_workload(array_size);
    std::vector<std::pair<std::string, double>> benchmark_results;
    
    // 1. Sequential sum (baseline)
    utils::Timer timer;
    timer.start();
    
    long long sequential_sum = 0;
    for (int i = 0; i < array_size; i++) {
        sequential_sum += array[i];
    }
    
    timer.stop();
    double sequential_time = timer.elapsed_ms();
    benchmark_results.push_back({"Sequential", sequential_time});
    
    // 2. Parallel with single critical section
    timer.reset();
    timer.start();
    
    long long critical_sum = 0;
    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < array_size; i++) {
            #pragma omp critical
            {
                critical_sum += array[i];
            }
        }
    }
    
    timer.stop();
    double critical_time = timer.elapsed_ms();
    benchmark_results.push_back({"Single Critical", critical_time});
    
    // 3. Parallel with thread-local sums and one final critical section
    timer.reset();
    timer.start();
    
    long long thread_local_sum = 0;
    #pragma omp parallel
    {
        long long local_sum = 0;
        
        #pragma omp for
        for (int i = 0; i < array_size; i++) {
            local_sum += array[i];
        }
        
        #pragma omp critical
        {
            thread_local_sum += local_sum;
        }
    }
    
    timer.stop();
    double thread_local_time = timer.elapsed_ms();
    benchmark_results.push_back({"Thread-Local", thread_local_time});
    
    // 4. Parallel with reduction (no explicit critical section)
    timer.reset();
    timer.start();
    
    long long reduction_sum = 0;
    #pragma omp parallel for reduction(+:reduction_sum)
    for (int i = 0; i < array_size; i++) {
        reduction_sum += array[i];
    }
    
    timer.stop();
    double reduction_time = timer.elapsed_ms();
    benchmark_results.push_back({"Reduction", reduction_time});
    
    // Print results
    utils::print_result("Sequential sum", static_cast<double>(sequential_sum));
    utils::print_result("Critical section sum", static_cast<double>(critical_sum));
    utils::print_result("Thread-local sum", static_cast<double>(thread_local_sum));
    utils::print_result("Reduction sum", static_cast<double>(reduction_sum));
    
    std::cout << "\nPerformance comparison:\n";
    utils::print_result("Sequential time", sequential_time, "ms");
    utils::print_result("Critical section time", critical_time, "ms");
    utils::print_result("Thread-local time", thread_local_time, "ms");
    utils::print_result("Reduction time", reduction_time, "ms");
    
    // Calculate speedups relative to sequential
    double critical_speedup = sequential_time / critical_time;
    double thread_local_speedup = sequential_time / thread_local_time;
    double reduction_speedup = sequential_time / reduction_time;
    
    std::cout << "\nSpeedups compared to sequential:\n";
    utils::print_result("Critical section speedup", critical_speedup, "x");
    utils::print_result("Thread-local speedup", thread_local_speedup, "x");
    utils::print_result("Reduction speedup", reduction_speedup, "x");
    
    // Draw ASCII chart
    utils::draw_bar_chart(benchmark_results, 60, 10);
    
    std::cout << "\nCritical section granularity has a significant impact on performance.\n";
    std::cout << "Using a critical section for every iteration creates high contention.\n";
    std::cout << "Using thread-local variables and a single critical section per thread reduces contention.\n";
    std::cout << "Using OpenMP's reduction clause avoids explicit critical sections and is usually the most efficient.\n";
}

// Main critical sections demo
void demo_critical_sections(int num_threads, int workload) {
    utils::print_header("Critical Sections Demo");
    std::cout << "This demo shows how to use #pragma omp critical to protect shared data\n";
    std::cout << "Critical sections ensure that only one thread executes a section at a time\n\n";
    
    // If threads not specified, use all available
    if (num_threads <= 0) {
        num_threads = omp_get_max_threads();
    }
    
    // Scale workload based on demo
    int iterations = std::min(workload / 100, 1000000);
    
    basic_critical_section(num_threads, iterations);
    
    utils::print_result("Demo completed", true);
}

// Named critical sections demo
void demo_named_critical_sections(int num_threads, int workload) {
    utils::print_header("Named Critical Sections Demo");
    std::cout << "This demo shows how to use named critical sections for fine-grained control\n";
    std::cout << "Named critical sections allow different critical regions to execute in parallel\n\n";
    
    // If threads not specified, use all available
    if (num_threads <= 0) {
        num_threads = omp_get_max_threads();
    }
    
    // Scale workload based on demo
    int iterations = std::min(workload / 100, 1000000);
    
    named_critical_sections(num_threads, iterations);
    
    utils::print_result("Demo completed", true);
}

// Nested critical sections demo
void demo_nested_critical_sections(int num_threads, int workload) {
    utils::print_header("Nested Critical Sections Demo");
    std::cout << "This demo shows the behavior of nested critical sections\n";
    std::cout << "Nested critical sections can be used in OpenMP but may have performance implications\n\n";
    
    // If threads not specified, use all available
    if (num_threads <= 0) {
        num_threads = omp_get_max_threads();
    }
    
    // Scale workload based on demo
    int iterations = std::min(workload / 1000, 100000);
    
    nested_critical_sections(num_threads, iterations);
    
    utils::print_result("Demo completed", true);
}

// Critical sections performance benchmark
void benchmark_critical_sections(int num_threads, int workload) {
    utils::print_header("Critical Sections Performance Benchmark");
    std::cout << "This benchmark measures the performance impact of different critical section approaches\n\n";
    
    // If threads not specified, use all available
    if (num_threads <= 0) {
        num_threads = omp_get_max_threads();
    }
    
    // Scale workload based on demo
    int array_size = std::min(workload, 10000000);
    
    critical_section_benchmark(num_threads, array_size);
    
    utils::print_result("Benchmark completed", true);
} 