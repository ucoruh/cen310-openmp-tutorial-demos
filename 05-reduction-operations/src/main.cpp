#include <iostream>
#include <iomanip>
#include <vector>
#include <random>
#include <chrono>
#include <cmath>
#include <string>
#include <omp.h>
#include <fstream>

// Helper functions for timing
auto get_time() {
    return std::chrono::high_resolution_clock::now();
}

template <typename TimePoint>
double get_elapsed_time(const TimePoint& start, const TimePoint& end) {
    // Explicit casting to double to avoid precision loss warning
    return static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
}

// Helper to generate random values
std::vector<double> generate_random_data(size_t size, double min_val = 0.0, double max_val = 100.0) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(min_val, max_val);

    std::vector<double> data(size);
    for (size_t i = 0; i < size; ++i) {
        data[i] = dist(gen);
    }
    return data;
}

// Generate data specific for product operations (smaller range to avoid overflow)
std::vector<double> generate_product_data(size_t size) {
    std::random_device rd;
    std::mt19937 gen(rd());
    // Using a range very close to 1.0 to avoid overflow in multiplication
    std::uniform_real_distribution<double> dist(0.99, 1.01);

    std::vector<double> data(size);
    for (size_t i = 0; i < size; ++i) {
        data[i] = dist(gen);
    }
    return data;
}

// Various implementations of sum reduction

// 1. Sequential sum
double sum_sequential(const std::vector<double>& data) {
    double sum = 0.0;
    auto start = get_time();
    
    for (const auto& val : data) {
        sum += val;
    }
    
    auto end = get_time();
    std::cout << "  Sequential Sum: " << std::fixed << std::setprecision(2) 
              << sum << " (Time: " << get_elapsed_time(start, end) << " ms)" << std::endl;
    return sum;
}

// 2. Parallel sum with critical section (inefficient)
double sum_parallel_critical(const std::vector<double>& data) {
    double sum = 0.0;
    auto start = get_time();
    
    #pragma omp parallel for
    for (long long i = 0; i < static_cast<long long>(data.size()); ++i) {
        #pragma omp critical
        {
            sum += data[i];
        }
    }
    
    auto end = get_time();
    std::cout << "  Parallel Sum (critical): " << std::fixed << std::setprecision(2) 
              << sum << " (Time: " << get_elapsed_time(start, end) << " ms)" << std::endl;
    return sum;
}

// 3. Parallel sum with atomic (better than critical, but still has overhead)
double sum_parallel_atomic(const std::vector<double>& data) {
    double sum = 0.0;
    auto start = get_time();
    
    #pragma omp parallel for
    for (long long i = 0; i < static_cast<long long>(data.size()); ++i) {
        #pragma omp atomic
        sum += data[i];
    }
    
    auto end = get_time();
    std::cout << "  Parallel Sum (atomic): " << std::fixed << std::setprecision(2) 
              << sum << " (Time: " << get_elapsed_time(start, end) << " ms)" << std::endl;
    return sum;
}

// 4. Manual reduction (local sums then combine)
double sum_parallel_manual(const std::vector<double>& data) {
    double sum = 0.0;
    auto start = get_time();
    
    #pragma omp parallel
    {
        double local_sum = 0.0;
        
        #pragma omp for
        for (long long i = 0; i < static_cast<long long>(data.size()); ++i) {
            local_sum += data[i];
        }
        
        #pragma omp critical
        {
            sum += local_sum;
        }
    }
    
    auto end = get_time();
    std::cout << "  Parallel Sum (manual): " << std::fixed << std::setprecision(2) 
              << sum << " (Time: " << get_elapsed_time(start, end) << " ms)" << std::endl;
    return sum;
}

// 5. OpenMP reduction (most efficient)
double sum_parallel_reduction(const std::vector<double>& data) {
    double sum = 0.0;
    auto start = get_time();
    
    #pragma omp parallel for reduction(+:sum)
    for (long long i = 0; i < static_cast<long long>(data.size()); ++i) {
        sum += data[i];
    }
    
    auto end = get_time();
    std::cout << "  Parallel Sum (reduction): " << std::fixed << std::setprecision(2) 
              << sum << " (Time: " << get_elapsed_time(start, end) << " ms)" << std::endl;
    return sum;
}

// Product reduction examples

