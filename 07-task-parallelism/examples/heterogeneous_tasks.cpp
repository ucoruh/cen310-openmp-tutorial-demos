#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <chrono>
#include <random>
#include <functional>
#include <algorithm>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <omp.h>
#include "../include/task_utils.h"

//==============================================================================
// Task Type Definitions
//==============================================================================

// Enum to represent different task types
enum class TaskType {
    ComputeBound,    // CPU-intensive (compute-bound) task
    MemoryBound,     // Memory-intensive task
    IOBound,         // I/O-bound task (simulated with sleep)
    Mixed            // Mixed workload task
};

// Structure to represent a heterogeneous task
struct HeterogeneousTask {
    int id;
    TaskType type;
    int work_amount;
    std::string name;
    
    HeterogeneousTask(int i, TaskType t, int work, const std::string& n = "")
        : id(i), type(t), work_amount(work), name(n.empty() ? "Task_" + std::to_string(i) : n) {}
    
    // Get a string representation of the task type
    std::string get_type_string() const {
        switch (type) {
            case TaskType::ComputeBound: return "Compute";
            case TaskType::MemoryBound: return "Memory";
            case TaskType::IOBound: return "IO";
            case TaskType::Mixed: return "Mixed";
            default: return "Unknown";
        }
    }
};

// Structure to record task execution data
struct TaskExecution {
    int task_id;
    int thread_id;
    TaskType type;
    double start_time;
    double end_time;
    
    TaskExecution(int id, int thread, TaskType t, double start)
        : task_id(id), thread_id(thread), type(t), start_time(start), end_time(0.0) {}
};

// Global recorder for task execution
class TaskExecutionRecorder {
private:
    std::vector<TaskExecution> executions;
    std::mutex recorder_mutex;
    double program_start_time;
    
public:
    TaskExecutionRecorder() {
        reset(); // Initialize with a proper reset
    }
    
    void reset() {
        std::lock_guard<std::mutex> lock(recorder_mutex);
        executions.clear();
        program_start_time = omp_get_wtime();
        std::cout << "Task recorder reset. Ready to record new executions." << std::endl;
    }
    
    void record_start(const HeterogeneousTask& task, int thread_id) {
        double current_time = omp_get_wtime() - program_start_time;
        
        try {
            std::lock_guard<std::mutex> lock(recorder_mutex);
            executions.emplace_back(task.id, thread_id, task.type, current_time);
        } catch (const std::exception& e) {
            std::cerr << "Error recording task start: " << e.what() << std::endl;
        }
    }
    
