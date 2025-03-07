#include <iostream>
#include <iomanip>
#include <vector>
#include <chrono>
#include <random>
#include <string>
#include <map>
#include <mutex>
#include <thread>
#include <algorithm>
#include <omp.h>

// Structure to represent a task with priority
struct PriorityTask {
    int id;
    std::string name;
    int duration_ms;
    int priority;
    
    PriorityTask(int i, const std::string& n, int dur, int pri)
        : id(i), name(n), duration_ms(dur), priority(pri) {}
};

// Simplified task execution without visualization
void execute_task(const PriorityTask& task) {
    int thread_id = omp_get_thread_num();
    
    // Simulate task execution with simple sleep
    std::this_thread::sleep_for(std::chrono::milliseconds(task.duration_ms));
    
    // Print task completion info
    #pragma omp critical
    {
        std::cout << "Task " << std::setw(3) << task.id 
                  << " ('" << task.name << "', pri=" << task.priority 
                  << ", dur=" << task.duration_ms << " ms) completed by thread " 
                  << thread_id << std::endl;
    }
}

// Create a set of tasks with different priorities - using fixed seed for reproducibility
std::vector<PriorityTask> create_priority_tasks(int count, int seed = 1234) {
    std::vector<PriorityTask> tasks;
    std::mt19937 gen(seed); // Sabit seed kullanımı
    std::uniform_int_distribution<> dur_dist(20, 50);  // Daha kısa süreler (20-50ms)
    
    // Create tasks with different priorities
    for (int i = 0; i < count; ++i) {
        std::string name = "Task_" + std::to_string(i);
        int duration = dur_dist(gen);
        
        // Assign priorities based on different strategies:
        int priority;
        
        if (i % 10 == 0) {
            // Critical tasks (high priority)
            priority = 100;
            name = "CRITICAL_" + std::to_string(i);
        } else if (i % 5 == 0) {
            // Important tasks (medium-high priority)
            priority = 75;
            name = "IMPORTANT_" + std::to_string(i);
        } else if (i % 3 == 0) {
            // Normal tasks (medium priority)
            priority = 50;
        } else {
            // Background tasks (low priority)
            priority = 0;
            name = "background_" + std::to_string(i);
        }
        
        tasks.emplace_back(i, name, duration, priority);
    }
    
    return tasks;
}

// Execute tasks without using priorities
void execute_without_priorities(const std::vector<PriorityTask>& tasks) {
    std::cout << "\nExecuting tasks WITHOUT priorities:" << std::endl;
    std::cout << "--------------------------------" << std::endl;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            for (const auto& task : tasks) {
                #pragma omp task
                {
                    execute_task(task);
                }
            }
        }
    }
    
    // Wait for all tasks to complete
    #pragma omp taskwait
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    
    std::cout << "\nAll tasks completed in " << duration << " ms." << std::endl;
}

// Execute tasks using priorities
void execute_with_priorities(const std::vector<PriorityTask>& tasks) {
    std::cout << "\nExecuting tasks WITH priorities:" << std::endl;
    std::cout << "--------------------------------" << std::endl;
    
    // Sort tasks by priority (highest priority first) for simulated priority
    auto sorted_tasks = tasks;
    std::sort(sorted_tasks.begin(), sorted_tasks.end(), 
              [](const PriorityTask& a, const PriorityTask& b) {
                  return a.priority > b.priority;
              });
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            for (const auto& task : sorted_tasks) {
                #pragma omp task
                {
                    execute_task(task);
                }
            }
        }
    }
    
    // Wait for all tasks to complete
    #pragma omp taskwait
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    
    std::cout << "\nAll tasks completed in " << duration << " ms." << std::endl;
}

