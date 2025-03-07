#include <iostream>
#include <iomanip>
#include <vector>
#include <chrono>
#include <random>
#include <atomic>
#include <thread>
#include <mutex>
#include <string>
#include <functional>
#include <algorithm>
#include <omp.h>
#include "../include/task_utils.h"

// Class to track task execution statistics
// Class to track task execution statistics
class TaskStatistics {
private:
    std::atomic<int> total_tasks{0};
    std::atomic<int> active_tasks{0};
    std::atomic<int> max_concurrent_tasks{0};
    std::atomic<int> completed_tasks{0};
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
    std::vector<std::pair<int, double>> completion_times; // task_id, completion_time
    mutable std::mutex completion_mutex; // Now marked as mutable
    
public:
    TaskStatistics() {
        reset();
    }
    
    void reset() {
        total_tasks = 0;
        active_tasks = 0;
        max_concurrent_tasks = 0;
        completed_tasks = 0;
        start_time = std::chrono::high_resolution_clock::now();
        
        std::lock_guard<std::mutex> lock(completion_mutex);
        completion_times.clear();
    }
    
    void task_started([[maybe_unused]] int task_id) {
        int current_active = ++active_tasks;
        ++total_tasks;
        
        // Update max concurrent tasks if needed
        int current_max = max_concurrent_tasks.load();
        while (current_active > current_max) {
            max_concurrent_tasks.compare_exchange_weak(current_max, current_active);
            current_max = max_concurrent_tasks.load();
        }
    }
    
    void task_completed(int task_id) {
        --active_tasks;
        ++completed_tasks;
        
        auto now = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double>(now - start_time).count();
        
        std::lock_guard<std::mutex> lock(completion_mutex);
        completion_times.emplace_back(task_id, elapsed);
    }
    
    int get_active_tasks() const {
        return active_tasks.load();
    }
    
    int get_completed_tasks() const {
        return completed_tasks.load();
    }
    
    int get_max_concurrent_tasks() const {
        return max_concurrent_tasks.load();
    }
    
    double get_elapsed_time() const {
        auto now = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double>(now - start_time).count();
    }
    
    double get_throughput() const {
        double elapsed = get_elapsed_time();
        if (elapsed > 0) {
            return completed_tasks.load() / elapsed;
        }
        return 0.0;
    }
    
    // Get throughput over time intervals
    std::vector<std::pair<double, double>> get_throughput_over_time(int num_intervals) const {
        std::lock_guard<std::mutex> lock(completion_mutex);
        
        if (completion_times.empty()) {
            return {};
        }
        
        // Sort by completion time
        std::vector<std::pair<int, double>> sorted_times = completion_times;
        std::sort(sorted_times.begin(), sorted_times.end(), 
                 [](const auto& a, const auto& b) { return a.second < b.second; });
        
        double total_time = sorted_times.back().second;
        if (total_time <= 0) {
            return {};
        }
        
        // Calculate throughput for each interval
        std::vector<std::pair<double, double>> result;
        double interval_size = total_time / num_intervals;
        
        for (int i = 1; i <= num_intervals; ++i) {
            double interval_end = i * interval_size;
            double interval_start = (i - 1) * interval_size;
            
            int tasks_in_interval = 0;
            for (const auto& [_, time] : sorted_times) {
                if (time > interval_start && time <= interval_end) {
                    tasks_in_interval++;
                }
            }
            
            double throughput = tasks_in_interval / interval_size;
            result.emplace_back(interval_end, throughput);
        }
        
        return result;
    }
    
    void print_summary() const {
        std::cout << "\nTask Execution Statistics:" << std::endl;
        std::cout << "---------------------------------" << std::endl;
        std::cout << "Total tasks: " << total_tasks.load() << std::endl;
        std::cout << "Completed tasks: " << completed_tasks.load() << std::endl;
        std::cout << "Active tasks: " << active_tasks.load() << std::endl;
        std::cout << "Max concurrent tasks: " << max_concurrent_tasks.load() << std::endl;
        std::cout << "Total execution time: " << std::fixed << std::setprecision(3) 
                  << get_elapsed_time() << " seconds" << std::endl;
        std::cout << "Average throughput: " << std::fixed << std::setprecision(2) 
                  << get_throughput() << " tasks/second" << std::endl;
    }
    
