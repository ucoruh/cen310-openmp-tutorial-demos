#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <map>
#include <omp.h>
#include <algorithm>
#include <random>
#include <thread>
#include <mutex>
#include <atomic>
#include <ctime>
#include <cstring>
#include <Windows.h>

#include "profiler.h"
#include "debug_utils.h"
#include "cli_parser.h"

// ThreadEvent represents a single event in the timeline
struct ThreadEvent {
    int threadId;
    std::string eventType;  // "start", "end", "sync", "idle", "work"
    std::chrono::steady_clock::time_point timestamp;
    std::string description;
    double duration; // in milliseconds
};

// Timeline class to track and visualize thread execution
class ThreadTimeline {
private:
    std::vector<ThreadEvent> events;
    std::chrono::steady_clock::time_point startTime;
    std::chrono::steady_clock::time_point endTime;
    std::map<int, std::vector<ThreadEvent>> threadEvents;
    std::mutex eventsMutex;

public:
    ThreadTimeline() {
        startTime = std::chrono::steady_clock::now();
    }

    void recordEvent(int threadId, const std::string& eventType, 
                    const std::string& description, double duration = 0.0) {
        ThreadEvent event;
        event.threadId = threadId;
        event.eventType = eventType;
        event.timestamp = std::chrono::steady_clock::now();
        event.description = description;
        event.duration = duration;
        
        std::lock_guard<std::mutex> lock(eventsMutex);
        events.push_back(event);
        threadEvents[threadId].push_back(event);
    }

    void finalize() {
        endTime = std::chrono::steady_clock::now();
    }

    void generateHTMLReport(const std::string& filename) {
        std::ofstream htmlFile(filename);
        if (!htmlFile.is_open()) {
            std::cerr << "Failed to open file for writing: " << filename << std::endl;
            return;
        }

        // Get total execution time in milliseconds
        auto totalDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
            endTime - startTime).count();

        // HTML header and styling
        htmlFile << "<!DOCTYPE html>\n"
                 << "<html lang=\"en\">\n"
                 << "<head>\n"
                 << "    <meta charset=\"UTF-8\">\n"
                 << "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
                 << "    <title>OpenMP Thread Timeline Visualization</title>\n"
                 << "    <style>\n"
                 << "        body { font-family: Arial, sans-serif; margin: 20px; }\n"
                 << "        .timeline-container { margin-top: 20px; }\n"
                 << "        .thread-row { height: 40px; margin-bottom: 5px; position: relative; }\n"
                 << "        .thread-label { position: absolute; left: 0; width: 80px; height: 40px; }\n"
                 << "        .timeline { position: absolute; left: 100px; right: 0; height: 40px; }\n"
                 << "        .timeline-scale { height: 20px; border-bottom: 1px solid #ccc; position: relative; margin-bottom: 10px; margin-left: 100px; }\n"
                 << "        .timeline-marker { position: absolute; width: 1px; height: 5px; background: #888; bottom: 0; }\n"
                 << "        .timeline-marker-label { position: absolute; font-size: 10px; color: #888; }\n"
                 << "        .event { position: absolute; height: 30px; border-radius: 4px; top: 5px; }\n"
                 << "        .event-start { background-color: #4CAF50; }\n"
                 << "        .event-end { background-color: #F44336; }\n"
                 << "        .event-sync { background-color: #FF9800; }\n"
                 << "        .event-idle { background-color: #BDBDBD; }\n"
                 << "        .event-work { background-color: #2196F3; }\n"
                 << "        .tooltip { display: none; position: absolute; background: #333; color: #fff; padding: 5px; border-radius: 3px; z-index: 100; font-size: 12px; }\n"
                 << "        h1 { color: #333; }\n"
                 << "        .summary { margin: 20px 0; }\n"
                 << "        .legend { display: flex; margin-top: 20px; }\n"
                 << "        .legend-item { display: flex; align-items: center; margin-right: 20px; }\n"
                 << "        .legend-color { width: 20px; height: 20px; margin-right: 5px; border-radius: 3px; }\n"
                 << "    </style>\n"
                 << "</head>\n"
                 << "<body>\n"
                 << "    <h1>OpenMP Thread Timeline Visualization</h1>\n"
                 << "    <div class=\"summary\">\n"
                 << "        <p>Total execution time: " << totalDuration << " ms</p>\n"
                 << "        <p>Number of threads: " << threadEvents.size() << "</p>\n"
                 << "    </div>\n";

