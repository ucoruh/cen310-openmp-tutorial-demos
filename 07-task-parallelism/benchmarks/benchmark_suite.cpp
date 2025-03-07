#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <chrono>
#include <random>
#include <functional>
#include <map>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <numeric>
#include <omp.h>
#include "../include/task_utils.h"

// Structure to store benchmark results
struct BenchmarkResult {
    std::string algorithm;
    std::string implementation;
    int problem_size;
    int num_threads;
    int task_granularity;
    double execution_time;
    double speedup;
    double efficiency;
    
    // Constructor with all parameters
    BenchmarkResult(const std::string& alg, const std::string& impl, int size, int threads, 
                   int grain, double time, double up, double eff)
        : algorithm(alg), implementation(impl), problem_size(size), num_threads(threads),
          task_granularity(grain), execution_time(time), speedup(up), efficiency(eff) {}
};

// Class to manage benchmark results
class BenchmarkManager {
private:
    std::vector<BenchmarkResult> results;
    std::string results_dir;
    
public:
    BenchmarkManager(const std::string& dir = "benchmark_results") : results_dir(dir) {
        // Create results directory if it doesn't exist
        std::filesystem::create_directory(results_dir);
    }
    
    void add_result(const BenchmarkResult& result) {
        results.push_back(result);
    }
    
    void add_result(const std::string& algorithm, const std::string& implementation, 
                   int problem_size, int num_threads, int task_granularity,
                   double execution_time, double speedup, double efficiency) {
        results.emplace_back(algorithm, implementation, problem_size, num_threads,
                            task_granularity, execution_time, speedup, efficiency);
    }
    
    void print_results() const {
        std::cout << "\nBenchmark Results:" << std::endl;
        std::cout << "------------------------------------------------------------------------------------" << std::endl;
        std::cout << "Algorithm       | Implementation | Size    | Threads | Granularity | Time (s) | Speedup | Efficiency" << std::endl;
        std::cout << "------------------------------------------------------------------------------------" << std::endl;
        
        for (const auto& result : results) {
            std::cout << std::setw(15) << std::left << result.algorithm << " | "
                      << std::setw(14) << std::left << result.implementation << " | "
                      << std::setw(8) << std::right << result.problem_size << " | "
                      << std::setw(8) << std::right << result.num_threads << " | "
                      << std::setw(11) << std::right << result.task_granularity << " | "
                      << std::fixed << std::setprecision(4) << std::setw(8) << std::right << result.execution_time << " | "
                      << std::fixed << std::setprecision(2) << std::setw(7) << std::right << result.speedup << " | "
                      << std::fixed << std::setprecision(1) << std::setw(6) << std::right << result.efficiency << "%" << std::endl;
        }
        
        std::cout << "------------------------------------------------------------------------------------" << std::endl;
    }
    
    void save_to_csv(const std::string& filename) const {
        std::string filepath = results_dir + "/" + filename;
        std::ofstream file(filepath);
        
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filepath << " for writing." << std::endl;
            return;
        }
        
        // Write header
        file << "Algorithm,Implementation,ProblemSize,NumThreads,TaskGranularity,Time,Speedup,Efficiency\n";
        
        // Write data
        for (const auto& result : results) {
            file << result.algorithm << ","
                 << result.implementation << ","
                 << result.problem_size << ","
                 << result.num_threads << ","
                 << result.task_granularity << ","
                 << std::fixed << std::setprecision(6) << result.execution_time << ","
                 << std::fixed << std::setprecision(4) << result.speedup << ","
                 << std::fixed << std::setprecision(4) << result.efficiency << "\n";
        }
        
