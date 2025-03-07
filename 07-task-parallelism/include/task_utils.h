#ifndef TASK_UTILS_H
#define TASK_UTILS_H

#include <iostream>
#include <iomanip>
#include <chrono>
#include <functional>
#include <vector>
#include <random>
#include <algorithm>
#include <cmath>
#include <string>
#include <map>
#include <mutex>
#include <atomic>
#include <thread>
#include <omp.h>

namespace task_utils {

//==============================================================================
// Timing utilities
//==============================================================================

// Measure execution time of a function
template<typename Func, typename... Args>
double measure_time(Func func, Args&&... args) {
    auto start = std::chrono::high_resolution_clock::now();
    func(std::forward<Args>(args)...);
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double>(end - start).count();
}

// Measure execution time with multiple runs and return statistics
template<typename Func, typename... Args>
std::pair<double, double> measure_time_stats(Func func, Args&&... args, int num_runs = 5) {
    std::vector<double> times;
    times.reserve(num_runs);
    
    for (int i = 0; i < num_runs; ++i) {
        double time = measure_time(func, std::forward<Args>(args)...);
        times.push_back(time);
    }
    
    // Calculate mean
    double sum = 0.0;
    for (double t : times) {
        sum += t;
    }
    double mean = sum / num_runs;
    
    // Calculate standard deviation
    double sq_sum = 0.0;
    for (double t : times) {
        sq_sum += (t - mean) * (t - mean);
    }
    double std_dev = std::sqrt(sq_sum / num_runs);
    
    return {mean, std_dev};
}

// Class for timing task executions
class TaskTimer {
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
    bool running;
    double elapsed_seconds;
    
public:
    TaskTimer() : running(false), elapsed_seconds(0.0) {}
    
    void start() {
        start_time = std::chrono::high_resolution_clock::now();
        running = true;
    }
    
    double stop() {
        if (!running) return elapsed_seconds;
        
        auto end_time = std::chrono::high_resolution_clock::now();
        elapsed_seconds = std::chrono::duration<double>(end_time - start_time).count();
        running = false;
        return elapsed_seconds;
    }
    
    void reset() {
        elapsed_seconds = 0.0;
        running = false;
    }
    
