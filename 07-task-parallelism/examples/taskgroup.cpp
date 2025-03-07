#include <iostream>
#include <iomanip>
#include <vector>
#include <chrono>
#include <random>
#include <string>
#include <omp.h>
#include "../include/task_utils.h"

// Structure to represent a work item
struct WorkItem {
    int id;
    int duration_ms;
    
    WorkItem(int i, int dur) : id(i), duration_ms(dur) {}
};

// Generate a collection of work items with random durations
std::vector<WorkItem> generate_work_items(int count, int min_duration = 20, int max_duration = 200) {
    std::vector<WorkItem> items;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(min_duration, max_duration);
    
    for (int i = 0; i < count; ++i) {
        items.emplace_back(i, dist(gen));
    }
    
    return items;
}

// Process work items using separate taskwaits for each group
void process_with_taskwaits(const std::vector<WorkItem>& items) {
    int total_groups = 4;
    int items_per_group = static_cast<int>(items.size()) / total_groups;
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            std::cout << "Processing with separate taskwaits:" << std::endl;
            
            for (int group = 0; group < total_groups; ++group) {
                std::cout << "  Starting group " << group + 1 << " of " << total_groups << std::endl;
                
                int start = group * items_per_group;
                int end = (group == total_groups - 1) ? static_cast<int>(items.size()) : (group + 1) * items_per_group;
                
                for (int i = start; i < end; ++i) {
                    #pragma omp task
                    {
                        int tid = omp_get_thread_num();
                        std::this_thread::sleep_for(std::chrono::milliseconds(items[i].duration_ms));
                        
                        #pragma omp critical
                        {
                            std::cout << "    Task " << std::setw(3) << items[i].id 
                                      << " (duration: " << std::setw(3) << items[i].duration_ms 
                                      << " ms) completed by thread " << tid << std::endl;
                        }
                    }
                }
                
                // Wait for all tasks in this group to complete before starting the next group
                #pragma omp taskwait
                std::cout << "  Group " << group + 1 << " completed" << std::endl;
            }
        }
    }
}

// Process work items using a simulation of taskgroup (using taskwait)
void process_with_simulated_taskgroup(const std::vector<WorkItem>& items) {
    int total_groups = 4;
    int items_per_group = static_cast<int>(items.size()) / total_groups;
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            std::cout << "Processing with simulated taskgroup (using taskwait):" << std::endl;
            
            for (int group = 0; group < total_groups; ++group) {
                std::cout << "  Starting group " << group + 1 << " of " << total_groups << std::endl;
                
                // Begin simulated taskgroup
                int start = group * items_per_group;
                int end = (group == total_groups - 1) ? static_cast<int>(items.size()) : (group + 1) * items_per_group;
                
                for (int i = start; i < end; ++i) {
                    #pragma omp task
                    {
                        int tid = omp_get_thread_num();
                        std::this_thread::sleep_for(std::chrono::milliseconds(items[i].duration_ms));
                        
                        #pragma omp critical
                        {
                            std::cout << "    Task " << std::setw(3) << items[i].id 
                                      << " (duration: " << std::setw(3) << items[i].duration_ms 
                                      << " ms) completed by thread " << tid << std::endl;
                        }
                    }
                }
                
                // Wait for all tasks in this group (simulating taskgroup end)
                #pragma omp taskwait
                
                std::cout << "  Group " << group + 1 << " completed" << std::endl;
            }
        }
    }
}

// Demonstrate nested task synchronization (using taskwait instead of taskgroup)
void nested_task_synchronization(int depth = 3, int width = 3) {
    #pragma omp parallel
    {
        #pragma omp single
        {
            std::cout << "Demonstrating nested task synchronization (using taskwait):" << std::endl;
            
            std::function<void(int, int, const std::string&)> process_level = 
                [&process_level](int current_depth, int width, const std::string& prefix) {
                    if (current_depth <= 0) return;
                    
                    // Create tasks for this level
                    for (int i = 0; i < width; ++i) {
                        std::string new_prefix = prefix + "." + std::to_string(i+1);
                        
                        #pragma omp task
                        {
                            int tid = omp_get_thread_num();
                            int duration = 50 + (current_depth * 20);
                            
                            #pragma omp critical
                            {
                                std::cout << "  Starting task " << new_prefix 
                                          << " (depth: " << current_depth << ")"
                                          << " on thread " << tid << std::endl;
                            }
                            
                            std::this_thread::sleep_for(std::chrono::milliseconds(duration));
                            
                            // Recursively create nested tasks
                            if (current_depth > 1) {
                                process_level(current_depth - 1, width, new_prefix);
                            }
                            
                            #pragma omp critical
                            {
                                std::cout << "  Completed task " << new_prefix 
                                          << " (depth: " << current_depth << ")"
                                          << " on thread " << tid << std::endl;
                            }
                        }
                    }
                    
                    // Wait for all tasks at this level to complete (simulating taskgroup)
                    #pragma omp taskwait
                };
            
            process_level(depth, width, "T");
        }
    }
}