        file.close();
        std::cout << "Results saved to " << filepath << std::endl;
    }
    
    void visualize_speedup_by_algorithm() const {
        std::cout << "\nSpeedup by Algorithm:" << std::endl;
        std::cout << "-----------------------------------" << std::endl;
        
        // Group results by algorithm
        std::map<std::string, std::vector<double>> speedups_by_algorithm;
        
        for (const auto& result : results) {
            if (result.implementation == "TaskParallel") {
                speedups_by_algorithm[result.algorithm].push_back(result.speedup);
            }
        }
        
        // Calculate average speedup for each algorithm
        std::vector<double> avg_speedups;
        std::vector<std::string> algorithm_names;
        
        for (const auto& [algorithm, speedups] : speedups_by_algorithm) {
            if (!speedups.empty()) {
                double avg_speedup = 0.0;
                for (double s : speedups) {
                    avg_speedup += s;
                }
                avg_speedup /= speedups.size();
                
                avg_speedups.push_back(avg_speedup);
                algorithm_names.push_back(algorithm);
            }
        }
        
        // Sort algorithms by speedup (descending)
        std::vector<size_t> indices(algorithm_names.size());
        std::iota(indices.begin(), indices.end(), 0);
        std::sort(indices.begin(), indices.end(), [&avg_speedups](size_t a, size_t b) {
            return avg_speedups[a] > avg_speedups[b];
        });
        
        std::vector<double> sorted_speedups;
        std::vector<std::string> sorted_names;
        
        for (size_t idx : indices) {
            sorted_speedups.push_back(avg_speedups[idx]);
            sorted_names.push_back(algorithm_names[idx]);
        }
        
        task_utils::draw_ascii_bar_chart(sorted_speedups, sorted_names, 40, "Average Speedup by Algorithm");
    }
    
    void visualize_thread_scaling() const {
        std::cout << "\nThread Scaling Analysis:" << std::endl;
        std::cout << "-----------------------------------" << std::endl;
        
        // Group results by thread count
        std::map<int, std::vector<double>> speedups_by_threads;
        std::map<int, std::vector<double>> efficiency_by_threads;
        
        for (const auto& result : results) {
            if (result.implementation == "TaskParallel") {
                speedups_by_threads[result.num_threads].push_back(result.speedup);
                efficiency_by_threads[result.num_threads].push_back(result.efficiency);
            }
        }
        
        // Calculate averages
        std::vector<int> thread_counts;
        std::vector<double> avg_speedups;
        std::vector<double> avg_efficiency;
        
        for (const auto& [threads, speedups] : speedups_by_threads) {
            if (!speedups.empty()) {
                double avg_speedup = 0.0;
                for (double s : speedups) {
                    avg_speedup += s;
                }
                avg_speedup /= speedups.size();
                
                double avg_eff = 0.0;
                for (double e : efficiency_by_threads[threads]) {
                    avg_eff += e;
                }
                avg_eff /= efficiency_by_threads[threads].size();
                
                thread_counts.push_back(threads);
                avg_speedups.push_back(avg_speedup);
                avg_efficiency.push_back(avg_eff);
            }
        }
        
        // Sort by thread count (ascending)
        std::vector<size_t> indices(thread_counts.size());
        std::iota(indices.begin(), indices.end(), 0);
        std::sort(indices.begin(), indices.end(), [&thread_counts](size_t a, size_t b) {
            return thread_counts[a] < thread_counts[b];
        });
        
        std::vector<int> sorted_threads;
        std::vector<double> sorted_speedups;
        std::vector<double> sorted_efficiency;
        
        for (size_t idx : indices) {
            sorted_threads.push_back(thread_counts[idx]);
            sorted_speedups.push_back(avg_speedups[idx]);
            sorted_efficiency.push_back(avg_efficiency[idx]);
        }
        
        // Print table
        std::cout << "Threads | Avg Speedup | Avg Efficiency | Ideal Speedup" << std::endl;
        std::cout << "--------------------------------------------------------" << std::endl;
        
        for (size_t i = 0; i < sorted_threads.size(); ++i) {
            std::cout << std::setw(7) << sorted_threads[i] << " | "
                      << std::fixed << std::setprecision(2) << std::setw(11) << sorted_speedups[i] << " | "
                      << std::fixed << std::setprecision(1) << std::setw(13) << sorted_efficiency[i] << "% | "
                      << std::setw(12) << sorted_threads[i] << std::endl;
        }
        
        // Display visual comparison
        std::cout << "\nSpeedup vs Threads:" << std::endl;
        
        for (size_t i = 0; i < sorted_threads.size(); ++i) {
            std::string label = std::to_string(sorted_threads[i]) + " threads";
            std::cout << std::setw(12) << std::left << label << " |";
            
            // Draw actual speedup bar
            int actual_bar = static_cast<int>(sorted_speedups[i] * 2);
            for (int j = 0; j < actual_bar; ++j) {
                std::cout << "█";
            }
            
            // Print the value
            std::cout << " " << std::fixed << std::setprecision(2) << sorted_speedups[i] << "x";
            
            // Draw ideal speedup marker
            int ideal_pos = sorted_threads[i] * 2;
            if (ideal_pos > actual_bar) {
                std::cout << std::string(ideal_pos - actual_bar, ' ') << "▲";
            } else {
                std::cout << " ▲";
            }
            
            std::cout << std::endl;
        }
        
        std::cout << "            |" << std::string(50, '-') << std::endl;
        std::cout << "            " << std::string(50, ' ') << "Speedup" << std::endl;
        std::cout << "            ▲ = Ideal linear speedup" << std::endl;
    }
    
    void visualize_granularity_impact() const {
        std::cout << "\nTask Granularity Impact Analysis:" << std::endl;
        std::cout << "-----------------------------------" << std::endl;
        
        // Group results by task granularity
        std::map<int, std::vector<double>> speedups_by_granularity;
        
        for (const auto& result : results) {
            if (result.implementation == "TaskParallel") {
                speedups_by_granularity[result.task_granularity].push_back(result.speedup);
            }
        }
        
        // Calculate averages
        std::vector<int> granularities;
        std::vector<double> avg_speedups;
        
        for (const auto& [granularity, speedups] : speedups_by_granularity) {
            if (!speedups.empty()) {
                double avg_speedup = 0.0;
                for (double s : speedups) {
                    avg_speedup += s;
                }
                avg_speedup /= speedups.size();
                
                granularities.push_back(granularity);
                avg_speedups.push_back(avg_speedup);
            }
        }
        
        // Sort by granularity (ascending)
        std::vector<size_t> indices(granularities.size());
        std::iota(indices.begin(), indices.end(), 0);
        std::sort(indices.begin(), indices.end(), [&granularities](size_t a, size_t b) {
            return granularities[a] < granularities[b];
        });
        
        std::vector<int> sorted_granularities;
        std::vector<double> sorted_speedups;
        
        for (size_t idx : indices) {
            sorted_granularities.push_back(granularities[idx]);
            sorted_speedups.push_back(avg_speedups[idx]);
        }
        
        // Convert granularities to strings for the chart
        std::vector<std::string> granularity_labels;
        for (int g : sorted_granularities) {
            granularity_labels.push_back(std::to_string(g));
        }
        
        std::cout << "Granularity | Avg Speedup" << std::endl;
        std::cout << "--------------------------" << std::endl;
        
        for (size_t i = 0; i < sorted_granularities.size(); ++i) {
            std::cout << std::setw(11) << sorted_granularities[i] << " | "
                      << std::fixed << std::setprecision(2) << sorted_speedups[i] << "x" << std::endl;
        }
        
        task_utils::draw_ascii_bar_chart(sorted_speedups, granularity_labels, 40, "Average Speedup by Task Granularity");
    }
};

