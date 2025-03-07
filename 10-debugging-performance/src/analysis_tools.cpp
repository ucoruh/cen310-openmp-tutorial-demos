#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <unordered_map>
#include <set>
#include <algorithm>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include <memory>
#include <iomanip>
#include <omp.h>
#include <Windows.h>

// Define min/max macros to avoid conflicts with std::min/std::max
#undef min
#undef max

#include "profiler.h"
#include "debug_utils.h"
#include "cli_parser.h"

// Thread access analyzer for detecting race conditions
class CustomRaceDetector {
private:
    struct MemoryAccess {
        void* address;
        int threadId;
        std::string accessType; // "read", "write"
        std::chrono::steady_clock::time_point timestamp;
        std::string sourceLocation;
    };

    std::vector<MemoryAccess> accesses;
    std::mutex accessMutex;
    std::atomic<bool> enabled;

public:
    CustomRaceDetector() : enabled(false) {}

    void enable() {
        enabled = true;
    }

    void disable() {
        enabled = false;
    }
    
    bool isEnabled() {
        return enabled;
    }

    void recordAccess(void* address, int threadId, const std::string& accessType, const std::string& sourceLocation) {
        if (!enabled) {
            return;
        }
        
        std::lock_guard<std::mutex> lock(accessMutex);
        
        MemoryAccess access;
        access.address = address;
        access.threadId = threadId;
        access.accessType = accessType;
        access.timestamp = std::chrono::steady_clock::now();
        access.sourceLocation = sourceLocation;
        
        accesses.push_back(access);
    }
    
    void clear() {
        std::lock_guard<std::mutex> lock(accessMutex);
        accesses.clear();
    }
    
    void analyzeArrayAccess(void* arrayStart, size_t elementSize, size_t numElements, int threads) {
        std::lock_guard<std::mutex> lock(accessMutex);
        
        // Create maps to track which threads accessed which elements
        std::vector<std::set<int>> readThreads(numElements);
        std::vector<std::set<int>> writeThreads(numElements);
        
        // Identify array elements from raw memory addresses
        for (const auto& access : accesses) {
            // Calculate if this access was within our array bounds
            uintptr_t accessAddr = reinterpret_cast<uintptr_t>(access.address);
            uintptr_t arrayAddr = reinterpret_cast<uintptr_t>(arrayStart);
            
            if (accessAddr >= arrayAddr && 
                accessAddr < arrayAddr + (elementSize * numElements)) {
                
                // Calculate which array element was accessed
                size_t elemOffset = accessAddr - arrayAddr;
                size_t elemIndex = elemOffset / elementSize;
                
                if (elemIndex < numElements) {
                    if (access.accessType == "read") {
                        readThreads[elemIndex].insert(access.threadId);
                    } else if (access.accessType == "write") {
                        writeThreads[elemIndex].insert(access.threadId);
                    }
                }
            }
        }
        
        // Analyze the access patterns
        std::cout << "Array access pattern analysis:\n";
        
        // Check which threads access which parts of the array
        std::vector<std::set<size_t>> threadElements(threads);
        for (size_t i = 0; i < numElements; i++) {
            // Combine read and write threads for this element
            std::set<int> allThreads = writeThreads[i]; // Start with writers
            allThreads.insert(readThreads[i].begin(), readThreads[i].end()); // Add readers
            
            for (int threadId : allThreads) {
                if (threadId < threads) {
                    threadElements[threadId].insert(i);
                }
            }
        }
        
        // Print thread workload distribution
        for (int threadId = 0; threadId < threads; threadId++) {
            const auto& elements = threadElements[threadId];
            if (!elements.empty()) {
                std::cout << "  Thread " << threadId << " accessed " << elements.size() 
                          << " elements (" << (elements.size() * 100 / numElements) << "% of array)\n";
                
                // Print range information if it's a continuous chunk
                if (elements.size() > 1) {
                    size_t rangeStart = *elements.begin();
                    size_t rangeEnd = *elements.rbegin();
                    
                    if (rangeEnd - rangeStart + 1 == elements.size()) {
                        std::cout << "    Continuous range: [" << rangeStart << " - " << rangeEnd << "]\n";
                    } else {
                        // Detect multiple continuous ranges
                        std::vector<std::pair<size_t, size_t>> ranges;
                        size_t start = *elements.begin();
                        size_t prev = start;
                        
                        for (auto it = ++elements.begin(); it != elements.end(); ++it) {
                            if (*it != prev + 1) {
                                ranges.push_back({start, prev});
                                start = *it;
                            }
                            prev = *it;
                        }
                        ranges.push_back({start, prev});
                        
                        if (ranges.size() <= 5) {
                            std::cout << "    Ranges: ";
                            for (const auto& range : ranges) {
                                std::cout << "[" << range.first << "-" << range.second << "] ";
                            }
                            std::cout << std::endl;
                        } else {
                            std::cout << "    Scattered access pattern with " << ranges.size() << " ranges\n";
                        }
                    }
                }
            }
        }
        
        // Check for race conditions
        size_t raceElements = 0;
        for (size_t i = 0; i < numElements; i++) {
            bool hasRace = false;
            
            // Multiple writers = race condition
            if (writeThreads[i].size() > 1) {
                hasRace = true;
            }
            
            // Writers + other readers = race condition
            if (writeThreads[i].size() == 1 && !readThreads[i].empty()) {
                int writerThread = *writeThreads[i].begin();
                for (int readerThread : readThreads[i]) {
                    if (readerThread != writerThread) {
                        hasRace = true;
                        break;
                    }
                }
            }
            
            if (hasRace) {
                raceElements++;
            }
        }
        
        if (raceElements > 0) {
            std::cout << "  WARNING: Detected " << raceElements << " array elements with potential race conditions\n";
            std::cout << "  (" << (raceElements * 100 / numElements) << "% of array)\n";
        } else {
            std::cout << "  No race conditions detected in array access pattern\n";
        }
        
        std::cout << std::endl;
    }
    