    void record_end(int task_id, int thread_id) {
        double current_time = omp_get_wtime() - program_start_time;
        
        try {
            std::lock_guard<std::mutex> lock(recorder_mutex);
            // Find the matching execution record and update its end time
            bool found = false;
            for (auto it = executions.rbegin(); it != executions.rend(); ++it) {
                if (it->task_id == task_id && it->thread_id == thread_id && it->end_time == 0.0) {
                    it->end_time = current_time;
                    found = true;
                    break;
                }
            }
            
            if (!found) {
                std::cerr << "Warning: Could not find matching start record for task " 
                          << task_id << " on thread " << thread_id << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error recording task end: " << e.what() << std::endl;
        }
    }
    
    // Calculate statistics by task type
    std::map<TaskType, std::pair<int, double>> calculate_type_stats() const {
        std::map<TaskType, std::pair<int, double>> stats; // type -> (count, total_time)
        
        for (const auto& exec : executions) {
            if (exec.end_time > 0.0) { // Only consider completed tasks
                double duration = exec.end_time - exec.start_time;
                stats[exec.type].first++;
                stats[exec.type].second += duration;
            }
        }
        
        return stats;
    }
    
    // Calculate statistics by thread
    std::map<int, std::map<TaskType, std::pair<int, double>>> calculate_thread_stats() const {
        std::map<int, std::map<TaskType, std::pair<int, double>>> stats; // thread -> type -> (count, total_time)
        
        for (const auto& exec : executions) {
            if (exec.end_time > 0.0) { // Only consider completed tasks
                double duration = exec.end_time - exec.start_time;
                stats[exec.thread_id][exec.type].first++;
                stats[exec.thread_id][exec.type].second += duration;
            }
        }
        
        return stats;
    }
    
    // Get average execution time by task type
    std::map<TaskType, double> calculate_avg_execution_time() const {
        std::map<TaskType, double> result;
        auto type_stats = calculate_type_stats();
        
        for (const auto& [type, stats] : type_stats) {
            int count = stats.first;
            double total_time = stats.second;
            result[type] = count > 0 ? total_time / count : 0.0;
        }
        
        return result;
    }
    
    // Print execution statistics
    void print_statistics() const {
        std::cout << "\nTask Execution Statistics:" << std::endl;
        std::cout << "==========================" << std::endl;
        
        // Ensure we're capturing task executions
        if (executions.empty()) {
            std::cout << "No task executions recorded!" << std::endl;
            std::cout << "Please check if your tasks are being executed properly." << std::endl;
            return;
        }
        
        auto type_stats = calculate_type_stats();
        auto thread_stats = calculate_thread_stats();
        auto avg_times = calculate_avg_execution_time();
        
        int total_tasks = 0;
        double total_time = 0.0;
        
        // Calculate total tasks and execution time
        for (const auto& [type, stats] : type_stats) {
            total_tasks += stats.first;
            total_time += stats.second;
        }
        
        // Print total tasks and execution time
        std::cout << "Total tasks executed: " << total_tasks << std::endl;
        std::cout << "Total execution time: " << std::fixed << std::setprecision(3) << total_time << " seconds" << std::endl;
        
        // Only continue if we have task data
        if (total_tasks == 0) {
            std::cout << "No completed tasks found in records." << std::endl;
            return;
        }
        
        // Print statistics by task type
        std::cout << "\nStatistics by Task Type:" << std::endl;
        std::cout << "----------------------------------------------------------" << std::endl;
        std::cout << "Type    | Count | Total Time (s) | Avg Time (s) | % of Total" << std::endl;
        std::cout << "----------------------------------------------------------" << std::endl;
        
        for (const auto& [type, stats] : type_stats) {
            int count = stats.first;
            double type_time = stats.second;
            double avg_time = avg_times.at(type);
            double percentage = (total_time > 0.0) ? (type_time / total_time * 100.0) : 0.0;
            
            std::cout << std::setw(8) << get_type_string(type) << " | "
                      << std::setw(5) << count << " | "
                      << std::fixed << std::setprecision(3) << std::setw(13) << type_time << " | "
                      << std::fixed << std::setprecision(3) << std::setw(12) << avg_time << " | "
                      << std::fixed << std::setprecision(1) << std::setw(9) << percentage << "%" 
                      << std::endl;
        }
        
        // Print statistics by thread
        std::cout << "\nTask Distribution by Thread:" << std::endl;
        std::cout << "--------------------------------------------" << std::endl;
        std::cout << "Thread | Compute | Memory | IO    | Mixed | Total" << std::endl;
        std::cout << "--------------------------------------------" << std::endl;
        
        for (const auto& [thread_id, type_counts] : thread_stats) {
            int compute_count = type_counts.count(TaskType::ComputeBound) ? type_counts.at(TaskType::ComputeBound).first : 0;
            int memory_count = type_counts.count(TaskType::MemoryBound) ? type_counts.at(TaskType::MemoryBound).first : 0;
            int io_count = type_counts.count(TaskType::IOBound) ? type_counts.at(TaskType::IOBound).first : 0;
            int mixed_count = type_counts.count(TaskType::Mixed) ? type_counts.at(TaskType::Mixed).first : 0;
            int thread_total = compute_count + memory_count + io_count + mixed_count;
            
            std::cout << std::setw(6) << thread_id << " | "
                      << std::setw(7) << compute_count << " | "
                      << std::setw(6) << memory_count << " | "
                      << std::setw(5) << io_count << " | "
                      << std::setw(5) << mixed_count << " | "
                      << std::setw(5) << thread_total << std::endl;
        }
        
        // Visualize thread utilization by task type
        print_thread_utilization();
    }
    
    // Visualize thread utilization
    void print_thread_utilization() const {
        auto thread_stats = calculate_thread_stats();
        
        if (thread_stats.empty()) {
            return;
        }
        
        // Find the max end time
        double max_time = 0.0;
        for (const auto& exec : executions) {
            max_time = std::max(max_time, exec.end_time);
        }
        
        // Group executions by thread
        std::map<int, std::vector<TaskExecution>> executions_by_thread;
        for (const auto& exec : executions) {
            if (exec.end_time > 0.0) { // Only consider completed tasks
                executions_by_thread[exec.thread_id].push_back(exec);
            }
        }
        
        std::cout << "\nThread Utilization By Task Type:" << std::endl;
        std::cout << "=====================================================================\n";
        
        const int timeline_width = 60;
        
        // Print timeline header
        std::cout << "Thread |";
        for (int i = 0; i <= 10; i++) {
            double time_point = (i / 10.0) * max_time;
            std::cout << std::fixed << std::setprecision(1) << std::setw(5) << time_point << " ";
        }
        std::cout << "\n";
        std::cout << "-------+";
        for (int i = 0; i < timeline_width; i++) {
            std::cout << "-";
        }
        std::cout << "\n";
        
        // Print timeline for each thread
        for (const auto& [thread_id, thread_executions] : executions_by_thread) {
            std::cout << std::setw(6) << thread_id << " |";
            
            std::vector<char> timeline(timeline_width, ' ');
            
            for (const auto& exec : thread_executions) {
                // Convert time points to positions on the timeline
                int start_pos = static_cast<int>((exec.start_time / max_time) * timeline_width);
                int end_pos = static_cast<int>((exec.end_time / max_time) * timeline_width);
                
                // Ensure positions are within bounds
                start_pos = std::min(std::max(0, start_pos), timeline_width - 1);
                end_pos = std::min(std::max(start_pos, end_pos), timeline_width - 1);
                
                // Fill the timeline with a symbol representing the task type
                char symbol = get_type_symbol(exec.type);
                for (int i = start_pos; i <= end_pos; ++i) {
                    timeline[i] = symbol;
                }
            }
            
            // Print the timeline
            for (char c : timeline) {
                std::cout << c;
            }
            std::cout << "\n";
        }
        
        // Print legend
        std::cout << "Legend: C = Compute, M = Memory, I = IO, X = Mixed\n";
    }
    
private:
    // Convert TaskType to string
    std::string get_type_string(TaskType type) const {
        switch (type) {
            case TaskType::ComputeBound: return "Compute";
            case TaskType::MemoryBound: return "Memory";
            case TaskType::IOBound: return "IO";
            case TaskType::Mixed: return "Mixed";
            default: return "Unknown";
        }
    }
    
    // Get a symbol representing the task type for timeline visualization
    char get_type_symbol(TaskType type) const {
        switch (type) {
            case TaskType::ComputeBound: return 'C';
            case TaskType::MemoryBound: return 'M';
            case TaskType::IOBound: return 'I';
            case TaskType::Mixed: return 'X';
            default: return '?';
        }
    }
};

// Global recorder
TaskExecutionRecorder recorder;

//==============================================================================
// Task Execution Functions
//==============================================================================

// Memory for memory-bound tasks
const int MEMORY_SIZE = 100 * 1024 * 1024; // 100 MB
std::vector<int> global_memory(MEMORY_SIZE);

// Execute a compute-bound task
void execute_compute_task(const HeterogeneousTask& task) {
    int thread_id = omp_get_thread_num();
    
    // Record start
    recorder.record_start(task, thread_id);
    
    // Actual computation (CPU-intensive)
    task_utils::do_compute_work(task.work_amount);
    
    // Record end
    recorder.record_end(task.id, thread_id);
}

// Execute a memory-bound task
void execute_memory_task(const HeterogeneousTask& task) {
    int thread_id = omp_get_thread_num();
    
    // Record start
    recorder.record_start(task, thread_id);
    
    try {
        // Allocate memory for this task - use a more reasonable size
        size_t array_size = std::min(static_cast<size_t>(task.work_amount) * 1000, 
                                    static_cast<size_t>(10 * 1024 * 1024)); // Cap at 10MB
        std::vector<int> local_data(array_size);
        
        // Memory intensive operations (repeated access patterns)
        for (int iter = 0; iter < 10; ++iter) {
            for (size_t i = 0; i < local_data.size(); i += 16) { // Use stride to defeat cache prefetching
                local_data[i] += iter;
            }
            
            // Access global memory as well - ensure we don't go out of bounds
            for (size_t i = 0; i < std::min(static_cast<size_t>(MEMORY_SIZE), array_size); i += 64) {
                global_memory[i] += local_data[i % local_data.size()];
            }
        }
        
        // Record end
        recorder.record_end(task.id, thread_id);
    } catch (const std::exception& e) {
        // If an error occurs, ensure we still record the end of the task
        recorder.record_end(task.id, thread_id);
        // Re-throw the exception
        throw;
    }
}

// Execute an I/O-bound task (simulated with sleep)
void execute_io_task(const HeterogeneousTask& task) {
    int thread_id = omp_get_thread_num();
    
    // Record start
    recorder.record_start(task, thread_id);
    
    // Simulate I/O operations with sleep
    std::this_thread::sleep_for(std::chrono::milliseconds(task.work_amount));
    
    // Record end
    recorder.record_end(task.id, thread_id);
}

// Execute a mixed task (compute + memory + I/O)
void execute_mixed_task(const HeterogeneousTask& task) {
    int thread_id = omp_get_thread_num();
    
    // Record start
    recorder.record_start(task, thread_id);
    
    // Distribute work amount across different resource types
    int compute_work = task.work_amount / 2;
    int memory_work = task.work_amount / 4;
    int io_work = task.work_amount / 4;
    
    // Compute part
    task_utils::do_compute_work(compute_work);
    
    // Memory part
    std::vector<int> local_data(memory_work * 100);
    for (int i = 0; i < memory_work; ++i) {
        for (size_t j = 0; j < local_data.size(); j += 32) {
            local_data[j] += i;
        }
    }
    
    // I/O part (simulated)
    std::this_thread::sleep_for(std::chrono::milliseconds(io_work));
    
    // Record end
    recorder.record_end(task.id, thread_id);
}

// Execute a task based on its type
void execute_task(const HeterogeneousTask& task) {
    try {
        #pragma omp critical(cout)
        {
            std::cout << "Executing task " << task.id << " of type " << task.get_type_string() << std::endl;
        }
        
        switch (task.type) {
            case TaskType::ComputeBound:
                execute_compute_task(task);
                break;
            case TaskType::MemoryBound:
                execute_memory_task(task);
                break;
            case TaskType::IOBound:
                execute_io_task(task);
                break;
            case TaskType::Mixed:
                execute_mixed_task(task);
                break;
            default:
                #pragma omp critical(cerr)
                {
                    std::cerr << "Unknown task type for task " << task.id << std::endl;
                }
                break;
        }
    } catch (const std::exception& e) {
        #pragma omp critical(cerr)
        {
            std::cerr << "Error executing task " << task.id << " (" << task.get_type_string() 
                      << "): " << e.what() << std::endl;
        }
    }
}

//==============================================================================
// Task Generation and Scheduling Functions
//==============================================================================

// Generate a mix of heterogeneous tasks
std::vector<HeterogeneousTask> generate_tasks(int num_tasks, int min_work, int max_work, 
                                           double compute_ratio = 0.4, 
                                           double memory_ratio = 0.3,
                                           double io_ratio = 0.2,
                                           [[maybe_unused]] double mixed_ratio = 0.1) {
    // Ensure reasonable task counts
    num_tasks = std::min(num_tasks, 100); // Limit to 100 tasks maximum
    min_work = std::max(10, std::min(min_work, 1000)); // Keep work amount reasonable
    max_work = std::max(min_work, std::min(max_work, 1000)); // Keep max work reasonable
    
    std::vector<HeterogeneousTask> tasks;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> work_dist(min_work, max_work);
    
    // Calculate the number of each task type
    int num_compute = std::max(1, static_cast<int>(num_tasks * compute_ratio));
    int num_memory = std::max(1, static_cast<int>(num_tasks * memory_ratio));
    int num_io = std::max(1, static_cast<int>(num_tasks * io_ratio));
    int num_mixed = std::max(1, num_tasks - num_compute - num_memory - num_io);
    
    // Ensure we have at least one of each type
    if (num_compute + num_memory + num_io + num_mixed < 4) {
        num_compute = num_memory = num_io = num_mixed = 1;
    }
    
    // Generate compute-bound tasks
    for (int i = 0; i < num_compute; ++i) {
        int work = work_dist(gen);
        tasks.emplace_back(tasks.size(), TaskType::ComputeBound, work, "Compute_" + std::to_string(i));
    }
    
    // Generate memory-bound tasks
    for (int i = 0; i < num_memory; ++i) {
        int work = work_dist(gen);
        tasks.emplace_back(tasks.size(), TaskType::MemoryBound, work, "Memory_" + std::to_string(i));
    }
    
    // Generate I/O-bound tasks
    for (int i = 0; i < num_io; ++i) {
        int work = work_dist(gen);
        tasks.emplace_back(tasks.size(), TaskType::IOBound, work, "IO_" + std::to_string(i));
    }
    
    // Generate mixed tasks
    for (int i = 0; i < num_mixed; ++i) {
        int work = work_dist(gen);
        tasks.emplace_back(tasks.size(), TaskType::Mixed, work, "Mixed_" + std::to_string(i));
    }
    
    // Shuffle the tasks
    std::shuffle(tasks.begin(), tasks.end(), gen);
    
    return tasks;
}

// Execute tasks with naive scheduling
void execute_naive(const std::vector<HeterogeneousTask>& tasks, int num_threads) {
    std::cout << "\nExecuting tasks with naive scheduling..." << std::endl;
    
    // Reset the task recorder
    recorder.reset();
    
    // Set the number of threads
    omp_set_num_threads(num_threads);
    std::cout << "Using " << num_threads << " threads for execution" << std::endl;
    std::cout << "Executing " << tasks.size() << " tasks" << std::endl;
    
    // Create a counter to track completed tasks
    std::atomic<int> completed_tasks(0);
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            // Changed from const auto& to auto to avoid reference type in firstprivate
            for (auto task : tasks) {
                #pragma omp task firstprivate(task)
                {
                    try {
                        // Debug output before task execution
                        int thread_id = omp_get_thread_num();
                        #pragma omp critical(cout)
                        {
                            std::cout << "Thread " << thread_id << " executing task " << task.id 
                                      << " (" << task.get_type_string() << ")" << std::endl;
                        }
                        
                        // Execute the task
                        execute_task(task);
                        completed_tasks++;
                    } catch (const std::exception& e) {
                        #pragma omp critical(cerr)
                        {
                            std::cerr << "Error executing task " << task.id << ": " << e.what() << std::endl;
                        }
                    }
                }
            }
            
            // Wait for all tasks to complete
            #pragma omp taskwait
        }
    }
    
    std::cout << "Completed " << completed_tasks << " tasks" << std::endl;
}