//==============================================================================
// Fibonacci Benchmark
//==============================================================================

// Sequential Fibonacci
long long fibonacci_sequential(int n) {
    if (n < 2) return n;
    return fibonacci_sequential(n - 1) + fibonacci_sequential(n - 2);
}

// Simple parallel Fibonacci without using tasks
long long fibonacci_parallel_simple(int n) {
    if (n < 2) return n;
    
    long long result1, result2;
    
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            result1 = fibonacci_sequential(n - 1);
        }
        
        #pragma omp section
        {
            result2 = fibonacci_sequential(n - 2);
        }
    }
    
    return result1 + result2;
}

// Task-based parallel Fibonacci
long long fibonacci_task(int n, int cutoff) {
    if (n < 2) return n;
    
    // Use sequential version for small values
    if (n <= cutoff) {
        return fibonacci_sequential(n);
    }
    
    long long x, y;
    
    #pragma omp task shared(x)
    {
        x = fibonacci_task(n - 1, cutoff);
    }
    
    #pragma omp task shared(y)
    {
        y = fibonacci_task(n - 2, cutoff);
    }
    
    #pragma omp taskwait
    
    return x + y;
}

// Wrapper for parallel task-based Fibonacci
long long fibonacci_parallel_task(int n, int cutoff) {
    long long result;
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            result = fibonacci_task(n, cutoff);
        }
    }
    
    return result;
}