    void detectFalseSharing(void* arrayStart, size_t elementSize, size_t numElements, int threads) {
        const size_t CACHE_LINE_SIZE = 64; // Typical cache line size in bytes
        std::lock_guard<std::mutex> lock(accessMutex);
        
        // Build a map of which threads access which cache lines
        size_t numCacheLines = (elementSize * numElements + CACHE_LINE_SIZE - 1) / CACHE_LINE_SIZE;
        std::vector<std::map<int, std::pair<int, int>>> cacheLineAccess(numCacheLines); // threadId -> {reads, writes}
        
        for (const auto& access : accesses) {
            // Calculate if this access was within our array bounds
            uintptr_t accessAddr = reinterpret_cast<uintptr_t>(access.address);
            uintptr_t arrayAddr = reinterpret_cast<uintptr_t>(arrayStart);
            
            if (accessAddr >= arrayAddr && 
                accessAddr < arrayAddr + (elementSize * numElements)) {
                
                // Calculate which cache line was accessed
                size_t offset = accessAddr - arrayAddr;
                size_t cacheLineIndex = offset / CACHE_LINE_SIZE;
                
                if (cacheLineIndex < numCacheLines) {
                    auto& threadStats = cacheLineAccess[cacheLineIndex][access.threadId];
                    if (access.accessType == "read") {
                        threadStats.first++;
                    } else if (access.accessType == "write") {
                        threadStats.second++;
                    }
                }
            }
        }
        
        // Analyze cache lines for false sharing
        std::cout << "False sharing analysis:\n";
        int falseSharingLines = 0;
        
        for (size_t i = 0; i < numCacheLines; i++) {
            const auto& threads = cacheLineAccess[i];
            
            if (threads.size() <= 1) {
                continue; // Only one thread accessing this cache line, no false sharing
            }
            
            // Count threads that write to this cache line
            int writingThreads = 0;
            for (const auto& thread : threads) {
                if (thread.second.second > 0) {
                    writingThreads++;
                }
            }
            
            // False sharing: multiple threads writing to the same cache line
            if (writingThreads > 1) {
                falseSharingLines++;
                
                // Calculate which array elements are in this cache line
                size_t startElement = (i * CACHE_LINE_SIZE) / elementSize;
                size_t endElement = std::min((((i + 1) * CACHE_LINE_SIZE) - 1) / elementSize, numElements - 1);
                
                std::cout << "  Cache line " << i << " (elements " << startElement << "-" << endElement 
                          << ") has potential false sharing:\n";
                
                for (const auto& thread : threads) {
                    std::cout << "    Thread " << thread.first << ": "
                              << thread.second.first << " reads, "
                              << thread.second.second << " writes\n";
                }
            }
        }
        
        if (falseSharingLines == 0) {
            std::cout << "  No false sharing detected\n";
        } else {
            std::cout << "  Detected " << falseSharingLines << " cache lines with potential false sharing\n";
            std::cout << "  Consider padding your data structure or reorganizing memory access patterns\n";
        }
        
        std::cout << std::endl;
    }
    
    void analyzeRaceConditions() {
        std::lock_guard<std::mutex> lock(accessMutex);
        
        // Group accesses by memory address
        std::map<void*, std::vector<MemoryAccess>> addressMap;
        for (const auto& access : accesses) {
            addressMap[access.address].push_back(access);
        }
        
        std::cout << "\nRace condition analysis:\n";
        
        int raceCount = 0;
        for (const auto& entry : addressMap) {
            void* address = entry.first;
            const auto& addressAccesses = entry.second;
            
            // Count read and write access by thread
            std::map<int, std::pair<int, int>> threadAccess; // threadId -> {reads, writes}
            for (const auto& access : addressAccesses) {
                auto& stats = threadAccess[access.threadId];
                if (access.accessType == "read") {
                    stats.first++;
                } else if (access.accessType == "write") {
                    stats.second++;
                }
            }
            
            // Check for race conditions
            bool hasRace = false;
            
            // Count threads that write to this address
            int writingThreads = 0;
            for (const auto& thread : threadAccess) {
                if (thread.second.second > 0) {
                    writingThreads++;
                }
            }
            
            // Multiple threads writing = race condition
            if (writingThreads > 1) {
                hasRace = true;
            }
            
            // One thread writing, others reading = race condition
            if (writingThreads == 1 && threadAccess.size() > 1) {
                int writerThread = -1;
                for (const auto& thread : threadAccess) {
                    if (thread.second.second > 0) {
                        writerThread = thread.first;
                        break;
                    }
                }
                
                for (const auto& thread : threadAccess) {
                    if (thread.first != writerThread && thread.second.first > 0) {
                        hasRace = true;
                        break;
                    }
                }
            }
            
            if (hasRace) {
                raceCount++;
                
                // Find a sample access to get the source location
                std::string location = "[Unknown]";
                for (const auto& access : addressAccesses) {
                    if (!access.sourceLocation.empty()) {
                        location = access.sourceLocation;
                        break;
                    }
                }
                
                std::cout << "  Race condition at address " << address 
                          << " (" << location << "):\n";
                
                for (const auto& thread : threadAccess) {
                    std::cout << "    Thread " << thread.first << ": " 
                              << thread.second.first << " reads, " 
                              << thread.second.second << " writes\n";
                }
            }
        }
        
        if (raceCount == 0) {
            std::cout << "  No race conditions detected\n";
        } else {
            std::cout << "  Detected " << raceCount << " potential race conditions\n";
        }
        
        std::cout << std::endl;
    }