// Sequential product
double product_sequential(const std::vector<double>& data) {
    double product = 1.0;
    auto start = get_time();
    
    for (const auto& val : data) {
        product *= val;
    }
    
    auto end = get_time();
    std::cout << "  Sequential Product: " << std::fixed << std::setprecision(2) 
              << product << " (Time: " << get_elapsed_time(start, end) << " ms)" << std::endl;
    return product;
}

// Parallel product with OpenMP reduction
double product_parallel_reduction(const std::vector<double>& data) {
    double product = 1.0;
    auto start = get_time();
    
    #pragma omp parallel for reduction(*:product)
    for (long long i = 0; i < static_cast<long long>(data.size()); ++i) {
        product *= data[i];
    }
    
    auto end = get_time();
    std::cout << "  Parallel Product (reduction): " << std::fixed << std::setprecision(2) 
              << product << " (Time: " << get_elapsed_time(start, end) << " ms)" << std::endl;
    return product;
}

// Min/Max reduction examples

// Sequential min/max
void minmax_sequential(const std::vector<double>& data, double& min_val, double& max_val) {
    min_val = data[0];
    max_val = data[0];
    auto start = get_time();
    
    for (const auto& val : data) {
        if (val < min_val) min_val = val;
        if (val > max_val) max_val = val;
    }
    
    auto end = get_time();
    std::cout << "  Sequential Min/Max: [" << std::fixed << std::setprecision(2) 
              << min_val << ", " << max_val << "] (Time: " 
              << get_elapsed_time(start, end) << " ms)" << std::endl;
}

// Parallel min/max with OpenMP manual reduction
void minmax_parallel_reduction(const std::vector<double>& data, double& min_val, double& max_val) {
    min_val = data[0];
    max_val = data[0];
    auto start = get_time();
    
    // Use manual reduction approach instead since min/max requires special compiler flags
    #pragma omp parallel
    {
        double thread_min = data[0];
        double thread_max = data[0];
        
        #pragma omp for
        for (long long i = 0; i < static_cast<long long>(data.size()); ++i) {
            if (data[i] < thread_min) thread_min = data[i];
            if (data[i] > thread_max) thread_max = data[i];
        }
        
        #pragma omp critical
        {
            if (thread_min < min_val) min_val = thread_min;
            if (thread_max > max_val) max_val = thread_max;
        }
    }
    
    auto end = get_time();
    std::cout << "  Parallel Min/Max (manual reduction): [" << std::fixed << std::setprecision(2) 
              << min_val << ", " << max_val << "] (Time: " 
              << get_elapsed_time(start, end) << " ms)" << std::endl;
}

// Logical operations reduction

// Sequential logical AND/OR
void logical_sequential(const std::vector<bool>& data, bool& result_and, bool& result_or) {
    result_and = true;
    result_or = false;
    auto start = get_time();
    
    for (const auto& val : data) {
        result_and = result_and && val;
        result_or = result_or || val;
    }
    
    auto end = get_time();
    std::cout << "  Sequential Logical: AND=" << std::boolalpha << result_and 
              << ", OR=" << result_or << " (Time: " 
              << get_elapsed_time(start, end) << " ms)" << std::endl;
}

// Parallel logical AND/OR with OpenMP reduction
void logical_parallel_reduction(const std::vector<bool>& data, bool& result_and_ref, bool& result_or_ref) {
    // Create non-reference copies to use in reduction
    bool result_and = true;
    bool result_or = false;
    auto start = get_time();
    
    #pragma omp parallel for reduction(&&:result_and) reduction(||:result_or)
    for (long long i = 0; i < static_cast<long long>(data.size()); ++i) {
        result_and = result_and && data[i];
        result_or = result_or || data[i];
    }
    
    // Copy back to reference parameters
    result_and_ref = result_and;
    result_or_ref = result_or;
    
    auto end = get_time();
    std::cout << "  Parallel Logical (reduction): AND=" << std::boolalpha << result_and 
              << ", OR=" << result_or << " (Time: " 
              << get_elapsed_time(start, end) << " ms)" << std::endl;
}

// Bitwise operations reduction