        // Create scale
        htmlFile << "    <div class=\"timeline-scale\">\n";
        
        // Add scale markers every 100ms
        for (int i = 0; i <= totalDuration; i += 100) {
            double position = (i / (double)totalDuration) * 100;
            htmlFile << "        <div class=\"timeline-marker\" style=\"left: " << position << "%;\"></div>\n";
            htmlFile << "        <div class=\"timeline-marker-label\" style=\"left: " << position << "%\">" << i << " ms</div>\n";
        }
        
        htmlFile << "    </div>\n";

        // Create timeline container
        htmlFile << "    <div class=\"timeline-container\">\n";

        // Generate a row for each thread
        for (const auto& thread : threadEvents) {
            int threadId = thread.first;
            
            htmlFile << "        <div class=\"thread-row\">\n";
            htmlFile << "            <div class=\"thread-label\">Thread " << threadId << "</div>\n";
            htmlFile << "            <div class=\"timeline\">\n";

            // Plot events for this thread
            for (const auto& event : thread.second) {
                auto eventTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                    event.timestamp - startTime).count();
                
                double position = (eventTime / (double)totalDuration) * 100;
                double width = (event.duration / (double)totalDuration) * 100;
                
                // Minimum width for visibility
                if (width < 0.5) width = 0.5;
                
                htmlFile << "                <div class=\"event event-" << event.eventType 
                         << "\" style=\"left: " << position << "%; width: " << width << "%;\" "
                         << "onmouseover=\"showTooltip(event, '" << event.description << " (" << event.duration << " ms)')\" "
                         << "onmouseout=\"hideTooltip()\"></div>\n";
            }
            
            htmlFile << "            </div>\n";
            htmlFile << "        </div>\n";
        }

        // Close timeline container
        htmlFile << "    </div>\n";

        // Add legend
        htmlFile << "    <div class=\"legend\">\n";
        htmlFile << "        <div class=\"legend-item\"><div class=\"legend-color event-start\"></div>Start</div>\n";
        htmlFile << "        <div class=\"legend-item\"><div class=\"legend-color event-end\"></div>End</div>\n";
        htmlFile << "        <div class=\"legend-item\"><div class=\"legend-color event-sync\"></div>Synchronization</div>\n";
        htmlFile << "        <div class=\"legend-item\"><div class=\"legend-color event-idle\"></div>Idle</div>\n";
        htmlFile << "        <div class=\"legend-item\"><div class=\"legend-color event-work\"></div>Work</div>\n";
        htmlFile << "    </div>\n";

        // Add tooltip and JavaScript
        htmlFile << "    <div class=\"tooltip\" id=\"tooltip\"></div>\n"
                 << "    <script>\n"
                 << "        function showTooltip(event, text) {\n"
                 << "            const tooltip = document.getElementById('tooltip');\n"
                 << "            tooltip.innerHTML = text;\n"
                 << "            tooltip.style.display = 'block';\n"
                 << "            tooltip.style.left = (event.pageX + 10) + 'px';\n"
                 << "            tooltip.style.top = (event.pageY + 10) + 'px';\n"
                 << "        }\n"
                 << "        function hideTooltip() {\n"
                 << "            document.getElementById('tooltip').style.display = 'none';\n"
                 << "        }\n"
                 << "    </script>\n";

        // Close HTML
        htmlFile << "</body>\n</html>\n";

        htmlFile.close();
        std::cout << "HTML timeline visualization generated: " << filename << std::endl;
    }
};