    struct RaceCondition {
        void* address;
        int thread1;
        int thread2;
        std::string access1Type;
        std::string access2Type;
        std::string location1;
        std::string location2;
        double timeGapMs;
    };

    std::vector<RaceCondition> detectRaces() {
        std::vector<RaceCondition> races;
        std::lock_guard<std::mutex> lock(accessMutex);

        // Group by memory address
        std::unordered_map<void*, std::vector<MemoryAccess>> accessesByAddress;
        for (const auto& access : accesses) {
            accessesByAddress[access.address].push_back(access);
        }

        // Check each memory address for potential races
        for (const auto& [address, addressAccesses] : accessesByAddress) {
            // Sort by timestamp
            auto sortedAccesses = addressAccesses;
            std::sort(sortedAccesses.begin(), sortedAccesses.end(),
                [](const MemoryAccess& a, const MemoryAccess& b) {
                    return a.timestamp < b.timestamp;
                });

            // Look for write-read, read-write, or write-write accesses from different threads
            for (size_t i = 0; i < sortedAccesses.size() - 1; i++) {
                for (size_t j = i + 1; j < sortedAccesses.size(); j++) {
                    const auto& access1 = sortedAccesses[i];
                    const auto& access2 = sortedAccesses[j];

                    // Skip if same thread
                    if (access1.threadId == access2.threadId) {
                        continue;
                    }

                    // At least one access must be a write to have a race
                    if (access1.accessType != "write" && access2.accessType != "write") {
                        continue;
                    }

                    // Calculate time gap
                    auto timeGap = std::chrono::duration_cast<std::chrono::microseconds>(
                        access2.timestamp - access1.timestamp).count() / 1000.0;

                    // If accesses are close in time, potential race condition
                    if (timeGap < 100.0) { // 100ms threshold for considering a race
                        RaceCondition race;
                        race.address = address;
                        race.thread1 = access1.threadId;
                        race.thread2 = access2.threadId;
                        race.access1Type = access1.accessType;
                        race.access2Type = access2.accessType;
                        race.location1 = access1.sourceLocation;
                        race.location2 = access2.sourceLocation;
                        race.timeGapMs = timeGap;
                        races.push_back(race);

                        // Break after finding the first race for this pair
                        break;
                    }
                }
            }
        }

        return races;
    }