void benchmark_fibonacci(BenchmarkManager& manager) {
    std::cout << "\nRunning Fibonacci Benchmark..." << std::endl;
    
    // Parameters
    std::vector<int> thread_counts = {1, 2, 4, 8, 16};
    std::vector<int> problem_sizes = {30, 35, 40};
    std::vector<int> cutoff_values = {10, 15, 20};
    
    // Adjust based on system capabilities
    int max_threads = omp_get_max_threads();
    thread_counts.erase(
        std::remove_if(thread_counts.begin(), thread_counts.end(), 
                      [max_threads](int t) { return t > max_threads; }),
        thread_counts.end());
    
    // Run benchmarks
    for (int n : problem_sizes) {
        std::cout << "  Problem size n = " << n << std::endl;
        
        // Measure sequential performance (baseline)
        double seq_time = task_utils::measure_time([n]() {
            fibonacci_sequential(n);
        });
        
        std::cout << "    Sequential time: " << std::fixed << std::setprecision(4) 
                  << seq_time << " seconds" << std::endl;
        
        // Add sequential result
        manager.add_result("Fibonacci", "Sequential", n, 1, 0, seq_time, 1.0, 100.0);
        
        // Measure simple parallel performance
        for (int threads : thread_counts) {
            omp_set_num_threads(threads);
            
            double simple_time = task_utils::measure_time([n]() {
                fibonacci_parallel_simple(n);
            });
            
            double speedup = seq_time / simple_time;
            double efficiency = (speedup / threads) * 100.0;
            
            std::cout << "    Simple Parallel (threads=" << threads << "): " 
                      << std::fixed << std::setprecision(4) << simple_time << " s, "
                      << "speedup=" << std::fixed << std::setprecision(2) << speedup << "x" << std::endl;
            
            manager.add_result("Fibonacci", "SimpleParallel", n, threads, 0, 
                              simple_time, speedup, efficiency);
        }
        
        // Measure task-based parallel performance
        for (int threads : thread_counts) {
            for (int cutoff : cutoff_values) {
                omp_set_num_threads(threads);
                
                double task_time = task_utils::measure_time([n, cutoff]() {
                    fibonacci_parallel_task(n, cutoff);
                });
                
                double speedup = seq_time / task_time;
                double efficiency = (speedup / threads) * 100.0;
                
                std::cout << "    Task Parallel (threads=" << threads 
                          << ", cutoff=" << cutoff << "): " 
                          << std::fixed << std::setprecision(4) << task_time << " s, "
                          << "speedup=" << std::fixed << std::setprecision(2) << speedup << "x" << std::endl;
                
                manager.add_result("Fibonacci", "TaskParallel", n, threads, cutoff, 
                                  task_time, speedup, efficiency);
            }
        }
    }
}

//==============================================================================
// Quicksort Benchmark
//==============================================================================

// Partition function for quicksort
int partition(std::vector<int>& arr, int low, int high) {
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
    if (low < high) {
        int pi = partition(arr, low, high);
        quicksort_sequential(arr, low, pi - 1);
        quicksort_sequential(arr, pi + 1, high);
    }
}

