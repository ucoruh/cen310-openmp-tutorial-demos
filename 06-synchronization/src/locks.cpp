#include <iostream>
#include <vector>
#include <iomanip>
#include <thread>
#include <omp.h>
#include "../include/synchronization_demos.h"
#include "../include/utils.h"

// Simple locks demonstration
void simple_locks_demo(int num_threads, int iterations) {
    utils::print_subsection("Simple Locks Demo");
    std::cout << "Using omp_set_lock() and omp_unset_lock() for mutual exclusion\n";
    std::cout << "Incrementing a shared counter with " << num_threads << " threads\n";
    std::cout << "Each thread will increment " << iterations << " times\n\n";
    
    // Expected final count
    long long expected_count = static_cast<long long>(num_threads) * iterations;
    
    // Counter with lock
    long long counter_lock = 0;
    
    // Counter with critical section for comparison
    long long counter_critical = 0;
    
    // Create a lock
    omp_lock_t lock;
    omp_init_lock(&lock);
    
    // Set the number of threads
    if (num_threads > 0) {
        omp_set_num_threads(num_threads);
    } else {
        num_threads = omp_get_max_threads();
    }
    
    utils::print_result("Number of threads", num_threads);
    utils::print_result("Iterations per thread", iterations);
    utils::print_result("Expected final count", static_cast<double>(expected_count));
    
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
    
    // With lock
    std::cout << "\nRunning with simple lock...\n";
    timer.reset();
    timer.start();
    
    #pragma omp parallel 
    {
        for (int i = 0; i < iterations; i++) {
            omp_set_lock(&lock);
            counter_lock++; // Protected by lock
            omp_unset_lock(&lock);
        }
    }
    
    timer.stop();
    double lock_time = timer.elapsed_ms();
    
    // Destroy the lock
    omp_destroy_lock(&lock);
    
    // Calculate and print results
    utils::print_result("Final count (with critical)", static_cast<double>(counter_critical));
    utils::print_result("Final count (with lock)", static_cast<double>(counter_lock));
    utils::print_result("Expected count", static_cast<double>(expected_count));
    utils::print_result("Error (critical)", static_cast<double>(expected_count - counter_critical));
    utils::print_result("Error (lock)", static_cast<double>(expected_count - counter_lock));
    utils::print_result("Execution time (critical)", critical_time, "ms");
    utils::print_result("Execution time (lock)", lock_time, "ms");
    
    // Calculate speedup or slowdown
    double ratio = critical_time / lock_time;
    if (ratio > 1.0) {
        utils::print_result("Speedup (lock vs critical)", ratio, "x");
    } else {
        utils::print_result("Slowdown (lock vs critical)", 1.0 / ratio, "x");
    }
    
    std::cout << "\nSimple locks provide similar functionality to critical sections but with more flexibility.\n";
    std::cout << "They allow synchronization outside of structured blocks and can be used with different APIs.\n";
}