    void generateReport(const std::string& filename) {
        auto races = detectRaces();

        std::ofstream report(filename);
        if (!report.is_open()) {
            std::cerr << "Failed to open file for writing: " << filename << std::endl;
            return;
        }

        // Generate HTML report
        report << "<!DOCTYPE html>\n"
               << "<html>\n"
               << "<head>\n"
               << "    <title>Race Condition Analysis Report</title>\n"
               << "    <style>\n"
               << "        body { font-family: Arial, sans-serif; margin: 20px; }\n"
               << "        h1, h2 { color: #333; }\n"
               << "        .summary { margin: 20px 0; }\n"
               << "        table { border-collapse: collapse; width: 100%; margin-bottom: 20px; }\n"
               << "        th, td { padding: 8px; text-align: left; border: 1px solid #ddd; }\n"
               << "        th { background-color: #4CAF50; color: white; }\n"
               << "        tr:nth-child(even) { background-color: #f2f2f2; }\n"
               << "        .high { background-color: #FFCDD2; }\n"
               << "        .medium { background-color: #FFF9C4; }\n"
               << "        .low { background-color: #C8E6C9; }\n"
               << "    </style>\n"
               << "</head>\n"
               << "<body>\n"
               << "    <h1>Race Condition Analysis Report</h1>\n";

        // Summary
        report << "    <div class=\"summary\">\n"
               << "        <h2>Summary</h2>\n"
               << "        <p>Total memory accesses tracked: " << accesses.size() << "</p>\n"
               << "        <p>Potential race conditions detected: " << races.size() << "</p>\n"
               << "    </div>\n";

        // Race conditions table
        if (!races.empty()) {
            report << "    <h2>Detected Race Conditions</h2>\n"
                   << "    <table>\n"
                   << "        <tr>\n"
                   << "            <th>Address</th>\n"
                   << "            <th>Thread 1</th>\n"
                   << "            <th>Access 1</th>\n"
                   << "            <th>Location 1</th>\n"
                   << "            <th>Thread 2</th>\n"
                   << "            <th>Access 2</th>\n"
                   << "            <th>Location 2</th>\n"
                   << "            <th>Time Gap (ms)</th>\n"
                   << "            <th>Severity</th>\n"
                   << "        </tr>\n";

            for (const auto& race : races) {
                // Determine severity based on access types and time gap
                std::string severityClass = "medium";
                if (race.access1Type == "write" && race.access2Type == "write" && race.timeGapMs < 1.0) {
                    severityClass = "high"; // Two writes very close in time
                } else if (race.timeGapMs > 10.0) {
                    severityClass = "low"; // Larger time gap
                }

                report << "        <tr class=\"" << severityClass << "\">\n"
                       << "            <td>" << race.address << "</td>\n"
                       << "            <td>" << race.thread1 << "</td>\n"
                       << "            <td>" << race.access1Type << "</td>\n"
                       << "            <td>" << race.location1 << "</td>\n"
                       << "            <td>" << race.thread2 << "</td>\n"
                       << "            <td>" << race.access2Type << "</td>\n"
                       << "            <td>" << race.location2 << "</td>\n"
                       << "            <td>" << std::fixed << std::setprecision(3) << race.timeGapMs << "</td>\n"
                       << "            <td>" << (severityClass == "high" ? "High" : (severityClass == "medium" ? "Medium" : "Low")) << "</td>\n"
                       << "        </tr>\n";
            }

            report << "    </table>\n";

            // Recommendations
            report << "    <h2>Recommendations</h2>\n"
                   << "    <ul>\n"
                   << "        <li>Use critical sections or atomic operations for shared variables</li>\n"
                   << "        <li>Consider using thread-private data where possible</li>\n"
                   << "        <li>Ensure proper synchronization with barriers or locks</li>\n"
                   << "        <li>Use reduction for accumulation operations</li>\n"
                   << "        <li>Review high severity race conditions first</li>\n"
                   << "    </ul>\n";
        } else {
            report << "    <p>No race conditions detected in the analyzed workload.</p>\n";
        }

        report << "</body>\n</html>\n";
        
        report.close();
        std::cout << "Race detection report generated: " << filename << std::endl;
    }
};

// Global race detector instance
CustomRaceDetector raceDetector;

// Macros for simplified access tracking
#define TRACK_READ(addr, loc) \
    raceDetector.recordAccess(addr, omp_get_thread_num(), "read", loc)

#define TRACK_WRITE(addr, loc) \
    raceDetector.recordAccess(addr, omp_get_thread_num(), "write", loc)

// Performance regression testing framework
class PerformanceRegression {
private:
    struct BenchmarkResult {
        std::string name;
        double executionTime; // in milliseconds
        std::chrono::system_clock::time_point timestamp;
    };

    std::vector<BenchmarkResult> benchmarkHistory;
    std::map<std::string, std::vector<double>> benchmarkStats;

public:
    void recordBenchmark(const std::string& name, double executionTime) {
        BenchmarkResult result;
        result.name = name;
        result.executionTime = executionTime;
        result.timestamp = std::chrono::system_clock::now();

        benchmarkHistory.push_back(result);
        benchmarkStats[name].push_back(executionTime);
    }

    struct RegressionResult {
        std::string benchmarkName;
        double baselineAvg;
        double currentAvg;
        double percentChange;
        bool isRegression;
    };

    std::vector<RegressionResult> detectRegressions(double thresholdPercent = 5.0) {
        std::vector<RegressionResult> regressions;

        for (const auto& [name, times] : benchmarkStats) {
            // Need at least 2 runs to compare
            if (times.size() < 2) {
                continue;
            }

            // Calculate averages for baseline (first half) and current (second half)
            size_t midpoint = times.size() / 2;
            
            double baselineSum = 0.0;
            for (size_t i = 0; i < midpoint; i++) {
                baselineSum += times[i];
            }
            double baselineAvg = baselineSum / midpoint;

            double currentSum = 0.0;
            for (size_t i = midpoint; i < times.size(); i++) {
                currentSum += times[i];
            }
            double currentAvg = currentSum / (times.size() - midpoint);

            // Calculate percent change
            double percentChange = ((currentAvg - baselineAvg) / baselineAvg) * 100.0;

            // Check if it's a regression (slower)
            bool isRegression = (percentChange > thresholdPercent);

            if (isRegression) {
                RegressionResult result;
                result.benchmarkName = name;
                result.baselineAvg = baselineAvg;
                result.currentAvg = currentAvg;
                result.percentChange = percentChange;
                result.isRegression = true;
                regressions.push_back(result);
            }
        }

        return regressions;
    }

