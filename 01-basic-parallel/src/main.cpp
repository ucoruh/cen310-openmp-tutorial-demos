#include <iostream>
#include <omp.h>
#include <chrono>
#include <vector>
#include <iomanip>

int main() {
    // Check if OpenMP is available
    #ifdef _OPENMP
        std::cout << "OpenMP is supported! Version: " << _OPENMP << std::endl;
    #else
        std::cerr << "OpenMP is not supported!" << std::endl;
        return 1;
    #endif

    const int NUM_THREADS = 4;
    
    // Set the number of threads
    omp_set_num_threads(NUM_THREADS);
    
    std::cout << "=== Basic Parallel Region Demo ===" << std::endl;
    
    // Basic parallel region where each thread identifies itself
    #pragma omp parallel
    {
        // Get this thread's ID
        int thread_id = omp_get_thread_num();
        
        // Get the total number of threads
        int total_threads = omp_get_num_threads();
        
        // This code is executed by every thread
        #pragma omp critical
        {
            std::cout << "Thread " << thread_id << " of " << total_threads 
                      << " is executing" << std::endl;
        }
    }
    
    std::cout << "\n=== Thread-Private vs Shared Variables Demo ===" << std::endl;
    
    int shared_var = 0;
    
    #pragma omp parallel default(none) shared(shared_var, std::cout) num_threads(NUM_THREADS)
    {
        // Thread-private variable (each thread has its own copy)
        int private_var = omp_get_thread_num();
        
        // Shared variable (all threads see and modify the same variable)
        #pragma omp critical
        {
            shared_var += 1;
            std::cout << "Thread " << private_var << ": "
                      << "Private var = " << private_var 
                      << ", Shared var = " << shared_var << std::endl;
        }
    }
    
    std::cout << "\n=== Performance Measurement Demo ===" << std::endl;
    
    // Size of the vectors for performance demonstration
    const size_t vector_size = 100000000;
    
    // Initialize vectors with random values
    std::vector<double> a(vector_size, 1.0);
    std::vector<double> b(vector_size, 2.0);
    std::vector<double> c(vector_size, 0.0);
    
    // Sequential execution
    auto start_sequential = std::chrono::high_resolution_clock::now();
    
    for (size_t i = 0; i < vector_size; ++i) {
        c[i] = a[i] + b[i];
    }
    
    auto end_sequential = std::chrono::high_resolution_clock::now();
    auto time_sequential = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_sequential - start_sequential).count();
    
    // Reset result vector
    std::fill(c.begin(), c.end(), 0.0);
    
    // Parallel execution
    auto start_parallel = std::chrono::high_resolution_clock::now();
    
    #pragma omp parallel for
    for (long long i = 0; i < static_cast<long long>(vector_size); ++i) {
        c[i] = a[i] + b[i];
    }
    
    auto end_parallel = std::chrono::high_resolution_clock::now();
    auto time_parallel = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_parallel - start_parallel).count();
    
    // Check result for one element
    std::cout << "Vector addition result (element 0): " << c[0] << std::endl;
    std::cout << "Sequential execution time: " << time_sequential << " ms" << std::endl;
    std::cout << "Parallel execution time: " << time_parallel << " ms" << std::endl;
    
    if (time_sequential > 0) {
        double speedup = static_cast<double>(time_sequential) / time_parallel;
        std::cout << "Speedup: " << std::fixed << std::setprecision(2) << speedup << "x" << std::endl;
        std::cout << "Efficiency: " << std::fixed << std::setprecision(2) 
                  << (speedup / NUM_THREADS) * 100 << "%" << std::endl;
    }
    
    return 0;
} 