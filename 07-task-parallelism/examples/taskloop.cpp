#include <iostream>
#include <iomanip>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <random>
#include <algorithm>
#include <numeric>
#include <omp.h>
#include <stdexcept>
#include <string>
#include <utility>

// Function to perform work (to measure real computation)
void do_work(int iterations) {
    volatile double result = 0.0;
    for (int i = 0; i < iterations; ++i) {
        result += std::sin(static_cast<double>(i));
    }
}

// Function to process a range of data sequentially
void process_range_sequential(std::vector<double>& data, int begin, int end) {
    for (int i = begin; i < end; ++i) {
        // Simulate work proportional to the value (creating irregular workload)
        int work = static_cast<int>(data[i]) + 1000;
        do_work(work);
        
        // Process the data item
        data[i] = std::sqrt(data[i]) * std::log(data[i] + 1.0);
    }
}

// Process all data sequentially
void process_sequential(std::vector<double>& data) {
    process_range_sequential(data, 0, static_cast<int>(data.size()));
}

// Process data using parallel for
void process_parallel_for(std::vector<double>& data) {
    #pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < static_cast<int>(data.size()); ++i) {
        // Simulate work proportional to the value (creating irregular workload)
        int work = static_cast<int>(data[i]) + 1000;
        do_work(work);
        
        // Process the data item
        data[i] = std::sqrt(data[i]) * std::log(data[i] + 1.0);
    }
}

// Process data using tasks manually with specified chunk size
void process_tasks(std::vector<double>& data, int chunk_size) {
    #pragma omp parallel
    {
        #pragma omp single
        {
            for (int i = 0; i < static_cast<int>(data.size()); i += chunk_size) {
                int end = std::min(i + chunk_size, static_cast<int>(data.size()));
                
                #pragma omp task
                {
                    process_range_sequential(data, i, end);
                }
            }
        }
    }
}

// Process data using tasks with grain size (taskloop alternative)
void process_tasks_with_grainsize(std::vector<double>& data, int grainsize) {
    #pragma omp parallel
    {
        #pragma omp single
        {
            for (int i = 0; i < static_cast<int>(data.size()); i += grainsize) {
                int end = std::min(i + grainsize, static_cast<int>(data.size()));
                
                #pragma omp task
                {
                    for (int j = i; j < end; ++j) {
                        // Simulate work proportional to the value (creating irregular workload)
                        int work = static_cast<int>(data[j]) + 1000;
                        do_work(work);
                        
                        // Process the data item
                        data[j] = std::sqrt(data[j]) * std::log(data[j] + 1.0);
                    }
                }
            }
        }
    }
}

// Process data using tasks with specified number of tasks (taskloop alternative)
void process_tasks_with_num_tasks(std::vector<double>& data, int num_tasks) {
    int data_size = static_cast<int>(data.size());
    int chunk_size = (data_size + num_tasks - 1) / num_tasks; // Ceiling division
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            for (int i = 0; i < data_size; i += chunk_size) {
                int end = std::min(i + chunk_size, data_size);
                
                #pragma omp task
                {
                    for (int j = i; j < end; ++j) {
                        // Simulate work proportional to the value (creating irregular workload)
                        int work = static_cast<int>(data[j]) + 1000;
                        do_work(work);
                        
                        // Process the data item
                        data[j] = std::sqrt(data[j]) * std::log(data[j] + 1.0);
                    }
                }
            }
        }
    }
}

// Generate random data for processing
std::vector<double> generate_data(int size) {
    std::vector<double> data(size);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(1.0, 1000.0);
    
    for (int i = 0; i < size; ++i) {
        data[i] = dis(gen);
    }
    
    return data;
}

// Verify results match between two vectors
bool verify_results(const std::vector<double>& expected, const std::vector<double>& actual, double tolerance = 1e-6) {
    if (expected.size() != actual.size()) return false;
    
    for (size_t i = 0; i < expected.size(); ++i) {
        if (std::fabs(expected[i] - actual[i]) > tolerance) {
            return false;
        }
    }
    
    return true;
}

// Measure execution time
template<typename Func, typename... Args>
double measure_time(Func func, Args... args) {
    auto start = std::chrono::high_resolution_clock::now();
    func(args...);
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double>(end - start).count();
}