// Demonstrate error handling with task synchronization
void task_error_handling() {
    std::cout << "Demonstrating task error handling:" << std::endl;
    std::cout << "Note: Full exception handling with taskgroup requires OpenMP 4.0+" << std::endl;
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            try {
                // Create several tasks
                for (int i = 0; i < 10; ++i) {
                    #pragma omp task
                    {
                        int tid = omp_get_thread_num();
                        
                        #pragma omp critical
                        {
                            std::cout << "  Starting task " << i << " on thread " << tid << std::endl;
                        }
                        
                        std::this_thread::sleep_for(std::chrono::milliseconds(50));
                        
                        // Simulate an error in one of the tasks
                        if (i == 5) {
                            #pragma omp critical
                            {
                                std::cout << "  Task " << i << " is throwing an exception!" << std::endl;
                            }
                            // In OpenMP 2.0, exceptions across threads are not well supported
                            // so we'll just report the error instead of throwing
                            #pragma omp critical
                            {
                                std::cout << "  ERROR: Simulated error in task " << i << std::endl;
                            }
                        }
                        
                        #pragma omp critical
                        {
                            std::cout << "  Completed task " << i << " on thread " << tid << std::endl;
                        }
                    }
                }
                
                // Wait for all tasks to complete
                #pragma omp taskwait
            }
            catch (const std::exception& e) {
                std::cout << "Caught exception: " << e.what() << std::endl;
                std::cout << "Note: Other tasks may still be executing or may have been canceled." << std::endl;
            }
        }
    }
}

// Compare performance of different synchronization approaches
void compare_performance(int num_items, [[maybe_unused]] int num_threads) {
    std::cout << "\nComparing performance of different synchronization approaches..." << std::endl;
    
    // Generate work items
    auto items = generate_work_items(num_items, 10, 50);
    
    // 1. Using separate taskwaits
    double taskwait_time = task_utils::measure_time([&items]() {
        process_with_taskwaits(items);
    });
    
    std::cout << std::endl;
    
    // 2. Using simulated taskgroup
    double simulated_taskgroup_time = task_utils::measure_time([&items]() {
        process_with_simulated_taskgroup(items);
    });
    
    // Print performance comparison
    std::cout << "\nPerformance Results:" << std::endl;
    std::cout << "---------------------------------" << std::endl;
    std::cout << "Taskwait approach: " << std::fixed << std::setprecision(4) << taskwait_time << " seconds" << std::endl;
    std::cout << "Simulated taskgroup approach: " << std::fixed << std::setprecision(4) << simulated_taskgroup_time << " seconds" << std::endl;
    std::cout << "---------------------------------" << std::endl;
    
    double speedup = taskwait_time / simulated_taskgroup_time;
    std::cout << "Speedup with simulated taskgroup: " << std::fixed << std::setprecision(2) << speedup << "x" << std::endl;
    
    // Visualize with a bar chart
    std::vector<double> times = {taskwait_time, simulated_taskgroup_time};
    std::vector<std::string> labels = {"Taskwait", "Simulated Taskgroup"};
    task_utils::draw_ascii_bar_chart(times, labels, 40, "Execution Time Comparison (lower is better)");
}

int main(int argc, char* argv[]) {
    // Parse command-line arguments
    int num_items = 20;
    int num_threads = omp_get_max_threads();
    int example_mode = 0;  // 0=all, 1=basic, 2=nested, 3=error, 4=performance
    
    if (argc > 1) num_items = atoi(argv[1]);
    if (argc > 2) num_threads = atoi(argv[2]);
    if (argc > 3) example_mode = atoi(argv[3]);
    
    omp_set_num_threads(num_threads);
    
    std::cout << "=== OpenMP Task Synchronization Example ===" << std::endl;
    std::cout << "Note: This is a simplified version compatible with OpenMP 2.0" << std::endl;
    std::cout << "      using taskwait instead of taskgroup (OpenMP 4.0+)" << std::endl;
    std::cout << "Number of work items: " << num_items << std::endl;
    std::cout << "Number of threads: " << num_threads << std::endl;
    
    // Run the selected example
    switch (example_mode) {
        case 1:  // Basic comparison
            {
                auto items = generate_work_items(num_items);
                process_with_taskwaits(items);
                std::cout << std::endl;
                process_with_simulated_taskgroup(items);
            }
            break;
        case 2:  // Nested tasks
            nested_task_synchronization();
            break;
        case 3:  // Error handling
            task_error_handling();
            break;
        case 4:  // Performance comparison
            compare_performance(num_items, num_threads);
            break;
        default:  // All examples
            {
                auto items = generate_work_items(num_items);
                process_with_taskwaits(items);
                std::cout << std::endl;
                process_with_simulated_taskgroup(items);
                std::cout << std::endl;
                nested_task_synchronization();
                std::cout << std::endl;
                task_error_handling();
                std::cout << std::endl;
                compare_performance(num_items, num_threads);
            }
            break;
    }
    
    return 0;
}