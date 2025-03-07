#include <iostream>
#include <iomanip>
#include <chrono>
#include <vector>
#include <omp.h>
#include <string>       // This provides std::string

// Sequential Fibonacci
long long fib_sequential(int n) {
    if (n < 2) return n;
    return fib_sequential(n - 1) + fib_sequential(n - 2);
}

// Task-based parallel Fibonacci
long long fib_task(int n, int cutoff) {
    if (n < 2) return n;
    
    // Use sequential version for small values to reduce overhead
    if (n < cutoff) {
        return fib_sequential(n);
    }
    
    long long x, y;
    
    // Create a task for computing fib(n-1)
    #pragma omp task shared(x)
    {
        x = fib_task(n - 1, cutoff);
    }
    
    // Computing fib(n-2) directly in the current task
    y = fib_task(n - 2, cutoff);
    
    // Wait for the task computing fib(n-1) to complete
    #pragma omp taskwait
    
    return x + y;
}

// Wrapper for parallel Fibonacci
long long fib_parallel(int n, int cutoff) {
    long long result;
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            result = fib_task(n, cutoff);
        }
    }
    
    return result;
}

// Measure execution time
template<typename Func, typename... Args>
std::pair<long long, double> measure_time(Func func, Args... args) {
    auto start = std::chrono::high_resolution_clock::now();
    long long result = func(args...);
    auto end = std::chrono::high_resolution_clock::now();
    double time = std::chrono::duration<double>(end - start).count();
    return {result, time};
}

// Display cutoff impact on performance
void analyze_cutoff_impact(int n, [[maybe_unused]] int num_threads) {
    std::cout << "\nAnalyzing cutoff impact on Fibonacci(" << n << "):" << std::endl;
    std::cout << "--------------------------------------------------" << std::endl;
    std::cout << "Cutoff | Result | Time (s) | Speedup" << std::endl;
    std::cout << "--------------------------------------------------" << std::endl;
    
    // First measure sequential time as baseline
    auto [seq_result, seq_time] = measure_time(fib_sequential, n);
    
    // Try different cutoff values
    for (int cutoff = 10; cutoff <= std::min(n, 30); cutoff += 5) {
        auto [result, time] = measure_time(fib_parallel, n, cutoff);
        
        // Calculate speedup
        double speedup = seq_time / time;
        
        std::cout << std::setw(6) << cutoff << " | "
                  << std::setw(8) << result << " | "
                  << std::fixed << std::setprecision(4) << std::setw(8) << time << " | "
                  << std::fixed << std::setprecision(2) << std::setw(6) << speedup << "x"
                  << std::endl;
    }
}

int main(int argc, char* argv[]) {
    // Parse command-line arguments
    int n = 40;
    int cutoff = 20;
    int num_threads = omp_get_max_threads();
    
    if (argc > 1) n = std::stoi(argv[1]);
    if (argc > 2) cutoff = std::stoi(argv[2]);
    if (argc > 3) num_threads = std::stoi(argv[3]);
    
    omp_set_num_threads(num_threads);
    
    std::cout << "=== Fibonacci Task Parallelism Example ===" << std::endl;
    std::cout << "Computing Fibonacci(" << n << ")" << std::endl;
    std::cout << "Number of threads: " << num_threads << std::endl;
    std::cout << "Cutoff for sequential execution: " << cutoff << std::endl;
    
    // Measure sequential time
    auto [seq_result, seq_time] = measure_time(fib_sequential, n);
    std::cout << "\nSequential Result: " << seq_result << std::endl;
    std::cout << "Sequential Time: " << std::fixed << std::setprecision(4) << seq_time << " seconds" << std::endl;
    
    // Measure parallel time
    auto [par_result, par_time] = measure_time(fib_parallel, n, cutoff);
    std::cout << "\nParallel Result: " << par_result << std::endl;
    std::cout << "Parallel Time: " << std::fixed << std::setprecision(4) << par_time << " seconds" << std::endl;
    
    // Calculate and display speedup
    double speedup = seq_time / par_time;
    double efficiency = (speedup / num_threads) * 100.0;
    
    std::cout << "\nSpeedup: " << std::fixed << std::setprecision(2) << speedup << "x" << std::endl;
    std::cout << "Efficiency: " << std::fixed << std::setprecision(2) << efficiency << "%" << std::endl;
    
    // Verify results match
    if (seq_result == par_result) {
        std::cout << "\nResults match! ✓" << std::endl;
    } else {
        std::cout << "\nResults do not match! ✗" << std::endl;
        std::cout << "Sequential: " << seq_result << ", Parallel: " << par_result << std::endl;
    }
    
    // Analyze impact of different cutoff values
    if (n <= 40) {  // Only do this for reasonably sized inputs
        analyze_cutoff_impact(n, num_threads);
    }
    
    return 0;
}