    void generateReport(const std::string& filename) {
        auto regressions = detectRegressions();

        std::ofstream report(filename);
        if (!report.is_open()) {
            std::cerr << "Failed to open file for writing: " << filename << std::endl;
            return;
        }

        // Generate HTML report
        report << "<!DOCTYPE html>\n"
               << "<html>\n"
               << "<head>\n"
               << "    <title>Performance Regression Analysis</title>\n"
               << "    <style>\n"
               << "        body { font-family: Arial, sans-serif; margin: 20px; }\n"
               << "        h1, h2 { color: #333; }\n"
               << "        .summary { margin: 20px 0; }\n"
               << "        table { border-collapse: collapse; width: 100%; margin-bottom: 20px; }\n"
               << "        th, td { padding: 8px; text-align: left; border: 1px solid #ddd; }\n"
               << "        th { background-color: #4CAF50; color: white; }\n"
               << "        tr:nth-child(even) { background-color: #f2f2f2; }\n"
               << "        .regression { background-color: #FFCDD2; }\n"
               << "        .improvement { background-color: #C8E6C9; }\n"
               << "        .chart-container { margin: 20px 0; width: 800px; height: 400px; }\n"
               << "    </style>\n"
               << "    <script src=\"https://cdn.jsdelivr.net/npm/chart.js\"></script>\n"
               << "</head>\n"
               << "<body>\n"
               << "    <h1>Performance Regression Analysis</h1>\n";

        // Summary
        report << "    <div class=\"summary\">\n"
               << "        <h2>Summary</h2>\n"
               << "        <p>Total benchmarks tracked: " << benchmarkStats.size() << "</p>\n"
               << "        <p>Total runs: " << benchmarkHistory.size() << "</p>\n"
               << "        <p>Regressions detected: " << regressions.size() << "</p>\n"
               << "    </div>\n";

        // Benchmark results table
        report << "    <h2>Benchmark Results</h2>\n"
               << "    <table>\n"
               << "        <tr>\n"
               << "            <th>Benchmark</th>\n"
               << "            <th>Baseline Avg (ms)</th>\n"
               << "            <th>Current Avg (ms)</th>\n"
               << "            <th>Change (%)</th>\n"
               << "            <th>Status</th>\n"
               << "        </tr>\n";

        for (const auto& [name, times] : benchmarkStats) {
            // Calculate stats
            size_t midpoint = std::max(size_t(1), times.size() / 2);
            
            double baselineSum = 0.0;
            for (size_t i = 0; i < midpoint; i++) {
                baselineSum += times[i];
            }
            double baselineAvg = baselineSum / midpoint;

            double currentSum = 0.0;
            size_t currentCount = 0;
            for (size_t i = midpoint; i < times.size(); i++) {
                currentSum += times[i];
                currentCount++;
            }
            double currentAvg = (currentCount > 0) ? currentSum / currentCount : baselineAvg;

            // Calculate percent change
            double percentChange = ((currentAvg - baselineAvg) / baselineAvg) * 100.0;

            // Determine status
            std::string statusClass = "normal";
            std::string status = "Stable";
            if (percentChange > 5.0) {
                statusClass = "regression";
                status = "Regression";
            } else if (percentChange < -5.0) {
                statusClass = "improvement";
                status = "Improvement";
            }

            report << "        <tr class=\"" << statusClass << "\">\n"
                   << "            <td>" << name << "</td>\n"
                   << "            <td>" << std::fixed << std::setprecision(2) << baselineAvg << "</td>\n"
                   << "            <td>" << std::fixed << std::setprecision(2) << currentAvg << "</td>\n"
                   << "            <td>" << std::fixed << std::setprecision(2) << percentChange << "%</td>\n"
                   << "            <td>" << status << "</td>\n"
                   << "        </tr>\n";
        }

        report << "    </table>\n";

        // Charts for each benchmark
        for (const auto& [name, times] : benchmarkStats) {
            if (times.size() < 2) continue;

            report << "    <h3>Benchmark: " << name << "</h3>\n"
                   << "    <div class=\"chart-container\">\n"
                   << "        <canvas id=\"chart_" << name << "\"></canvas>\n"
                   << "    </div>\n"
                   << "    <script>\n"
                   << "        new Chart(document.getElementById('chart_" << name << "'), {\n"
                   << "            type: 'line',\n"
                   << "            data: {\n"
                   << "                labels: [";

            for (size_t i = 0; i < times.size(); i++) {
                if (i > 0) report << ", ";
                report << "'" << i + 1 << "'";
            }

            report << "],\n"
                   << "                datasets: [{\n"
                   << "                    label: '" << name << " Execution Time (ms)',\n"
                   << "                    data: [";

            for (size_t i = 0; i < times.size(); i++) {
                if (i > 0) report << ", ";
                report << times[i];
            }

            report << "],\n"
                   << "                    borderColor: '#4CAF50',\n"
                   << "                    fill: false\n"
                   << "                }]\n"
                   << "            },\n"
                   << "            options: {\n"
                   << "                responsive: true,\n"
                   << "                scales: {\n"
                   << "                    y: {\n"
                   << "                        title: { display: true, text: 'Time (ms)' },\n"
                   << "                        beginAtZero: true\n"
                   << "                    },\n"
                   << "                    x: {\n"
                   << "                        title: { display: true, text: 'Run' }\n"
                   << "                    }\n"
                   << "                }\n"
                   << "            }\n"
                   << "        });\n"
                   << "    </script>\n";
        }

        report << "</body>\n</html>\n";
        
        report.close();
        std::cout << "Performance regression report generated: " << filename << std::endl;
    }
};

