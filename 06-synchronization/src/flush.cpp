#include <iostream>
#include <vector>
#include <iomanip>
#include <thread>
#include <chrono>
#include <omp.h>
#include "../include/synchronization_demos.h"
#include "../include/utils.h"

// Simple flag-based signaling between threads using flush
void demo_flush_signaling(int /*num_threads*/, int /*workload*/) {
    utils::print_subsection("Thread Signaling with Flush");
    std::cout << "Demonstrating how flush can be used for thread signaling\n\n";
    
    int num_threads = 2; // Need at least 2 threads for this demo
    
    utils::print_result("Number of threads", static_cast<double>(num_threads));
    
    // Signal flags
    volatile bool ready_flags[2] = {false, false};
    volatile int shared_data = 0;
    
    std::cout << "Thread 0 will produce data and signal Thread 1\n";
    std::cout << "Thread 1 will wait for the signal, then consume the data\n\n";
    
    #pragma omp parallel num_threads(2)
    {
        int tid = omp_get_thread_num();
        
        if (tid == 0) { // Producer thread
            std::cout << "Producer: Thread 0 starting work...\n";
            
            // Simulate producing data
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            
            std::cout << "Producer: Work complete, updating shared data...\n";
            shared_data = 42; // Update the shared data
            
            // Make sure the data is visible to other threads before setting flag
            #pragma omp flush(shared_data)
            
            std::cout << "Producer: Setting ready flag for consumer...\n";
            ready_flags[0] = true; // Signal that data is ready
            
            // Flush the flag variable
            #pragma omp flush(ready_flags)
            
            // Wait for consumer to process
            std::cout << "Producer: Waiting for consumer to process data...\n";
            while (!ready_flags[1]) {
                // Periodically flush to see updates
                #pragma omp flush(ready_flags)
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            
            std::cout << "Producer: Consumer has processed data, exiting.\n";
        }
        else if (tid == 1) { // Consumer thread
            std::cout << "Consumer: Thread 1 waiting for data...\n";
            
            // Wait for the producer to signal data is ready
            while (!ready_flags[0]) {
                // Periodically flush to see updates
                #pragma omp flush(ready_flags)
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            
            // Flush to ensure we see the latest shared_data value
            #pragma omp flush(shared_data)
            
            std::cout << "Consumer: Signal received, shared data = " << shared_data << "\n";
            
            // Simulate processing the data
            std::cout << "Consumer: Processing data...\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            
            std::cout << "Consumer: Processing complete, signaling producer...\n";
            ready_flags[1] = true; // Signal that we're done
            
            // Flush the flag
            #pragma omp flush(ready_flags)
        }
    }
    
    std::cout << "\nKey points about flush:\n";
    std::cout << "1. Flush ensures memory consistency at that point\n";
    std::cout << "2. It makes stored values visible to other threads\n";
    std::cout << "3. It's often used with volatile variables for flags\n";
    std::cout << "4. Memory model details can be complex and platform-dependent\n";
}

// Demonstrate memory inconsistency without flush
void demo_memory_consistency(int /*num_threads*/, int /*workload*/) {
    utils::print_subsection("Memory Consistency Demonstration");
    std::cout << "Showing potential memory inconsistencies without proper flushing\n\n";
    
    int num_threads = omp_get_max_threads();
    
    utils::print_result("Number of threads", num_threads);
    
    // Structure to hold test results
    struct TestResult {
        int runs;
        int inconsistencies;
        double inconsistency_rate;
    };
    
    // Run with and without flush
    TestResult with_flush = {0, 0, 0.0};
    TestResult without_flush = {0, 0, 0.0};
    
    const int test_iterations = 1000;
    
    utils::print_section("Test Without Flush");
    for (int i = 0; i < test_iterations; i++) {
        volatile int data = 0;
        volatile bool flag = false;
        bool inconsistency_detected = false;
        
        #pragma omp parallel num_threads(2)
        {
            int tid = omp_get_thread_num();
            
            if (tid == 0) {
                // Producer thread
                data = 42;
                // No flush here
                flag = true;
            }
            else if (tid == 1) {
                // Consumer thread
                // Spin briefly to increase chance of race condition
                for (volatile int j = 0; j < 100; j++) { }
                
                if (flag && data != 42) {
                    inconsistency_detected = true;
                }
            }
        }
        
        without_flush.runs++;
        if (inconsistency_detected) {
            without_flush.inconsistencies++;
        }
    }
    
    without_flush.inconsistency_rate = 
        (double)without_flush.inconsistencies / without_flush.runs * 100.0;
    
    utils::print_section("Test With Flush");
    for (int i = 0; i < test_iterations; i++) {
        volatile int data = 0;
        volatile bool flag = false;
        bool inconsistency_detected = false;
        
        #pragma omp parallel num_threads(2)
        {
            int tid = omp_get_thread_num();
            
            if (tid == 0) {
                // Producer thread
                data = 42;
                #pragma omp flush(data)
                flag = true;
                #pragma omp flush(flag)
            }
            else if (tid == 1) {
                // Consumer thread
                // Spin briefly to increase chance of race condition
                for (volatile int j = 0; j < 100; j++) { }
                
                if (flag) {
                    #pragma omp flush(data)
                    if (data != 42) {
                        inconsistency_detected = true;
                    }
                }
            }
        }
        
        with_flush.runs++;
        if (inconsistency_detected) {
            with_flush.inconsistencies++;
        }
    }
    
    with_flush.inconsistency_rate = 
        (double)with_flush.inconsistencies / with_flush.runs * 100.0;
    
    // Results
    utils::print_section("Results");
    std::cout << "Without flush: " << without_flush.inconsistencies << " inconsistencies in " 
              << without_flush.runs << " runs (" << std::fixed << std::setprecision(2)
              << without_flush.inconsistency_rate << "%)\n";
    
    std::cout << "With flush: " << with_flush.inconsistencies << " inconsistencies in " 
              << with_flush.runs << " runs (" << std::fixed << std::setprecision(2)
              << with_flush.inconsistency_rate << "%)\n";
    
    std::cout << "\nNote: The actual inconsistency rate depends on many factors including:\n";
    std::cout << "- CPU architecture and memory model\n";
    std::cout << "- Compiler optimizations\n";
    std::cout << "- System load and timing\n\n";
    
    std::cout << "Even with flush, you might still see some inconsistencies\n";
    std::cout << "due to the complexity of modern memory systems. For robust\n";
    std::cout << "synchronization, consider using atomic operations or locks.\n";
}

// Demonstrate implicit flushes in OpenMP
void demo_implicit_flushes(int /*num_threads*/, int /*workload*/) {
    utils::print_subsection("Implicit Flush Behavior");
    std::cout << "OpenMP automatically adds implicit flushes at certain points\n\n";
    
    utils::print_section("Implicit Flush Points in OpenMP");
    std::cout << "The OpenMP specification states that flushes occur implicitly at:\n\n";
    
    std::cout << "1. Entry and exit of parallel regions\n";
    std::cout << "2. Entry and exit of critical regions\n";
    std::cout << "3. After barrier directives\n";
    std::cout << "4. At entry to and exit from ordered regions\n";
    std::cout << "5. After the completion of worksharing constructs without nowait\n";
    std::cout << "6. At the end of single directives (without nowait)\n";
    std::cout << "7. After atomic directives\n\n";
    
    std::cout << "These implicit flushes ensure memory consistency at these synchronization points\n";
    std::cout << "without needing explicit flush directives.\n\n";
    
    std::cout << "However, explicit flushes may still be needed in some scenarios,\n";
    std::cout << "especially when implementing your own synchronization mechanisms.\n";
}

// Main flush demo function
void demo_flush(int num_threads, int workload) {
    utils::print_section("OpenMP Flush Directive");
    std::cout << "The flush directive ensures memory consistency between threads\n";
    std::cout << "It forces variables to be synchronized with main memory\n\n";
    
    std::cout << "Using parameters: num_threads=" << num_threads << ", workload=" << workload << "\n\n";
    
    // Show implicit flush points
    demo_implicit_flushes(num_threads, workload > 0 ? workload : 1000);
    utils::pause_console();
    
    // Demonstrate thread signaling with flush
    demo_flush_signaling(num_threads, workload > 0 ? workload : 1000);
    utils::pause_console();
    
    // Demonstrate memory consistency issues
    demo_memory_consistency(num_threads, workload > 0 ? workload : 1000);
} 