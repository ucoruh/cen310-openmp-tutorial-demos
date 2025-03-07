#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <algorithm>
#include <omp.h>
#include <chrono>
#include <thread>
#include "../include/synchronization_demos.h"
#include "../include/utils.h"

// Structure to track thread activity
struct ThreadEvent {
    double start_time;
    double end_time;
    std::string event_type;
    int thread_id;
    std::string description;
};

// Visualization class for thread timelines
class ThreadTimelineVisualizer {
private:
    std::vector<ThreadEvent> events;
    double start_time;
    int num_threads;
    
public:
    ThreadTimelineVisualizer(int num_threads) : num_threads(num_threads) {
        // Record start time
        start_time = omp_get_wtime();
    }
    
    // Add a thread event
    void add_event(int thread_id, const std::string& event_type, const std::string& description = "") {
        ThreadEvent event;
        event.start_time = omp_get_wtime() - start_time;
        event.end_time = 0; // Will be set when event ends
        event.thread_id = thread_id;
        event.event_type = event_type;
        event.description = description;
        
        #pragma omp critical(timeline_update)
        {
            events.push_back(event);
        }
    }
    
    // End the last event for a thread
    void end_event(int thread_id) {
        double end_time = omp_get_wtime() - start_time;
        
        #pragma omp critical(timeline_update)
        {
            // Find the last event for this thread
            for (int i = static_cast<int>(events.size()) - 1; i >= 0; i--) {
                if (events[i].thread_id == thread_id && events[i].end_time == 0) {
                    events[i].end_time = end_time;
                    break;
                }
            }
        }
    }
    
    // Display the timeline in the console
    void display() {
        // Sort events by start time
        std::sort(events.begin(), events.end(), [](const ThreadEvent& a, const ThreadEvent& b) {
            return a.start_time < b.start_time;
        });
        
        // Find the total time span
        double max_time = 0;
        for (const auto& event : events) {
            max_time = std::max(max_time, event.end_time);
        }
        
        // Set the width of the timeline display
        const int timeline_width = 80;
        const int label_width = 20;
        
        utils::print_section("Thread Execution Timeline");
        std::cout << "Each character represents approximately " 
                  << std::fixed << std::setprecision(2) 
                  << (max_time / (timeline_width - label_width)) << " seconds\n\n";
        
        // Create a timeline for each thread
        for (int t = 0; t < num_threads; t++) {
            std::string timeline(timeline_width - label_width, ' ');
            
            // Add events for this thread to the timeline
            for (const auto& event : events) {
                if (event.thread_id == t) {
                    int start_pos = static_cast<int>((event.start_time / max_time) * 
                                                   (timeline_width - label_width));
                    int end_pos = static_cast<int>((event.end_time / max_time) * 
                                                 (timeline_width - label_width));
                    
                    // Ensure valid positions
                    start_pos = std::min(std::max(start_pos, 0), 
                                        static_cast<int>(timeline.size()) - 1);
                    end_pos = std::min(std::max(end_pos, start_pos + 1), 
                                      static_cast<int>(timeline.size()));
                    
                    // Fill the timeline with a symbol based on event type
                    char symbol = '?';
                    if (event.event_type == "work") symbol = '#';
                    else if (event.event_type == "barrier") symbol = '|';
                    else if (event.event_type == "critical") symbol = 'C';
                    else if (event.event_type == "wait") symbol = '-';
                    else if (event.event_type == "idle") symbol = '.';
                    
                    for (int i = start_pos; i < end_pos; i++) {
                        timeline[i] = symbol;
                    }
                }
            }
            
            // Display the timeline for this thread
            std::cout << "Thread " << std::setw(2) << t << ": " << timeline << std::endl;
        }
        
        // Display legend
        std::cout << "\nLegend:\n";
        std::cout << "  # - Active work\n";
        std::cout << "  | - Barrier waiting\n";
        std::cout << "  C - Critical section\n";
        std::cout << "  - - Waiting for lock/resource\n";
        std::cout << "  . - Idle\n";
        
        // Display event details
        std::cout << "\nEvent Details:\n";
        std::cout << std::setw(5) << "Thread" << std::setw(10) << "Start" 
                  << std::setw(10) << "End" << std::setw(10) << "Duration" 
                  << "  " << "Type" << " - Description\n";
        
        for (const auto& event : events) {
            double duration = event.end_time - event.start_time;
            std::cout << std::setw(5) << event.thread_id 
                      << std::setw(10) << std::fixed << std::setprecision(4) << event.start_time 
                      << std::setw(10) << std::fixed << std::setprecision(4) << event.end_time 
                      << std::setw(10) << std::fixed << std::setprecision(4) << duration 
                      << "  " << event.event_type;
            
            if (!event.description.empty()) {
                std::cout << " - " << event.description;
            }
            
            std::cout << std::endl;
        }
    }
};