// Execute tasks grouped by type
void execute_grouped(const std::vector<HeterogeneousTask>& tasks, int num_threads) {
    std::cout << "\nExecuting tasks grouped by type..." << std::endl;
    
    // Reset the task recorder
    recorder.reset();
    
    // Group tasks by type
    std::map<TaskType, std::vector<HeterogeneousTask>> grouped_tasks;
    for (const auto& task : tasks) {
        grouped_tasks[task.type].push_back(task);
    }
    
    // Set the number of threads
    omp_set_num_threads(num_threads);
    std::cout << "Using " << num_threads << " threads for execution" << std::endl;
    
    // Create a counter to track completed tasks
    std::atomic<int> completed_tasks(0);
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            // Execute compute-bound tasks
            std::cout << "Executing " << grouped_tasks[TaskType::ComputeBound].size() << " compute-bound tasks" << std::endl;
            for (int i = 0; i < grouped_tasks[TaskType::ComputeBound].size(); i++) {
                // Make a copy of the task to avoid the reference issue
                HeterogeneousTask task = grouped_tasks[TaskType::ComputeBound][i];
                
                #pragma omp task
                {
                    try {
                        // Debug output before task execution
                        int thread_id = omp_get_thread_num();
                        #pragma omp critical(cout)
                        {
                            std::cout << "Thread " << thread_id << " executing task " << task.id 
                                      << " (" << task.get_type_string() << ")" << std::endl;
                        }
                        
                        // Execute the task
                        execute_task(task);
                        completed_tasks++;
                    } catch (const std::exception& e) {
                        #pragma omp critical(cerr)
                        {
                            std::cerr << "Error executing task " << task.id << ": " << e.what() << std::endl;
                        }
                    }
                }
            }
            
            // Execute memory-bound tasks
            std::cout << "Executing " << grouped_tasks[TaskType::MemoryBound].size() << " memory-bound tasks" << std::endl;
            for (int i = 0; i < grouped_tasks[TaskType::MemoryBound].size(); i++) {
                // Make a copy of the task to avoid the reference issue
                HeterogeneousTask task = grouped_tasks[TaskType::MemoryBound][i];
                
                #pragma omp task
                {
                    try {
                        // Debug output before task execution
                        int thread_id = omp_get_thread_num();
                        #pragma omp critical(cout)
                        {
                            std::cout << "Thread " << thread_id << " executing task " << task.id 
                                      << " (" << task.get_type_string() << ")" << std::endl;
                        }
                        
                        // Execute the task
                        execute_task(task);
                        completed_tasks++;
                    } catch (const std::exception& e) {
                        #pragma omp critical(cerr)
                        {
                            std::cerr << "Error executing task " << task.id << ": " << e.what() << std::endl;
                        }
                    }
                }
            }
            
            // Execute I/O-bound tasks
            std::cout << "Executing " << grouped_tasks[TaskType::IOBound].size() << " I/O-bound tasks" << std::endl;
            for (int i = 0; i < grouped_tasks[TaskType::IOBound].size(); i++) {
                // Make a copy of the task to avoid the reference issue
                HeterogeneousTask task = grouped_tasks[TaskType::IOBound][i];
                
                #pragma omp task
                {
                    try {
                        // Debug output before task execution
                        int thread_id = omp_get_thread_num();
                        #pragma omp critical(cout)
                        {
                            std::cout << "Thread " << thread_id << " executing task " << task.id 
                                      << " (" << task.get_type_string() << ")" << std::endl;
                        }
                        
                        // Execute the task
                        execute_task(task);
                        completed_tasks++;
                    } catch (const std::exception& e) {
                        #pragma omp critical(cerr)
                        {
                            std::cerr << "Error executing task " << task.id << ": " << e.what() << std::endl;
                        }
                    }
                }
            }
            
            // Execute mixed tasks
            std::cout << "Executing " << grouped_tasks[TaskType::Mixed].size() << " mixed tasks" << std::endl;
            for (int i = 0; i < grouped_tasks[TaskType::Mixed].size(); i++) {
                // Make a copy of the task to avoid the reference issue
                HeterogeneousTask task = grouped_tasks[TaskType::Mixed][i];
                
                #pragma omp task
                {
                    try {
                        // Debug output before task execution
                        int thread_id = omp_get_thread_num();
                        #pragma omp critical(cout)
                        {
                            std::cout << "Thread " << thread_id << " executing task " << task.id 
                                      << " (" << task.get_type_string() << ")" << std::endl;
                        }
                        
                        // Execute the task
                        execute_task(task);
                        completed_tasks++;
                    } catch (const std::exception& e) {
                        #pragma omp critical(cerr)
                        {
                            std::cerr << "Error executing task " << task.id << ": " << e.what() << std::endl;
                        }
                    }
                }
            }
            
            // Wait for all tasks to complete
            #pragma omp taskwait
        }
    }
    
    std::cout << "Completed " << completed_tasks << " tasks" << std::endl;
}