// Global performance regression instance
PerformanceRegression perfRegression;

// Pattern recognition for common OpenMP issues
class PatternRecognizer {
public:
    enum class PatternType {
        RaceCondition,
        FalseSharing,
        LoadImbalance,
        ExcessiveSynchronization,
        MemoryIssue,
        Unknown
    };

    struct RecognizedPattern {
        PatternType type;
        std::string description;
        double confidence; // 0.0 to 1.0
        std::string recommendation;
    };

    RecognizedPattern analyzePerformancePattern(const std::map<int, double>& threadTimes, 
                                              const std::map<std::string, int>& syncPoints,
                                              double totalTime) {
        RecognizedPattern pattern;
        pattern.type = PatternType::Unknown;
        pattern.confidence = 0.0;

        // Calculate statistics for thread times
        double minTime = std::numeric_limits<double>::max();
        double maxTime = 0.0;
        double avgTime = 0.0;
        
        for (const auto& [threadId, time] : threadTimes) {
            minTime = std::min(minTime, time);
            maxTime = std::max(maxTime, time);
            avgTime += time;
        }
        avgTime /= threadTimes.size();

        // Calculate imbalance ratio
        double imbalanceRatio = maxTime / minTime;
        
        // Count sync operations
        int totalSyncOps = 0;
        for (const auto& [type, count] : syncPoints) {
            totalSyncOps += count;
        }
        
        // Detect load imbalance
        if (imbalanceRatio > 1.5 && threadTimes.size() > 1) {
            pattern.type = PatternType::LoadImbalance;
            pattern.description = "Thread execution times vary significantly, indicating load imbalance";
            pattern.confidence = std::min(1.0, (imbalanceRatio - 1.0) / 2.0);
            pattern.recommendation = "Consider using dynamic scheduling or manually balancing workload";
            return pattern;
        }
        
        // Detect excessive synchronization
        double syncRatio = (totalTime > 0) ? totalSyncOps / totalTime : 0;
        if (syncRatio > 0.1) {
            pattern.type = PatternType::ExcessiveSynchronization;
            pattern.description = "High frequency of synchronization operations detected";
            pattern.confidence = std::min(1.0, syncRatio * 5.0);
            pattern.recommendation = "Reduce barriers/critical sections or reorganize algorithm to require less synchronization";
            return pattern;
        }
        
        // If no specific pattern detected
        pattern.description = "No clear performance pattern detected";
        pattern.recommendation = "Collect more detailed metrics for further analysis";
        
        return pattern;
    }

    void generateReport(const std::string& filename, const std::vector<RecognizedPattern>& patterns) {
        std::ofstream report(filename);
        if (!report.is_open()) {
            std::cerr << "Failed to open file for writing: " << filename << std::endl;
            return;
        }

        // Generate HTML report
        report << "<!DOCTYPE html>\n"
               << "<html>\n"
               << "<head>\n"
               << "    <title>OpenMP Pattern Recognition Report</title>\n"
               << "    <style>\n"
               << "        body { font-family: Arial, sans-serif; margin: 20px; }\n"
               << "        h1, h2 { color: #333; }\n"
               << "        .summary { margin: 20px 0; }\n"
               << "        table { border-collapse: collapse; width: 100%; margin-bottom: 20px; }\n"
               << "        th, td { padding: 8px; text-align: left; border: 1px solid #ddd; }\n"
               << "        th { background-color: #4CAF50; color: white; }\n"
               << "        tr:nth-child(even) { background-color: #f2f2f2; }\n"
               << "        .high { background-color: #FFCDD2; }\n"
               << "        .medium { background-color: #FFF9C4; }\n"
               << "        .low { background-color: #C8E6C9; }\n"
               << "    </style>\n"
               << "</head>\n"
               << "<body>\n"
               << "    <h1>OpenMP Pattern Recognition Report</h1>\n";

        // Summary
        report << "    <div class=\"summary\">\n"
               << "        <h2>Summary</h2>\n"
               << "        <p>Total patterns analyzed: " << patterns.size() << "</p>\n"
               << "    </div>\n";

        // Patterns table
        if (!patterns.empty()) {
            report << "    <h2>Detected Patterns</h2>\n"
                   << "    <table>\n"
                   << "        <tr>\n"
                   << "            <th>Pattern Type</th>\n"
                   << "            <th>Description</th>\n"
                   << "            <th>Confidence</th>\n"
                   << "            <th>Recommendation</th>\n"
                   << "        </tr>\n";

            for (const auto& pattern : patterns) {
                // Determine confidence class
                std::string confidenceClass = "medium";
                if (pattern.confidence > 0.7) {
                    confidenceClass = "high";
                } else if (pattern.confidence < 0.3) {
                    confidenceClass = "low";
                }

                std::string patternName;
                switch (pattern.type) {
                    case PatternType::RaceCondition: patternName = "Race Condition"; break;
                    case PatternType::FalseSharing: patternName = "False Sharing"; break;
                    case PatternType::LoadImbalance: patternName = "Load Imbalance"; break;
                    case PatternType::ExcessiveSynchronization: patternName = "Excessive Synchronization"; break;
                    case PatternType::MemoryIssue: patternName = "Memory Issue"; break;
                    default: patternName = "Unknown"; break;
                }

                report << "        <tr class=\"" << confidenceClass << "\">\n"
                       << "            <td>" << patternName << "</td>\n"
                       << "            <td>" << pattern.description << "</td>\n"
                       << "            <td>" << std::fixed << std::setprecision(2) << (pattern.confidence * 100.0) << "%</td>\n"
                       << "            <td>" << pattern.recommendation << "</td>\n"
                       << "        </tr>\n";
            }

            report << "    </table>\n";
        } else {
            report << "    <p>No patterns were detected in the analyzed workload.</p>\n";
        }

        // Best practices section
        report << "    <h2>OpenMP Best Practices</h2>\n"
               << "    <ul>\n"
               << "        <li><strong>Race Conditions:</strong> Protect shared variables with critical sections, atomic operations, or locks</li>\n"
               << "        <li><strong>False Sharing:</strong> Align data to cache lines and add padding between thread-local data</li>\n"
               << "        <li><strong>Load Imbalance:</strong> Use dynamic or guided scheduling for uneven workloads</li>\n"
               << "        <li><strong>Synchronization:</strong> Minimize barriers and critical sections for better scalability</li>\n"
               << "        <li><strong>Memory:</strong> Be aware of NUMA effects and organize data for better locality</li>\n"
               << "    </ul>\n";

        report << "</body>\n</html>\n";
        
        report.close();
        std::cout << "Pattern recognition report generated: " << filename << std::endl;
    }
};

