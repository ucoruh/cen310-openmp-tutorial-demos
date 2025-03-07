#include <iostream>
#include <vector>
#include <iomanip>
#include <thread>
#include <algorithm>
#include <string>
#include <omp.h>
#include "../include/synchronization_demos.h"
#include "../include/utils.h"

// Atomic update demonstration
void atomic_update_demo(int num_threads, int iterations) {
    utils::print_subsection("Atomic Update Demo");
    std::cout << "Using #pragma omp atomic update for atomic updates\n";
    std::cout << "Incrementing a shared counter with " << num_threads << " threads\n";
    std::cout << "Each thread will increment " << iterations << " times\n\n";
    
    // Expected final count
    long long expected_count = static_cast<long long>(num_threads) * iterations;
    
    // Counter with atomic update
    long long counter_atomic = 0;
    
    // Counter with critical section for comparison
    long long counter_critical = 0;
    
    // Set the number of threads
    if (num_threads > 0) {
        omp_set_num_threads(num_threads);
    } else {
        num_threads = omp_get_max_threads();
    }
    
    utils::print_result("Number of threads", num_threads);
    utils::print_result("Iterations per thread", iterations);
    utils::print_result("Expected final count", (int)expected_count, "");
    
    // With critical section
    std::cout << "\nRunning with critical section...\n";
    utils::Timer timer;
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
    
    // With atomic update
    std::cout << "\nRunning with atomic update...\n";
    timer.reset();
    timer.start();
    
    #pragma omp parallel 
    {
        for (int i = 0; i < iterations; i++) {
            #pragma omp atomic update
            counter_atomic++; // Protected by atomic update
        }
    }
    
    timer.stop();
    double atomic_time = timer.elapsed_ms();
    
    // Calculate and print results
    utils::print_result("Final count (with critical)", (int)counter_critical, "");
    utils::print_result("Final count (with atomic)", (int)counter_atomic, "");
    utils::print_result("Expected count", (int)expected_count, "");
    utils::print_result("Error (critical)", (int)(expected_count - counter_critical), "");
    utils::print_result("Error (atomic)", (int)(expected_count - counter_atomic), "");
    utils::print_result("Execution time (critical)", (double)critical_time, "ms");
    utils::print_result("Execution time (atomic)", (double)atomic_time, "ms");
    
    // Calculate speedup
    double speedup = critical_time / atomic_time;
    utils::print_result("Speedup (atomic vs critical)", speedup, "x");
    
    std::cout << "\nAtomic operations are generally faster than critical sections for simple operations.\n";
    std::cout << "Both approaches ensure correct results, but atomic operations have lower overhead.\n";
}

// Atomic read demonstration
void atomic_read_demo(int num_threads, int iterations) {
    utils::print_subsection("Atomic Read Demo");
    std::cout << "Using #pragma omp atomic read for atomic reads\n\n";
    
    // Create and initialize shared variable
    long long shared_var = 0;
    std::vector<long long> read_values(num_threads * iterations);
    
    // Set the number of threads
    if (num_threads > 0) {
        omp_set_num_threads(num_threads);
    } else {
        num_threads = omp_get_max_threads();
    }
    
    utils::print_result("Number of threads", num_threads);
    utils::print_result("Read operations per thread", iterations);
    
    // Initialize writer thread
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task
            {
                // Writer thread
                for (int i = 1; i <= num_threads * iterations; i++) {
                    shared_var = i;
                    // Small delay to allow readers to observe changes
                    for (volatile int j = 0; j < 1000; j++) { }
                }
            }
        }
        
        // Reader threads
        int thread_id = omp_get_thread_num();
        for (int i = 0; i < iterations; i++) {
            long long local_copy;
            
            // Atomic read ensures we get a consistent value
            #pragma omp atomic read
            local_copy = shared_var;
            
            // Store the read value for later analysis
            read_values[thread_id * iterations + i] = local_copy;
            
            // Small delay between reads
            for (volatile int j = 0; j < 1000; j++) { }
        }
    }
    
    // Display some of the read values
    std::cout << "\nSample of read values (first " << std::min(20, static_cast<int>(read_values.size())) << " reads):\n";
    for (int i = 0; i < std::min(20, static_cast<int>(read_values.size())); i++) {
        std::cout << "Read " << i << ": " << read_values[i] << std::endl;
    }
    
    std::cout << "\nAtomic read ensures that each thread reads a consistent value of the shared variable,\n";
    std::cout << "even if another thread is writing to it simultaneously.\n";
}

