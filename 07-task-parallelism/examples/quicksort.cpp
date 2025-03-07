#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>
#include <iomanip>
#include <omp.h>
#include <stdexcept>
#include <string>
#include <utility>

// Partition function for quicksort
int partition(std::vector<int>& arr, int low, int high) {
    // Boundary check
    if (low < 0 || high < 0 || low >= static_cast<int>(arr.size()) || high >= static_cast<int>(arr.size()) || low > high) {
        return low; // Güvenli bir değer döndür
    }
    
    int pivot = arr[high];
    int i = low - 1;
    
    for (int j = low; j < high; j++) {
        if (arr[j] <= pivot) {
            i++;
            std::swap(arr[i], arr[j]);
        }
    }
    
    std::swap(arr[i + 1], arr[high]);
    return i + 1;
}

// Sequential quicksort
void quicksort_sequential(std::vector<int>& arr, int low, int high) {
    // Boundary check
    if (low < 0 || high < 0 || low >= static_cast<int>(arr.size()) || high >= static_cast<int>(arr.size())) {
        return;
    }
    
    if (low < high) {
        int pi = partition(arr, low, high);
        // Extra check to ensure partition returned a valid value
        if (pi > low && pi < high) {
            quicksort_sequential(arr, low, pi - 1);
            quicksort_sequential(arr, pi + 1, high);
        }
    }
}

// Task-based parallel quicksort
void quicksort_task(std::vector<int>& arr, int low, int high, int cutoff) {
    // Boundary check
    if (low < 0 || high < 0 || low >= static_cast<int>(arr.size()) || high >= static_cast<int>(arr.size())) {
        return;
    }
    
    if (low < high) {
        // Use sequential version for small arrays to reduce overhead
        if (high - low < cutoff) {
            quicksort_sequential(arr, low, high);
            return;
        }
        
        int pi = partition(arr, low, high);
        // Extra check to ensure partition returned a valid value
        if (pi <= low || pi >= high) {
            // Fallback to sequential for problematic partitions
            quicksort_sequential(arr, low, high);
            return;
        }
        
        // If subproblem is still large enough, create a task
        if (pi - low > cutoff) {
            #pragma omp task
            {
                quicksort_task(arr, low, pi - 1, cutoff);
            }
        } else {
            // Use sequential for smaller subproblems
            quicksort_sequential(arr, low, pi - 1);
        }
        
        // If subproblem is still large enough, create a task
        if (high - pi > cutoff) {
            #pragma omp task
            {
                quicksort_task(arr, pi + 1, high, cutoff);
            }
            // Only wait if we created tasks
            #pragma omp taskwait
        } else {
            // Use sequential for smaller subproblems
            quicksort_sequential(arr, pi + 1, high);
        }
    }
}

// Wrapper for parallel quicksort
void quicksort_parallel(std::vector<int>& arr, int cutoff) {
    #pragma omp parallel
    {
        #pragma omp single
        {
            quicksort_task(arr, 0, static_cast<int>(arr.size()) - 1, cutoff);
        }
    }
}

// Function to check if a vector is sorted
bool is_sorted(const std::vector<int>& arr) {
    return std::is_sorted(arr.begin(), arr.end());
}

// Function to generate a random vector
std::vector<int> generate_random_vector(int size) {
    std::vector<int> vec(size);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, size * 10);
    
    for (int i = 0; i < size; ++i) {
        vec[i] = dis(gen);
    }
    
    return vec;
}

// Measure execution time
template<typename Func, typename... Args>
double measure_time(Func func, Args... args) {
    auto start = std::chrono::high_resolution_clock::now();
    func(args...);
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double>(end - start).count();
}

// Function to analyze cutoff impact
void analyze_cutoff_impact(int size, [[maybe_unused]] int num_threads) {
    std::cout << "\nAnalyzing cutoff impact on array size " << size << ":" << std::endl;
    std::cout << "--------------------------------------------------" << std::endl;
    std::cout << "Cutoff | Time (s) | Speedup" << std::endl;
    std::cout << "--------------------------------------------------" << std::endl;
    
    // Generate a large random array
    auto original = generate_random_vector(size);
    
    // First measure sequential time as baseline
    auto sequential = original;
    double seq_time = measure_time([&sequential]() {
        quicksort_sequential(sequential, 0, static_cast<int>(sequential.size()) - 1);
    });
    
    // Try different cutoff values
    std::vector<int> cutoffs = {10, 50, 100, 500, 1000, 5000, 10000};
    
    for (int cutoff : cutoffs) {
        auto parallel = original;
        double par_time = measure_time([&parallel, cutoff]() {
            quicksort_parallel(parallel, cutoff);
        });
        
        // Calculate speedup
        double speedup = seq_time / par_time;
        
        std::cout << std::setw(6) << cutoff << " | "
                  << std::fixed << std::setprecision(4) << std::setw(8) << par_time << " | "
                  << std::fixed << std::setprecision(2) << std::setw(6) << speedup << "x"
                  << std::endl;
    }
}