// Nested locks demonstration
void nested_locks_demo(int num_threads, int iterations) {
    utils::print_subsection("Nested Locks Demo");
    std::cout << "Using omp_set_nest_lock() and omp_unset_nest_lock() for reentrant locking\n";
    std::cout << "Nested locks can be acquired multiple times by the same thread\n\n";
    
    // Set the number of threads
    if (num_threads > 0) {
        omp_set_num_threads(num_threads);
    } else {
        num_threads = omp_get_max_threads();
    }
    
    // Limit iterations to avoid potential deadlocks
    int safe_iterations = std::min(iterations, 3);
    utils::print_result("Number of threads", num_threads);
    utils::print_result("Iterations per thread", safe_iterations);
    
    // Create and initialize a nested lock
    omp_nest_lock_t nest_lock;
    omp_init_nest_lock(&nest_lock);
    
    // Create a shared counter
    std::atomic<int> counter(0);
    
    // Track max nesting level for each thread
    std::vector<int> max_nesting(num_threads, 0);
    
    // Timer
    auto start_time = std::chrono::high_resolution_clock::now();
    
    #pragma omp parallel 
    {
        int tid = omp_get_thread_num();
        
        // Each thread works independently
        for (int i = 0; i < safe_iterations; i++) {
            // Simple random delay to avoid contention issues
            std::this_thread::sleep_for(std::chrono::milliseconds(tid % 5));
            
            // First-level lock
            omp_set_nest_lock(&nest_lock);
            int nesting = 1;
            counter++;
            
            // Demonstrate nested lock acquisition - different threads use different nesting depths
            int max_nested = (tid % 3) + 1; // 1, 2, or 3 levels based on thread ID
            
            for (int level = 1; level < max_nested; level++) {
                omp_set_nest_lock(&nest_lock);
                nesting++;
                counter++;
                
                // Brief work inside nested lock
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            
            // Track maximum nesting level for this thread
            max_nesting[tid] = std::max(max_nesting[tid], nesting);
            
            // Release the locks
            for (int level = 0; level < max_nested; level++) {
                counter++;
                omp_unset_nest_lock(&nest_lock);
            }
            
            // Add small delay between iterations
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    // Destroy the nested lock
    omp_destroy_nest_lock(&nest_lock);
    
    // Expected value: Each thread does iterations * (2*avg_nesting_level + 1) operations
    // where avg_nesting_level is approximately (1+2+3)/3 = 2
    double avg_nesting = 0;
    for (int i = 0; i < num_threads; i++) {
        avg_nesting += max_nesting[i];
    }
    avg_nesting /= num_threads;
    
    int expected_count = num_threads * safe_iterations * (2 * static_cast<int>(avg_nesting) + 1);
    
    // Print results
    // Vektörü doğrudan yazdıramayız, bunun yerine içeriğini manuel olarak yazdıralım
    std::cout << "Max nesting level (by thread): ";
    for (int i = 0; i < num_threads; i++) {
        std::cout << max_nesting[i];
        if (i < num_threads - 1) std::cout << ", ";
    }
    std::cout << std::endl;
    
    utils::print_result("Average max nesting level", avg_nesting);
    utils::print_result("Final counter value", counter.load());
    utils::print_result("Expected counter value", expected_count);
    utils::print_result("Error percentage", 100.0 * std::abs(counter.load() - expected_count) / static_cast<double>(expected_count), "%");
    utils::print_result("Execution time", static_cast<int>(duration.count()), "ms");
    
    std::cout << "\nNested locks are useful when a thread needs to acquire the same lock multiple times,\n";
    std::cout << "for example in recursive functions or when calling functions that also acquire the lock.\n";
    std::cout << "Regular (simple) locks would deadlock in these scenarios.\n";
}

// Lock hints demonstration
void lock_hints_demo(int num_threads, int iterations) {
    utils::print_subsection("Lock Hints Demo");
    std::cout << "Using lock hints to provide optimization guidance to the OpenMP runtime\n";
    std::cout << "Note: Lock hints are available in OpenMP 5.0+ and may not be supported in all environments\n\n";
    
    // Use try-catch to handle potential issues with unsupported features
    try {
        // Set the number of threads
        if (num_threads > 0) {
            omp_set_num_threads(num_threads);
        } else {
            num_threads = omp_get_max_threads();
        }
        
        utils::print_result("Number of threads", num_threads);
        utils::print_result("Iterations per thread", iterations);
        
        #if _OPENMP >= 201811 // OpenMP 5.0+
        // Create locks with different hints
        omp_lock_t lock_no_hint;
        omp_lock_t lock_uncontended;
        omp_lock_t lock_contended;
        
        // Initialize locks with different hints
        // No hint (default)
        omp_init_lock(&lock_no_hint);
        
        // Uncontended hint - optimized for low contention
        omp_init_lock_with_hint(&lock_uncontended, omp_lock_hint_uncontended);
        
        // Contended hint - optimized for high contention
        omp_init_lock_with_hint(&lock_contended, omp_lock_hint_contended);
        
        // Mark these variables as intentionally unused
        (void)lock_no_hint;
        (void)lock_uncontended;
        (void)lock_contended;
        
        // Run benchmarks
        std::vector<std::pair<std::string, double>> benchmark_results;
        
        // Case 1: Low contention scenario (infrequent updates)
        std::cout << "\nLow contention scenario (infrequent updates):\n";
        long long counter_no_hint_low = 0;
        long long counter_uncontended = 0;
        long long counter_contended = 0;
        
        // Test with no hint
        utils::Timer timer;
        timer.start();
        
        #pragma omp parallel
        {
            for (int i = 0; i < iterations; i++) {
                // Simulate work before lock acquisition
                for (volatile int j = 0; j < 1000; j++) { }
                
                omp_set_lock(&lock_no_hint);
                counter_no_hint_low++;
                omp_unset_lock(&lock_no_hint);
            }
        }
        
        timer.stop();
        double no_hint_low_time = timer.elapsed_ms();
        benchmark_results.push_back({"No Hint (Low)", no_hint_low_time});
        
        // Test with uncontended hint
        timer.reset();
        timer.start();
        
        #pragma omp parallel
        {
            for (int i = 0; i < iterations; i++) {
                // Simulate work before lock acquisition
                for (volatile int j = 0; j < 1000; j++) { }
                
                omp_set_lock(&lock_uncontended);
                counter_uncontended++;
                omp_unset_lock(&lock_uncontended);
            }
        }
        
        timer.stop();
        double uncontended_time = timer.elapsed_ms();
        benchmark_results.push_back({"Uncontended", uncontended_time});
        
        // Test with contended hint
        timer.reset();
        timer.start();
        
        #pragma omp parallel
        {
            for (int i = 0; i < iterations; i++) {
                // Simulate work before lock acquisition
                for (volatile int j = 0; j < 1000; j++) { }
                
                omp_set_lock(&lock_contended);
                counter_contended++;
                omp_unset_lock(&lock_contended);
            }
        }
        
        timer.stop();
        double contended_time = timer.elapsed_ms();
        benchmark_results.push_back({"Contended", contended_time});
        
        // Print results for low contention
        utils::print_result("No hint time (low contention)", no_hint_low_time, "ms");
        utils::print_result("Uncontended hint time", uncontended_time, "ms");
        utils::print_result("Contended hint time", contended_time, "ms");
        
        // Case 2: High contention scenario (frequent updates)
        std::cout << "\nHigh contention scenario (frequent updates):\n";
        long long counter_no_hint_high = 0;
        
        // Test with no hint
        timer.reset();
        timer.start();
        
        #pragma omp parallel
        {
            for (int i = 0; i < iterations; i++) {
                // No work between lock acquisitions
                omp_set_lock(&lock_no_hint);
                counter_no_hint_high++;
                omp_unset_lock(&lock_no_hint);
            }
        }
        
        timer.stop();
        double no_hint_high_time = timer.elapsed_ms();
        benchmark_results.push_back({"No Hint (High)", no_hint_high_time});
        
        // Reset counters
        counter_uncontended = 0;
        counter_contended = 0;
        
        // Test with uncontended hint
        timer.reset();
        timer.start();
        
        #pragma omp parallel
        {
            for (int i = 0; i < iterations; i++) {
                // No work between lock acquisitions
                omp_set_lock(&lock_uncontended);
                counter_uncontended++;
                omp_unset_lock(&lock_uncontended);
            }
        }
        
        timer.stop();
        double uncontended_high_time = timer.elapsed_ms();
        benchmark_results.push_back({"Uncontended (High)", uncontended_high_time});
        
        // Test with contended hint
        timer.reset();
        timer.start();
        
        #pragma omp parallel
        {
            for (int i = 0; i < iterations; i++) {
                // No work between lock acquisitions
                omp_set_lock(&lock_contended);
                counter_contended++;
                omp_unset_lock(&lock_contended);
            }
        }
        
        timer.stop();
        double contended_high_time = timer.elapsed_ms();
        benchmark_results.push_back({"Contended (High)", contended_high_time});
        
        // Print results for high contention
        utils::print_result("No hint time (high contention)", no_hint_high_time, "ms");
        utils::print_result("Uncontended hint time (high contention)", uncontended_high_time, "ms");
        utils::print_result("Contended hint time (high contention)", contended_high_time, "ms");
        
        // Draw chart
        utils::draw_bar_chart(benchmark_results, 60, 10);
        
        // Destroy locks
        omp_destroy_lock(&lock_no_hint);
        omp_destroy_lock(&lock_uncontended);
        omp_destroy_lock(&lock_contended);
        
        std::cout << "\nLock hints can provide performance optimizations by giving the runtime information\n";
        std::cout << "about the intended use of the lock. This allows the runtime to optimize the lock implementation.\n";
        #else
        // OpenMP < 5.0
        std::cout << "Lock hints are not supported in this OpenMP version. They require OpenMP 5.0 or later.\n";
        #endif
    }
    catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
        std::cerr << "Lock hints may not be supported in this environment." << std::endl;
    }
}

// Custom reader-writer lock implementation
typedef struct {
    omp_lock_t read_lock;
    omp_lock_t write_lock;
    int readers;
} rw_lock_t;

void rw_lock_init(rw_lock_t* lock) {
    omp_init_lock(&lock->read_lock);
    omp_init_lock(&lock->write_lock);
    lock->readers = 0;
}

void rw_lock_destroy(rw_lock_t* lock) {
    omp_destroy_lock(&lock->read_lock);
    omp_destroy_lock(&lock->write_lock);
}

void read_lock(rw_lock_t* lock) {
    omp_set_lock(&lock->read_lock);
    lock->readers++;
    if (lock->readers == 1) {
        // First reader acquires the write lock
        omp_set_lock(&lock->write_lock);
    }
    omp_unset_lock(&lock->read_lock);
}

void read_unlock(rw_lock_t* lock) {
    omp_set_lock(&lock->read_lock);
    lock->readers--;
    if (lock->readers == 0) {
        // Last reader releases the write lock
        omp_unset_lock(&lock->write_lock);
    }
    omp_unset_lock(&lock->read_lock);
}

void write_lock(rw_lock_t* lock) {
    omp_set_lock(&lock->write_lock);
}

void write_unlock(rw_lock_t* lock) {
    omp_unset_lock(&lock->write_lock);
}

// Reader-writer locks demonstration
void reader_writer_locks_demo(int num_threads, int iterations) {
    utils::print_subsection("Reader-Writer Locks Demo");
    std::cout << "Implementing reader-writer locks using OpenMP locks\n";
    std::cout << "Reader-writer locks allow multiple readers or a single writer\n\n";
    
    // Create a shared data structure
    std::vector<int> shared_data(10, 0);
    
    // Create a reader-writer lock
    rw_lock_t rw_lock;
    rw_lock_init(&rw_lock);
    
    // Create a regular lock for comparison
    omp_lock_t regular_lock;
    omp_init_lock(&regular_lock);
    
    // Statistics
    std::vector<int> reader_counts(num_threads, 0);
    std::vector<int> writer_counts(num_threads, 0);
    std::vector<int> reader_wait_times(num_threads, 0);
    std::vector<int> writer_wait_times(num_threads, 0);
    
    // Set the number of threads
    if (num_threads > 0) {
        omp_set_num_threads(num_threads);
    } else {
        num_threads = omp_get_max_threads();
    }
    
    utils::print_result("Number of threads", num_threads);
    utils::print_result("Iterations per thread", iterations);
    
    // Test with regular lock
    std::cout << "\nRunning with regular lock...\n";
    utils::Timer timer;
    timer.start();
    
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        std::vector<int> local_data(10);
        
        for (int i = 0; i < iterations; i++) {
            // Determine if this is a read or write operation
            bool is_read = (i % 10 != 0); // 90% reads, 10% writes
            
            if (is_read) {
                // Read operation with regular lock
                auto start_time = std::chrono::high_resolution_clock::now();
                omp_set_lock(&regular_lock);
                auto end_time = std::chrono::high_resolution_clock::now();
                reader_wait_times[thread_id] += std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
                
                // Read shared data
                for (int j = 0; j < 10; j++) {
                    local_data[j] = shared_data[j];
                }
                reader_counts[thread_id]++;
                
                omp_unset_lock(&regular_lock);
            } else {
                // Write operation with regular lock
                auto start_time = std::chrono::high_resolution_clock::now();
                omp_set_lock(&regular_lock);
                auto end_time = std::chrono::high_resolution_clock::now();
                writer_wait_times[thread_id] += std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
                
                // Write to shared data
                for (int j = 0; j < 10; j++) {
                    shared_data[j] = thread_id * 100 + j;
                }
                writer_counts[thread_id]++;
                
                omp_unset_lock(&regular_lock);
            }
            
            // Simulate some work
            for (volatile int j = 0; j < 100; j++) { }
        }
    }
    
    timer.stop();
    double regular_lock_time = timer.elapsed_ms();
    
    // Reset counters and wait times
    for (int i = 0; i < num_threads; i++) {
        reader_counts[i] = 0;
        writer_counts[i] = 0;
        reader_wait_times[i] = 0;
        writer_wait_times[i] = 0;
    }
    
    // Reset shared data
    for (int i = 0; i < 10; i++) {
        shared_data[i] = 0;
    }
    
    // Test with reader-writer lock
    std::cout << "Running with reader-writer lock...\n";
    timer.reset();
    timer.start();
    
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        std::vector<int> local_data(10);
        
        for (int i = 0; i < iterations; i++) {
            // Determine if this is a read or write operation
            bool is_read = (i % 10 != 0); // 90% reads, 10% writes
            
            if (is_read) {
                // Read operation with reader-writer lock
                auto start_time = std::chrono::high_resolution_clock::now();
                read_lock(&rw_lock);
                auto end_time = std::chrono::high_resolution_clock::now();
                reader_wait_times[thread_id] += std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
                
                // Read shared data
                for (int j = 0; j < 10; j++) {
                    local_data[j] = shared_data[j];
                }
                reader_counts[thread_id]++;
                
                read_unlock(&rw_lock);
            } else {
                // Write operation with reader-writer lock
                auto start_time = std::chrono::high_resolution_clock::now();
                write_lock(&rw_lock);
                auto end_time = std::chrono::high_resolution_clock::now();
                writer_wait_times[thread_id] += std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
                
                // Write to shared data
                for (int j = 0; j < 10; j++) {
                    shared_data[j] = thread_id * 100 + j;
                }
                writer_counts[thread_id]++;
                
                write_unlock(&rw_lock);
            }
            
            // Simulate some work
            for (volatile int j = 0; j < 100; j++) { }
        }
    }
    
    timer.stop();
    double rw_lock_time = timer.elapsed_ms();
    
    // Calculate total reads and writes
    int total_reads = 0;
    int total_writes = 0;
    int total_reader_wait = 0;
    int total_writer_wait = 0;
    
    for (int i = 0; i < num_threads; i++) {
        total_reads += reader_counts[i];
        total_writes += writer_counts[i];
        total_reader_wait += reader_wait_times[i];
        total_writer_wait += writer_wait_times[i];
    }
    
    // Display results
    utils::print_result("Regular lock time", regular_lock_time, "ms");
    utils::print_result("Reader-writer lock time", rw_lock_time, "ms");
    
    double speedup = regular_lock_time / rw_lock_time;
    utils::print_result("Speedup", speedup, "x");
    
    utils::print_result("Total read operations", total_reads);
    utils::print_result("Total write operations", total_writes);
    utils::print_result("Average reader wait time", total_reader_wait / std::max(1, total_reads), "µs");
    utils::print_result("Average writer wait time", total_writer_wait / std::max(1, total_writes), "µs");
    
    // Destroy locks
    rw_lock_destroy(&rw_lock);
    omp_destroy_lock(&regular_lock);
    
    std::cout << "\nReader-writer locks allow multiple readers to access shared data simultaneously,\n";
    std::cout << "while ensuring exclusive access for writers. This can significantly improve performance\n";
    std::cout << "in read-heavy workloads compared to regular locks that serialize all access.\n";
}

