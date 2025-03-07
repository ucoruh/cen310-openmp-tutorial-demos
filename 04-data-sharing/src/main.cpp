#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <chrono>
#include <omp.h>
#include <cmath>

// ------------------------------
// Utility Classes and Functions
// ------------------------------

// Timer class for performance measurements
class Timer {
private:
    std::chrono::high_resolution_clock::time_point start_time;
    std::chrono::high_resolution_clock::time_point end_time;
    bool running;

public:
    Timer() : running(false) {}

    void start() {
        start_time = std::chrono::high_resolution_clock::now();
        running = true;
    }

    void stop() {
        end_time = std::chrono::high_resolution_clock::now();
        running = false;
    }

    double elapsedMilliseconds() {
        auto end = running ? std::chrono::high_resolution_clock::now() : end_time;
        return std::chrono::duration<double, std::milli>(end - start_time).count();
    }
};

// Memory state visualizer
void visualizeMemory(const std::string& title, const std::vector<int>& values, int thread_count) {
    std::cout << "=== " << title << " ===\n";
    
    // Display thread private vs shared memory diagram
    std::cout << "Memory Layout:\n";
    std::cout << "+---------------------------+\n";
    std::cout << "| Shared Memory             |\n";
    std::cout << "| " << std::setw(25) << std::left << "Values: ";
    
    for (size_t i = 0; i < values.size() && i < 8; ++i) {
        std::cout << values[i] << " ";
    }
    if (values.size() > 8) std::cout << "...";
    
    std::cout << "|\n+---------------------------+\n";
    
    for (int t = 0; t < thread_count; ++t) {
        std::cout << "| Thread " << t << " Private Memory    |\n";
        std::cout << "+---------------------------+\n";
    }
    std::cout << "\n";
}

// ------------------------------
// Data Sharing Demo Functions
// ------------------------------

// Example 1: Shared Variables (demonstrates race condition)
void sharedVariablesDemo() {
    std::cout << "\n========================================\n";
    std::cout << "DEMO 1: SHARED VARIABLES (Race Condition)\n";
    std::cout << "========================================\n";

    const int iterations = 1000000;
    int counter = 0;
    
    // Before parallel region
    std::vector<int> values = {counter};
    visualizeMemory("Before Parallel Region", values, 4);
    
    Timer timer;
    timer.start();
    
    // By default, variables declared outside the parallel region are shared
    #pragma omp parallel num_threads(4)
    {
        #pragma omp for
        for (int i = 0; i < iterations; i++) {
            counter++; // Race condition: threads update the same variable
        }
    }
    
    timer.stop();
    
    // After parallel region
    values[0] = counter;
    visualizeMemory("After Parallel Region (With Race Condition)", values, 4);
    
    std::cout << "Counter value: " << counter << " (Expected: " << iterations << ")\n";
    std::cout << "Execution time: " << timer.elapsedMilliseconds() << " ms\n";
    std::cout << "Notice: Due to race condition, the final value is likely incorrect\n\n";
}

// Example 2: Shared Variables with Protection
void sharedVariablesProtectedDemo() {
    std::cout << "\n===============================================\n";
    std::cout << "DEMO 2: SHARED VARIABLES (Protected with atomic)\n";
    std::cout << "===============================================\n";

    const int iterations = 1000000;
    int counter = 0;
    
    // Before parallel region
    std::vector<int> values = {counter};
    visualizeMemory("Before Parallel Region", values, 4);
    
    Timer timer;
    timer.start();
    
    #pragma omp parallel num_threads(4)
    {
        #pragma omp for
        for (int i = 0; i < iterations; i++) {
            #pragma omp atomic
            counter++; // Protected update with atomic
        }
    }
    
    timer.stop();
    
    // After parallel region
    values[0] = counter;
    visualizeMemory("After Parallel Region (Protected with atomic)", values, 4);
    
    std::cout << "Counter value: " << counter << " (Expected: " << iterations << ")\n";
    std::cout << "Execution time: " << timer.elapsedMilliseconds() << " ms\n";
    std::cout << "Notice: With protection, the final value is correct but execution is slower\n\n";
}