    double elapsed() const {
        if (running) {
            auto current_time = std::chrono::high_resolution_clock::now();
            return std::chrono::duration<double>(current_time - start_time).count();
        }
        return elapsed_seconds;
    }
};

// Display performance metrics
inline void print_performance_metrics(double seq_time, double par_time, int num_threads) {
    double speedup = seq_time / par_time;
    double efficiency = (speedup / num_threads) * 100.0;
    
    std::cout << "\nPerformance Metrics:" << std::endl;
    std::cout << "---------------------------------" << std::endl;
    std::cout << "Sequential time: " << std::fixed << std::setprecision(4) << seq_time << " seconds" << std::endl;
    std::cout << "Parallel time: " << std::fixed << std::setprecision(4) << par_time << " seconds" << std::endl;
    std::cout << "---------------------------------" << std::endl;
    std::cout << "Speedup: " << std::fixed << std::setprecision(2) << speedup << "x" << std::endl;
    std::cout << "Efficiency: " << std::fixed << std::setprecision(1) << efficiency << "%" << std::endl;
    
    // Optional: Check for super-linear speedup
    if (speedup > num_threads) {
        std::cout << "Note: Super-linear speedup detected (likely due to cache effects)" << std::endl;
    }
}

//==============================================================================
// Workload utilities
//==============================================================================

// Function to simulate work (compute-bound)
inline void do_compute_work(int iterations) {
    volatile double result = 0.0;
    for (int i = 0; i < iterations; ++i) {
        result += std::sin(static_cast<double>(i));
    }
}

// Function to simulate work (memory-bound)
inline void do_memory_work(std::vector<double>& data, int iterations) {
    const size_t size = data.size();
    for (int iter = 0; iter < iterations; ++iter) {
        for (size_t i = 1; i < size; ++i) {
            data[i] += data[i-1] * 0.5;
        }
    }
}

// Function to simulate work with sleep
inline void do_sleep_work(int milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

// Generate random data (integers)
inline std::vector<int> generate_random_ints(size_t size, int min_value, int max_value) {
    std::vector<int> data(size);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(min_value, max_value);
    
    for (size_t i = 0; i < size; ++i) {
        data[i] = dist(gen);
    }
    
    return data;
}

// Generate random data (doubles)
inline std::vector<double> generate_random_doubles(size_t size, double min_value, double max_value) {
    std::vector<double> data(size);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(min_value, max_value);
    
    for (size_t i = 0; i < size; ++i) {
        data[i] = dist(gen);
    }
    
    return data;
}

// Generate an ordered vector
template<typename T>
std::vector<T> generate_ordered_vector(size_t size, T start_value = 0, T step = 1) {
    std::vector<T> data(size);
    T value = start_value;
    
    for (size_t i = 0; i < size; ++i) {
        data[i] = value;
        value += step;
    }
    
    return data;
}

// Shuffle a vector
template<typename T>
void shuffle_vector(std::vector<T>& data) {
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(data.begin(), data.end(), g);
}

// Verify if two vectors are equal
template<typename T>
bool verify_vectors(const std::vector<T>& a, const std::vector<T>& b, double tolerance = 1e-10) {
    if (a.size() != b.size()) return false;
    
    for (size_t i = 0; i < a.size(); ++i) {
        if constexpr (std::is_floating_point<T>::value) {
            if (std::abs(a[i] - b[i]) > tolerance) return false;
        } else {
            if (a[i] != b[i]) return false;
        }
    }
    
    return true;
}

// Create a sorted copy of a vector
template<typename T>
std::vector<T> sorted_copy(const std::vector<T>& v) {
    std::vector<T> result = v;
    std::sort(result.begin(), result.end());
    return result;
}

// Check if two vectors contain the same elements (possibly in different order)
template<typename T>
bool same_elements(const std::vector<T>& a, const std::vector<T>& b, double tolerance = 1e-10) {
    if (a.size() != b.size()) return false;
    
    auto a_sorted = sorted_copy(a);
    auto b_sorted = sorted_copy(b);
    
    return verify_vectors(a_sorted, b_sorted, tolerance);
}

//==============================================================================
// Visualization utilities
//==============================================================================

// Draw a horizontal bar
inline void draw_horizontal_bar(int length, char symbol = '=') {
    for (int i = 0; i < length; ++i) {
        std::cout << symbol;
    }
    std::cout << std::endl;
}

// Print section header
inline void print_section_header(const std::string& title) {
    std::cout << "\n" << title << std::endl;
    draw_horizontal_bar(static_cast<int>(title.length()));
}

// Create a simple ASCII bar chart
inline void draw_ascii_bar_chart(const std::vector<double>& values, 
                                const std::vector<std::string>& labels, 
                                int max_width = 40, 
                                const std::string& title = "") {
    if (values.empty() || labels.empty() || values.size() != labels.size()) {
        std::cerr << "Error: Invalid data for bar chart." << std::endl;
        return;
    }
    
    // Find the maximum value
    double max_value = *std::max_element(values.begin(), values.end());
    
    // Find the longest label
    size_t max_label_len = 0;
    for (const auto& label : labels) {
        max_label_len = std::max(max_label_len, label.length());
    }
    
    // Print the title
    if (!title.empty()) {
        std::cout << title << std::endl;
        draw_horizontal_bar(static_cast<int>(title.length()), '-');
    }
    
    // Print each bar
    for (size_t i = 0; i < values.size(); ++i) {
        // Normalize the bar length
        int bar_length = static_cast<int>((values[i] / max_value) * static_cast<double>(max_width));
        
        // Print the label with padding
        std::cout << std::setw(static_cast<int>(max_label_len)) << std::left << labels[i] << " |";
        
        // Print the bar
        for (int j = 0; j < bar_length; ++j) {
            std::cout << "#";
        }
        
        // Print the value
        std::cout << " " << values[i] << std::endl;
    }
}

// Print a table row with even spacing
template<typename... Args>
void print_table_row(Args... args) {
    ([&] {
        std::cout << std::setw(14) << std::left << args;
    } (), ...);
    std::cout << std::endl;
}

// Draw an ASCII timeline chart
inline void draw_ascii_timeline(const std::map<int, std::vector<std::pair<double, double>>>& events_by_thread,
                              double max_time, int width = 80) {
    std::cout << "\nExecution Timeline:" << std::endl;
    std::cout << "0" << std::string(width - 6, ' ') << max_time << " sec" << std::endl;
    
    for (const auto& [thread_id, events] : events_by_thread) {
        std::cout << "Thread " << std::setw(2) << thread_id << ": ";
        
        std::string timeline(width, ' ');
        
        for (const auto& [start, end] : events) {
            int start_pos = static_cast<int>((start / max_time) * width);
            int end_pos = static_cast<int>((end / max_time) * width);
            
            // Ensure positions are within bounds
            start_pos = std::min(std::max(0, start_pos), width - 1);
            end_pos = std::min(std::max(0, end_pos), width - 1);
            
            // Mark the event in the timeline
            for (int i = start_pos; i <= end_pos; ++i) {
                timeline[i] = '=';
            }
            
            // Add event boundaries
            if (start_pos < width) timeline[start_pos] = '[';
            if (end_pos < width) timeline[end_pos] = ']';
        }
        
        std::cout << timeline << std::endl;
    }
}

//==============================================================================
// Thread utilities
//==============================================================================

// Simple thread binding enum compatible with OpenMP 2.0
enum ThreadBindType {
    BIND_NONE,   // No binding
    BIND_DEFAULT // Default binding
};

// Set thread count (simplified for OpenMP 2.0 compatibility)
inline bool set_thread_count(int num_threads) {
    omp_set_num_threads(num_threads);
    
    // Verify that settings were applied
    bool success = true;
    #pragma omp parallel
    {
        #pragma omp single
        {
            if (omp_get_num_threads() != num_threads) {
                std::cerr << "Warning: Failed to set number of threads." << std::endl;
                success = false;
            }
        }
    }
    
    return success;
}

// Print thread info
inline void print_thread_info() {
    #pragma omp parallel
    {
        #pragma omp critical
        {
            std::cout << "Thread " << omp_get_thread_num() 
                      << " of " << omp_get_num_threads() << std::endl;
        }
    }
}

// Convert ThreadBindType to string
inline std::string thread_bind_to_string(ThreadBindType bind_type) {
    switch (bind_type) {
        case BIND_NONE: return "none (no binding)";
        case BIND_DEFAULT: return "default (implementation defined)";
        default: return "unknown";
    }
}

//==============================================================================
// Task monitoring utilities
//==============================================================================

// Structure to track task execution data
struct TaskEvent {
    int task_id;
    int thread_id;
    double start_time;
    double end_time;
    std::string task_name;
    
    TaskEvent(int id, int thread, double start, std::string name = "")
        : task_id(id), thread_id(thread), start_time(start), end_time(0), task_name(name) {}
};

// Global task tracker
class TaskTracker {
private:
    std::vector<TaskEvent> events;
    std::mutex tracker_mutex;
    double program_start_time;
    
public:
    TaskTracker() : program_start_time(omp_get_wtime()) {}
    
    // Delete copy constructor and assignment operator
    TaskTracker(const TaskTracker&) = delete;
    TaskTracker& operator=(const TaskTracker&) = delete;
    
    // Move constructor and assignment operator
    TaskTracker(TaskTracker&& other) noexcept {
        // Lock the source object's mutex
        std::lock_guard<std::mutex> lock(other.tracker_mutex);
        events = std::move(other.events);
        program_start_time = other.program_start_time;
    }
    
    TaskTracker& operator=(TaskTracker&& other) noexcept {
        if (this != &other) {
            // Lock both mutexes to prevent deadlock
            std::lock(tracker_mutex, other.tracker_mutex);
            std::lock_guard<std::mutex> lock1(tracker_mutex, std::adopt_lock);
            std::lock_guard<std::mutex> lock2(other.tracker_mutex, std::adopt_lock);
            
            events = std::move(other.events);
            program_start_time = other.program_start_time;
        }
        return *this;
    }
    
    void reset() {
        std::lock_guard<std::mutex> lock(tracker_mutex);
        events.clear();
        program_start_time = omp_get_wtime();
    }
    
    void record_start(int task_id, int thread_id, const std::string& task_name = "") {
        std::lock_guard<std::mutex> lock(tracker_mutex);
        events.emplace_back(task_id, thread_id, omp_get_wtime() - program_start_time, task_name);
    }
    
    void record_end(int task_id, int thread_id) {
        std::lock_guard<std::mutex> lock(tracker_mutex);
        
        // Find the task event and update its end time
        for (auto it = events.rbegin(); it != events.rend(); ++it) {
            if (it->task_id == task_id && it->thread_id == thread_id && it->end_time == 0) {
                it->end_time = omp_get_wtime() - program_start_time;
                break;
            }
        }
    }
    
    const std::vector<TaskEvent>& get_events() const {
        return events;
    }
    
    // Get events grouped by thread
    std::map<int, std::vector<std::pair<double, double>>> get_events_by_thread() const {
        std::map<int, std::vector<std::pair<double, double>>> result;
        
        for (const auto& event : events) {
            result[event.thread_id].emplace_back(event.start_time, event.end_time);
        }
        
        return result;
    }
    
    // Calculate thread utilization
    std::map<int, double> calculate_thread_utilization() const {
        // Find the maximum end time
        double max_time = 0;
        for (const auto& event : events) {
            max_time = std::max(max_time, event.end_time);
        }
        
        // Calculate busy time for each thread
        std::map<int, double> thread_busy_time;
        for (const auto& event : events) {
            thread_busy_time[event.thread_id] += (event.end_time - event.start_time);
        }
        
        // Calculate utilization percentage
        std::map<int, double> utilization;
        for (const auto& [thread_id, busy_time] : thread_busy_time) {
            utilization[thread_id] = (busy_time / max_time) * 100.0;
        }
        
        return utilization;
    }
    
    // Get the maximum execution time
    double get_max_time() const {
        double max_time = 0;
        for (const auto& event : events) {
            max_time = std::max(max_time, event.end_time);
        }
        return max_time;
    }
    
    // Visualize task execution
    void visualize_execution() const {
        if (events.empty()) {
            std::cout << "No task events recorded." << std::endl;
            return;
        }
        
        // Draw timeline
        auto events_by_thread = get_events_by_thread();
        double max_time = get_max_time();
        
        // Eğer tamamlanmamış task varsa, gösterimden çıkar
        for (auto it = events_by_thread.begin(); it != events_by_thread.end(); ++it) {
            auto& thread_events = it->second;
            thread_events.erase(
                std::remove_if(thread_events.begin(), thread_events.end(), 
                    [](const std::pair<double, double>& event) {
                        return event.second == 0.0; // Tamamlanmamış
                    }
                ), 
                thread_events.end()
            );
        }
        
        // Timeline göster, eğer geçerli olaylar varsa
        if (!events_by_thread.empty()) {
            draw_ascii_timeline(events_by_thread, max_time);
        } else {
            std::cout << "No completed task events to display." << std::endl;
            return;
        }
        
        // Print utilization stats
        auto utilization = calculate_thread_utilization();
        
        if (utilization.empty()) {
            std::cout << "\nNo thread utilization data available." << std::endl;
            return;
        }
        
        std::cout << "\nThread Utilization:" << std::endl;
        std::cout << "Thread | Busy Time (s) | Utilization" << std::endl;
        std::cout << "--------------------------------------" << std::endl;
        
        // Calculate busy times
        std::map<int, double> thread_busy_time;
        for (const auto& event : events) {
            // Sadece tamamlanmış işleri hesapla
            if (event.end_time > 0) {
                thread_busy_time[event.thread_id] += (event.end_time - event.start_time);
            }
        }
        
        double total_busy_time = 0;
        for (const auto& [thread_id, util] : utilization) {
            if (thread_busy_time.find(thread_id) == thread_busy_time.end()) {
                continue; // Bu thread için veri yok
            }
            
            double busy_time = thread_busy_time.at(thread_id);
            std::cout << std::setw(6) << thread_id << " | "
                      << std::fixed << std::setprecision(4) << std::setw(12) << busy_time << " | "
                      << std::fixed << std::setprecision(1) << std::setw(6) << util << "% |";
            
            // Visual representation
            int bar_length = static_cast<int>(util / 2);
            for (int j = 0; j < bar_length; ++j) {
                std::cout << "█";
            }
            std::cout << std::endl;
            
            total_busy_time += busy_time;
        }
        
        // Check if we have valid data for overall utilization
        if (utilization.size() > 0 && max_time > 0) {
            // Calculate overall utilization
            double overall_utilization = (total_busy_time / (max_time * utilization.size())) * 100.0;
            std::cout << "--------------------------------------" << std::endl;
            std::cout << "Overall Utilization: " << std::fixed << std::setprecision(1) << overall_utilization << "%" << std::endl;
        }
    }
};

//==============================================================================
// Task-specific utilities
//==============================================================================

// Determine if a task should be executed in parallel based on size and cutoff
inline bool should_parallelize(int size, int cutoff) {
    return size >= cutoff;
}

// Structure to define a dependency graph
struct DependencyGraph {
    struct Node {
        int id;
        std::string name;
        int cost;  // in milliseconds
        std::vector<int> dependencies;
        
        Node(int i, const std::string& n, int c, std::initializer_list<int> deps = {})
            : id(i), name(n), cost(c), dependencies(deps) {}
        
        // Constructor with vector of dependencies
        Node(int i, const std::string& n, int c, const std::vector<int>& deps)
            : id(i), name(n), cost(c), dependencies(deps) {}
    };
    
    std::vector<Node> nodes;
    
    void add_node(int id, const std::string& name, int cost, std::initializer_list<int> deps = {}) {
        nodes.push_back(Node(id, name, cost, deps));
    }
    
    // Overload to accept a vector of dependencies
    void add_node(int id, const std::string& name, int cost, const std::vector<int>& deps) {
        nodes.push_back(Node(id, name, cost, deps));
    }
    
    void print() const {
        std::cout << "Dependency Graph:" << std::endl;
        std::cout << "ID | Name | Cost (ms) | Dependencies" << std::endl;
        std::cout << "------------------------------------" << std::endl;
        
        for (const auto& node : nodes) {
            std::cout << std::setw(2) << node.id << " | "
                      << std::setw(10) << node.name << " | "
                      << std::setw(8) << node.cost << " | ";
            
            if (node.dependencies.empty()) {
                std::cout << "None";
            } else {
                for (size_t i = 0; i < node.dependencies.size(); ++i) {
                    if (i > 0) std::cout << ", ";
                    std::cout << node.dependencies[i];
                }
            }
            
            std::cout << std::endl;
        }
    }
};

// Task throttling mechanism
class TaskThrottler {
private:
    std::atomic<int> active_tasks;
    int max_tasks;
    
public:
    TaskThrottler(int max) : active_tasks(0), max_tasks(max) {}
    
    void before_task() {
        while (true) {
            int current = active_tasks.load();
            if (current < max_tasks && 
                active_tasks.compare_exchange_weak(current, current + 1)) {
                break;
            }
            // Use a simple yield instead of taskyield for OpenMP 2.0 compatibility
            #pragma omp flush
            std::this_thread::yield();
        }
    }
    
    void after_task() {
        active_tasks--;
    }
    
    int get_active_tasks() const {
        return active_tasks.load();
    }
    
    int get_max_tasks() const {
        return max_tasks;
    }
};

} // namespace task_utils

#endif // TASK_UTILS_H