// Execute tasks with simulated priority (for OpenMP 2.0 compatibility)
void execute_priority(const std::vector<HeterogeneousTask>& tasks, int num_threads) {
    std::cout << "\nExecuting tasks with simulated priority-based scheduling..." << std::endl;
    std::cout << "Note: Using manual priority simulation (OpenMP priority requires 4.5+)" << std::endl;
    
    // Reset the task recorder
    recorder.reset();
    
    // Group tasks by priority (based on task type)
    std::vector<HeterogeneousTask> high_priority; // Compute-bound tasks
    std::vector<HeterogeneousTask> medium_high_priority; // Memory-bound tasks
    std::vector<HeterogeneousTask> medium_low_priority; // Mixed tasks
    std::vector<HeterogeneousTask> low_priority; // I/O-bound tasks
    
    for (const auto& task : tasks) {
        switch (task.type) {
            case TaskType::ComputeBound:
                high_priority.push_back(task);
                break;
            case TaskType::MemoryBound:
                medium_high_priority.push_back(task);
                break;
            case TaskType::Mixed:
                medium_low_priority.push_back(task);
                break;
            case TaskType::IOBound:
                low_priority.push_back(task);
                break;
        }
    }
    
    // Set the number of threads
    omp_set_num_threads(num_threads);
    std::cout << "Using " << num_threads << " threads for execution" << std::endl;
    
    // Create a counter to track completed tasks
    std::atomic<int> completed_tasks(0);
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            // Execute high priority tasks first (compute-bound)
            std::cout << "Executing " << high_priority.size() << " high priority tasks (compute-bound)" << std::endl;
            for (int i = 0; i < high_priority.size(); i++) {
                // Make a copy of the task to avoid the reference issue
                HeterogeneousTask task = high_priority[i];
                
                #pragma omp task
                {
                    try {
                        // Debug output before task execution
                        int thread_id = omp_get_thread_num();
                        #pragma omp critical(cout)
                        {
                            std::cout << "Thread " << thread_id << " executing high priority task " << task.id 
                                      << " (" << task.get_type_string() << ")" << std::endl;
                        }
                        
                        // Execute the task
                        execute_task(task);
                        completed_tasks++;
                    } catch (const std::exception& e) {
                        #pragma omp critical(cerr)
                        {
                            std::cerr << "Error executing task " << task.id << ": " << e.what() << std::endl;
                        }
                    }
                }
            }
            
            // Wait for high priority tasks to complete
            #pragma omp taskwait
            
            // Execute medium-high priority tasks (memory-bound)
            std::cout << "Executing " << medium_high_priority.size() << " medium-high priority tasks (memory-bound)" << std::endl;
            for (int i = 0; i < medium_high_priority.size(); i++) {
                // Make a copy of the task to avoid the reference issue
                HeterogeneousTask task = medium_high_priority[i];
                
                #pragma omp task
                {
                    try {
                        // Debug output before task execution
                        int thread_id = omp_get_thread_num();
                        #pragma omp critical(cout)
                        {
                            std::cout << "Thread " << thread_id << " executing medium-high priority task " << task.id 
                                      << " (" << task.get_type_string() << ")" << std::endl;
                        }
                        
                        // Execute the task
                        execute_task(task);
                        completed_tasks++;
                    } catch (const std::exception& e) {
                        #pragma omp critical(cerr)
                        {
                            std::cerr << "Error executing task " << task.id << ": " << e.what() << std::endl;
                        }
                    }
                }
            }
            
            // Wait for medium-high priority tasks to complete
            #pragma omp taskwait
            
            // Execute medium-low priority tasks (mixed)
            std::cout << "Executing " << medium_low_priority.size() << " medium-low priority tasks (mixed)" << std::endl;
            for (int i = 0; i < medium_low_priority.size(); i++) {
                // Make a copy of the task to avoid the reference issue
                HeterogeneousTask task = medium_low_priority[i];
                
                #pragma omp task
                {
                    try {
                        // Debug output before task execution
                        int thread_id = omp_get_thread_num();
                        #pragma omp critical(cout)
                        {
                            std::cout << "Thread " << thread_id << " executing medium-low priority task " << task.id 
                                      << " (" << task.get_type_string() << ")" << std::endl;
                        }
                        
                        // Execute the task
                        execute_task(task);
                        completed_tasks++;
                    } catch (const std::exception& e) {
                        #pragma omp critical(cerr)
                        {
                            std::cerr << "Error executing task " << task.id << ": " << e.what() << std::endl;
                        }
                    }
                }
            }
            
            // Execute low priority tasks (I/O-bound)
            std::cout << "Executing " << low_priority.size() << " low priority tasks (I/O-bound)" << std::endl;
            for (int i = 0; i < low_priority.size(); i++) {
                // Make a copy of the task to avoid the reference issue
                HeterogeneousTask task = low_priority[i];
                
                #pragma omp task
                {
                    try {
                        // Debug output before task execution
                        int thread_id = omp_get_thread_num();
                        #pragma omp critical(cout)
                        {
                            std::cout << "Thread " << thread_id << " executing low priority task " << task.id 
                                      << " (" << task.get_type_string() << ")" << std::endl;
                        }
                        
                        // Execute the task
                        execute_task(task);
                        completed_tasks++;
                    } catch (const std::exception& e) {
                        #pragma omp critical(cerr)
                        {
                            std::cerr << "Error executing task " << task.id << ": " << e.what() << std::endl;
                        }
                    }
                }
            }
        }
    }
    
    std::cout << "Completed " << completed_tasks << " tasks" << std::endl;
}