// Demonstrate race detection with a simple example
void demonstrateRaceDetection() {
    std::cout << "Demonstrating race detection..." << std::endl;
    
    const int arraySize = 100;
    std::vector<int> sharedArray(arraySize, 0);
    
    // Enable race detection
    raceDetector.enable();
    
    // Parallel code with race conditions
    #pragma omp parallel num_threads(4)
    {
        int tid = omp_get_thread_num();
        
        // Intentional race condition - multiple threads incrementing shared variables
        for (int i = 0; i < arraySize; i++) {
            // Read the current value
            int value = sharedArray[i];
            TRACK_READ(&sharedArray[i], "demonstrateRaceDetection:line321");
            
            // Simulate some work
            std::this_thread::sleep_for(std::chrono::microseconds(tid + 1));
            
            // Write back the incremented value (race condition)
            sharedArray[i] = value + 1;
            TRACK_WRITE(&sharedArray[i], "demonstrateRaceDetection:line328");
        }
    }
    
    // Disable race detection
    raceDetector.disable();
    
    // Generate race detection report
    std::string reportsDir = "../reports";
    CreateDirectoryA(reportsDir.c_str(), NULL);
    raceDetector.generateReport("../reports/race_detection.html");
}

// Demonstrate performance regression testing
void demonstratePerformanceRegression() {
    std::cout << "Demonstrating performance regression testing..." << std::endl;
    
    // Function to benchmark
    auto benchmarkFunction = [](int size, int iterations) {
        double result = 0.0;
        
        // Start timing
        auto start = std::chrono::high_resolution_clock::now();
        
        // Do some work
        #pragma omp parallel for reduction(+:result)
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < iterations; j++) {
                result += std::sin(j * 0.01) * std::cos(i * 0.01);
            }
        }
        
        // End timing
        auto end = std::chrono::high_resolution_clock::now();
        double duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        // Return execution time
        return duration;
    };
    
    // Baseline runs
    for (int i = 0; i < 5; i++) {
        double time = benchmarkFunction(1000, 100);
        perfRegression.recordBenchmark("Trigonometry", time);
    }
    
    // Regression runs (intentionally slower)
    for (int i = 0; i < 5; i++) {
        double time = benchmarkFunction(1000, 120); // 20% more iterations
        perfRegression.recordBenchmark("Trigonometry", time);
    }
    
    // Generate regression report
    std::string reportsDir = "../reports";
    CreateDirectoryA(reportsDir.c_str(), NULL);
    perfRegression.generateReport("../reports/performance_regression.html");
}

// Demonstrate pattern recognition
void demonstratePatternRecognition() {
    std::cout << "Demonstrating pattern recognition..." << std::endl;
    
    // Create a pattern recognizer
    PatternRecognizer recognizer;
    std::vector<PatternRecognizer::RecognizedPattern> patterns;
    
    // Simulate load imbalance pattern
    std::map<int, double> threadTimes = {
        {0, 100.0},
        {1, 150.0},
        {2, 90.0},
        {3, 200.0}
    };
    
    std::map<std::string, int> syncPoints = {
        {"barrier", 4},
        {"critical", 10}
    };
    
    double totalTime = 200.0;
    
    // Analyze the pattern
    auto pattern = recognizer.analyzePerformancePattern(threadTimes, syncPoints, totalTime);
    patterns.push_back(pattern);
    
    // Simulate excessive synchronization pattern
    threadTimes = {
        {0, 120.0},
        {1, 125.0},
        {2, 118.0},
        {3, 122.0}
    };
    
    syncPoints = {
        {"barrier", 20},
        {"critical", 50}
    };
    
    totalTime = 130.0;
    
    // Analyze the pattern
    pattern = recognizer.analyzePerformancePattern(threadTimes, syncPoints, totalTime);
    patterns.push_back(pattern);
    
    // Generate pattern recognition report
    std::string reportsDir = "../reports";
    CreateDirectoryA(reportsDir.c_str(), NULL);
    recognizer.generateReport("../reports/pattern_recognition.html", patterns);
}