    void visualize_throughput() const {
        std::cout << "\nThroughput Over Time:" << std::endl;
        std::cout << "---------------------------------" << std::endl;
        
        auto throughput_data = get_throughput_over_time(5);
        if (throughput_data.empty()) {
            std::cout << "No throughput data available." << std::endl;
            return;
        }
        
        std::cout << "Time (s) | Throughput (tasks/s)" << std::endl;
        std::cout << "---------------------------------" << std::endl;
        
        for (const auto& [time, throughput] : throughput_data) {
            std::cout << std::setw(8) << std::fixed << std::setprecision(2) << time
                      << " | " << std::setw(10) << std::fixed << std::setprecision(2) 
                      << throughput << std::endl;
        }
        
        // Visualize as a bar chart
        std::vector<double> values;
        std::vector<std::string> labels;
        
        for (const auto& [time, throughput] : throughput_data) {
            values.push_back(throughput);
            labels.push_back(std::to_string(static_cast<int>(time)) + "s");
        }
        
        task_utils::draw_ascii_bar_chart(values, labels, 40, "Throughput Over Time");
    }
};

// Function to simulate work with variable duration
void do_work(int task_id, int duration_ms, TaskStatistics& stats) {
    int thread_id = omp_get_thread_num();
    
    // Record task start
    stats.task_started(task_id);
    
    #pragma omp critical
    {
        std::cout << "Task " << std::setw(3) << task_id 
                  << " started on thread " << thread_id 
                  << " (duration: " << duration_ms << " ms, active: " 
                  << stats.get_active_tasks() << ")" << std::endl;
    }
    
    // Simulate work
    std::this_thread::sleep_for(std::chrono::milliseconds(duration_ms));
    
    // Record task completion
    stats.task_completed(task_id);
    
    #pragma omp critical
    {
        std::cout << "Task " << std::setw(3) << task_id 
                  << " completed on thread " << thread_id 
                  << " (active: " << stats.get_active_tasks() 
                  << ", completed: " << stats.get_completed_tasks() << ")" << std::endl;
    }
}

// Generate random task durations
std::vector<int> generate_task_durations(int num_tasks, int min_duration, int max_duration, int seed = 42) {
    std::vector<int> durations(num_tasks);
    std::mt19937 gen(seed);
    std::uniform_int_distribution<> dist(min_duration, max_duration);
    
    for (int i = 0; i < num_tasks; ++i) {
        durations[i] = dist(gen);
    }
    
    return durations;
}

//==============================================================================
// 1. No Throttling - Create all tasks at once
//==============================================================================
void run_without_throttling(const std::vector<int>& task_durations) {
    TaskStatistics stats;
    
    task_utils::print_section_header("Running WITHOUT Task Throttling");
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            for (size_t i = 0; i < task_durations.size(); ++i) {
                #pragma omp task
                {
                    do_work(static_cast<int>(i), task_durations[i], stats);
                }
            }
        }
    }
    
    stats.print_summary();
    stats.visualize_throughput();
}

//==============================================================================
// 2. Simple Atomic Counter Throttling
//==============================================================================
void run_with_atomic_throttling(const std::vector<int>& task_durations, int max_tasks) {
    TaskStatistics stats;
    std::atomic<int> active_tasks(0);
    
    task_utils::print_section_header("Running WITH Atomic Counter Throttling (max " + 
                                   std::to_string(max_tasks) + " active tasks)");
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            for (size_t i = 0; i < task_durations.size(); ++i) {
                // Wait until we have capacity for another task
                while (active_tasks.load() >= max_tasks) {
                    // Simple yield without taskyield (for OpenMP 2.0)
                    #pragma omp flush
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
                
                // Increment active tasks counter
                active_tasks++;
                
                #pragma omp task
                {
                    do_work(static_cast<int>(i), task_durations[i], stats);
                    
                    // Decrement active tasks counter
                    active_tasks--;
                }
            }
        }
    }
    
    stats.print_summary();
    stats.visualize_throughput();
}

//==============================================================================
// 3. Batched Execution with taskwait
//==============================================================================
void run_with_batched_execution(const std::vector<int>& task_durations, int batch_size) {
    TaskStatistics stats;
    
    task_utils::print_section_header("Running WITH Batched Execution (batch size " + 
                                   std::to_string(batch_size) + ")");
    std::cout << "Note: Using taskwait instead of taskgroup for OpenMP 2.0 compatibility" << std::endl;
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            for (size_t batch_start = 0; batch_start < task_durations.size(); batch_start += batch_size) {
                size_t batch_end = std::min(batch_start + batch_size, task_durations.size());
                
                #pragma omp critical
                {
                    std::cout << "Starting batch " 
                              << (batch_start / batch_size) + 1 
                              << " (tasks " << batch_start 
                              << " to " << batch_end - 1 << ")" << std::endl;
                }
                
                // Create tasks for this batch
                // Using taskwait instead of taskgroup for compatibility
                for (size_t i = batch_start; i < batch_end; ++i) {
                    #pragma omp task
                    {
                        do_work(static_cast<int>(i), task_durations[i], stats);
                    }
                }
                
                // Wait for all tasks in this batch to complete
                #pragma omp taskwait
                
                #pragma omp critical
                {
                    std::cout << "Completed batch " 
                              << (batch_start / batch_size) + 1 
                              << " (tasks " << batch_start 
                              << " to " << batch_end - 1 << ")" << std::endl;
                }
            }
        }
    }
    
    stats.print_summary();
    stats.visualize_throughput();
}