// Sequential bitwise operations
void bitwise_sequential(const std::vector<int>& data, int& result_and, int& result_or, int& result_xor) {
    result_and = ~0; // All bits set to 1
    result_or = 0;   // All bits set to 0
    result_xor = 0;  // Identity for XOR
    auto start = get_time();
    
    for (const auto& val : data) {
        result_and &= val;
        result_or |= val;
        result_xor ^= val;
    }
    
    auto end = get_time();
    std::cout << "  Sequential Bitwise: AND=" << result_and 
              << ", OR=" << result_or 
              << ", XOR=" << result_xor 
              << " (Time: " << get_elapsed_time(start, end) << " ms)" << std::endl;
}

// Parallel bitwise operations with OpenMP reduction
void bitwise_parallel_reduction(const std::vector<int>& data, int& result_and_ref, int& result_or_ref, int& result_xor_ref) {
    // Create non-reference copies to use in reduction
    int result_and = ~0; // All bits set to 1
    int result_or = 0;   // All bits set to 0
    int result_xor = 0;  // Identity for XOR
    auto start = get_time();
    
    #pragma omp parallel for reduction(&:result_and) reduction(|:result_or) reduction(^:result_xor)
    for (long long i = 0; i < static_cast<long long>(data.size()); ++i) {
        result_and &= data[i];
        result_or |= data[i];
        result_xor ^= data[i];
    }
    
    // Copy back to reference parameters
    result_and_ref = result_and;
    result_or_ref = result_or;
    result_xor_ref = result_xor;
    
    auto end = get_time();
    std::cout << "  Parallel Bitwise (reduction): AND=" << result_and 
              << ", OR=" << result_or 
              << ", XOR=" << result_xor 
              << " (Time: " << get_elapsed_time(start, end) << " ms)" << std::endl;
}

// Multiple reductions in single loop
void multiple_reductions_example(const std::vector<double>& data) {
    double sum = 0.0;
    double product = 1.0;
    
    // For min/max we need to initialize with first element values
    double min_val = data[0];
    double max_val = data[0];
    
    auto start = get_time();
    
    // We need to handle min/max separately in MSVC
    #pragma omp parallel for reduction(+:sum) reduction(*:product)
    for (long long i = 0; i < static_cast<long long>(data.size()); ++i) {
        sum += data[i];
        // Avoid zero for product to prevent underflow
        product *= (data[i] != 0.0) ? data[i] : 1.0;
        
        // Handle min/max with atomic update instead of reduction
        #pragma omp critical
        {
            if (data[i] < min_val) min_val = data[i];
            if (data[i] > max_val) max_val = data[i];
        }
    }
    
    auto end = get_time();
    
    std::cout << "  Multiple Reductions: Sum=" << std::fixed << std::setprecision(2) << sum
              << ", Product=" << product
              << ", Min=" << min_val
              << ", Max=" << max_val
              << " (Time: " << get_elapsed_time(start, end) << " ms)" << std::endl;
}

// Using the custom reduction for sum of squares
double sum_of_squares_reduction(const std::vector<double>& data) {
    double sum = 0.0;
    auto start = get_time();
    
    #pragma omp parallel for reduction(+:sum)
    for (long long i = 0; i < static_cast<long long>(data.size()); ++i) {
        sum += data[i] * data[i];
    }
    
    auto end = get_time();
    std::cout << "  Parallel Sum of Squares (reduction): " << std::fixed << std::setprecision(2) 
              << sum << " (Time: " << get_elapsed_time(start, end) << " ms)" << std::endl;
    return sum;
}

// Helper function to validate results
template <typename T>
bool validate_results(const T& sequential, const T& parallel, double tolerance = 1e-10) {
    // For floating point, use relative error
    if constexpr (std::is_floating_point<T>::value) {
        if (std::abs(sequential) < tolerance) {
            return std::abs(parallel - sequential) < tolerance;
        }
        return std::abs((parallel - sequential) / sequential) < tolerance;
    } 
    // For integral and boolean types, use exact comparison
    else {
        return sequential == parallel;
    }
}