// Add more functions to record access in the detector call
void recordRead(void* address, const char* file, int line) {
    if (raceDetector.isEnabled()) {
        std::stringstream location;
        location << file << ":" << line;
        raceDetector.recordAccess(address, omp_get_thread_num(), "read", location.str());
    }
}

void recordWrite(void* address, const char* file, int line) {
    if (raceDetector.isEnabled()) {
        std::stringstream location;
        location << file << ":" << line;
        raceDetector.recordAccess(address, omp_get_thread_num(), "write", location.str());
    }
}

// Function to analyze race conditions in arrays
void analyzeArrayAccess(const char* arrayName, void* arrayStart, size_t elementSize, 
                        size_t numElements, int threads, bool detectFalseSharing = true) {
    std::cout << "\n=== Array Access Analysis for '" << arrayName << "' ===\n";
    
    std::cout << "Array information:\n";
    std::cout << "  - Start address: " << arrayStart << "\n";
    std::cout << "  - Element size: " << elementSize << " bytes\n";
    std::cout << "  - Elements: " << numElements << "\n";
    std::cout << "  - Total size: " << (elementSize * numElements) << " bytes\n\n";
    
    // Enable race detection
    raceDetector.enable();
    
    // Process the memory access log
    raceDetector.analyzeArrayAccess(arrayStart, elementSize, numElements, threads);
    
    // Detect false sharing if requested
    if (detectFalseSharing) {
        raceDetector.detectFalseSharing(arrayStart, elementSize, numElements, threads);
    }
    
    // Disable race detection
    raceDetector.disable();
    raceDetector.clear();
}

void analyzeDataRaces(int numElements, int numThreads, bool dynamicSchedule) {
    std::vector<int> data(numElements, 0);
    
    raceDetector.enable();
    
    // Example of a race condition on a shared variable
    int sharedSum = 0;
    
    #pragma omp parallel num_threads(numThreads)
    {
        int threadId = omp_get_thread_num();
        
        // Choose schedule type based on parameter
        if (dynamicSchedule) {
            #pragma omp for schedule(dynamic, 10)
            for (int i = 0; i < numElements; i++) {
                // Race condition: multiple threads writing to shared variable
                if (i % 2 == 0) {
                    recordRead(&sharedSum, __FILE__, __LINE__);
                    sharedSum += 1;  // Race condition
                    recordWrite(&sharedSum, __FILE__, __LINE__);
                }
                
                // Array access - each thread writes to its own chunk
                recordRead(&data[i], __FILE__, __LINE__);
                data[i] = threadId;
                recordWrite(&data[i], __FILE__, __LINE__);
            }
        } else {
            #pragma omp for schedule(static)
            for (int i = 0; i < numElements; i++) {
                // Race condition: multiple threads writing to shared variable
                if (i % 2 == 0) {
                    recordRead(&sharedSum, __FILE__, __LINE__);
                    sharedSum += 1;  // Race condition
                    recordWrite(&sharedSum, __FILE__, __LINE__);
                }
                
                // Array access - each thread writes to its own chunk
                recordRead(&data[i], __FILE__, __LINE__);
                data[i] = threadId;
                recordWrite(&data[i], __FILE__, __LINE__);
            }
        }
    }
    
    raceDetector.analyzeRaceConditions();
    raceDetector.disable();
    raceDetector.clear();
    
    // Use the result to avoid optimization
    std::cout << "  Final sum: " << sharedSum << std::endl;
}

int main(int argc, char* argv[]) {
    CliParser parser(argc, argv);
    parser.addOption("demo", 'd', "Demo to run (race, regression, pattern, all)", true);
    parser.addOption("threads", 't', "Number of threads to use (default: system cores)", true);
    parser.parse();

    // Set the number of threads
    int numThreads = parser.getIntValue("threads", omp_get_num_procs());
    omp_set_num_threads(numThreads);
    
    std::string demo = parser.getStringValue("demo", "all");
    
    std::cout << "OpenMP Analysis Tools" << std::endl;
    std::cout << "====================" << std::endl;
    std::cout << "Running with " << numThreads << " threads" << std::endl;
    
    // Create reports directory if it doesn't exist
    std::string reportsDir = "../reports";
    CreateDirectoryA(reportsDir.c_str(), NULL);
    
    if (demo == "race" || demo == "all") {
        demonstrateRaceDetection();
    }
    
    if (demo == "regression" || demo == "all") {
        demonstratePerformanceRegression();
    }
    
    if (demo == "pattern" || demo == "all") {
        demonstratePatternRecognition();
    }
    
    std::cout << "\nAnalysis complete! Check reports in the '../reports' directory." << std::endl;
    
    return 0;
} 