// Test different grain sizes for tasks (replacing taskloop)
void test_grainsize_impact(int data_size) {
    std::cout << "\nTesting impact of grain size on task performance:" << std::endl;
    std::cout << "--------------------------------------------------" << std::endl;
    std::cout << "Grain Size | Time (s) | Speedup" << std::endl;
    std::cout << "--------------------------------------------------" << std::endl;
    
    // Generate a fresh copy of data for each test
    auto original_data = generate_data(data_size);
    
    // First measure sequential performance as baseline
    auto sequential_data = original_data;
    double seq_time = measure_time(process_sequential, std::ref(sequential_data));
    
    // Test different grain sizes
    std::vector<int> grain_sizes = {1, 10, 100, 500, 1000, data_size / 100, data_size / 10};
    
    for (int grain : grain_sizes) {
        if (grain <= 0) continue;  // Skip invalid grain sizes
        
        auto test_data = original_data;
        double time = measure_time(process_tasks_with_grainsize, std::ref(test_data), grain);
        
        // Verify results
        bool correct = verify_results(sequential_data, test_data);
        
        // Calculate speedup
        double speedup = seq_time / time;
        
        std::cout << std::setw(10) << grain << " | "
                  << std::fixed << std::setprecision(4) << std::setw(8) << time << " | "
                  << std::fixed << std::setprecision(2) << std::setw(6) << speedup
                  << (correct ? "" : " (INCORRECT)") << std::endl;
    }
}

// Test different num_tasks values for tasks (replacing taskloop)
void test_num_tasks_impact(int data_size) {
    std::cout << "\nTesting impact of num_tasks on task performance:" << std::endl;
    std::cout << "--------------------------------------------------" << std::endl;
    std::cout << "Num Tasks | Time (s) | Speedup" << std::endl;
    std::cout << "--------------------------------------------------" << std::endl;
    
    // Generate a fresh copy of data for each test
    auto original_data = generate_data(data_size);
    
    // First measure sequential performance as baseline
    auto sequential_data = original_data;
    double seq_time = measure_time(process_sequential, std::ref(sequential_data));
    
    // Test different numbers of tasks
    std::vector<int> task_counts = {1, 2, 4, 8, 16, 32, 64, 128};
    
    for (int num_tasks : task_counts) {
        auto test_data = original_data;
        double time = measure_time(process_tasks_with_num_tasks, std::ref(test_data), num_tasks);
        
        // Verify results
        bool correct = verify_results(sequential_data, test_data);
        
        // Calculate speedup
        double speedup = seq_time / time;
        
        std::cout << std::setw(9) << num_tasks << " | "
                  << std::fixed << std::setprecision(4) << std::setw(8) << time << " | "
                  << std::fixed << std::setprecision(2) << std::setw(6) << speedup
                  << (correct ? "" : " (INCORRECT)") << std::endl;
    }
}

