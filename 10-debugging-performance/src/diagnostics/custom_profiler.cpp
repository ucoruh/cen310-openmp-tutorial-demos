#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <mutex>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <map>
#include <algorithm>
#include <memory>
#include <limits>
#include <omp.h>
#include <Windows.h>
#include <Pdh.h>
#include <PdhMsg.h>

// Undefine Windows macros that conflict with C++ standard library
#undef min
#undef max

#include "profiler.h"
#include "debug_utils.h"
#include "cli_parser.h"

#pragma comment(lib, "pdh.lib")

// Define chart colors
const std::vector<std::string> colors = {
    "rgba(75, 192, 192, 1)",
    "rgba(255, 99, 132, 1)",
    "rgba(54, 162, 235, 1)",
    "rgba(255, 206, 86, 1)",
    "rgba(153, 102, 255, 1)",
    "rgba(255, 159, 64, 1)",
    "rgba(199, 199, 199, 1)"
};

// Custom Timer implementation to measure code segments
class ScopedTimer {
private:
    const std::string name;
    std::chrono::high_resolution_clock::time_point startTime;
    bool finished;
    int threadId;

public:
    ScopedTimer(const std::string& timerName) 
        : name(timerName), 
          startTime(std::chrono::high_resolution_clock::now()), 
          finished(false),
          threadId(omp_get_thread_num()) {
        
        // Register the start event
        Profiler::getInstance().startEvent(name, threadId);
    }

    ~ScopedTimer() {
        if (!finished) {
            stop();
        }
    }

    void stop() {
        if (!finished) {
            auto endTime = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
            
            // Register the end event
            Profiler::getInstance().endEvent(name, threadId, duration);
            
            finished = true;
        }
    }
};

// Performance counters for hardware metrics
class PerformanceCounters {
private:
    PDH_HQUERY queryHandle;
    std::map<std::string, PDH_HCOUNTER> counters;
    bool initialized;

public:
    PerformanceCounters() : initialized(false) {
        initialize();
    }

    ~PerformanceCounters() {
        if (initialized) {
            PdhCloseQuery(queryHandle);
        }
    }

    bool initialize() {
        PDH_STATUS status = PdhOpenQuery(nullptr, 0, &queryHandle);
        if (status != ERROR_SUCCESS) {
            std::cerr << "Failed to open PDH query. Error: " << status << std::endl;
            return false;
        }

        // Add common performance counters
        addCounter("Processor", "% Processor Time", "_Total");
        addCounter("Memory", "Available MBytes");
        addCounter("PhysicalDisk", "% Disk Time", "_Total");
        addCounter("PhysicalDisk", "Avg. Disk Queue Length", "_Total");
        
        // Start collecting data
        status = PdhCollectQueryData(queryHandle);
        if (status != ERROR_SUCCESS) {
            std::cerr << "Failed to collect initial data. Error: " << status << std::endl;
            PdhCloseQuery(queryHandle);
            return false;
        }

        initialized = true;
        return true;
    }

    bool addCounter(const std::string& object, const std::string& counter, const std::string& instance = "") {
        if (!initialized) return false;
        
        std::string counterPath = "\\";
        counterPath += object + "\\";
        counterPath += counter;
        
        if (!instance.empty()) {
            counterPath += "\\";
            counterPath += instance;
        }
        
        PDH_HCOUNTER counterHandle;
        // Use the ANSI version of PdhAddEnglishCounter
        PDH_STATUS status = PdhAddEnglishCounterA(queryHandle, counterPath.c_str(), 0, &counterHandle);
        
        if (status != ERROR_SUCCESS) {
            std::cerr << "Failed to add counter: " << counterPath << ". Error: " << status << std::endl;
            return false;
        }
        
        // Create a shorter name for this counter
        std::string shortName = object + "." + counter;
        if (!instance.empty()) {
            shortName += "." + instance;
        }
        
        counters[shortName] = counterHandle;
        return true;
    }

    bool collectData() {
        if (!initialized) return false;
        
        PDH_STATUS status = PdhCollectQueryData(queryHandle);
        return (status == ERROR_SUCCESS);
    }