// Execute tasks with thread binding (specific thread types handle specific task types)
void execute_thread_binding(const std::vector<HeterogeneousTask>& tasks, int num_threads) {
    std::cout << "\nExecuting tasks with type-specific thread binding..." << std::endl;
    
    // Group tasks by type
    std::map<TaskType, std::vector<HeterogeneousTask>> grouped_tasks;
    for (const auto& task : tasks) {
        grouped_tasks[task.type].push_back(task);
    }
    
    // Calculate how many threads to allocate to each type
    int compute_threads = std::max(1, static_cast<int>(num_threads * 0.5)); // 50% for compute
    int memory_threads = std::max(1, static_cast<int>(num_threads * 0.3)); // 30% for memory
    int io_threads = std::max(1, static_cast<int>(num_threads * 0.1)); // 10% for I/O
    int mixed_threads = num_threads - compute_threads - memory_threads - io_threads; // Remainder for mixed
    
    std::cout << "Thread allocation: " 
              << compute_threads << " compute, "
              << memory_threads << " memory, "
              << io_threads << " I/O, "
              << mixed_threads << " mixed" << std::endl;
    
    // Execute each group with its allocated threads
    std::atomic<int> completed_tasks(0);
    
    // Function to execute a group of tasks with a specific number of threads
    auto execute_group = [&completed_tasks](const std::vector<HeterogeneousTask>& group, int threads) {
        omp_set_num_threads(threads);
        
        #pragma omp parallel
        {
            #pragma omp single
            {
                for (const auto& task : group) {
                    #pragma omp task
                    {
                        execute_task(task);
                        completed_tasks++;
                    }
                }
            }
        }
    };
    
    // Execute each group sequentially (for OpenMP 2.0 compatibility)
    std::cout << "Running compute tasks..." << std::endl;
    execute_group(grouped_tasks[TaskType::ComputeBound], compute_threads);
    
    std::cout << "Running memory tasks..." << std::endl;
    execute_group(grouped_tasks[TaskType::MemoryBound], memory_threads);
    
    std::cout << "Running I/O tasks..." << std::endl;
    execute_group(grouped_tasks[TaskType::IOBound], io_threads);
    
    std::cout << "Running mixed tasks..." << std::endl;
    execute_group(grouped_tasks[TaskType::Mixed], mixed_threads);
    
    std::cout << "Completed " << completed_tasks << " tasks" << std::endl;
}