// Atomic write demonstration
void atomic_write_demo(int num_threads, int iterations) {
    utils::print_subsection("Atomic Write Demo");
    std::cout << "Using #pragma omp atomic write for atomic writes\n\n";
    
    // Create and initialize shared variables
    long long shared_var = 0;
    long long last_writer = -1;
    
    // Set the number of threads
    if (num_threads > 0) {
        omp_set_num_threads(num_threads);
    } else {
        num_threads = omp_get_max_threads();
    }
    
    utils::print_result("Number of threads", num_threads);
    utils::print_result("Write operations per thread", iterations);
    
    // Atomic writes from multiple threads
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        for (int i = 0; i < iterations; i++) {
            // Generate a unique value: thread_id * 10000 + i
            long long value = thread_id * 10000 + i;
            
            // Atomic write ensures the entire write is seen atomically by other threads
            #pragma omp atomic write
            shared_var = value;
            
            // Record which thread last wrote to the variable
            #pragma omp atomic write
            last_writer = thread_id;
            
            // Small delay between writes
            for (volatile int j = 0; j < 1000; j++) { }
        }
    }
    
    // Print final state
    std::cout << "\nFinal value of shared variable: " << shared_var << std::endl;
    std::cout << "Last writer thread: " << last_writer << std::endl;
    
    std::cout << "\nAtomic write ensures that each thread's write is performed atomically,\n";
    std::cout << "so other threads will see either the old value or the new value, but not a partial update.\n";
}

// Atomic capture demonstration
void atomic_capture_demo(int num_threads, int iterations) {
    utils::print_subsection("Atomic Capture Demo");
    std::cout << "Using #pragma omp atomic capture for read-modify-write operations\n";
    std::cout << "Implementing a thread-safe counter with unique values\n\n";
    
    // Create shared counter
    long long counter = 0;
    std::vector<long long> captured_values(num_threads * iterations);
    
    // Set the number of threads
    if (num_threads > 0) {
        omp_set_num_threads(num_threads);
    } else {
        num_threads = omp_get_max_threads();
    }
    
    utils::print_result("Number of threads", num_threads);
    utils::print_result("Operations per thread", iterations);
    
    // Use atomic capture to get unique values from the counter
    utils::Timer timer;
    timer.start();
    
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        for (int i = 0; i < iterations; i++) {
            long long my_value;
            
            // Atomic capture: read the current value and increment in one atomic operation
            #pragma omp atomic capture
            my_value = counter++;
            
            // Store the captured value
            captured_values[thread_id * iterations + i] = my_value;
        }
    }
    
    timer.stop();
    
    // Verify uniqueness of captured values
    std::sort(captured_values.begin(), captured_values.end());
    bool all_unique = true;
    for (size_t i = 1; i < captured_values.size(); i++) {
        if (captured_values[i] == captured_values[i-1]) {
            all_unique = false;
            break;
        }
    }
    
    // Display results
    utils::print_result("Final counter value", (int)counter, "");
    utils::print_result("Expected counter value", (int)(num_threads * iterations), "");
    utils::print_result("All captured values unique", all_unique ? 1 : 0, "");
    utils::print_result("Execution time", timer.elapsed_ms(), "ms");
    
    // Display some of the captured values
    std::cout << "\nSample of captured values (first " << std::min(20, static_cast<int>(captured_values.size())) << " values):\n";
    for (int i = 0; i < std::min(20, static_cast<int>(captured_values.size())); i++) {
        std::cout << "Value " << i << ": " << captured_values[i] << std::endl;
    }
    
    std::cout << "\nAtomic capture combines a read and a write into a single atomic operation.\n";
    std::cout << "This ensures that each thread gets a unique value from the counter.\n";
    std::cout << "It's particularly useful for scenarios like allocating unique IDs or indices.\n";
}

