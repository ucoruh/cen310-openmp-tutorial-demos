#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <mutex>
#include <map>
#include <algorithm>
#include <omp.h>

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

// Global data for task tracking
struct TaskTracker {
    std::vector<TaskEvent> events;
    std::mutex tracker_mutex;
    double start_time;
    
    TaskTracker() : start_time(omp_get_wtime()) {}
    
    // Delete copy constructor and assignment operator
    TaskTracker(const TaskTracker&) = delete;
    TaskTracker& operator=(const TaskTracker&) = delete;
    
    void record_start(int task_id, int thread_id, std::string task_name = "") {
        std::lock_guard<std::mutex> lock(tracker_mutex);
        events.emplace_back(task_id, thread_id, omp_get_wtime() - start_time, task_name);
    }
    
    void record_end(int task_id, int thread_id) {
        std::lock_guard<std::mutex> lock(tracker_mutex);
        
        // Find the task event and update its end time
        for (auto it = events.rbegin(); it != events.rend(); ++it) {
            if (it->task_id == task_id && it->thread_id == thread_id && it->end_time == 0) {
                it->end_time = omp_get_wtime() - start_time;
                break;
            }
        }
    }
    
    void reset() {
        std::lock_guard<std::mutex> lock(tracker_mutex);
        events.clear();
        start_time = omp_get_wtime();
    }
};

// Global task tracker
TaskTracker task_tracker;

// Function to do some work for a task
void do_work(int task_id, std::string task_name, int duration_ms) {
    int thread_id = omp_get_thread_num();
    
    // Record task start
    task_tracker.record_start(task_id, thread_id, task_name);
    
    // Simulate work
    std::this_thread::sleep_for(std::chrono::milliseconds(duration_ms));
    
    // Record task end
    task_tracker.record_end(task_id, thread_id);
}

// Function to run a simple task example
void run_simple_tasks() {
    std::cout << "Running simple tasks example..." << std::endl;
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            for (int i = 0; i < 20; ++i) {
                #pragma omp task
                {
                    // Task duration varies to show load balancing
                    int duration = 50 + (i % 5) * 30;
                    do_work(i, "SimpleTask_" + std::to_string(i), duration);
                }
            }
        }
    }
}

// Function to run a dependency example using manual synchronization
void run_simulated_dependency_tasks() {
    std::cout << "Running simulated dependency tasks example..." << std::endl;
    std::cout << "Note: Using manual synchronization for dependencies (OpenMP 2.0 compatible)" << std::endl;
    
    // In OpenMP 2.0, we can't use task dependencies directly
    // So we'll use taskwait to simulate sequential phases
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            // Stage 1: Initialize data
            std::cout << "Stage 1: Initialization" << std::endl;
            for (int i = 0; i < 5; ++i) {
                #pragma omp task
                {
                    do_work(100 + i, "Init_" + std::to_string(i), 100);
                }
            }
            
            // Wait for all initialization tasks to complete
            #pragma omp taskwait
            
            // Stage 2: Process data
            std::cout << "Stage 2: Processing" << std::endl;
            for (int i = 0; i < 5; ++i) {
                #pragma omp task
                {
                    do_work(200 + i, "Process_" + std::to_string(i), 150);
                }
            }
            
            // Wait for all processing tasks to complete
            #pragma omp taskwait
            
            // Stage 3: Finalize
            std::cout << "Stage 3: Finalization" << std::endl;
            #pragma omp task
            {
                do_work(300, "Finalize", 200);
            }
        }
    }
}

// Function to run nested tasks example
void run_nested_tasks() {
    std::cout << "Running nested tasks example..." << std::endl;
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task
            {
                do_work(400, "Parent_1", 100);
                
                // Create nested tasks
                #pragma omp task
                {
                    do_work(401, "Child_1_1", 150);
                }
                
                #pragma omp task
                {
                    do_work(402, "Child_1_2", 120);
                    
                    // Create a deeper nested task
                    #pragma omp task
                    {
                        do_work(403, "Grandchild_1_2_1", 80);
                    }
                }
                
                #pragma omp taskwait
                do_work(404, "Parent_1_Continue", 50);
            }
            
            #pragma omp task
            {
                do_work(500, "Parent_2", 80);
                
                // Create nested tasks
                #pragma omp task
                {
                    do_work(501, "Child_2_1", 100);
                }
                
                #pragma omp task
                {
                    do_work(502, "Child_2_2", 120);
                }
            }
        }
    }
}