// Compare different implementation approaches
void compare_implementations(int data_size, int grainsize, int num_tasks) {
    std::cout << "\nComparing different implementation approaches:" << std::endl;
    std::cout << "-------------------------------------------------------------" << std::endl;
    std::cout << "Implementation | Time (s) | Speedup | Efficiency" << std::endl;
    std::cout << "-------------------------------------------------------------" << std::endl;
    
    // Generate data for all tests
    auto original_data = generate_data(data_size);
    int num_threads = omp_get_max_threads();
    
    // Sequential execution (baseline)
    auto sequential_data = original_data;
    double seq_time = measure_time(process_sequential, std::ref(sequential_data));
    
    std::cout << std::setw(14) << "Sequential" << " | "
              << std::fixed << std::setprecision(4) << std::setw(8) << seq_time << " | "
              << std::setw(7) << "1.00" << " | "
              << std::setw(10) << "100.0%" << std::endl;
    
    // Parallel for with dynamic scheduling
    auto parallel_for_data = original_data;
    double par_for_time = measure_time(process_parallel_for, std::ref(parallel_for_data));
    bool par_for_correct = verify_results(sequential_data, parallel_for_data);
    double par_for_speedup = seq_time / par_for_time;
    double par_for_efficiency = (par_for_speedup / num_threads) * 100.0;
    
    std::cout << std::setw(14) << "Parallel For" << " | "
              << std::fixed << std::setprecision(4) << std::setw(8) << par_for_time << " | "
              << std::fixed << std::setprecision(2) << std::setw(7) << par_for_speedup << " | "
              << std::fixed << std::setprecision(1) << std::setw(9) << par_for_efficiency << "%"
              << (par_for_correct ? "" : " (INCORRECT)") << std::endl;
    
    // Manual tasks with chosen grain size
    auto tasks_data = original_data;
    double tasks_time = measure_time(process_tasks, std::ref(tasks_data), grainsize);
    bool tasks_correct = verify_results(sequential_data, tasks_data);
    double tasks_speedup = seq_time / tasks_time;
    double tasks_efficiency = (tasks_speedup / num_threads) * 100.0;
    
    std::cout << std::setw(14) << "Manual Tasks" << " | "
              << std::fixed << std::setprecision(4) << std::setw(8) << tasks_time << " | "
              << std::fixed << std::setprecision(2) << std::setw(7) << tasks_speedup << " | "
              << std::fixed << std::setprecision(1) << std::setw(9) << tasks_efficiency << "%"
              << (tasks_correct ? "" : " (INCORRECT)") << std::endl;
    
    // Tasks with grainsize (alternative to taskloop)
    auto tasks_grain_data = original_data;
    double tasks_grain_time = measure_time(process_tasks_with_grainsize, std::ref(tasks_grain_data), grainsize);
    bool tasks_grain_correct = verify_results(sequential_data, tasks_grain_data);
    double tasks_grain_speedup = seq_time / tasks_grain_time;
    double tasks_grain_efficiency = (tasks_grain_speedup / num_threads) * 100.0;
    
    std::cout << std::setw(14) << "Tasks Grain" << " | "
              << std::fixed << std::setprecision(4) << std::setw(8) << tasks_grain_time << " | "
              << std::fixed << std::setprecision(2) << std::setw(7) << tasks_grain_speedup << " | "
              << std::fixed << std::setprecision(1) << std::setw(9) << tasks_grain_efficiency << "%"
              << (tasks_grain_correct ? "" : " (INCORRECT)") << std::endl;
    
    // Tasks with num_tasks (alternative to taskloop)
    auto tasks_num_data = original_data;
    double tasks_num_time = measure_time(process_tasks_with_num_tasks, std::ref(tasks_num_data), num_tasks);
    bool tasks_num_correct = verify_results(sequential_data, tasks_num_data);
    double tasks_num_speedup = seq_time / tasks_num_time;
    double tasks_num_efficiency = (tasks_num_speedup / num_threads) * 100.0;
    
    std::cout << std::setw(14) << "Tasks Num" << " | "
              << std::fixed << std::setprecision(4) << std::setw(8) << tasks_num_time << " | "
              << std::fixed << std::setprecision(2) << std::setw(7) << tasks_num_speedup << " | "
              << std::fixed << std::setprecision(1) << std::setw(9) << tasks_num_efficiency << "%"
              << (tasks_num_correct ? "" : " (INCORRECT)") << std::endl;
}

int main(int argc, char* argv[]) {
    // Parse command-line arguments
    int data_size = 10000;
    int grainsize = 100;
    int num_tasks = 16;
    int num_threads = omp_get_max_threads();
    
    if (argc > 1) data_size = atoi(argv[1]);
    if (argc > 2) grainsize = atoi(argv[2]);
    if (argc > 3) num_tasks = atoi(argv[3]);
    if (argc > 4) num_threads = atoi(argv[4]);
    
    omp_set_num_threads(num_threads);
    
    std::cout << "=== OpenMP Task Parallelism Example ===" << std::endl;
    std::cout << "Note: This is a simplified version using basic tasks instead of taskloop (OpenMP 4.5+)" << std::endl;
    std::cout << "Data size: " << data_size << std::endl;
    std::cout << "Default grainsize: " << grainsize << std::endl;
    std::cout << "Default num_tasks: " << num_tasks << std::endl;
    std::cout << "Number of threads: " << num_threads << std::endl;
    
    // Demonstrate task parallelism basics with irregular workload
    compare_implementations(data_size, grainsize, num_tasks);
    
    // Test impact of grain size on performance
    test_grainsize_impact(data_size);
    
    // Test impact of num_tasks on performance
    test_num_tasks_impact(data_size);
    
    return 0;
}