//==============================================================================
// 4. Task Throttling with Task Throttler Class
//==============================================================================
void run_with_throttler_class(const std::vector<int>& task_durations, int max_tasks) {
    TaskStatistics stats;
    task_utils::TaskThrottler throttler(max_tasks);
    
    task_utils::print_section_header("Running WITH TaskThrottler Class (max " + 
                                   std::to_string(max_tasks) + " active tasks)");
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            for (size_t i = 0; i < task_durations.size(); ++i) {
                // Wait until we have capacity for another task
                throttler.before_task();
                
                #pragma omp task
                {
                    do_work(static_cast<int>(i), task_durations[i], stats);
                    
                    // Signal task completion
                    throttler.after_task();
                }
            }
        }
    }
    
    stats.print_summary();
    stats.visualize_throughput();
}

//==============================================================================
// 5. Adaptive Task Throttling
//==============================================================================
void run_with_adaptive_throttling(const std::vector<int>& task_durations) {
    TaskStatistics stats;
    
    // Initial settings
    int num_threads = omp_get_max_threads();
    std::atomic<int> max_active_tasks(num_threads * 2); // Start with 2 tasks per thread
    std::atomic<int> active_tasks(0);
    
    task_utils::print_section_header("Running WITH Adaptive Throttling");
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            // Create a monitoring task that adjusts max_active_tasks based on throughput
            #pragma omp task
            {
                double prev_time = stats.get_elapsed_time();
                int prev_completed = 0;
                double target_time = 0.5; // Target measurement interval in seconds
                
                while (stats.get_completed_tasks() < static_cast<int>(task_durations.size())) {
                    // Wait for the target measurement interval
                    std::this_thread::sleep_for(std::chrono::milliseconds(
                        static_cast<int>(target_time * 1000)));
                    
                    // Get current stats
                    double current_time = stats.get_elapsed_time();
                    int current_completed = stats.get_completed_tasks();
                    int current_active = active_tasks.load();
                    int current_max = max_active_tasks.load();
                    
                    // Calculate throughput in this interval
                    double time_delta = current_time - prev_time;
                    int completed_delta = current_completed - prev_completed;
                    double throughput = (time_delta > 0) ? (completed_delta / time_delta) : 0;
                    
                    #pragma omp critical
                    {
                        std::cout << "Monitor: Time=" << std::fixed << std::setprecision(1) 
                                  << current_time << "s, Completed=" << current_completed
                                  << ", Throughput=" << std::fixed << std::setprecision(1) 
                                  << throughput << " tasks/s, Active=" << current_active 
                                  << ", MaxActive=" << current_max << std::endl;
                    }
                    
                    // Adjust max_active_tasks based on current utilization and throughput
                    if (current_active >= current_max * 0.8) {
                        // We're at 80%+ capacity, increase if throughput is good
                        max_active_tasks = current_max + num_threads;
                        
                        #pragma omp critical
                        {
                            std::cout << "Monitor: Increasing max active tasks to " 
                                      << max_active_tasks.load() << std::endl;
                        }
                    } else if (current_active <= current_max * 0.4 && current_max > num_threads) {
                        // We're using less than 40% capacity, decrease to save resources
                        max_active_tasks = std::max(num_threads, current_max - num_threads);
                        
                        #pragma omp critical
                        {
                            std::cout << "Monitor: Decreasing max active tasks to " 
                                      << max_active_tasks.load() << std::endl;
                        }
                    }
                    
                    // Update previous values for next iteration
                    prev_time = current_time;
                    prev_completed = current_completed;
                }
            }
            
            // Create the actual work tasks with adaptive throttling
            for (size_t i = 0; i < task_durations.size(); ++i) {
                // Wait until we have capacity for another task
                while (active_tasks.load() >= max_active_tasks.load()) {
                    // Simple yield without taskyield (for OpenMP 2.0)
                    #pragma omp flush
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
                
                // Increment active tasks counter
                active_tasks++;
                
                #pragma omp task
                {
                    do_work(static_cast<int>(i), task_durations[i], stats);
                    
                    // Decrement active tasks counter
                    active_tasks--;
                }
            }
        }
    }
    
    stats.print_summary();
    stats.visualize_throughput();
}