// Function to visualize task timelines using ASCII art
void visualize_task_timeline() {
    const int TIMELINE_WIDTH = 80;
    const int NUM_THREADS = omp_get_max_threads();
    
    // Find the maximum end time to scale the timeline
    double max_time = 0;
    for (const auto& event : task_tracker.events) {
        max_time = std::max(max_time, event.end_time);
    }
    
    // Scale factor to fit timeline into the desired width
    double scale = TIMELINE_WIDTH / max_time;
    
    std::cout << "\nTask Execution Timeline:" << std::endl;
    std::cout << "--------------------------" << std::endl;
    
    // Print the time scale
    std::cout << "Time (ms): ";
    for (int i = 0; i <= TIMELINE_WIDTH; i += 10) {
        std::cout << std::setw(9) << std::left << static_cast<int>(i / scale);
    }
    std::cout << std::endl;
    
    std::cout << "          ";
    for (int i = 0; i < TIMELINE_WIDTH; ++i) {
        if (i % 10 == 0) {
            std::cout << "|";
        } else if (i % 5 == 0) {
            std::cout << ":";
        } else {
            std::cout << ".";
        }
    }
    std::cout << std::endl;
    
    // Sort events by thread_id and start_time for better visualization
    std::vector<TaskEvent> sorted_events = task_tracker.events;
    std::sort(sorted_events.begin(), sorted_events.end(), 
              [](const TaskEvent& a, const TaskEvent& b) {
                  if (a.thread_id != b.thread_id) {
                      return a.thread_id < b.thread_id;
                  }
                  return a.start_time < b.start_time;
              });
    
    // Group events by thread
    std::map<int, std::vector<TaskEvent>> events_by_thread;
    for (const auto& event : sorted_events) {
        events_by_thread[event.thread_id].push_back(event);
    }
    
    // Generate the timeline for each thread
    for (int thread_id = 0; thread_id < NUM_THREADS; ++thread_id) {
        if (events_by_thread.count(thread_id) == 0) continue;
        
        std::cout << "Thread " << std::setw(2) << thread_id << ": ";
        
        std::vector<char> timeline(TIMELINE_WIDTH, ' ');
        std::vector<std::pair<int, int>> task_positions;
        
        // Place tasks on the timeline
        for (const auto& event : events_by_thread[thread_id]) {
            int start_pos = static_cast<int>(event.start_time * scale);
            int end_pos = static_cast<int>(event.end_time * scale);
            
            // Ensure positions are within bounds
            start_pos = std::min(start_pos, TIMELINE_WIDTH - 1);
            end_pos = std::min(end_pos, TIMELINE_WIDTH - 1);
            
            // Record the middle position for task ID
            int mid_pos = (start_pos + end_pos) / 2;
            task_positions.emplace_back(mid_pos, event.task_id);
            
            // Mark the task duration on the timeline
            for (int i = start_pos; i <= end_pos; ++i) {
                timeline[i] = '=';
            }
            
            // Mark the start and end
            if (start_pos < TIMELINE_WIDTH) timeline[start_pos] = '[';
            if (end_pos < TIMELINE_WIDTH) timeline[end_pos] = ']';
        }
        
        // Print the timeline
        for (char c : timeline) {
            std::cout << c;
        }
        std::cout << std::endl;
        
        // Print task IDs below the timeline
        std::cout << "          ";
        for (int i = 0; i < TIMELINE_WIDTH; ++i) {
            bool printed = false;
            for (const auto& pos : task_positions) {
                if (pos.first == i) {
                    std::cout << (pos.second % 10);
                    printed = true;
                    break;
                }
            }
            if (!printed) {
                std::cout << " ";
            }
        }
        std::cout << std::endl;
    }
    
    // Print task mapping
    std::cout << "\nTask Mapping:" << std::endl;
    std::cout << "--------------------------" << std::endl;
    
    std::map<int, std::string> task_names;
    for (const auto& event : task_tracker.events) {
        task_names[event.task_id] = event.task_name;
    }
    
    for (const auto& pair : task_names) {
        std::cout << "Task " << std::setw(3) << pair.first << ": " << pair.second << std::endl;
    }
}