// Demonstrate thread timeline visualization for different synchronization mechanisms
void visualize_thread_timeline(int num_threads, int workload) {
    utils::print_section("Thread Timeline Visualization");
    std::cout << "This demo visualizes the timeline of thread execution with\n";
    std::cout << "different synchronization mechanisms\n\n";
    
    if (num_threads <= 0) {
        num_threads = omp_get_max_threads();
    }
    
    // Use a smaller workload for clearer visualization
    int reduced_workload = std::min(workload, 10);
    
    utils::print_result("Number of threads", static_cast<double>(num_threads));
    utils::print_result("Workload items", static_cast<double>(reduced_workload));
    
    // Create the visualizer
    ThreadTimelineVisualizer visualizer(num_threads);
    
    utils::print_subsection("Critical Section vs. Atomic Operations");
    std::cout << "Comparing execution timelines for critical sections and atomic operations\n\n";
    
    // Counter variables for demonstration
    int counter_critical = 0;
    int counter_atomic = 0;
    
    // First example: Critical Sections
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        
        // Each thread processes items
        for (int i = 0; i < reduced_workload; i++) {
            // Start work event
            visualizer.add_event(tid, "work", "Processing item " + std::to_string(i));
            
            // Simulate some processing
            std::this_thread::sleep_for(std::chrono::milliseconds(20 + tid * 5));
            
            // End work event
            visualizer.end_event(tid);
            
            // Enter critical section
            visualizer.add_event(tid, "critical", "Updating counter");
            
            #pragma omp critical
            {
                // Simulate critical section work
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                counter_critical++;
            }
            
            // End critical section event
            visualizer.end_event(tid);
        }
    }
    
    // Small pause between demos
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Second example: Atomic Operations
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        
        // Each thread processes items
        for (int i = 0; i < reduced_workload; i++) {
            // Start work event
            visualizer.add_event(tid, "work", "Processing item " + std::to_string(i));
            
            // Simulate some processing (same as before)
            std::this_thread::sleep_for(std::chrono::milliseconds(20 + tid * 5));
            
            // End work event
            visualizer.end_event(tid);
            
            // Atomic operation is much faster
            visualizer.add_event(tid, "critical", "Atomic update");
            
            #pragma omp atomic
            counter_atomic++;
            
            // Simulate a tiny delay for visualization purposes
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            
            // End atomic event
            visualizer.end_event(tid);
        }
    }
    
    // Small pause between demos
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Third example: Barrier Synchronization
    utils::print_subsection("Barrier Synchronization");
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        
        // First phase
        visualizer.add_event(tid, "work", "Phase 1 work");
        
        // Threads take varying time to complete phase 1
        std::this_thread::sleep_for(std::chrono::milliseconds(100 + tid * 50));
        
        visualizer.end_event(tid);
        
        // Barrier wait
        visualizer.add_event(tid, "barrier", "Waiting at barrier 1");
        
        #pragma omp barrier
        
        visualizer.end_event(tid);
        
        // Second phase
        visualizer.add_event(tid, "work", "Phase 2 work");
        
        // Reverse the timing pattern for phase 2
        std::this_thread::sleep_for(std::chrono::milliseconds(100 + (num_threads - tid) * 50));
        
        visualizer.end_event(tid);
    }
    
    // Display the timeline
    visualizer.display();
}