// Example 3: Private Variables
void privateVariablesDemo() {
    std::cout << "\n========================================\n";
    std::cout << "DEMO 3: PRIVATE VARIABLES\n";
    std::cout << "========================================\n";

    const int num_threads = 4;
    int sum = 0;
    int thread_sums[num_threads] = {0};
    
    // Before parallel region
    std::vector<int> values = {sum};
    for (int i = 0; i < num_threads; i++) {
        values.push_back(thread_sums[i]);
    }
    visualizeMemory("Before Parallel Region", values, num_threads);
    
    #pragma omp parallel num_threads(num_threads)
    {
        int thread_id = omp_get_thread_num();
        int local_sum = 0; // Private variable - each thread has its own copy
        
        #pragma omp for
        for (int i = 1; i <= 100; i++) {
            local_sum += i; // Update thread's private copy
        }
        
        thread_sums[thread_id] = local_sum; // Store each thread's result
        
        #pragma omp atomic
        sum += local_sum; // Safely combine results
    }
    
    // After parallel region
    values[0] = sum;
    for (int i = 0; i < num_threads; i++) {
        values[i+1] = thread_sums[i];
    }
    visualizeMemory("After Parallel Region (With Private Variables)", values, num_threads);
    
    std::cout << "Total sum: " << sum << " (Expected: " << (100 * 101) / 2 << ")\n";
    std::cout << "Individual thread contributions:\n";
    for (int i = 0; i < num_threads; i++) {
        std::cout << "  Thread " << i << ": " << thread_sums[i] << "\n";
    }
    std::cout << "\n";
}

// Example 4: Firstprivate Clause
void firstprivateDemo() {
    std::cout << "\n========================================\n";
    std::cout << "DEMO 4: FIRSTPRIVATE VARIABLES\n";
    std::cout << "========================================\n";

    int x = 100; // Initial value
    const int num_threads = 4;
    std::vector<int> thread_values(num_threads);
    
    // Before parallel region
    std::vector<int> values = {x};
    visualizeMemory("Before Parallel Region", values, num_threads);
    
    #pragma omp parallel num_threads(num_threads) firstprivate(x)
    {
        int thread_id = omp_get_thread_num();
        
        // Each thread has its own copy of x, initialized to 100
        x += thread_id; // Modify the thread's private copy
        thread_values[thread_id] = x; // Store the modified value
    }
    
    // After parallel region
    std::cout << "Original x (unchanged): " << x << "\n";
    std::cout << "Thread-private copies of x after modification:\n";
    for (int i = 0; i < num_threads; i++) {
        std::cout << "  Thread " << i << ": " << thread_values[i] << " (100 + " << i << ")\n";
    }
    
    values = thread_values;
    visualizeMemory("After Parallel Region (With Firstprivate)", values, num_threads);
    std::cout << "Notice: Each thread gets its own initialized copy of the variable\n\n";
}

// Example 5: Lastprivate Clause
void lastprivateDemo() {
    std::cout << "\n========================================\n";
    std::cout << "DEMO 5: LASTPRIVATE VARIABLES\n";
    std::cout << "========================================\n";

    int last_value = -1;
    const int iterations = 100;
    
    // Before parallel region
    std::vector<int> values = {last_value};
    visualizeMemory("Before Parallel Region", values, 4);
    
    #pragma omp parallel num_threads(4)
    {
        #pragma omp for lastprivate(last_value)
        for (int i = 0; i < iterations; i++) {
            last_value = i; // The value from the "last" iteration (99) will be copied out
        }
    }
    
    // After parallel region
    values[0] = last_value;
    visualizeMemory("After Parallel Region (With Lastprivate)", values, 4);
    
    std::cout << "Final value of last_value: " << last_value << " (Expected: " << iterations - 1 << ")\n";
    std::cout << "Notice: The value from the last logical iteration is copied back to the original variable\n\n";
}

// Example 6: Threadprivate Variables
// Global variable for threadprivate demo
int thread_specific_counter;
#pragma omp threadprivate(thread_specific_counter)