// Simple parallel quicksort using sections
void quicksort_parallel_simple(std::vector<int>& arr, int low, int high) {
    if (low < high) {
        if (high - low < 1000) {
            // Use sequential for small arrays
            quicksort_sequential(arr, low, high);
            return;
        }
        
        int pi = partition(arr, low, high);
        
        #pragma omp parallel sections
        {
            #pragma omp section
            {
                quicksort_parallel_simple(arr, low, pi - 1);
            }
            
            #pragma omp section
            {
                quicksort_parallel_simple(arr, pi + 1, high);
            }
        }
    }
}

// Task-based parallel quicksort
void quicksort_task(std::vector<int>& arr, int low, int high, int cutoff) {
    if (low < high) {
        // Use sequential for small arrays
        if (high - low < cutoff) {
            quicksort_sequential(arr, low, high);
            return;
        }
        
        int pi = partition(arr, low, high);
        
        #pragma omp task
        {
            quicksort_task(arr, low, pi - 1, cutoff);
        }
        
        #pragma omp task
        {
            quicksort_task(arr, pi + 1, high, cutoff);
        }
        
        #pragma omp taskwait
    }
}

// Wrapper for parallel task-based quicksort
void quicksort_parallel_task(std::vector<int>& arr, int cutoff) {
    #pragma omp parallel
    {
        #pragma omp single
        {
            quicksort_task(arr, 0, static_cast<int>(arr.size() - 1), cutoff);
        }
    }
}

void benchmark_quicksort(BenchmarkManager& manager) {
    std::cout << "\nRunning Quicksort Benchmark..." << std::endl;
    
    // Parameters
    std::vector<int> thread_counts = {1, 2, 4, 8, 16};
    std::vector<int> problem_sizes = {100000, 1000000, 10000000};
    std::vector<int> cutoff_values = {100, 1000, 10000};
    
    // Adjust based on system capabilities
    int max_threads = omp_get_max_threads();
    thread_counts.erase(
        std::remove_if(thread_counts.begin(), thread_counts.end(), 
                      [max_threads](int t) { return t > max_threads; }),
        thread_counts.end());
    
    std::random_device rd;
    std::mt19937 gen(rd());
    
    // Run benchmarks
    for (int size : problem_sizes) {
        std::cout << "  Problem size = " << size << std::endl;
        
        // Generate random data
        std::uniform_int_distribution<> dis(0, size);
        std::vector<int> original_data(size);
        for (int i = 0; i < size; ++i) {
            original_data[i] = dis(gen);
        }
        
        // Measure sequential performance (baseline)
        auto seq_data = original_data;
        double seq_time = task_utils::measure_time([&seq_data]() {
            quicksort_sequential(seq_data, 0, static_cast<int>(seq_data.size() - 1));
        });
        
        std::cout << "    Sequential time: " << std::fixed << std::setprecision(4) 
                  << seq_time << " seconds" << std::endl;
        
        // Add sequential result
        manager.add_result("Quicksort", "Sequential", size, 1, 0, seq_time, 1.0, 100.0);
        
        // Measure simple parallel performance
        for (int threads : thread_counts) {
            omp_set_num_threads(threads);
            
            auto simple_data = original_data;
            double simple_time = task_utils::measure_time([&simple_data]() {
                quicksort_parallel_simple(simple_data, 0, static_cast<int>(simple_data.size() - 1));
            });
            
            double speedup = seq_time / simple_time;
            double efficiency = (speedup / threads) * 100.0;
            
            std::cout << "    Simple Parallel (threads=" << threads << "): " 
                      << std::fixed << std::setprecision(4) << simple_time << " s, "
                      << "speedup=" << std::fixed << std::setprecision(2) << speedup << "x" << std::endl;
            
            // Verify result
            bool is_correct = std::is_sorted(simple_data.begin(), simple_data.end());
            if (!is_correct) {
                std::cerr << "      Error: Simple parallel quicksort result is incorrect!" << std::endl;
            }
            
            manager.add_result("Quicksort", "SimpleParallel", size, threads, 0, 
                              simple_time, speedup, efficiency);
        }
        
        // Measure task-based parallel performance
        for (int threads : thread_counts) {
            for (int cutoff : cutoff_values) {
                omp_set_num_threads(threads);
                
                auto task_data = original_data;
                double task_time = task_utils::measure_time([&task_data, cutoff]() {
                    quicksort_parallel_task(task_data, cutoff);
                });
                
                double speedup = seq_time / task_time;
                double efficiency = (speedup / threads) * 100.0;
                
                std::cout << "    Task Parallel (threads=" << threads 
                          << ", cutoff=" << cutoff << "): " 
                          << std::fixed << std::setprecision(4) << task_time << " s, "
                          << "speedup=" << std::fixed << std::setprecision(2) << speedup << "x" << std::endl;
                
                // Verify result
                bool is_correct = std::is_sorted(task_data.begin(), task_data.end());
                if (!is_correct) {
                    std::cerr << "      Error: Task parallel quicksort result is incorrect!" << std::endl;
                }
                
                manager.add_result("Quicksort", "TaskParallel", size, threads, cutoff, 
                                  task_time, speedup, efficiency);
            }
        }
    }
}