// Simplified critical path optimization
void critical_path_optimization() {
    std::cout << "\nCritical Path Optimization Example:" << std::endl;
    std::cout << "--------------------------------" << std::endl;
    
    // Create a simple program execution graph - much smaller now
    const int NUM_TASKS = 5; // Daha az task
    
    // Define task properties: name, duration, priority - kısa süreler
    std::vector<std::tuple<std::string, int, int>> task_properties = {
        {"Init", 20, 0},               // 0
        {"LoadConfig", 25, 0},         // 1
        {"CRITICAL_Connect", 30, 100}, // 2: Critical task
        {"LoadResources", 30, 0},      // 3
        {"CRITICAL_Auth", 25, 100}     // 4: Critical task
    };
    
    std::cout << "Task list (name, duration, priority):" << std::endl;
    for (size_t i = 0; i < task_properties.size(); ++i) {
        const auto& [name, duration, priority] = task_properties[i];
        std::cout << i << ": " << std::setw(20) << name << ", " 
                  << std::setw(3) << duration << " ms, priority=" 
                  << priority << std::endl;
    }
    
    std::cout << "\nExecuting tasks with priorities to optimize critical path..." << std::endl;
    
    // Prepare tasks with their priorities
    std::vector<PriorityTask> all_tasks;
    for (int i = 0; i < NUM_TASKS && i < static_cast<int>(task_properties.size()); ++i) {
        const auto& [name, duration, priority] = task_properties[i];
        all_tasks.emplace_back(i, name, duration, priority);
    }
    
    // Sort by priority for best compatibility
    std::sort(all_tasks.begin(), all_tasks.end(),
              [](const PriorityTask& a, const PriorityTask& b) {
                  return a.priority > b.priority;
              });
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            for (const auto& task : all_tasks) {
                #pragma omp task
                {
                    execute_task(task);
                }
            }
        }
    }
    
    // Ensure all tasks are completed
    #pragma omp taskwait
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    
    std::cout << "\nAll critical path tasks completed in " << duration << " ms." << std::endl;
}

// Simple measure just executing a simple set of tasks
void execute_simple_tasks(int num_threads) {
    std::cout << "\nSimple Task Example for Stability Testing" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    omp_set_num_threads(num_threads);
    
    std::cout << "Running with " << num_threads << " threads..." << std::endl;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            for (int i = 0; i < 5; i++) {
                #pragma omp task
                {
                    int thread_id = omp_get_thread_num();
                    std::this_thread::sleep_for(std::chrono::milliseconds(20));
                    
                    #pragma omp critical
                    {
                        std::cout << "Simple task " << i << " completed by thread " 
                                  << thread_id << std::endl;
                    }
                }
            }
        }
    }
    
    // Wait for all tasks to complete
    #pragma omp taskwait
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    
    std::cout << "\nAll simple tasks completed in " << duration << " ms." << std::endl;
}

int main(int argc, char* argv[]) {
    // Parse command-line arguments
    int num_tasks = 5;  // Çok daha az task sayısı
    int num_threads = std::min(2, omp_get_max_threads());  // Sadece 2 thread
    int example_mode = 4;  // Sadece simple_tasks örneğini çalıştır
    
    if (argc > 1) num_tasks = std::min(std::stoi(argv[1]), 10);  // Maksimum 10 task
    if (argc > 2) num_threads = std::min(std::stoi(argv[2]), 4);  // Maksimum 4 thread
    if (argc > 3) example_mode = std::stoi(argv[3]);
    
    omp_set_num_threads(num_threads);
    
    std::cout << "=== OpenMP Task Priority Example ===" << std::endl;
    std::cout << "Number of tasks: " << num_tasks << std::endl;
    std::cout << "Number of threads: " << num_threads << std::endl;
    
    try {
        // Run the selected example
        switch (example_mode) {
            case 1:  // Basic comparison
                {
                    auto tasks = create_priority_tasks(std::min(num_tasks, 10));
                    execute_without_priorities(tasks);
                    execute_with_priorities(tasks);
                }
                break;
            case 2:  // Critical path optimization
                critical_path_optimization();
                break;
            case 3:  // Simple tasks
                execute_simple_tasks(num_threads);
                break;
            case 4:  // Ultra simple test
                std::cout << "\nRunning ultra simple test..." << std::endl;
                #pragma omp parallel num_threads(1)
                {
                    #pragma omp single
                    {
                        #pragma omp task
                        {
                            std::cout << "Task 1 executing on thread " << omp_get_thread_num() << std::endl;
                        }
                        
                        #pragma omp task
                        {
                            std::cout << "Task 2 executing on thread " << omp_get_thread_num() << std::endl;
                        }
                    }
                }
                break;
            default:  // All examples, starting with simplest
                {
                    execute_simple_tasks(1); // 1 thread için basit test
                    
                    std::cout << "\nContinuing with multi-threaded tests..." << std::endl;
                    execute_simple_tasks(num_threads);
                    
                    auto tasks = create_priority_tasks(std::min(num_tasks, 10));
                    execute_without_priorities(tasks);
                    execute_with_priorities(tasks);
                    
                    critical_path_optimization();
                }
                break;
        }
        
        std::cout << "\nAll examples completed successfully!" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "Unknown error occurred" << std::endl;
        return 1;
    }
    
    return 0;
}