void threadprivateDemo() {
    std::cout << "\n========================================\n";
    std::cout << "DEMO 6: THREADPRIVATE VARIABLES\n";
    std::cout << "========================================\n";

    const int num_threads = 4;
    std::vector<int> first_values(num_threads);
    std::vector<int> second_values(num_threads);
    
    // Initialize the threadprivate variable
    thread_specific_counter = 0;
    
    // First parallel region
    #pragma omp parallel num_threads(num_threads)
    {
        int thread_id = omp_get_thread_num();
        thread_specific_counter = thread_id + 100; // Set thread-specific value
        first_values[thread_id] = thread_specific_counter; // Store the value
    }
    
    std::cout << "Values after first parallel region:\n";
    for (int i = 0; i < num_threads; i++) {
        std::cout << "  Thread " << i << ": " << first_values[i] << "\n";
    }
    
    // Second parallel region - the threadprivate variable retains its value
    #pragma omp parallel num_threads(num_threads)
    {
        int thread_id = omp_get_thread_num();
        thread_specific_counter += 10; // Modify thread-specific value
        second_values[thread_id] = thread_specific_counter; // Store the new value
    }
    
    std::cout << "Values after second parallel region:\n";
    for (int i = 0; i < num_threads; i++) {
        std::cout << "  Thread " << i << ": " << second_values[i] << " (previous value + 10)\n";
    }
    
    std::vector<int> values;
    for (int i = 0; i < num_threads; i++) {
        values.push_back(second_values[i]);
    }
    visualizeMemory("After Second Parallel Region (Threadprivate)", values, num_threads);
    std::cout << "Notice: Each thread maintains its own persistent copy across parallel regions\n\n";
}

// Example 7: Complex Example - Matrix Multiplication with Data Sharing
void matrixMultiplicationDemo() {
    std::cout << "\n================================================\n";
    std::cout << "DEMO 7: MATRIX MULTIPLICATION WITH DATA SHARING\n";
    std::cout << "================================================\n";

    const int size = 600; // Matrix size
    
    // Allocate matrices
    std::vector<double> A(size * size, 1.0);
    std::vector<double> B(size * size, 2.0);
    std::vector<double> C(size * size, 0.0);
    
    std::cout << "Matrix size: " << size << "x" << size << "\n";
    std::cout << "Performing matrix multiplication C = A × B\n";
    
    Timer timer;
    timer.start();
    
    // Sequential version for comparison
    std::vector<double> C_sequential(size * size, 0.0);
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            double sum = 0.0;
            for (int k = 0; k < size; k++) {
                sum += A[i * size + k] * B[k * size + j];
            }
            C_sequential[i * size + j] = sum;
        }
    }
    
    double sequential_time = timer.elapsedMilliseconds();
    std::cout << "Sequential execution time: " << sequential_time << " ms\n";
    
    // Reset timer
    timer.start();
    
    // Parallel version with appropriate data sharing
    #pragma omp parallel
    {
        // A, B, C are shared by default (all threads access the same arrays)
        #pragma omp for
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                // sum is private to each thread (automatic variable in the loop)
                double sum = 0.0;
                for (int k = 0; k < size; k++) {
                    sum += A[i * size + k] * B[k * size + j];
                }
                C[i * size + j] = sum;
            }
        }
    }
    
    double parallel_time = timer.elapsedMilliseconds();
    
    // Verify results
    bool correct = true;
    for (int i = 0; i < size * size && correct; i++) {
        if (std::abs(C[i] - C_sequential[i]) > 1e-10) {
            correct = false;
        }
    }
    
    std::cout << "Parallel execution time: " << parallel_time << " ms\n";
    std::cout << "Speedup: " << sequential_time / parallel_time << "x\n";
    std::cout << "Result verification: " << (correct ? "PASSED" : "FAILED") << "\n";
    std::cout << "\nData sharing in this example:\n";
    std::cout << "- Matrices A, B, C: shared (all threads access same memory)\n";
    std::cout << "- Loop counters i, j, k: private to each thread\n";
    std::cout << "- Accumulator 'sum': private to each thread\n\n";
}

// ------------------------------
// Main Function
// ------------------------------

int main() {
    std::cout << "=========================================\n";
    std::cout << "   OpenMP Data Sharing Clauses Demo\n";
    std::cout << "=========================================\n\n";

    std::cout << "Number of available threads: " << omp_get_max_threads() << "\n\n";
    
    std::cout << "This demo illustrates OpenMP data sharing clauses and variable scoping.\n";
    std::cout << "Each example demonstrates different aspects of shared, private, firstprivate,\n"; 
    std::cout << "lastprivate, and threadprivate variables with memory visualizations.\n\n";
    
    // Run all demos
    sharedVariablesDemo();
    sharedVariablesProtectedDemo();
    privateVariablesDemo();
    firstprivateDemo();
    lastprivateDemo();
    threadprivateDemo();
    matrixMultiplicationDemo();
    
    std::cout << "=========================================\n";
    std::cout << "              Demo Complete\n";
    std::cout << "=========================================\n";
    
    return 0;
} 