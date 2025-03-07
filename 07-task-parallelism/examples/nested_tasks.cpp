#include <iostream>
#include <vector>
#include <string>
#include <omp.h>

// Simple recursive Fibonacci calculation
int fibonacci(int n) {
    if (n < 2) return n;
    
    int x, y;
    
    // Create a task for calculating fib(n-1)
    #pragma omp task shared(x)
    {
        x = fibonacci(n-1);
    }
    
    // Calculate fib(n-2) directly
    y = fibonacci(n-2);
    
    // Wait for the task to complete
    #pragma omp taskwait
    
    return x + y;
}

int main(int argc, char* argv[]) {
    int n = 20;
    int num_threads = 4;
    
    // Parse command line arguments
    if (argc > 1) n = std::stoi(argv[1]);
    if (argc > 2) num_threads = std::stoi(argv[2]);
    
    std::cout << "=== OpenMP Task Example: Fibonacci ===" << std::endl;
    std::cout << "Computing fibonacci(" << n << ") with " << num_threads << " threads" << std::endl;
    
    double start_time = omp_get_wtime();
    int result;
    
    #pragma omp parallel num_threads(num_threads)
    {
        #pragma omp single
        {
            result = fibonacci(n);
        }
    }
    
    double end_time = omp_get_wtime();
    
    std::cout << "fibonacci(" << n << ") = " << result << std::endl;
    std::cout << "Computation time: " << (end_time - start_time) << " seconds" << std::endl;
    
    return 0;
} 