//==============================================================================
// Matrix Multiplication Benchmark
//==============================================================================

// Generate a random matrix
std::vector<double> generate_matrix(int size) {
    std::vector<double> matrix(size * size);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    
    for (int i = 0; i < size * size; ++i) {
        matrix[i] = dis(gen);
    }
    
    return matrix;
}

// Sequential matrix multiplication
void matrix_multiply_sequential(const std::vector<double>& A, const std::vector<double>& B, 
                               std::vector<double>& C, int size) {
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            double sum = 0.0;
            for (int k = 0; k < size; ++k) {
                sum += A[i * size + k] * B[k * size + j];
            }
            C[i * size + j] = sum;
        }
    }
}

// Simple parallel matrix multiplication
void matrix_multiply_parallel_simple(const std::vector<double>& A, const std::vector<double>& B, 
                                    std::vector<double>& C, int size) {
    #pragma omp parallel for
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            double sum = 0.0;
            for (int k = 0; k < size; ++k) {
                sum += A[i * size + k] * B[k * size + j];
            }
            C[i * size + j] = sum;
        }
    }
}

// Task-based parallel matrix multiplication
void matrix_multiply_task(const std::vector<double>& A, const std::vector<double>& B, 
                         std::vector<double>& C, int size, int block_size) {
    #pragma omp parallel
    {
        #pragma omp single
        {
            for (int i = 0; i < size; i += block_size) {
                for (int j = 0; j < size; j += block_size) {
                    #pragma omp task firstprivate(i, j)
                    {
                        // Process a block
                        for (int ii = i; ii < std::min(i + block_size, size); ++ii) {
                            for (int jj = j; jj < std::min(j + block_size, size); ++jj) {
                                double sum = 0.0;
                                for (int k = 0; k < size; ++k) {
                                    sum += A[ii * size + k] * B[k * size + jj];
                                }
                                C[ii * size + jj] = sum;
                            }
                        }
                    }
                }
            }
        }
    }
}

// Verify matrix multiplication result
bool verify_matrix_result(const std::vector<double>& expected, const std::vector<double>& actual, 
                         int size, double tolerance = 1e-10) {
    for (int i = 0; i < size * size; ++i) {
        if (std::abs(expected[i] - actual[i]) > tolerance) {
            return false;
        }
    }
    return true;
}