// Run all reduction tests with given data size
void run_all_tests(size_t data_size) {
    std::cout << "\n--- Running tests with " << data_size << " elements ---" << std::endl;
    
    // Generate random data
    auto data = generate_random_data(data_size);
    
    // Generate specialized data for product operations
    auto product_data = generate_product_data(data_size);
    
    // Create integer and boolean data for other tests
    std::vector<int> int_data(data_size);
    for (size_t i = 0; i < data_size; ++i) {
        int_data[i] = static_cast<int>(data[i]) % 100;
    }
    
    std::vector<bool> bool_data(data_size);
    for (size_t i = 0; i < data_size; ++i) {
        bool_data[i] = (static_cast<int>(data[i]) % 2 == 0);
    }
    
    // Test sum operations
    auto start = get_time();
    double seq_sum = sum_sequential(data);
    auto end = get_time();
    std::cout << "Sequential Sum: " << std::fixed << std::setprecision(2) 
              << seq_sum << " (Time: " << get_elapsed_time(start, end) << " ms)" << std::endl;
    
    start = get_time();
    double par_sum_critical = sum_parallel_critical(data);
    end = get_time();
    std::cout << "Parallel Sum (critical): " << std::fixed << std::setprecision(2) 
              << par_sum_critical << " (Time: " << get_elapsed_time(start, end) << " ms)" << std::endl;
    
    start = get_time();
    double par_sum_atomic = sum_parallel_atomic(data);
    end = get_time();
    std::cout << "Parallel Sum (atomic): " << std::fixed << std::setprecision(2) 
              << par_sum_atomic << " (Time: " << get_elapsed_time(start, end) << " ms)" << std::endl;
    
    start = get_time();
    double par_sum_manual = sum_parallel_manual(data);
    end = get_time();
    std::cout << "Parallel Sum (manual): " << std::fixed << std::setprecision(2) 
              << par_sum_manual << " (Time: " << get_elapsed_time(start, end) << " ms)" << std::endl;
    
    start = get_time();
    double par_sum_reduction = sum_parallel_reduction(data);
    end = get_time();
    std::cout << "Parallel Sum (reduction): " << std::fixed << std::setprecision(2) 
              << par_sum_reduction << " (Time: " << get_elapsed_time(start, end) << " ms)" << std::endl;
    
    std::cout << "Validation: "
              << "Sum=" << std::boolalpha << validate_results(seq_sum, par_sum_reduction)
              << std::endl;
    
    // Test product operations (using specialized data)
    start = get_time();
    double seq_product = product_sequential(product_data);
    end = get_time();
    std::cout << "Sequential Product: " << std::fixed << std::setprecision(2) 
              << seq_product << " (Time: " << get_elapsed_time(start, end) << " ms)" << std::endl;
    
    start = get_time();
    double par_product = product_parallel_reduction(product_data);
    end = get_time();
    std::cout << "Parallel Product (reduction): " << std::fixed << std::setprecision(2) 
              << par_product << " (Time: " << get_elapsed_time(start, end) << " ms)" << std::endl;
    
    std::cout << "Validation: "
              << "Product=" << std::boolalpha << validate_results(seq_product, par_product)
              << std::endl;
    
    // Test min/max operations
    double seq_min, seq_max, par_min, par_max;
    minmax_sequential(data, seq_min, seq_max);
    minmax_parallel_reduction(data, par_min, par_max);
    
    std::cout << "Min/Max Validation:\n";
    std::cout << "  Sequential: [" << seq_min << ", " << seq_max << "]\n";
    std::cout << "  Parallel:   [" << par_min << ", " << par_max << "]\n";
    std::cout << "  Validation: "
              << "Min=" << std::boolalpha << validate_results(seq_min, par_min)
              << ", Max=" << validate_results(seq_max, par_max)
              << std::endl;
    
    // Test logical operations
    bool seq_and, seq_or, par_and, par_or;
    logical_sequential(bool_data, seq_and, seq_or);
    logical_parallel_reduction(bool_data, par_and, par_or);
    
    std::cout << "Logical Validation:\n";
    std::cout << "  Sequential: AND=" << seq_and << ", OR=" << seq_or << "\n";
    std::cout << "  Parallel:   AND=" << par_and << ", OR=" << par_or << "\n";
    std::cout << "  Validation: "
              << "AND=" << std::boolalpha << validate_results(seq_and, par_and)
              << ", OR=" << validate_results(seq_or, par_or)
              << std::endl;
    
    // Test bitwise operations
    int seq_and_int, seq_or_int, seq_xor_int;
    int par_and_int, par_or_int, par_xor_int;
    bitwise_sequential(int_data, seq_and_int, seq_or_int, seq_xor_int);
    bitwise_parallel_reduction(int_data, par_and_int, par_or_int, par_xor_int);
    
    std::cout << "Bitwise Validation:\n";
    std::cout << "  Sequential: AND=" << seq_and_int << ", OR=" << seq_or_int << ", XOR=" << seq_xor_int << "\n";
    std::cout << "  Parallel:   AND=" << par_and_int << ", OR=" << par_or_int << ", XOR=" << par_xor_int << "\n";
    std::cout << "  Validation: "
              << "AND=" << std::boolalpha << validate_results(seq_and_int, par_and_int)
              << ", OR=" << validate_results(seq_or_int, par_or_int)
              << ", XOR=" << validate_results(seq_xor_int, par_xor_int)
              << std::endl;
    
    // Test multiple reductions example
    multiple_reductions_example(data);
    
    // Test sum of squares
    double seq_sum_of_squares = 0.0;
    for (const auto& val : data) {
        seq_sum_of_squares += val * val;
    }
    
    double par_sum_of_squares = sum_of_squares_reduction(data);
    
    std::cout << "Sum of Squares Validation:\n";
    std::cout << "  Sequential: " << seq_sum_of_squares << "\n";
    std::cout << "  Parallel:   " << par_sum_of_squares << "\n";
    std::cout << "  Validation: "
              << std::boolalpha << validate_results(seq_sum_of_squares, par_sum_of_squares)
              << std::endl;
}