// Benchmark comparing atomic operations and critical sections
void atomic_vs_critical_benchmark(int num_threads, int workload) {
    utils::print_subsection("Atomic vs Critical Performance Benchmark");
    std::cout << "Comparing performance of different atomic operations vs critical sections\n\n";
    
    // Set the number of threads
    if (num_threads > 0) {
        omp_set_num_threads(num_threads);
    } else {
        num_threads = omp_get_max_threads();
    }
    
    utils::print_result("Number of threads", num_threads);
    utils::print_result("Workload size", workload);
    
    const int iterations = workload / num_threads;
    std::vector<std::pair<std::string, double>> benchmark_results;
    
    // Test different scenarios
    
    // 1. Increment with critical section
    utils::Timer timer;
    timer.start();
    
    long long counter_critical = 0;
    #pragma omp parallel
    {
        for (int i = 0; i < iterations; i++) {
            #pragma omp critical
            {
                counter_critical++;
            }
        }
    }
    
    timer.stop();
    double critical_inc_time = timer.elapsed_ms();
    benchmark_results.push_back({"Critical Increment", critical_inc_time});
    
    // 2. Increment with atomic update
    timer.reset();
    timer.start();
    
    long long counter_atomic = 0;
    #pragma omp parallel
    {
        for (int i = 0; i < iterations; i++) {
            #pragma omp atomic update
            counter_atomic++;
        }
    }
    
    timer.stop();
    double atomic_inc_time = timer.elapsed_ms();
    benchmark_results.push_back({"Atomic Increment", atomic_inc_time});
    
    // 3. Complex operation with critical section
    timer.reset();
    timer.start();
    
    long long complex_critical = 0;
    #pragma omp parallel
    {
        for (int i = 0; i < iterations; i++) {
            #pragma omp critical
            {
                complex_critical = complex_critical * 2 + 1;
            }
        }
    }
    
    timer.stop();
    double critical_complex_time = timer.elapsed_ms();
    benchmark_results.push_back({"Critical Complex", critical_complex_time});
    
    // 4. Equivalent operation with atomic update
    timer.reset();
    timer.start();
    
    long long complex_atomic = 0;
    #pragma omp parallel
    {
        for (int i = 0; i < iterations; i++) {
            // Need to break this down into multiple atomic operations
            long long temp;
            
            #pragma omp atomic read
            temp = complex_atomic;
            
            temp = temp * 2 + 1;
            
            #pragma omp atomic write
            complex_atomic = temp;
        }
    }
    
    timer.stop();
    double atomic_complex_time = timer.elapsed_ms();
    benchmark_results.push_back({"Atomic Complex", atomic_complex_time});
    
    // Print results
    utils::print_result("Critical section increment time", critical_inc_time, "ms");
    utils::print_result("Atomic update increment time", atomic_inc_time, "ms");
    utils::print_result("Critical section complex time", critical_complex_time, "ms");
    utils::print_result("Atomic complex operation time", atomic_complex_time, "ms");
    
    // Calculate speedups
    double increment_speedup = critical_inc_time / atomic_inc_time;
    double complex_speedup = critical_complex_time / atomic_complex_time;
    
    utils::print_result("Increment speedup (atomic vs critical)", increment_speedup, "x");
    utils::print_result("Complex operation speedup", complex_speedup, "x");
    
    // Draw chart
    utils::draw_bar_chart(benchmark_results, 60, 10);
    
    std::cout << "\nObservations:\n";
    std::cout << "1. Atomic operations are generally faster than critical sections for simple operations\n";
    std::cout << "2. For complex operations that require multiple atomic operations, critical sections may be more efficient\n";
    std::cout << "3. The performance gap between atomic and critical operations grows with thread count\n";
}

// Overview of atomic operations
void atomic_operations_overview(int /*num_threads*/, int /*workload*/) {
    utils::print_section("Atomic Operations Overview");
    std::cout << "OpenMP provides several atomic operations for thread synchronization:\n\n";
    std::cout << "1. atomic update: Performs an atomic update operation\n";
    std::cout << "2. atomic read: Performs an atomic read operation\n";
    std::cout << "3. atomic write: Performs an atomic write operation\n";
    std::cout << "4. atomic capture: Performs an atomic update and captures the result\n\n";
    
    std::cout << "Supported expressions for update:\n";
    std::cout << "  x++, ++x, x--, --x\n";
    std::cout << "  x += expr, x -= expr, x *= expr, x /= expr\n";
    std::cout << "  x &= expr, x |= expr, x ^= expr\n";
    std::cout << "  x >>= expr, x <<= expr\n\n";
    
    std::cout << "Advantages of atomic operations:\n";
    std::cout << "- Lower overhead than critical sections\n";
    std::cout << "- May use hardware-specific atomic instructions\n";
    std::cout << "- More scalable with increasing thread count\n\n";
    
    std::cout << "Limitations:\n";
    std::cout << "- Limited to specific operations and expressions\n";
    std::cout << "- Complex operations may require multiple atomic directives\n";
    std::cout << "- Not suitable for protecting larger blocks of code\n\n";
    
    std::cout << "Examples will follow demonstrating each type of atomic operation.\n";
}