    double getCounterValue(const std::string& name) {
        if (!initialized || counters.find(name) == counters.end()) {
            return -1.0;
        }
        
        PDH_FMT_COUNTERVALUE value;
        PDH_STATUS status = PdhGetFormattedCounterValue(counters[name], PDH_FMT_DOUBLE, nullptr, &value);
        
        if (status != ERROR_SUCCESS) {
            std::cerr << "Failed to get counter value for: " << name << ". Error: " << status << std::endl;
            return -1.0;
        }
        
        return value.doubleValue;
    }

    std::map<std::string, double> getAllCounterValues() {
        std::map<std::string, double> result;
        if (!initialized) return result;
        
        collectData();
        
        for (const auto& counter : counters) {
            result[counter.first] = getCounterValue(counter.first);
        }
        
        return result;
    }
};

// Implementation of the ProfilerImpl class
class Profiler::ProfilerImpl {
private:
    struct ProfileEvent {
        std::string name;
        int threadId;
        std::chrono::high_resolution_clock::time_point startTime;
        std::chrono::high_resolution_clock::time_point endTime;
        double duration; // in microseconds
        bool complete;
    };

    struct ThreadMetrics {
        int threadId;
        double totalTime; // in microseconds
        int eventCount;
        double maxDuration;
        double minDuration;
        std::map<std::string, double> eventTotals;
    };

    std::vector<ProfileEvent> events;
    std::map<int, std::map<std::string, std::chrono::high_resolution_clock::time_point>> activeEvents;
    std::mutex eventsMutex;
    std::chrono::high_resolution_clock::time_point profilingStartTime;
    std::shared_ptr<PerformanceCounters> perfCounters;
    std::vector<std::map<std::string, double>> systemMetricSamples;
    bool collectingSystemMetrics;
    std::thread metricCollectionThread;

public:
    ProfilerImpl() : 
        profilingStartTime(std::chrono::high_resolution_clock::now()),
        perfCounters(std::make_shared<PerformanceCounters>()),
        collectingSystemMetrics(false) {}

    ~ProfilerImpl() {
        stopSystemMetricCollection();
    }

    void startEvent(const std::string& name, int threadId) {
        std::lock_guard<std::mutex> lock(eventsMutex);
        activeEvents[threadId][name] = std::chrono::high_resolution_clock::now();
    }

    void endEvent(const std::string& name, int threadId, double durationMicros) {
        std::lock_guard<std::mutex> lock(eventsMutex);
        
        auto now = std::chrono::high_resolution_clock::now();
        
        // Check if the event was started
        auto& threadEvents = activeEvents[threadId];
        auto it = threadEvents.find(name);
        
        if (it != threadEvents.end()) {
            // Calculate duration
            auto start = it->second;
            double calculatedDuration = 0.0;
            
            if (durationMicros <= 0) {
                calculatedDuration = std::chrono::duration_cast<std::chrono::microseconds>(now - start).count();
            } else {
                calculatedDuration = durationMicros;
            }
            
            // Create event
            ProfileEvent event;
            event.name = name;
            event.threadId = threadId;
            event.startTime = start;
            event.endTime = now;
            event.duration = calculatedDuration;
            event.complete = true;
            
            events.push_back(event);
            
            // Remove from active events
            threadEvents.erase(it);
        }
    }