//==============================================================================
// Comparison of all throttling methods
//==============================================================================
void compare_throttling_methods(const std::vector<int>& task_durations, int num_threads) {
    struct ThrottlingMethod {
        std::string name;
        std::string parameter;
        std::function<void()> run;
        double execution_time;
    };
    
    std::vector<ThrottlingMethod> methods;
    
    // Setup comparisons
    int max_tasks = num_threads * 2;
    int batch_size = num_threads * 2;
    
    // Add methods to compare
    methods.push_back({
        "No Throttling",
        "N/A",
        [&task_durations]() { run_without_throttling(task_durations); },
        0.0
    });
    
    methods.push_back({
        "Atomic Counter",
        "max=" + std::to_string(max_tasks),
        [&task_durations, max_tasks]() { run_with_atomic_throttling(task_durations, max_tasks); },
        0.0
    });
    
    methods.push_back({
        "Batched Execution",
        "batch=" + std::to_string(batch_size),
        [&task_durations, batch_size]() { run_with_batched_execution(task_durations, batch_size); },
        0.0
    });
    
    methods.push_back({
        "TaskThrottler",
        "max=" + std::to_string(max_tasks),
        [&task_durations, max_tasks]() { run_with_throttler_class(task_durations, max_tasks); },
        0.0
    });
    
    methods.push_back({
        "Adaptive",
        "dynamic",
        [&task_durations]() { run_with_adaptive_throttling(task_durations); },
        0.0
    });
    
    // Run methods and measure execution time
    task_utils::print_section_header("Comparing Task Throttling Methods");
    
    for (auto& method : methods) {
        std::cout << "\nTesting method: " << method.name 
                  << " (" << method.parameter << ")" << std::endl;
        
        auto start = std::chrono::high_resolution_clock::now();
        method.run();
        auto end = std::chrono::high_resolution_clock::now();
        
        method.execution_time = std::chrono::duration<double>(end - start).count();
        
        std::cout << "Total execution time: " << std::fixed << std::setprecision(3) 
                  << method.execution_time << " seconds" << std::endl;
    }
    
    // Summarize results
    task_utils::print_section_header("Task Throttling Performance Summary");
    
    std::cout << "Method               | Parameter | Time (s)" << std::endl;
    std::cout << "---------------------------------------" << std::endl;
    
    std::vector<double> times;
    std::vector<std::string> labels;
    
    for (const auto& method : methods) {
        std::cout << std::setw(20) << std::left << method.name << " | "
                  << std::setw(9) << std::left << method.parameter << " | "
                  << std::fixed << std::setprecision(3) << method.execution_time << std::endl;
        
        times.push_back(method.execution_time);
        labels.push_back(method.name);
    }
    
    // Visualize comparison
    std::cout << std::endl;
    task_utils::draw_ascii_bar_chart(times, labels, 40, "Execution Time (lower is better)");
}

//==============================================================================
// Main function
//==============================================================================
int main(int argc, char* argv[]) {
    // Parse command-line arguments
    int num_tasks = 50;
    int min_duration = 50;
    int max_duration = 200;
    int num_threads = omp_get_max_threads();
    int example_mode = 0;  // 0=all, 1=no throttling, 2=atomic, 3=taskgroup, 4=throttler, 5=adaptive
    
    if (argc > 1) num_tasks = atoi(argv[1]);
    if (argc > 2) num_threads = atoi(argv[2]);
    if (argc > 3) example_mode = atoi(argv[3]);
    if (argc > 4) min_duration = atoi(argv[4]);
    if (argc > 5) max_duration = atoi(argv[5]);
    
    // Set thread count
    omp_set_num_threads(num_threads);
    
    std::cout << "=== OpenMP Task Throttling Example ===" << std::endl;
    std::cout << "Note: This version is compatible with OpenMP 2.0" << std::endl;
    std::cout << "Number of tasks: " << num_tasks << std::endl;
    std::cout << "Task duration range: " << min_duration << "-" << max_duration << " ms" << std::endl;
    std::cout << "Number of threads: " << num_threads << std::endl;
    
    // Generate task durations
    auto task_durations = generate_task_durations(num_tasks, min_duration, max_duration);
    
    // Run the selected example
    switch (example_mode) {
        case 1:  // No throttling
            run_without_throttling(task_durations);
            break;
        case 2:  // Atomic counter throttling
            run_with_atomic_throttling(task_durations, num_threads * 2);
            break;
        case 3:  // Batched execution (renamed from taskgroup-based)
            run_with_batched_execution(task_durations, num_threads * 2);
            break;
        case 4:  // TaskThrottler class
            run_with_throttler_class(task_durations, num_threads * 2);
            break;
        case 5:  // Adaptive throttling
            run_with_adaptive_throttling(task_durations);
            break;
        default:  // Comparison of all methods
            compare_throttling_methods(task_durations, num_threads);
            break;
    }
    
    return 0;
}