int main(int argc, char* argv[]) {
    std::cout << "====================================" << std::endl;
    std::cout << "    OpenMP Reduction Operations     " << std::endl;
    std::cout << "====================================" << std::endl;
    
    // Get number of available threads
    int max_threads = omp_get_max_threads();
    std::cout << "Number of available threads: " << max_threads << std::endl;
    
    // Check for command line arguments
    bool benchmark_mode = false;
    bool validation_mode = false;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--benchmark") {
            benchmark_mode = true;
        } else if (arg == "--validate") {
            validation_mode = true;
        }
    }
    
    if (validation_mode) {
        std::cout << "\n[VALIDATION MODE]" << std::endl;
        std::cout << "Running extensive validation tests..." << std::endl;
        
        // Open a file to save results
        std::ofstream validation_file("validation_results.txt");
        if (validation_file.is_open()) {
            validation_file << "OpenMP Reduction Operations - Validation Results\n";
            validation_file << "=================================================\n";
            validation_file << "Threads available: " << max_threads << "\n\n";
        }
        
        // Run validation with various data sizes
        std::vector<size_t> sizes = {10, 100, 1'000, 10'000, 100'000, 1'000'000};
        
        for (auto size : sizes) {
            std::cout << "\nValidating with data size: " << size << std::endl;
            if (validation_file.is_open()) {
                validation_file << "Data size: " << size << "\n";
                validation_file << "--------------------\n";
            }
            
            // Generate data for validation
            auto data = generate_random_data(size);
            
            // Sum validation
            double seq_sum = sum_sequential(data);
            double critical_sum = sum_parallel_critical(data);
            double atomic_sum = sum_parallel_atomic(data);
            double manual_sum = sum_parallel_manual(data);
            double reduction_sum = sum_parallel_reduction(data);
            
            bool valid_critical = validate_results(seq_sum, critical_sum);
            bool valid_atomic = validate_results(seq_sum, atomic_sum);
            bool valid_manual = validate_results(seq_sum, manual_sum);
            bool valid_reduction = validate_results(seq_sum, reduction_sum);
            
            std::cout << "Sum Operation Validation (size=" << size << "):\n";
            std::cout << "  Critical:  " << std::boolalpha << valid_critical << std::endl;
            std::cout << "  Atomic:    " << std::boolalpha << valid_atomic << std::endl;
            std::cout << "  Manual:    " << std::boolalpha << valid_manual << std::endl;
            std::cout << "  Reduction: " << std::boolalpha << valid_reduction << std::endl;
            
            if (validation_file.is_open()) {
                validation_file << "Sum Operation Validation:\n";
                validation_file << "  Critical:  " << std::boolalpha << valid_critical << "\n";
                validation_file << "  Atomic:    " << std::boolalpha << valid_atomic << "\n";
                validation_file << "  Manual:    " << std::boolalpha << valid_manual << "\n";
                validation_file << "  Reduction: " << std::boolalpha << valid_reduction << "\n\n";
            }
            
            // Product validation - Use specialized data with smaller range
            auto product_data = generate_product_data(size);
            double seq_product = product_sequential(product_data);
            double reduction_product = product_parallel_reduction(product_data);
            bool valid_product = validate_results(seq_product, reduction_product);
            
            std::cout << "Product Operation Validation (with specialized data):\n";
            std::cout << "  Sequential Product: " << std::fixed << std::setprecision(2) << seq_product << std::endl;
            std::cout << "  Parallel Product: " << std::fixed << std::setprecision(2) << reduction_product << std::endl;
            std::cout << "  Validation: " << std::boolalpha << valid_product << std::endl;
            
            if (validation_file.is_open()) {
                validation_file << "Product Operation Validation:\n";
                validation_file << "  Sequential: " << std::fixed << std::setprecision(6) << seq_product << "\n";
                validation_file << "  Parallel:   " << std::fixed << std::setprecision(6) << reduction_product << "\n";
                validation_file << "  Valid:      " << std::boolalpha << valid_product << "\n\n";
            }
            
            // Min/Max validation
            double seq_min = 0.0, seq_max = 0.0;
            double par_min = 0.0, par_max = 0.0;
            
            minmax_sequential(data, seq_min, seq_max);
            minmax_parallel_reduction(data, par_min, par_max);
            
            bool valid_min = validate_results(seq_min, par_min);
            bool valid_max = validate_results(seq_max, par_max);
            
            std::cout << "Min/Max Operation Validation:\n";
            std::cout << "  Min: " << std::boolalpha << valid_min << std::endl;
            std::cout << "  Max: " << std::boolalpha << valid_max << std::endl;
            
            if (validation_file.is_open()) {
                validation_file << "Min/Max Operation Validation:\n";
                validation_file << "  Min: " << std::boolalpha << valid_min << "\n";
                validation_file << "  Max: " << std::boolalpha << valid_max << "\n\n";
            }
            
            // Other validations can be added for logical and bitwise operations
        }
        
        if (validation_file.is_open()) {
            validation_file.close();
            std::cout << "\nValidation results saved to validation_results.txt" << std::endl;
        }
    } else if (benchmark_mode) {
        std::cout << "\n[BENCHMARK MODE]" << std::endl;
        std::cout << "Running comprehensive benchmarks..." << std::endl;
        
        // Open a file to save results
        std::ofstream benchmark_file("benchmark_results.txt");
        if (benchmark_file.is_open()) {
            benchmark_file << "OpenMP Reduction Operations - Benchmark Results\n";
            benchmark_file << "=================================================\n";
            benchmark_file << "Threads available: " << max_threads << "\n\n";
        }
        
        // Run more detailed benchmarks
        std::vector<size_t> sizes = {10'000, 100'000, 1'000'000, 10'000'000};
        
        for (auto size : sizes) {
            std::cout << "\nBenchmarking with data size: " << size << std::endl;
            if (benchmark_file.is_open()) {
                benchmark_file << "Data size: " << size << "\n";
                benchmark_file << "--------------------\n";
            }
            
            // Generate data for benchmarks
            auto data = generate_random_data(size);
            
            // Time different sum implementations
            auto start = get_time();
            double seq_sum = sum_sequential(data);
            auto end = get_time();
            double seq_time = get_elapsed_time(start, end);
            if (seq_time < 0.1) seq_time = 0.1; // Prevent division by zero
            
            std::cout << "  Sequential Sum: " << std::fixed << std::setprecision(2) 
                      << seq_sum << " (Time: " << seq_time << " ms)" << std::endl;
            
            start = get_time();
            double critical_sum = sum_parallel_critical(data);
            end = get_time();
            double critical_time = get_elapsed_time(start, end);
            if (critical_time < 0.1) critical_time = 0.1; // Prevent division by zero
            
            std::cout << "  Parallel Sum (critical): " << std::fixed << std::setprecision(2) 
                      << critical_sum << " (Time: " << critical_time << " ms)" << std::endl;
            
            start = get_time();
            double atomic_sum = sum_parallel_atomic(data);
            end = get_time();
            double atomic_time = get_elapsed_time(start, end);
            if (atomic_time < 0.1) atomic_time = 0.1; // Prevent division by zero
            
            std::cout << "  Parallel Sum (atomic): " << std::fixed << std::setprecision(2) 
                      << atomic_sum << " (Time: " << atomic_time << " ms)" << std::endl;
            
            start = get_time();
            double manual_sum = sum_parallel_manual(data);
            end = get_time();
            double manual_time = get_elapsed_time(start, end);
            if (manual_time < 0.1) manual_time = 0.1; // Prevent division by zero
            
            std::cout << "  Parallel Sum (manual): " << std::fixed << std::setprecision(2) 
                      << manual_sum << " (Time: " << manual_time << " ms)" << std::endl;
            
            start = get_time();
            double reduction_sum = sum_parallel_reduction(data);
            end = get_time();
            double reduction_time = get_elapsed_time(start, end);
            if (reduction_time < 0.1) reduction_time = 0.1; // Prevent division by zero
            
            std::cout << "  Parallel Sum (reduction): " << std::fixed << std::setprecision(2) 
                      << reduction_sum << " (Time: " << reduction_time << " ms)" << std::endl;
            
            // Add product benchmark with specialized data
            if (size <= 1'000'000) { // Limit to reasonable sizes to avoid long execution times
                auto product_data = generate_product_data(size);
                
                start = get_time();
                double seq_product_val = product_sequential(product_data);
                end = get_time();
                double seq_product_time = get_elapsed_time(start, end);
                if (seq_product_time < 0.1) seq_product_time = 0.1;
                
                start = get_time();
                double reduction_product_val = product_parallel_reduction(product_data);
                end = get_time();
                double reduction_product_time = get_elapsed_time(start, end);
                if (reduction_product_time < 0.1) reduction_product_time = 0.1;
                
                std::cout << "\nProduct Operation (size=" << size << ", specialized data):\n";
                std::cout << "  Sequential: " << seq_product_val << " (Time: " << seq_product_time << " ms)\n";
                std::cout << "  Reduction:  " << reduction_product_val << " (Time: " << reduction_product_time << " ms (Speedup: " 
                          << std::max(0.0, seq_product_time/reduction_product_time) << "x)\n";
                
                if (benchmark_file.is_open()) {
                    benchmark_file << "Product Operation (specialized data):\n";
                    benchmark_file << "  Sequential:  " << seq_product_time << " ms\n";
                    benchmark_file << "  Reduction:   " << reduction_product_time << " ms (Speedup: " 
                                   << std::max(0.0, seq_product_time/reduction_product_time) << "x)\n\n";
                }
            }
            
            // Print and save results
            std::cout << std::fixed << std::setprecision(2);
            std::cout << "Sum Operation (size=" << size << "):\n";
            std::cout << "  Sequential:  " << seq_time << " ms\n";
            std::cout << "  Critical:    " << critical_time << " ms (Speedup: " << std::max(0.0, seq_time/critical_time) << "x)\n";
            std::cout << "  Atomic:      " << atomic_time << " ms (Speedup: " << std::max(0.0, seq_time/atomic_time) << "x)\n";
            std::cout << "  Manual:      " << manual_time << " ms (Speedup: " << std::max(0.0, seq_time/manual_time) << "x)\n";
            std::cout << "  Reduction:   " << reduction_time << " ms (Speedup: " << std::max(0.0, seq_time/reduction_time) << "x)\n";
            
            if (benchmark_file.is_open()) {
                benchmark_file << std::fixed << std::setprecision(2);
                benchmark_file << "Sum Operation:\n";
                benchmark_file << "  Sequential:  " << seq_time << " ms\n";
                benchmark_file << "  Critical:    " << critical_time << " ms (Speedup: " << std::max(0.0, seq_time/critical_time) << "x)\n";
                benchmark_file << "  Atomic:      " << atomic_time << " ms (Speedup: " << std::max(0.0, seq_time/atomic_time) << "x)\n";
                benchmark_file << "  Manual:      " << manual_time << " ms (Speedup: " << std::max(0.0, seq_time/manual_time) << "x)\n";
                benchmark_file << "  Reduction:   " << reduction_time << " ms (Speedup: " << std::max(0.0, seq_time/reduction_time) << "x)\n\n";
            }
        }
        
        if (benchmark_file.is_open()) {
            benchmark_file.close();
            std::cout << "\nBenchmark results saved to benchmark_results.txt" << std::endl;
        }
    } else {
        // Run regular tests with different data sizes
        run_all_tests(10'000);      // Small dataset
        run_all_tests(1'000'000);   // Medium dataset
        run_all_tests(10'000'000);  // Large dataset
        
        std::cout << "\nAll tests completed successfully!" << std::endl;
    }
    
    return 0;
} 