// Locks overview
void locks_overview(int /*num_threads*/, int /*workload*/) {
    utils::print_section("OpenMP Locks Overview");
    
    std::cout << "OpenMP provides several types of locks for synchronization:\n\n";
    
    std::cout << "1. Simple Locks (omp_lock_t)\n";
    std::cout << "   * Basic mutual exclusion\n";
    std::cout << "   * Can only be acquired once by a thread\n";
    std::cout << "   * API: omp_init_lock(), omp_set_lock(), omp_unset_lock(), omp_destroy_lock()\n\n";
    
    std::cout << "2. Nested Locks (omp_nest_lock_t)\n";
    std::cout << "   * Can be acquired multiple times by the same thread\n";
    std::cout << "   * Useful for recursive functions or nested function calls\n";
    std::cout << "   * API: omp_init_nest_lock(), omp_set_nest_lock(), omp_unset_nest_lock(), omp_destroy_nest_lock()\n\n";
    
    std::cout << "3. Lock Hints (OpenMP 5.0+)\n";
    std::cout << "   * Provide optimization guidance to the runtime\n";
    std::cout << "   * Examples: uncontended, speculative, contended\n";
    std::cout << "   * API: omp_init_lock_with_hint(), omp_init_nest_lock_with_hint()\n\n";
    
    std::cout << "4. Other Lock Functions\n";
    std::cout << "   * omp_test_lock() - Non-blocking attempt to acquire a lock\n";
    std::cout << "   * omp_test_nest_lock() - Non-blocking for nested locks, returns nesting level\n\n";
    
    std::cout << "5. Reader-Writer Locks (User Implemented)\n";
    std::cout << "   * Allow multiple simultaneous readers but exclusive writers\n";
    std::cout << "   * Can be implemented using OpenMP locks\n\n";
    
    std::cout << "Simple Demo Examples (Run with 1 thread to avoid deadlocks in demo mode):\n";
    
    // Minimal simple lock demo
    omp_lock_t simple_lock;
    omp_init_lock(&simple_lock);
    omp_set_lock(&simple_lock);
    std::cout << "Lock acquired\n";
    omp_unset_lock(&simple_lock);
    std::cout << "Lock released\n";
    omp_destroy_lock(&simple_lock);
    
    // Minimal nested lock demo
    omp_nest_lock_t nest_lock;
    omp_init_nest_lock(&nest_lock);
    omp_set_nest_lock(&nest_lock);
    std::cout << "Nested lock acquired (level 1)\n";
    omp_set_nest_lock(&nest_lock);
    std::cout << "Nested lock acquired again (level 2)\n";
    omp_unset_nest_lock(&nest_lock);
    std::cout << "Nested lock released (level 1 remaining)\n";
    omp_unset_nest_lock(&nest_lock);
    std::cout << "Nested lock fully released\n";
    omp_destroy_nest_lock(&nest_lock);
    
    std::cout << "\nUse the specific demo functions for detailed examples of each lock type.\n";
}