    void startSystemMetricCollection(int intervalMs = 500) {
        if (collectingSystemMetrics) return;
        
        collectingSystemMetrics = true;
        metricCollectionThread = std::thread([this, intervalMs]() {
            while (collectingSystemMetrics) {
                auto metrics = perfCounters->getAllCounterValues();
                {
                    std::lock_guard<std::mutex> lock(eventsMutex);
                    systemMetricSamples.push_back(metrics);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
            }
        });
    }

    void stopSystemMetricCollection() {
        if (!collectingSystemMetrics) return;
        
        collectingSystemMetrics = false;
        if (metricCollectionThread.joinable()) {
            metricCollectionThread.join();
        }
    }

    std::map<int, ThreadMetrics> getThreadMetrics() {
        std::map<int, ThreadMetrics> result;
        
        std::lock_guard<std::mutex> lock(eventsMutex);
        
        for (const auto& event : events) {
            if (!event.complete) continue;
            
            int threadId = event.threadId;
            
            if (result.find(threadId) == result.end()) {
                ThreadMetrics metrics;
                metrics.threadId = threadId;
                metrics.totalTime = 0;
                metrics.eventCount = 0;
                metrics.maxDuration = 0;
                metrics.minDuration = std::numeric_limits<double>::max();
                result[threadId] = metrics;
            }
            
            auto& metrics = result[threadId];
            metrics.totalTime += event.duration;
            metrics.eventCount++;
            metrics.maxDuration = std::max(metrics.maxDuration, event.duration);
            metrics.minDuration = std::min(metrics.minDuration, event.duration);
            
            metrics.eventTotals[event.name] += event.duration;
        }
        
        return result;
    }

    void generateReport(const std::string& filename) {
        std::ofstream report(filename);
        if (!report.is_open()) {
            std::cerr << "Failed to open file for writing: " << filename << std::endl;
            return;
        }
        
        auto threadMetrics = getThreadMetrics();
        auto totalDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - profilingStartTime).count();
        
        // HTML report header
        report << "<!DOCTYPE html>\n"
               << "<html>\n"
               << "<head>\n"
               << "    <title>OpenMP Custom Profiler Report</title>\n"
               << "    <style>\n"
               << "        body { font-family: Arial, sans-serif; margin: 20px; }\n"
               << "        h1, h2, h3 { color: #333; }\n"
               << "        .summary { margin: 20px 0; }\n"
               << "        table { border-collapse: collapse; margin: 20px 0; width: 100%; }\n"
               << "        th, td { padding: 8px; text-align: left; border: 1px solid #ddd; }\n"
               << "        th { background-color: #4CAF50; color: white; }\n"
               << "        tr:nth-child(even) { background-color: #f2f2f2; }\n"
               << "        .chart-container { margin: 20px 0; width: 800px; height: 400px; }\n"
               << "    </style>\n"
               << "    <script src=\"https://cdn.jsdelivr.net/npm/chart.js\"></script>\n"
               << "</head>\n"
               << "<body>\n"
               << "    <h1>OpenMP Custom Profiler Report</h1>\n";
        
        // Summary section
        report << "    <div class=\"summary\">\n"
               << "        <h2>Summary</h2>\n"
               << "        <p>Total profiling time: " << totalDuration << " ms</p>\n"
               << "        <p>Number of threads: " << threadMetrics.size() << "</p>\n"
               << "        <p>Total events recorded: " << events.size() << "</p>\n"
               << "    </div>\n";
        
        // Thread metrics section
        report << "    <h2>Thread Metrics</h2>\n"
               << "    <table>\n"
               << "        <tr>\n"
               << "            <th>Thread ID</th>\n"
               << "            <th>Total Time (ms)</th>\n"
               << "            <th>Event Count</th>\n"
               << "            <th>Max Duration (ms)</th>\n"
               << "            <th>Min Duration (ms)</th>\n"
               << "            <th>Avg Duration (ms)</th>\n"
               << "        </tr>\n";
        
        for (const auto& [threadId, metrics] : threadMetrics) {
            double totalTimeMs = metrics.totalTime / 1000.0;
            double maxDurationMs = metrics.maxDuration / 1000.0;
            double minDurationMs = metrics.minDuration / 1000.0;
            double avgDurationMs = (metrics.eventCount > 0) ? (totalTimeMs / metrics.eventCount) : 0;
            
            report << "        <tr>\n"
                   << "            <td>" << threadId << "</td>\n"
                   << "            <td>" << std::fixed << std::setprecision(2) << totalTimeMs << "</td>\n"
                   << "            <td>" << metrics.eventCount << "</td>\n"
                   << "            <td>" << std::fixed << std::setprecision(2) << maxDurationMs << "</td>\n"
                   << "            <td>" << std::fixed << std::setprecision(2) << minDurationMs << "</td>\n"
                   << "            <td>" << std::fixed << std::setprecision(2) << avgDurationMs << "</td>\n"
                   << "        </tr>\n";
        }
        
        report << "    </table>\n";
        
        // Event breakdown section
        report << "    <h2>Event Breakdown by Thread</h2>\n";
        
        for (const auto& [threadId, metrics] : threadMetrics) {
            report << "    <h3>Thread " << threadId << " Events</h3>\n"
                   << "    <table>\n"
                   << "        <tr>\n"
                   << "            <th>Event Name</th>\n"
                   << "            <th>Total Time (ms)</th>\n"
                   << "            <th>Percentage</th>\n"
                   << "        </tr>\n";
            
            for (const auto& [eventName, duration] : metrics.eventTotals) {
                double durationMs = duration / 1000.0;
                double percentage = (metrics.totalTime > 0) ? ((duration / metrics.totalTime) * 100.0) : 0;
                
                report << "        <tr>\n"
                       << "            <td>" << eventName << "</td>\n"
                       << "            <td>" << std::fixed << std::setprecision(2) << durationMs << "</td>\n"
                       << "            <td>" << std::fixed << std::setprecision(1) << percentage << "%</td>\n"
                       << "        </tr>\n";
            }
            
            report << "    </table>\n";
            
            // Create pie chart for this thread
            report << "    <div class=\"chart-container\">\n"
                   << "        <canvas id=\"threadChart" << threadId << "\"></canvas>\n"
                   << "    </div>\n"
                   << "    <script>\n"
                   << "        new Chart(document.getElementById('threadChart" << threadId << "'), {\n"
                   << "            type: 'pie',\n"
                   << "            data: {\n"
                   << "                labels: [";
            
            bool first = true;
            for (const auto& [eventName, duration] : metrics.eventTotals) {
                if (!first) report << ", ";
                report << "'" << eventName << "'";
                first = false;
            }
            
            report << "],\n"
                   << "                datasets: [{\n"
                   << "                    data: [";
            
            first = true;
            for (const auto& [eventName, duration] : metrics.eventTotals) {
                if (!first) report << ", ";
                report << (duration / 1000.0);
                first = false;
            }
            
            report << "],\n"
                   << "                    backgroundColor: [";
            
            first = true;
            int colorIndex = 0;
            for (const auto& [eventName, duration] : metrics.eventTotals) {
                if (!first) report << ", ";
                report << colors[colorIndex % colors.size()];
                first = false;
                colorIndex++;
            }
            
            report << "]\n"
                   << "                }]\n"
                   << "            },\n"
                   << "            options: {\n"
                   << "                responsive: true,\n"
                   << "                plugins: {\n"
                   << "                    legend: { position: 'right' },\n"
                   << "                    title: {\n"
                   << "                        display: true,\n"
                   << "                        text: 'Thread " << threadId << " Time Distribution (ms)'\n"
                   << "                    }\n"
                   << "                }\n"
                   << "            }\n"
                   << "        });\n"
                   << "    </script>\n";
        }
        
        // System metrics section
        if (!systemMetricSamples.empty()) {
            report << "    <h2>System Performance Metrics</h2>\n"
                   << "    <div class=\"chart-container\">\n"
                   << "        <canvas id=\"systemMetricsChart\"></canvas>\n"
                   << "    </div>\n"
                   << "    <script>\n"
                   << "        new Chart(document.getElementById('systemMetricsChart'), {\n"
                   << "            type: 'line',\n"
                   << "            data: {\n"
                   << "                labels: [";
            
            for (size_t i = 0; i < systemMetricSamples.size(); i++) {
                if (i > 0) report << ", ";
                report << i;
            }
            
            report << "],\n"
                   << "                datasets: [";
            
            // Assume all samples have the same metrics
            if (!systemMetricSamples.empty()) {
                bool firstMetric = true;
                
                for (const auto& [metricName, value] : systemMetricSamples[0]) {
                    if (!firstMetric) report << ", ";
                    
                    report << "{\n"
                           << "                    label: '" << metricName << "',\n"
                           << "                    data: [";
                    
                    bool firstValue = true;
                    for (const auto& sample : systemMetricSamples) {
                        if (!firstValue) report << ", ";
                        auto it = sample.find(metricName);
                        double val = (it != sample.end()) ? it->second : 0.0;
                        report << val;
                        firstValue = false;
                    }
                    
                    int colorIndex = firstMetric ? 0 : 1;
                    report << "],\n"
                           << "                    borderColor: '" << colors[colorIndex % colors.size()] << "',\n"
                           << "                    fill: false\n"
                           << "                }";
                    
                    firstMetric = false;
                }
            }
            
            report << "]\n"
                   << "            },\n"
                   << "            options: {\n"
                   << "                responsive: true,\n"
                   << "                plugins: {\n"
                   << "                    legend: { position: 'top' },\n"
                   << "                    title: {\n"
                   << "                        display: true,\n"
                   << "                        text: 'System Performance Metrics'\n"
                   << "                    }\n"
                   << "                }\n"
                   << "            }\n"
                   << "        });\n"
                   << "    </script>\n";
        }
        
        // Footer
        report << "</body>\n</html>\n";
        
        report.close();
        std::cout << "Profiler report generated: " << filename << std::endl;
    }
};