// Execute tasks with adaptive scheduling
void execute_adaptive(const std::vector<HeterogeneousTask>& tasks, int num_threads) {
    std::cout << "\nExecuting tasks with adaptive scheduling..." << std::endl;
    std::cout << "Note: Using simplified approach for OpenMP 2.0 compatibility" << std::endl;
    
    // Set up counters for each task type
    std::atomic<int> active_compute(0);
    std::atomic<int> active_memory(0);
    std::atomic<int> active_io(0);
    
    // Calculate the maximum number of tasks per type
    int max_compute = std::max(1, num_threads / 2); // Allow more compute tasks
    int max_memory = std::max(1, num_threads / 3); // Limit memory tasks
    int max_io = std::max(1, num_threads / 4);     // Limit I/O tasks
    
    omp_set_num_threads(num_threads);
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            for (const auto& task : tasks) {
                // Wait if we have too many active tasks of this type
                switch (task.type) {
                    case TaskType::ComputeBound:
                        while (active_compute.load() >= max_compute) {
                            // Simple yield for OpenMP 2.0
                            #pragma omp flush
                            std::this_thread::sleep_for(std::chrono::milliseconds(1));
                        }
                        active_compute++;
                        break;
                    case TaskType::MemoryBound:
                        while (active_memory.load() >= max_memory) {
                            // Simple yield for OpenMP 2.0
                            #pragma omp flush
                            std::this_thread::sleep_for(std::chrono::milliseconds(1));
                        }
                        active_memory++;
                        break;
                    case TaskType::IOBound:
                        while (active_io.load() >= max_io) {
                            // Simple yield for OpenMP 2.0
                            #pragma omp flush
                            std::this_thread::sleep_for(std::chrono::milliseconds(1));
                        }
                        active_io++;
                        break;
                    default:
                        break; // No limit on mixed tasks
                }
                
                #pragma omp task
                {
                    execute_task(task);
                    
                    // Decrement the counter
                    switch (task.type) {
                        case TaskType::ComputeBound:
                            active_compute--;
                            break;
                        case TaskType::MemoryBound:
                            active_memory--;
                            break;
                        case TaskType::IOBound:
                            active_io--;
                            break;
                        default:
                            break;
                    }
                }
            }
        }
    }
}

