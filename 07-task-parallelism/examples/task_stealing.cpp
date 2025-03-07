#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <chrono>
#include <random>
#include <omp.h>
#include "task_utils.h"

// Task execution recorder
class TaskExecutionRecorder {
private:
    struct TaskRecord {
        int task_id;
        int thread_id;
        int original_thread;
        double start_time;
        double end_time;
    };
    
    std::vector<TaskRecord> records;
    double start_time;
    
public:
    TaskExecutionRecorder() : start_time(omp_get_wtime()) {}
    
    void reset() {
        records.clear();
        start_time = omp_get_wtime();
    }
    
    void record_task(int task_id, int thread_id, int original_thread) {
        #pragma omp critical
        {
            TaskRecord record;
            record.task_id = task_id;
            record.thread_id = thread_id;
            record.original_thread = original_thread;
            record.start_time = omp_get_wtime() - start_time;
            record.end_time = -1;
            records.push_back(record);
        }
    }
    
    void complete_task(int task_id, int thread_id) {
        double end_time = omp_get_wtime() - start_time;
        #pragma omp critical
        {
            for (auto it = records.rbegin(); it != records.rend(); ++it) {
                if (it->task_id == task_id && it->thread_id == thread_id && it->end_time < 0) {
                    it->end_time = end_time;
                    break;
                }
            }
        }
    }
    
    // Count tasks that were stolen (executed by a different thread than their original thread)
    int count_stolen_tasks() const {
        int stolen_count = 0;
        for (const auto& record : records) {
            if (record.thread_id != record.original_thread) {
                stolen_count++;
            }
        }
        return stolen_count;
    }
    
    // Get the total number of tasks
    int count_total_tasks() const {
        return static_cast<int>(records.size());
    }
    
    void print_stealing_stats() const {
        int total_tasks = count_total_tasks();
        int stolen_tasks = count_stolen_tasks();
        
        std::cout << "Task Stealing Statistics:" << std::endl;
        std::cout << "Total tasks: " << total_tasks << std::endl;
        std::cout << "Stolen tasks: " << stolen_tasks << " (" 
                  << (total_tasks > 0 ? (stolen_tasks * 100.0 / total_tasks) : 0)
                  << "%)" << std::endl;
                  
        // Count tasks by thread
        std::map<int, int> tasks_by_thread;
        for (const auto& record : records) {
            tasks_by_thread[record.thread_id]++;
        }
        
        std::cout << "\nTask distribution by thread:" << std::endl;
        for (const auto& [thread_id, count] : tasks_by_thread) {
            std::cout << "Thread " << thread_id << ": " << count << " tasks" << std::endl;
        }
    }
};

// Global recorder
TaskExecutionRecorder recorder;

// Perform work for a task
void do_work(int task_id, int original_thread, int work_amount) {
    int thread_id = omp_get_thread_num();
    
    // Record task start
    recorder.record_task(task_id, thread_id, original_thread);
    
    // Do some computational work
    task_utils::do_compute_work(work_amount);
    
    // Record task completion
    recorder.complete_task(task_id, thread_id);
}

// Run a balanced workload
void balanced_workload(int num_threads) {
    recorder.reset();
    
    #pragma omp parallel num_threads(num_threads)
    {
        #pragma omp single
        {
            // Create equal number of tasks for each thread
            for (int t = 0; t < num_threads; ++t) {
                for (int i = 0; i < 10; ++i) {
                    int task_id = t * 100 + i;
                    
                    #pragma omp task
                    {
                        do_work(task_id, t, 1000000);
                    }
                }
            }
        }
    }
    
    std::cout << "\n=== Balanced Workload with " << num_threads << " threads ===" << std::endl;
    recorder.print_stealing_stats();
}

// Run an unbalanced workload
void unbalanced_workload(int num_threads) {
    // Reset the recorder
    recorder.reset();
    
    #pragma omp parallel num_threads(num_threads)
    {
        #pragma omp single
        {
            // Create tasks for each thread with flattened structure
            for (int t = 0; t < num_threads; ++t) {
                int parent_id = t * 100;
                
                // Create parent task
                #pragma omp task
                {
                    do_work(parent_id, t, 100); // Parent task
                }
                
                // Create child tasks separately
                for (int i = 1; i <= 5; ++i) {
                    int child_id = parent_id + i;
                    
                    #pragma omp task
                    {
                        do_work(child_id, t, 50); // Child task
                    }
                }
            }
            
            // Wait for all tasks to complete at the end
            #pragma omp taskwait
        }
    }
    
    std::cout << "\n=== Unbalanced Workload with " << num_threads << " threads ===" << std::endl;
    recorder.print_stealing_stats();
}

int main(int argc, char* argv[]) {
    int num_threads = 4;
    
    // Parse command line arguments
    if (argc > 1) {
        num_threads = std::stoi(argv[1]);
    }
    
    std::cout << "=== Task Stealing Example ===" << std::endl;
    std::cout << "Threads: " << num_threads << std::endl;
    
    // Run balanced workload
    balanced_workload(num_threads);
    
    // Run unbalanced workload
    unbalanced_workload(num_threads);
    
    return 0;
}