// Function to visualize logical task dependencies based on timing
void visualize_task_dependencies() {
    // This is a simplified version that just shows potential dependencies
    // based on task start and end times
    
    std::cout << "\nPotential Task Dependencies:" << std::endl;
    std::cout << "--------------------------" << std::endl;
    
    // Sort events by start time
    std::vector<TaskEvent> sorted_events = task_tracker.events;
    std::sort(sorted_events.begin(), sorted_events.end(), 
              [](const TaskEvent& a, const TaskEvent& b) {
                  return a.start_time < b.start_time;
              });
    
    // Map to track the latest end time for each thread
    std::map<int, std::pair<double, int>> thread_last_task;
    
    // Analyze potential dependencies
    for (const auto& event : sorted_events) {
        // Print task info
        std::cout << "Task " << std::setw(3) << event.task_id 
                  << " (" << event.task_name << ")"
                  << " on Thread " << event.thread_id;
        
        // Check for potential dependencies (if a task started after another ended)
        std::vector<int> potential_deps;
        for (const auto& [thread_id, last] : thread_last_task) {
            if (event.start_time >= last.first) {
                potential_deps.push_back(last.second);
            }
        }
        
        if (!potential_deps.empty()) {
            std::cout << " may depend on: ";
            for (size_t i = 0; i < potential_deps.size(); ++i) {
                if (i > 0) std::cout << ", ";
                std::cout << potential_deps[i];
            }
        }
        std::cout << std::endl;
        
        // Update the last task for this thread
        thread_last_task[event.thread_id] = {event.end_time, event.task_id};
    }
}

// Function to visualize thread utilization
void visualize_thread_utilization() {
    const int NUM_THREADS = omp_get_max_threads();
    
    std::cout << "\nThread Utilization:" << std::endl;
    std::cout << "--------------------------" << std::endl;
    
    // Calculate the total execution time
    double total_time = 0;
    for (const auto& event : task_tracker.events) {
        total_time = std::max(total_time, event.end_time);
    }
    
    // Calculate thread busy times
    std::vector<double> thread_busy_time(NUM_THREADS, 0);
    
    for (const auto& event : task_tracker.events) {
        thread_busy_time[event.thread_id] += (event.end_time - event.start_time);
    }
    
    // Calculate and display utilization
    std::cout << "Thread | Busy Time (ms) | Utilization" << std::endl;
    std::cout << "------------------------------------" << std::endl;
    
    double total_busy_time = 0;
    for (int i = 0; i < NUM_THREADS; ++i) {
        double utilization = (thread_busy_time[i] / total_time) * 100.0;
        
        std::cout << std::setw(6) << i << " | "
                  << std::fixed << std::setprecision(2) << std::setw(13) << thread_busy_time[i] * 1000 << " | "
                  << std::fixed << std::setprecision(1) << std::setw(6) << utilization << "% |";
        
        // Visual representation of utilization
        int bar_length = static_cast<int>(utilization / 2);
        for (int j = 0; j < bar_length; ++j) {
            std::cout << "█";
        }
        std::cout << std::endl;
        
        total_busy_time += thread_busy_time[i];
    }
    
    // Calculate and display overall utilization
    double overall_utilization = (total_busy_time / (total_time * NUM_THREADS)) * 100.0;
    std::cout << "------------------------------------" << std::endl;
    std::cout << "Overall Utilization: " << std::fixed << std::setprecision(1) << overall_utilization << "%" << std::endl;
}

int main(int argc, char* argv[]) {
    // Parse command-line arguments
    int example_type = 0;  // 0=all, 1=simple, 2=dependency, 3=nested
    int num_threads = omp_get_max_threads();
    
    if (argc > 1) {
        example_type = atoi(argv[1]); // Using atoi instead of std::stoi
    }
    if (argc > 2) {
        num_threads = atoi(argv[2]); // Using atoi instead of std::stoi
    }
    
    omp_set_num_threads(num_threads);
    
    std::cout << "=== OpenMP Task Visualization Example ===" << std::endl;
    std::cout << "Note: This version is compatible with OpenMP 2.0" << std::endl;
    std::cout << "Number of threads: " << num_threads << std::endl;
    
    // Run the appropriate example
    switch (example_type) {
        case 1:
            task_tracker.reset();
            run_simple_tasks();
            break;
        case 2:
            task_tracker.reset();
            run_simulated_dependency_tasks();
            break;
        case 3:
            task_tracker.reset();
            run_nested_tasks();
            break;
        default:
            task_tracker.reset();
            run_simple_tasks();
            task_tracker.reset();
            run_simulated_dependency_tasks();
            task_tracker.reset();
            run_nested_tasks();
            break;
    }
    
    // Visualize the task execution
    visualize_task_timeline();
    visualize_task_dependencies();
    visualize_thread_utilization();
    
    return 0;
}