void benchmark_matrix_multiply(BenchmarkManager& manager) {
    std::cout << "\nRunning Matrix Multiplication Benchmark..." << std::endl;
    
    // Parameters
    std::vector<int> thread_counts = {1, 2, 4, 8, 16};
    std::vector<int> problem_sizes = {500, 1000, 2000};
    std::vector<int> block_sizes = {16, 32, 64, 128};
    
    // Adjust based on system capabilities
    int max_threads = omp_get_max_threads();
    thread_counts.erase(
        std::remove_if(thread_counts.begin(), thread_counts.end(), 
                      [max_threads](int t) { return t > max_threads; }),
        thread_counts.end());
    
    // Run benchmarks
    for (int size : problem_sizes) {
        std::cout << "  Matrix size = " << size << " x " << size << std::endl;
        
        // Generate matrices
        auto A = generate_matrix(size);
        auto B = generate_matrix(size);
        std::vector<double> C_seq(size * size, 0.0);
        
        // Measure sequential performance (baseline)
        double seq_time = task_utils::measure_time([&]() {
            matrix_multiply_sequential(A, B, C_seq, size);
        });
        
        std::cout << "    Sequential time: " << std::fixed << std::setprecision(4) 
                  << seq_time << " seconds" << std::endl;
        
        // Add sequential result
        manager.add_result("MatrixMultiply", "Sequential", size, 1, 0, seq_time, 1.0, 100.0);
        
        // Measure simple parallel performance
        for (int threads : thread_counts) {
            omp_set_num_threads(threads);
            
            std::vector<double> C_simple(size * size, 0.0);
            double simple_time = task_utils::measure_time([&]() {
                matrix_multiply_parallel_simple(A, B, C_simple, size);
            });
            
            double speedup = seq_time / simple_time;
            double efficiency = (speedup / threads) * 100.0;
            
            // Verify result
            bool is_correct = verify_matrix_result(C_seq, C_simple, size);
            
            std::cout << "    Simple Parallel (threads=" << threads << "): " 
                      << std::fixed << std::setprecision(4) << simple_time << " s, "
                      << "speedup=" << std::fixed << std::setprecision(2) << speedup << "x"
                      << (is_correct ? "" : " (INCORRECT)") << std::endl;
            
            manager.add_result("MatrixMultiply", "SimpleParallel", size, threads, 0, 
                              simple_time, speedup, efficiency);
        }
        
        // Measure task-based parallel performance
        for (int threads : thread_counts) {
            for (int block_size : block_sizes) {
                omp_set_num_threads(threads);
                
                std::vector<double> C_task(size * size, 0.0);
                double task_time = task_utils::measure_time([&]() {
                    matrix_multiply_task(A, B, C_task, size, block_size);
                });
                
                double speedup = seq_time / task_time;
                double efficiency = (speedup / threads) * 100.0;
                
                // Verify result
                bool is_correct = verify_matrix_result(C_seq, C_task, size);
                
                std::cout << "    Task Parallel (threads=" << threads 
                          << ", block_size=" << block_size << "): " 
                          << std::fixed << std::setprecision(4) << task_time << " s, "
                          << "speedup=" << std::fixed << std::setprecision(2) << speedup << "x"
                          << (is_correct ? "" : " (INCORRECT)") << std::endl;
                
                manager.add_result("MatrixMultiply", "TaskParallel", size, threads, block_size, 
                                  task_time, speedup, efficiency);
            }
        }
    }
}

//==============================================================================
// Main function
//==============================================================================
int main(int argc, char* argv[]) {
    // Process command-line arguments
    bool run_fibonacci = true;
    bool run_quicksort = true;
    bool run_matrix = true;
    
    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "fibonacci") {
            run_quicksort = false;
            run_matrix = false;
        } else if (arg == "quicksort") {
            run_fibonacci = false;
            run_matrix = false;
        } else if (arg == "matrix") {
            run_fibonacci = false;
            run_quicksort = false;
        }
    }
    
    std::cout << "=== OpenMP Task Parallelism Benchmark Suite ===" << std::endl;
    std::cout << "Detected " << omp_get_max_threads() << " available threads" << std::endl;
    
    // Initialize benchmark manager
    BenchmarkManager manager;
    
    // Run benchmarks
    if (run_fibonacci) {
        benchmark_fibonacci(manager);
    }
    
    if (run_quicksort) {
        benchmark_quicksort(manager);
    }
    
    if (run_matrix) {
        benchmark_matrix_multiply(manager);
    }
    
    // Print and save results
    manager.print_results();
    manager.save_to_csv("benchmark_results.csv");
    
    // Visualize results
    manager.visualize_speedup_by_algorithm();
    manager.visualize_thread_scaling();
    manager.visualize_granularity_impact();
    
    return 0;
}