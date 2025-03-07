#include <iostream>
#include <omp.h>
#include <vector>
#include <chrono>
#include <iomanip>
#include <random>
#include <algorithm>
#include <memory>
#include <string>
#include <stdexcept>

// Utility function for timing measurements
class Timer {
private:
    std::chrono::high_resolution_clock::time_point start_time;
    std::string operation_name;

public:
    Timer(const std::string& name) : operation_name(name) {
        start_time = std::chrono::high_resolution_clock::now();
        std::cout << "Starting " << operation_name << "..." << std::endl;
    }

    ~Timer() {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            end_time - start_time).count();
        std::cout << operation_name << " completed in " << duration << " ms" << std::endl;
    }

    long long elapsed_ms() {
        auto end_time = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            end_time - start_time).count();
    }
};

// Thread distribution visualization
void visualize_thread_distribution(int num_threads, long long array_size) {
    const int chunks_to_show = std::min(20, num_threads);
    const long long chunk_size = array_size / num_threads;
    
    std::cout << "\nThread Distribution Visualization:" << std::endl;
    std::cout << "Array size: " << array_size << ", " << num_threads << " threads" << std::endl;
    
    for (int i = 0; i < chunks_to_show; ++i) {
        std::cout << "Thread " << std::setw(2) << i << ": ";
        
        // Calculate chunk boundaries
        long long start = i * chunk_size;
        long long end = (i == num_threads - 1) ? array_size : (i + 1) * chunk_size;
        
        // Display percentage and visual indicator
        double percentage = (double)(end - start) / array_size * 100.0;
        int bar_length = static_cast<int>(percentage / 2);
        
        std::cout << "[" << std::setw(10) << start << " to " << std::setw(10) << (end-1) << "] ";
        std::cout << std::fixed << std::setprecision(2) << std::setw(6) << percentage << "% ";
        std::cout << std::string(bar_length, '|') << std::endl;
    }
    
    if (chunks_to_show < num_threads) {
        std::cout << "... and " << (num_threads - chunks_to_show) << " more threads" << std::endl;
    }
    std::cout << std::endl;
}

int main() {
    // Check if OpenMP is available
    #ifdef _OPENMP
        std::cout << "OpenMP is supported! Version: " << _OPENMP << std::endl;
    #else
        std::cerr << "OpenMP is not supported! Exiting." << std::endl;
        return 1;
    #endif
    
    // Array size (100 million integers)
    const long long ARRAY_SIZE = 100'000'000;
    
    std::cout << "=== OpenMP Parallel For Loop Demo ===" << std::endl;
    std::cout << "Array size: " << ARRAY_SIZE << " integers" << std::endl;
    
    // Get the number of available threads
    int max_threads = omp_get_max_threads();
    std::cout << "Maximum available threads: " << max_threads << std::endl;
    
    // Create a large array - using smart pointer for automatic cleanup
    std::unique_ptr<int[]> array;
    
    try {
        std::cout << "Allocating memory for array..." << std::endl;
        array = std::make_unique<int[]>(ARRAY_SIZE);
        std::cout << "Memory allocation successful." << std::endl;
    }
    catch (const std::bad_alloc& e) {
        std::cerr << "Memory allocation failed: " << e.what() << std::endl;
        std::cerr << "Try reducing ARRAY_SIZE or using a machine with more memory." << std::endl;
        return 1;
    }
    
    // Store timing results for comparison
    long long time_seq_init = 0;
    long long time_par_init = 0;
    long long time_seq_sum = 0;
    long long time_par_sum = 0;
    
    // 1. Sequential array initialization
    {
        Timer timer("Sequential initialization");
        for (long long i = 0; i < ARRAY_SIZE; ++i) {
            array[i] = static_cast<int>(i % 100); // Simple pattern
        }
        time_seq_init = timer.elapsed_ms();
    }
    
    // Sequential sum calculation
    long long sum_sequential = 0;
    {
        Timer timer("Sequential sum calculation");
        for (long long i = 0; i < ARRAY_SIZE; ++i) {
            sum_sequential += array[i];
        }
        time_seq_sum = timer.elapsed_ms();
    }
    std::cout << "Sequential sum result: " << sum_sequential << std::endl;
    
    // Reset array for parallel initialization
    std::fill_n(array.get(), ARRAY_SIZE, 0);
    
    // 2. Parallel array initialization with OpenMP
    {
        Timer timer("Parallel initialization");
        
        // Parallel for loop for initialization
        // Each thread initializes its own chunk of the array
        #pragma omp parallel for
        for (long long i = 0; i < ARRAY_SIZE; ++i) {
            array[i] = static_cast<int>(i % 100); // Same pattern as sequential
        }
        time_par_init = timer.elapsed_ms();
    }
    
    // Display thread distribution visualization
    visualize_thread_distribution(max_threads, ARRAY_SIZE);
    
    // 3. Parallel sum calculation with reduction
    long long sum_parallel = 0;
    {
        Timer timer("Parallel sum calculation with reduction");
        
        // Parallel for loop with reduction
        // Each thread calculates sum for its chunk, then results are combined
        #pragma omp parallel for reduction(+:sum_parallel)
        for (long long i = 0; i < ARRAY_SIZE; ++i) {
            sum_parallel += array[i];
        }
        time_par_sum = timer.elapsed_ms();
    }
    std::cout << "Parallel sum result: " << sum_parallel << std::endl;
    
    // Verify results match
    if (sum_sequential == sum_parallel) {
        std::cout << "✓ Sequential and parallel results match!" << std::endl;
    } else {
        std::cerr << "❌ Error: Sequential and parallel results do not match!" << std::endl;
        std::cerr << "  Sequential: " << sum_sequential << std::endl;
        std::cerr << "  Parallel: " << sum_parallel << std::endl;
    }
    
    // Performance comparison
    std::cout << "\n=== Performance Comparison ===" << std::endl;
    
    // Initialization comparison
    double init_speedup = time_seq_init > 0 ? 
        static_cast<double>(time_seq_init) / time_par_init : 0;
    
    std::cout << "Initialization:" << std::endl;
    std::cout << "  Sequential: " << time_seq_init << " ms" << std::endl;
    std::cout << "  Parallel: " << time_par_init << " ms" << std::endl;
    std::cout << "  Speedup: " << std::fixed << std::setprecision(2) 
              << init_speedup << "x" << std::endl;
    std::cout << "  Efficiency: " << std::fixed << std::setprecision(2) 
              << (init_speedup / max_threads * 100) << "%" << std::endl;
    
    // Sum calculation comparison
    double sum_speedup = time_seq_sum > 0 ? 
        static_cast<double>(time_seq_sum) / time_par_sum : 0;
    
    std::cout << "Sum Calculation:" << std::endl;
    std::cout << "  Sequential: " << time_seq_sum << " ms" << std::endl;
    std::cout << "  Parallel: " << time_par_sum << " ms" << std::endl;
    std::cout << "  Speedup: " << std::fixed << std::setprecision(2) 
              << sum_speedup << "x" << std::endl;
    std::cout << "  Efficiency: " << std::fixed << std::setprecision(2) 
              << (sum_speedup / max_threads * 100) << "%" << std::endl;
    
    std::cout << "\nNote: Efficiency less than 100% is normal due to threading overhead, "
              << "memory bandwidth limitations, and other factors." << std::endl;
    
    return 0;
} 