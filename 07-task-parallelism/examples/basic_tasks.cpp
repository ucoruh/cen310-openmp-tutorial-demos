#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <random>
#include <omp.h>
#include <string>       // This provides std::string

// Function to simulate work
void do_work(int work_amount) {
    volatile double result = 0.0;
    for (int i = 0; i < work_amount * 1000; ++i) {
        result += std::sin(static_cast<double>(i));
    }
}

// Sequential implementation for comparison
void process_data_sequential(std::vector<int>& data, int work_amount) {
    for (size_t i = 0; i < data.size(); ++i) {
        do_work(work_amount);
        data[i] *= 2;  // Simple operation on the data
    }
}

// Task-based parallel implementation
void process_data_tasks(std::vector<int>& data, int work_amount) {
    #pragma omp parallel
    {
        #pragma omp single
        {
            for (size_t i = 0; i < data.size(); ++i) {
                #pragma omp task
                {
                    do_work(work_amount);
                    data[i] *= 2;  // Simple operation on the data
                }
            }
        }
    }
}

// Simple parallel for implementation for comparison
void process_data_parallel_for(std::vector<int>& data, int work_amount) {
    #pragma omp parallel for
    for (size_t i = 0; i < data.size(); ++i) {
        do_work(work_amount);
        data[i] *= 2;  // Simple operation on the data
    }
}

int main(int argc, char* argv[]) {
    // Parse command-line arguments
    int num_elements = 100;
    int work_amount = 10;
    int num_threads = omp_get_max_threads();
    
    if (argc > 1) num_elements = std::stoi(argv[1]);
    if (argc > 2) work_amount = std::stoi(argv[2]);
    if (argc > 3) num_threads = std::stoi(argv[3]);
    
    omp_set_num_threads(num_threads);
    
    std::cout << "=== Basic OpenMP Tasks Example ===" << std::endl;
    std::cout << "Number of threads: " << num_threads << std::endl;
    std::cout << "Number of elements: " << num_elements << std::endl;
    std::cout << "Work amount per element: " << work_amount << std::endl;
    
    // Initialize data
    std::vector<int> data_seq(num_elements, 1);
    std::vector<int> data_task(num_elements, 1);
    std::vector<int> data_par_for(num_elements, 1);
    
    // Measure sequential execution time
    auto start = std::chrono::high_resolution_clock::now();
    process_data_sequential(data_seq, work_amount);
    auto end = std::chrono::high_resolution_clock::now();
    auto sequential_time = std::chrono::duration<double>(end - start).count();
    
    // Measure task-based execution time
    start = std::chrono::high_resolution_clock::now();
    process_data_tasks(data_task, work_amount);
    end = std::chrono::high_resolution_clock::now();
    auto task_time = std::chrono::duration<double>(end - start).count();
    
    // Measure parallel for execution time
    start = std::chrono::high_resolution_clock::now();
    process_data_parallel_for(data_par_for, work_amount);
    end = std::chrono::high_resolution_clock::now();
    auto par_for_time = std::chrono::duration<double>(end - start).count();
    
    // Verify results
    bool results_match = true;
    for (size_t i = 0; i < num_elements; ++i) {
        if (data_seq[i] != data_task[i] || data_seq[i] != data_par_for[i]) {
            results_match = false;
            break;
        }
    }
    
    // Display results
    std::cout << "\nPerformance Results:" << std::endl;
    std::cout << "---------------------------------" << std::endl;
    std::cout << "Sequential time: " << std::fixed << std::setprecision(4) << sequential_time << " seconds" << std::endl;
    std::cout << "Task-based time: " << std::fixed << std::setprecision(4) << task_time << " seconds" << std::endl;
    std::cout << "Parallel for time: " << std::fixed << std::setprecision(4) << par_for_time << " seconds" << std::endl;
    std::cout << "---------------------------------" << std::endl;
    std::cout << "Task-based speedup: " << std::fixed << std::setprecision(2) << sequential_time / task_time << "x" << std::endl;
    std::cout << "Parallel for speedup: " << std::fixed << std::setprecision(2) << sequential_time / par_for_time << "x" << std::endl;
    std::cout << "---------------------------------" << std::endl;
    std::cout << "Results match: " << (results_match ? "Yes" : "No") << std::endl;
    
    // Demonstrate task scheduling behavior
    std::cout << "\nTask Scheduling Behavior Demonstration:" << std::endl;
    std::cout << "---------------------------------" << std::endl;
    std::cout << "Creating tasks with different computation times..." << std::endl;
    
    const int num_varying_tasks = 10;
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            for (int i = 0; i < num_varying_tasks; ++i) {
                int work = (i + 1) * work_amount;
                
                #pragma omp task
                {
                    int thread_id = omp_get_thread_num();
                    auto start_time = std::chrono::high_resolution_clock::now();
                    
                    do_work(work);
                    
                    auto end_time = std::chrono::high_resolution_clock::now();
                    auto duration = std::chrono::duration<double>(end_time - start_time).count();
                    
                    #pragma omp critical
                    {
                        std::cout << "Task " << i << " (work=" << work << ") executed by thread " 
                                  << thread_id << " in " << std::fixed << std::setprecision(4) 
                                  << duration << " seconds" << std::endl;
                    }
                }
            }
        }
    }
    
    return 0;
}