//==============================================================================
// Performance Comparison Functions
//==============================================================================

// Compare different scheduling approaches
void compare_scheduling_approaches(const std::vector<HeterogeneousTask>& tasks, int num_threads) {
    struct SchedulingResult {
        std::string name;
        double execution_time;
    };
    
    std::vector<SchedulingResult> results;
    
    // Test different scheduling approaches
    auto test_approach = [&](const std::string& name, 
                            std::function<void(const std::vector<HeterogeneousTask>&, int)> approach) {
        recorder.reset();
        
        auto start_time = std::chrono::high_resolution_clock::now();
        approach(tasks, num_threads);
        auto end_time = std::chrono::high_resolution_clock::now();
        
        double execution_time = std::chrono::duration<double>(end_time - start_time).count();
        
        results.push_back({name, execution_time});
        recorder.print_statistics();
        
        return execution_time;
    };
    
// Run tests
    std::cout << "\n=== Comparing Scheduling Approaches ===" << std::endl;
    
    double naive_time = test_approach("Naive", execute_naive);
    [[maybe_unused]] double grouped_time = test_approach("Grouped", execute_grouped);
    [[maybe_unused]] double priority_time = test_approach("Priority", execute_priority);
    [[maybe_unused]] double binding_time = test_approach("Thread Binding", execute_thread_binding);
    [[maybe_unused]] double adaptive_time = test_approach("Adaptive", execute_adaptive);
    
    // Print summary
    std::cout << "\nPerformance Summary:" << std::endl;
    std::cout << "----------------------------------------------------------" << std::endl;
    std::cout << "Approach        | Execution Time (s) | Speedup vs. Naive" << std::endl;
    std::cout << "----------------------------------------------------------" << std::endl;
    
    for (const auto& result : results) {
        double speedup = naive_time / result.execution_time;
        std::cout << std::setw(15) << result.name << " | "
                  << std::fixed << std::setprecision(3) << std::setw(17) << result.execution_time << " | "
                  << std::fixed << std::setprecision(2) << std::setw(17) << speedup << "x" << std::endl;
    }
    
    // Visualize the results
    std::vector<double> execution_times;
    std::vector<std::string> approach_names;
    
    for (const auto& result : results) {
        execution_times.push_back(result.execution_time);
        approach_names.push_back(result.name);
    }
    
    task_utils::draw_ascii_bar_chart(execution_times, approach_names, 50, 
                                  "Execution Time by Approach (lower is better)");
    
    // Print recommendations
    std::cout << "\nRecommendations for heterogeneous task scheduling:" << std::endl;
    std::cout << "1. Use task grouping to prioritize critical tasks" << std::endl;
    std::cout << "2. Group similar tasks together to improve cache locality" << std::endl;
    std::cout << "3. Limit the number of concurrent memory-bound or I/O-bound tasks" << std::endl;
    std::cout << "4. Consider adaptive approaches that balance different task types" << std::endl;
    std::cout << "5. Use thread binding for NUMA systems or when tasks have specific resource requirements" << std::endl;
}