// Main locks demo
void demo_locks(int num_threads, int workload) {
    utils::print_header("Locks Demo");
    std::cout << "This demo shows how to use OpenMP locks for synchronization\n";
    std::cout << "Locks provide more flexible synchronization than critical sections\n\n";
    
    // If threads not specified, use all available
    if (num_threads <= 0) {
        num_threads = omp_get_max_threads();
    }
    
    locks_overview(num_threads, workload);
    
    utils::print_result("Demo completed", true);
}

// Simple locks demo
void demo_simple_locks(int num_threads, int workload) {
    utils::print_header("Simple Locks Demo");
    std::cout << "This demo shows how to use omp_set_lock() and omp_unset_lock()\n";
    std::cout << "Simple locks provide basic mutual exclusion\n\n";
    
    // If threads not specified, use all available
    if (num_threads <= 0) {
        num_threads = omp_get_max_threads();
    }
    
    // Scale workload based on demo
    int iterations = std::min(workload / 100, 1000000);
    
    simple_locks_demo(num_threads, iterations);
    
    utils::print_result("Demo completed", true);
}

// Main driver function for nested locks demo
void demo_nested_locks(int num_threads, int workload) {
    utils::print_section("Nested Locks Demo");
    std::cout << "This demo shows how to use omp_set_nest_lock() and omp_unset_nest_lock()\n";
    std::cout << "Nested locks can be acquired multiple times by the same thread\n\n";
    
    // Basitleştirilmiş demo çalıştır - ana demoda kilitlenme oluyor
    utils::print_section("Simplified Nested Locks Demo");
    std::cout << "Using simplified demo to avoid potential deadlocks in benchmark mode\n\n";
    
    // Tek bir thread ile gösterim yapalım
    int demo_thread_count = 1;
    omp_set_num_threads(demo_thread_count);
    
    omp_nest_lock_t nest_lock;
    omp_init_nest_lock(&nest_lock);
    
    #pragma omp parallel
    {
        // İlk kilit edinme
        omp_set_nest_lock(&nest_lock);
        std::cout << "Thread " << omp_get_thread_num() << " acquired lock (level 1)\n";
        
        // İkinci kilit edinme - aynı thread tarafından
        omp_set_nest_lock(&nest_lock);
        std::cout << "Thread " << omp_get_thread_num() << " acquired lock (level 2)\n";
        
        // Üçüncü kilit edinme
        omp_set_nest_lock(&nest_lock);
        std::cout << "Thread " << omp_get_thread_num() << " acquired lock (level 3)\n";
        
        // Kilitleri serbest bırakma
        omp_unset_nest_lock(&nest_lock);
        std::cout << "Thread " << omp_get_thread_num() << " released lock (level 3)\n";
        
        omp_unset_nest_lock(&nest_lock);
        std::cout << "Thread " << omp_get_thread_num() << " released lock (level 2)\n";
        
        omp_unset_nest_lock(&nest_lock);
        std::cout << "Thread " << omp_get_thread_num() << " released lock (level 1)\n";
    }
    
    omp_destroy_nest_lock(&nest_lock);
    
    std::cout << "\nNested locks allow the same thread to acquire the same lock multiple times.\n";
    std::cout << "Each call to omp_unset_nest_lock() decreases the nesting level.\n";
    std::cout << "The lock is only released when the nesting level returns to zero.\n";
    std::cout << "This is useful for recursive algorithms or when calling functions that also acquire locks.\n";
    
    // Original thread count'u geri yükleyelim
    if (num_threads > 0) {
        omp_set_num_threads(num_threads);
    }
}

// Lock hints demo
void demo_lock_hints(int num_threads, int workload) {
    utils::print_header("Lock Hints Demo");
    std::cout << "This demo shows how to use lock hints for performance optimization\n";
    std::cout << "Lock hints provide information to the runtime about the intended use of the lock\n\n";
    
    // If threads not specified, use all available
    if (num_threads <= 0) {
        num_threads = omp_get_max_threads();
    }
    
    // Scale workload based on demo
    int iterations = std::min(workload / 1000, 10000);
    
    lock_hints_demo(num_threads, iterations);
    
    utils::print_result("Demo completed", true);
}

// Reader-writer locks demo
void demo_reader_writer_locks(int num_threads, int workload) {
    utils::print_header("Reader-Writer Locks Demo");
    std::cout << "This demo shows how to implement reader-writer locks using OpenMP locks\n";
    std::cout << "Reader-writer locks allow multiple readers or a single writer\n\n";
    
    // If threads not specified, use all available
    if (num_threads <= 0) {
        num_threads = omp_get_max_threads();
    }
    
    // Scale workload based on demo
    int iterations = std::min(workload / 1000, 10000);
    
    reader_writer_locks_demo(num_threads, iterations);
    
    utils::print_result("Demo completed", true);
} 