// Main atomic operations demo
void demo_atomic_operations(int num_threads, int workload) {
    utils::print_header("Atomic Operations Demo");
    std::cout << "This demo shows how to use #pragma omp atomic for lightweight synchronization\n";
    std::cout << "Atomic operations provide efficient alternatives to critical sections for simple operations\n\n";
    
    // If threads not specified, use all available
    if (num_threads <= 0) {
        num_threads = omp_get_max_threads();
    }
    
    atomic_operations_overview(num_threads, workload);
    
    utils::print_result("Demo completed", true);
}

// Atomic update demo
void demo_atomic_update(int num_threads, int workload) {
    utils::print_header("Atomic Update Demo");
    std::cout << "This demo shows how to use #pragma omp atomic update\n";
    std::cout << "Atomic update operations modify a variable atomically\n\n";
    
    // If threads not specified, use all available
    if (num_threads <= 0) {
        num_threads = omp_get_max_threads();
    }
    
    // Scale workload based on demo
    int iterations = std::min(workload / 100, 1000000);
    
    atomic_update_demo(num_threads, iterations);
    
    utils::print_result("Demo completed", true);
}

// Atomic read demo
void demo_atomic_read(int num_threads, int workload) {
    utils::print_header("Atomic Read Demo");
    std::cout << "This demo shows how to use #pragma omp atomic read\n";
    std::cout << "Atomic read operations read a variable atomically\n\n";
    
    // If threads not specified, use all available
    if (num_threads <= 0) {
        num_threads = omp_get_max_threads();
    }
    
    // Scale workload based on demo
    int iterations = std::min(workload / 1000, 100);
    
    atomic_read_demo(num_threads, iterations);
    
    utils::print_result("Demo completed", true);
}

// Atomic write demo
void demo_atomic_write(int num_threads, int workload) {
    utils::print_header("Atomic Write Demo");
    std::cout << "This demo shows how to use #pragma omp atomic write\n";
    std::cout << "Atomic write operations write to a variable atomically\n\n";
    
    // If threads not specified, use all available
    if (num_threads <= 0) {
        num_threads = omp_get_max_threads();
    }
    
    // Scale workload based on demo
    int iterations = std::min(workload / 1000, 100);
    
    atomic_write_demo(num_threads, iterations);
    
    utils::print_result("Demo completed", true);
}

// Atomic capture demo
void demo_atomic_capture(int num_threads, int workload) {
    utils::print_header("Atomic Capture Demo");
    std::cout << "This demo shows how to use #pragma omp atomic capture\n";
    std::cout << "Atomic capture operations read and modify a variable atomically\n\n";
    
    // If threads not specified, use all available
    if (num_threads <= 0) {
        num_threads = omp_get_max_threads();
    }
    
    // Scale workload based on demo
    int iterations = std::min(workload / 1000, 1000);
    
    atomic_capture_demo(num_threads, iterations);
    
    utils::print_result("Demo completed", true);
}

// Benchmark atomic vs critical
void benchmark_atomic_vs_critical(int num_threads, int workload) {
    utils::print_header("Atomic vs Critical Performance Benchmark");
    std::cout << "This benchmark compares the performance of atomic operations vs critical sections\n\n";
    
    // If threads not specified, use all available
    if (num_threads <= 0) {
        num_threads = omp_get_max_threads();
    }
    
    // Scale workload based on benchmark
    int scaled_workload = std::min(workload, 10000000);
    
    atomic_vs_critical_benchmark(num_threads, scaled_workload);
    
    utils::print_result("Benchmark completed", true);
} 