int main(int argc, char* argv[]) {
    try {
        // Parse command-line arguments
        int size = 1000000; // Default to a smaller size
        int cutoff = 1000;
        int num_threads = omp_get_max_threads();
        
        if (argc > 1) size = std::stoi(argv[1]);
        if (argc > 2) cutoff = std::stoi(argv[2]);
        if (argc > 3) num_threads = std::stoi(argv[3]);
        
        // Sanitize inputs - daha güvenli limitler
        const int MAX_SIZE = 100000; // Reduced max size to avoid memory issues
        if (size > MAX_SIZE) {
            std::cout << "Warning: Array size limited to " << MAX_SIZE << std::endl;
            size = MAX_SIZE;
        }
        if (size < 1) size = 1000; // Minimum size
        if (cutoff < 10) cutoff = 10; // Minimum cutoff
        if (cutoff > size / 5) cutoff = size / 5; // More sensible maximum cutoff
        if (num_threads < 1) num_threads = 1;
        if (num_threads > 32) num_threads = 32; // Limit threads to avoid overwhelm
        
        omp_set_num_threads(num_threads);
        
        std::cout << "=== Quicksort Task Parallelism Example ===" << std::endl;
        std::cout << "Array size: " << size << std::endl;
        std::cout << "Number of threads: " << num_threads << std::endl;
        std::cout << "Cutoff for sequential execution: " << cutoff << std::endl;
        
        // Generate random data
        std::cout << "\nGenerating random data..." << std::endl;
        auto original_data = generate_random_vector(size);
        
        // Create copies for sequential and parallel sorting
        auto sequential_data = original_data;
        auto parallel_data = original_data;
        
        // Measure sequential time
        std::cout << "Running sequential quicksort..." << std::endl;
        double sequential_time = measure_time([&sequential_data]() {
            quicksort_sequential(sequential_data, 0, static_cast<int>(sequential_data.size()) - 1);
        });
        
        // Measure parallel time
        std::cout << "Running parallel quicksort..." << std::endl;
        double parallel_time;
        
        try {
            parallel_time = measure_time([&parallel_data, cutoff]() {
                quicksort_parallel(parallel_data, cutoff);
            });
            
            // Verify results
            bool sequential_sorted = is_sorted(sequential_data);
            bool parallel_sorted = is_sorted(parallel_data);
            bool results_match = std::equal(sequential_data.begin(), sequential_data.end(), parallel_data.begin());
            
            // Display results
            std::cout << "\nPerformance Results:" << std::endl;
            std::cout << "---------------------------------" << std::endl;
            std::cout << "Sequential time: " << std::fixed << std::setprecision(4) << sequential_time << " seconds" << std::endl;
            std::cout << "Parallel time: " << std::fixed << std::setprecision(4) << parallel_time << " seconds" << std::endl;
            std::cout << "---------------------------------" << std::endl;
            
            double speedup = sequential_time / parallel_time;
            double efficiency = (speedup / num_threads) * 100.0;
            
            std::cout << "Speedup: " << std::fixed << std::setprecision(2) << speedup << "x" << std::endl;
            std::cout << "Efficiency: " << std::fixed << std::setprecision(2) << efficiency << "%" << std::endl;
            std::cout << "---------------------------------" << std::endl;
            
            std::cout << "Sequential result sorted: " << (sequential_sorted ? "Yes" : "No") << std::endl;
            std::cout << "Parallel result sorted: " << (parallel_sorted ? "Yes" : "No") << std::endl;
            std::cout << "Results match: " << (results_match ? "Yes" : "No") << std::endl;
            
            // Skip cutoff analysis to improve stability
            std::cout << "\nSkipping cutoff analysis for stability" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "\nError during parallel quicksort: " << e.what() << std::endl;
            return 1;
        }
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}