// Global timeline object for OpenMP threads
ThreadTimeline timeline;

// Demo workload for visualization
void simulateWorkload() {
    const int numThreads = omp_get_max_threads();
    std::vector<int> workDistribution(numThreads);
    
    // Create imbalanced workloads for demonstration
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(50, 500);
    
    for (int i = 0; i < numThreads; ++i) {
        workDistribution[i] = dist(rng);
    }
    
    // Begin the parallel region
    timeline.recordEvent(0, "start", "Starting parallel region", 0);
    
    #pragma omp parallel
    {
        int threadId = omp_get_thread_num();
        int threadWork = workDistribution[threadId];
        
        // Record thread start
        timeline.recordEvent(threadId, "start", "Thread started", 0);
        
        // Simulate some initial work
        auto workStart = std::chrono::steady_clock::now();
        std::this_thread::sleep_for(std::chrono::milliseconds(threadWork / 4));
        auto workEnd = std::chrono::steady_clock::now();
        
        double workDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
            workEnd - workStart).count();
        timeline.recordEvent(threadId, "work", "Initial work", workDuration);
        
        // Simulate a synchronization point
        #pragma omp barrier
        timeline.recordEvent(threadId, "sync", "Barrier synchronization", 0);
        
        // Simulate threads waiting for slower ones (imbalance)
        if (threadId % 2 == 0) {
            auto idleStart = std::chrono::steady_clock::now();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            auto idleEnd = std::chrono::steady_clock::now();
            
            double idleDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
                idleEnd - idleStart).count();
            timeline.recordEvent(threadId, "idle", "Waiting for work", idleDuration);
        }
        
        // Simulate more work after synchronization
        #pragma omp for schedule(dynamic)
        for (int i = 0; i < numThreads * 10; ++i) {
            auto taskStart = std::chrono::steady_clock::now();
            std::this_thread::sleep_for(std::chrono::milliseconds(10 + (i % 30)));
            auto taskEnd = std::chrono::steady_clock::now();
            
            double taskDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
                taskEnd - taskStart).count();
            
            std::stringstream desc;
            desc << "Task " << i << " execution";
            timeline.recordEvent(threadId, "work", desc.str(), taskDuration);
        }
        
        // Record thread end
        timeline.recordEvent(threadId, "end", "Thread completed", 0);
    }
    
    // Record parallel region end
    timeline.recordEvent(0, "end", "Parallel region completed", 0);
}

int main(int argc, char* argv[]) {
    CliParser parser(argc, argv);
    parser.addOption("threads", 't', "Number of threads to use (default: system cores)", true);
    parser.addOption("output", 'o', "Output HTML file name (default: timeline.html)", true);
    parser.parse();

    // Set the number of threads
    int numThreads = parser.getIntValue("threads", omp_get_num_procs());
    omp_set_num_threads(numThreads);
    
    std::string outputFile = parser.getStringValue("output", "timeline.html");
    
    std::cout << "OpenMP Thread Timeline Visualizer" << std::endl;
    std::cout << "=================================" << std::endl;
    std::cout << "Running with " << numThreads << " threads" << std::endl;
    std::cout << "Output will be saved to: " << outputFile << std::endl;
    std::cout << std::endl;
    
    // Run the simulation
    simulateWorkload();
    
    // Finalize and generate the HTML report
    timeline.finalize();
    
    std::string htmlPath = "../reports/" + outputFile;
    
    // Ensure reports directory exists
    std::string reportsDir = "../reports";
    CreateDirectoryA(reportsDir.c_str(), NULL);
    
    timeline.generateHTMLReport(htmlPath);
    
    std::cout << "\nVisualization complete! Open " << htmlPath << " in a web browser to view the results." << std::endl;
    
    return 0;
} 