//==============================================================================
// Main Function
//==============================================================================

int main(int argc, char* argv[]) {
    // Parse command-line arguments
    int num_tasks = 40;
    int min_work = 50;
    int max_work = 200; // Reduced from 500 to be more reasonable
    int num_threads = omp_get_max_threads();
    int scheduling_type = 0;  // 0=compare all, 1=naive, 2=grouped, 3=priority, 4=binding, 5=adaptive
    
    if (argc > 1) num_tasks = std::min(atoi(argv[1]), 100); // Limit max tasks
    if (argc > 2) num_threads = std::min(atoi(argv[2]), 32); // Limit max threads
    if (argc > 3) scheduling_type = std::min(atoi(argv[3]), 5); // Ensure valid scheduling type
    if (argc > 4) min_work = std::min(std::max(10, atoi(argv[4])), 1000); // Keep work amount reasonable
    if (argc > 5) max_work = std::min(std::max(min_work, atoi(argv[5])), 1000); // Keep max work reasonable
    
    // Set the number of OpenMP threads
    omp_set_num_threads(num_threads);
    
    std::cout << "=== OpenMP Heterogeneous Tasks Example ===" << std::endl;
    std::cout << "Number of threads: " << num_threads << std::endl;
    std::cout << "Number of tasks: " << num_tasks << std::endl;
    std::cout << "Work range: " << min_work << "-" << max_work << std::endl;
    
    // Generate the tasks
    auto tasks = generate_tasks(num_tasks, min_work, max_work);
    
    // Reset the recorder at the start
    recorder.reset();
    
    // Count tasks by type
    int compute_tasks = 0, memory_tasks = 0, io_tasks = 0, mixed_tasks = 0;
    for (const auto& task : tasks) {
        switch (task.type) {
            case TaskType::ComputeBound: compute_tasks++; break;
            case TaskType::MemoryBound: memory_tasks++; break;
            case TaskType::IOBound: io_tasks++; break;
            case TaskType::Mixed: mixed_tasks++; break;
        }
    }
    
    std::cout << "Task distribution: " 
              << compute_tasks << " compute, "
              << memory_tasks << " memory, "
              << io_tasks << " I/O, "
              << mixed_tasks << " mixed" << std::endl;
    
    // Run the selected scheduling approach
    try {
        switch (scheduling_type) {
            case 1:  // Naive
                recorder.reset();
                execute_naive(tasks, num_threads);
                recorder.print_statistics();
                break;
            case 2:  // Grouped
                recorder.reset();
                execute_grouped(tasks, num_threads);
                recorder.print_statistics();
                break;
            case 3:  // Priority
                recorder.reset();
                execute_priority(tasks, num_threads);
                recorder.print_statistics();
                break;
            case 4:  // Thread binding
                recorder.reset();
                execute_thread_binding(tasks, num_threads);
                recorder.print_statistics();
                break;
            case 5:  // Adaptive
                recorder.reset();
                execute_adaptive(tasks, num_threads);
                recorder.print_statistics();
                break;
            default:  // Compare all approaches
                compare_scheduling_approaches(tasks, num_threads);
                break;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error during task execution: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}