// Initialize the global Profiler instance
Profiler& Profiler::getInstance() {
    static Profiler instance;
    return instance;
}

// Profiler implementation
Profiler::Profiler() : m_nextId(0), m_enabled(true), impl(new ProfilerImpl()) {
}

Profiler::~Profiler() {
    delete impl;
}

void Profiler::startEvent(const std::string& name, int threadId) {
    if (impl) {
        impl->startEvent(name, threadId);
    }
}

void Profiler::endEvent(const std::string& name, int threadId, double durationMicros) {
    if (impl) {
        impl->endEvent(name, threadId, durationMicros);
    }
}

void Profiler::startSystemMetricCollection(int intervalMs) {
    if (impl) {
        impl->startSystemMetricCollection(intervalMs);
    }
}

void Profiler::stopSystemMetricCollection() {
    if (impl) {
        impl->stopSystemMetricCollection();
    }
}

void Profiler::generateReport(const std::string& filename) {
    if (impl) {
        impl->generateReport(filename);
    }
}

// Example workload to demonstrate custom profiling
void demoWorkload() {
    const int numThreads = omp_get_max_threads();
    
    // Start collecting system metrics
    Profiler::getInstance().startSystemMetricCollection(1000);
    
    std::cout << "Running demo workload with " << numThreads << " threads..." << std::endl;
    
    // Perform a computationally intensive task
    ScopedTimer mainTimer("TotalExecution");
    
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        // Simulate different work per thread
        {
            ScopedTimer timer("ThreadInitialization");
            std::this_thread::sleep_for(std::chrono::milliseconds(50 + tid * 10));
        }
        
        #pragma omp barrier
        
        // Parallel loop with different work patterns
        #pragma omp for schedule(dynamic)
        for (int i = 0; i < 100; i++) {
            ScopedTimer loopTimer("ProcessingItem");
            
            // Different processing times based on item and thread
            int processingTime = 10 + (i % 20) + (tid % 4) * 5;
            std::this_thread::sleep_for(std::chrono::milliseconds(processingTime));
            
            // Some additional work for certain items
            if (i % 10 == 0) {
                ScopedTimer extraTimer("ExtraProcessing");
                std::this_thread::sleep_for(std::chrono::milliseconds(25));
            }
        }
        
        // Thread finalization
        {
            ScopedTimer timer("ThreadFinalization");
            std::this_thread::sleep_for(std::chrono::milliseconds(30 + (numThreads - tid) * 5));
        }
    }
    
    // Stop system metric collection
    Profiler::getInstance().stopSystemMetricCollection();
}

int main(int argc, char* argv[]) {
    CliParser parser(argc, argv);
    parser.addOption("threads", 't', "Number of threads to use (default: system cores)", true);
    parser.addOption("output", 'o', "Output HTML report file name (default: profiler_report.html)", true);
    parser.parse();

    // Set parameters
    int numThreads = parser.getIntValue("threads", omp_get_num_procs());
    std::string outputFile = parser.getStringValue("output", "profiler_report.html");
    
    omp_set_num_threads(numThreads);
    
    std::cout << "OpenMP Custom Profiler" << std::endl;
    std::cout << "====================" << std::endl;
    std::cout << "Running with " << numThreads << " threads" << std::endl;
    std::cout << "Output will be saved to: " << outputFile << std::endl;
    std::cout << std::endl;
    
    // Run the demo workload
    demoWorkload();
    
    // Generate profiler report
    std::string reportPath = "../reports/" + outputFile;
    
    // Ensure reports directory exists
    std::string reportsDir = "../reports";
    CreateDirectoryA(reportsDir.c_str(), NULL);
    
    Profiler::getInstance().generateReport(reportPath);
    
    std::cout << "\nProfiling complete! Open " << reportPath << " in a web browser to view the results." << std::endl;
    
    return 0;
} 