/**
 * OpenMP Parallel Region Demonstration
 * This program shows the basic usage of OpenMP parallel regions
 * 
 * Compilation instructions:
 * - For GCC/G++: g++ -fopenmp openmp_parallel_demo.cpp -o openmp_demo
 * - For MSVC: cl /openmp openmp_parallel_demo.cpp
 */

#include <iostream>   // For standard input/output operations
#include <omp.h>      // OpenMP header for parallel programming directives

int main() {
    std::cout << "Starting OpenMP demonstration program..." << std::endl;
    std::cout << "Sequential code before parallel region is executed by a single thread" << std::endl;

    // Set the number of threads for the parallel region
    omp_set_num_threads(4);

    // Create a parallel region with 4 threads
    // Each thread will execute the code inside this region
    #pragma omp parallel
    {
        // This code is executed by all threads in the parallel region
        int thread_id = omp_get_thread_num();    // Get current thread ID
        int total_threads = omp_get_num_threads(); // Get total number of threads
        
        // Each thread will print its own ID and the total number of threads
        std::cout << "Thread " << thread_id << " of " << total_threads << " is executing" << std::endl;

        // Create a barrier to synchronize all threads at this point
        #pragma omp barrier
        
        // Demonstration of code that runs ONCE by a single thread
        #pragma omp single
        {
            std::cout << "\nThis message is printed only ONCE by thread " 
                      << omp_get_thread_num() << std::endl;
            std::cout << "All other threads wait at this point due to implicit barrier" << std::endl;
        }
        // Implicit barrier at the end of the single directive

        // Demonstration of code that runs by the master thread only
        #pragma omp master
        {
            std::cout << "\nThis is executed only by the master thread (thread 0)" << std::endl;
            std::cout << "Other threads do NOT wait (no implicit barrier)" << std::endl;
        }
        // No implicit barrier after master
        
        // Ensure all threads synchronize before proceeding
        #pragma omp barrier
        
        // Example of thread-specific work - each thread performs a different task
        if (thread_id == 0) {
            std::cout << "Thread 0 is doing task A" << std::endl;
        } else if (thread_id == 1) {
            std::cout << "Thread 1 is doing task B" << std::endl;
        } else if (thread_id == 2) {
            std::cout << "Thread 2 is doing task C" << std::endl;
        } else {
            std::cout << "Thread " << thread_id << " is doing task D" << std::endl;
        }
    }
    // End of parallel region - implicit barrier here

    std::cout << "\nBack to sequential execution with a single thread" << std::endl;
    std::cout << "OpenMP demonstration completed." << std::endl;
    
    return 0;
} 