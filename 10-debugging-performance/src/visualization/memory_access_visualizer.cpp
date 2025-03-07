#include "../../include/memory_access_visualizer.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>

// Singleton instance getter
MemoryAccessTracker& MemoryAccessTracker::getInstance() {
    static MemoryAccessTracker instance;
    return instance;
}

// Constructor
MemoryAccessTracker::MemoryAccessTracker() 
    : m_cacheLineSize(64), m_enabled(true) {
    m_startTime = std::chrono::steady_clock::now();
}

// Track a memory region
void MemoryAccessTracker::trackMemoryRegion(void* baseAddress, size_t size, const std::string& name) {
    if (!m_enabled) return;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Check if this region is already being tracked
    for (const auto& region : m_trackedRegions) {
        if (region.baseAddress == baseAddress && region.size == size) {
            return; // Already tracking this region
        }
    }
    
    TrackedRegion region = {baseAddress, size, name};
    m_trackedRegions.push_back(region);
}

// Record a memory access
void MemoryAccessTracker::recordAccess(void* address, int threadId, bool isWrite) {
    if (!m_enabled) return;
    
    if (threadId == -1) {
        threadId = omp_get_thread_num();
    }
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Only record accesses to tracked regions
    for (const auto& region : m_trackedRegions) {
        if (isAddressInRegion(address, region)) {
            MemoryAccess access = {
                address,
                threadId,
                isWrite,
                std::chrono::steady_clock::now()
            };
            m_accesses.push_back(access);
            return;
        }
    }
}

// Generate HTML visualization
void MemoryAccessTracker::generateVisualization(const std::string& filename) {
    if (!m_enabled || m_trackedRegions.empty() || m_accesses.empty()) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::string outputFile = filename.empty() ? "memory_access.html" : filename;
    std::ofstream file(outputFile);
    
    if (!file.is_open()) {
        return;
    }
    
    // Analyze cache line usage
    auto cacheLineInfo = analyzeCacheLineUsage();
    
    // Generate HTML
    file << "<!DOCTYPE html>\n"
         << "<html>\n"
         << "<head>\n"
         << "    <title>Memory Access Visualization</title>\n"
         << "    <style>\n"
         << "        body { font-family: Arial, sans-serif; margin: 20px; }\n"
         << "        .container { margin-bottom: 30px; }\n"
         << "        h1, h2 { color: #333; }\n"
         << "        table { border-collapse: collapse; margin-bottom: 20px; }\n"
         << "        th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }\n"
         << "        th { background-color: #f2f2f2; }\n"
         << "        .heatmap { display: flex; flex-wrap: wrap; margin-bottom: 20px; }\n"
         << "        .cell { width: 20px; height: 20px; margin: 1px; display: flex; justify-content: center; align-items: center; color: white; font-size: 10px; }\n"
         << "        .cache-line { border: 1px solid #000; margin: 2px; display: flex; }\n"
         << "        .timeline { width: 100%; height: 400px; border: 1px solid #ddd; margin-top: 20px; position: relative; }\n"
         << "        .access { position: absolute; width: 8px; height: 8px; border-radius: 50%; }\n"
         << "        .read { background-color: blue; }\n"
         << "        .write { background-color: red; }\n"
         << "        .legend { display: flex; margin-bottom: 10px; }\n"
         << "        .legend-item { display: flex; align-items: center; margin-right: 20px; }\n"
         << "        .legend-color { width: 15px; height: 15px; margin-right: 5px; }\n"
         << "        .warning { color: red; font-weight: bold; }\n"
         << "    </style>\n"
         << "</head>\n"
         << "<body>\n"
         << "    <h1>Memory Access Visualization</h1>\n";
    
    // Generate summary
    file << "    <div class='container'>\n"
         << "        <h2>Summary</h2>\n";
    
    generateSummary(file, cacheLineInfo);
    
    // Generate heatmaps for each tracked region
    file << "    <div class='container'>\n"
         << "        <h2>Memory Access Heatmaps</h2>\n"
         << "        <div class='legend'>\n"
         << "            <div class='legend-item'><div class='legend-color' style='background-color: #e0f7fa;'></div>No access</div>\n"
         << "            <div class='legend-item'><div class='legend-color' style='background-color: #4fc3f7;'></div>Low access</div>\n"
         << "            <div class='legend-item'><div class='legend-color' style='background-color: #0277bd;'></div>Medium access</div>\n"
         << "            <div class='legend-item'><div class='legend-color' style='background-color: #01579b;'></div>High access</div>\n"
         << "            <div class='legend-item'><div class='legend-color' style='background-color: #d50000;'></div>Potential false sharing</div>\n"
         << "        </div>\n";
    
    for (const auto& region : m_trackedRegions) {
        file << "        <h3>Region: " << region.name << "</h3>\n";
        generateHeatmap(file, region, cacheLineInfo);
    }
    
    file << "    </div>\n";
    
    // Generate access timeline
    file << "    <div class='container'>\n"
         << "        <h2>Access Timeline</h2>\n"
         << "        <div class='legend'>\n"
         << "            <div class='legend-item'><div class='legend-color read'></div>Read</div>\n"
         << "            <div class='legend-item'><div class='legend-color write'></div>Write</div>\n"
         << "        </div>\n";
    
    generateAccessTimeline(file);
    
    file << "    </div>\n"
         << "</body>\n"
         << "</html>\n";
    
    file.close();
}

// Analyze for false sharing
void MemoryAccessTracker::analyzeFalseSharing(std::ostream& out) {
    if (!m_enabled || m_trackedRegions.empty() || m_accesses.empty()) {
        out << "No data available for false sharing analysis." << std::endl;
        return;
    }
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto cacheLineInfo = analyzeCacheLineUsage();
    
    out << "=== False Sharing Analysis ===" << std::endl;
    out << "Cache line size: " << m_cacheLineSize << " bytes" << std::endl;
    out << "Total memory regions tracked: " << m_trackedRegions.size() << std::endl;
    out << "Total memory accesses recorded: " << m_accesses.size() << std::endl << std::endl;
    
    bool foundFalseSharing = false;
    
    for (const auto& entry : cacheLineInfo) {
        const auto& info = entry.second;
        
        if (info.hasFalseSharing) {
            foundFalseSharing = true;
            
            out << "Potential false sharing detected in cache line " << info.lineNumber << ":" << std::endl;
            out << "  Address range: 0x" << std::hex << (info.lineNumber * m_cacheLineSize) 
                << " - 0x" << ((info.lineNumber + 1) * m_cacheLineSize - 1) << std::dec << std::endl;
            out << "  Total accesses: " << info.accessCount << " (" << info.writeCount << " writes)" << std::endl;
            out << "  Threads accessing this line: ";
            
            for (auto it = info.threads.begin(); it != info.threads.end(); ++it) {
                out << *it;
                if (std::next(it) != info.threads.end()) {
                    out << ", ";
                }
            }
            
            out << std::endl << std::endl;
        }
    }
    
    if (!foundFalseSharing) {
        out << "No potential false sharing detected." << std::endl;
    }
}

// Reset the tracker
void MemoryAccessTracker::reset() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_trackedRegions.clear();
    m_accesses.clear();
    m_startTime = std::chrono::steady_clock::now();
}

// Set cache line size
void MemoryAccessTracker::setCacheLineSize(size_t size) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_cacheLineSize = size;
}

// Enable/disable tracking
void MemoryAccessTracker::setEnabled(bool enabled) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_enabled = enabled;
}

// Get tracking status
bool MemoryAccessTracker::isEnabled() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_enabled;
}

// Check if address is in region
bool MemoryAccessTracker::isAddressInRegion(void* address, const TrackedRegion& region) const {
    char* addrPtr = static_cast<char*>(address);
    char* regionStart = static_cast<char*>(region.baseAddress);
    char* regionEnd = regionStart + region.size;
    
    return (addrPtr >= regionStart && addrPtr < regionEnd);
}

// Get cache line number for address
size_t MemoryAccessTracker::getCacheLineNumber(void* address) const {
    return reinterpret_cast<uintptr_t>(address) / m_cacheLineSize;
}

// Get region name for address
std::string MemoryAccessTracker::getRegionName(void* address) const {
    for (const auto& region : m_trackedRegions) {
        if (isAddressInRegion(address, region)) {
            return region.name;
        }
    }
    return "Unknown";
}

// Analyze cache line usage
std::map<size_t, CacheLineInfo> MemoryAccessTracker::analyzeCacheLineUsage() const {
    std::map<size_t, CacheLineInfo> cacheLines;
    
    // Count accesses per cache line
    for (const auto& access : m_accesses) {
        size_t lineNumber = getCacheLineNumber(access.address);
        
        if (cacheLines.find(lineNumber) == cacheLines.end()) {
            cacheLines[lineNumber] = {lineNumber, 0, 0, {}, false};
        }
        
        cacheLines[lineNumber].accessCount++;
        if (access.isWrite) {
            cacheLines[lineNumber].writeCount++;
        }
        cacheLines[lineNumber].threads.insert(access.threadId);
    }
    
    // Detect potential false sharing
    for (auto& entry : cacheLines) {
        auto& info = entry.second;
        
        // Potential false sharing if:
        // 1. Multiple threads access the same cache line
        // 2. At least one thread is writing
        // 3. At least 2 threads have accessed this line
        if (info.threads.size() >= 2 && info.writeCount > 0) {
            info.hasFalseSharing = true;
        }
    }
    
    return cacheLines;
}

// Generate heatmap
void MemoryAccessTracker::generateHeatmap(std::ostream& out, const TrackedRegion& region, 
                                         const std::map<size_t, CacheLineInfo>& cacheLines) const {
    out << "        <div class='heatmap'>\n";
    
    size_t startLine = getCacheLineNumber(region.baseAddress);
    size_t endLine = getCacheLineNumber(static_cast<char*>(region.baseAddress) + region.size - 1);
    
    for (size_t lineNum = startLine; lineNum <= endLine; lineNum++) {
        out << "            <div class='cache-line'>\n";
        
        // Find cache line info
        auto it = cacheLines.find(lineNum);
        bool hasFalseSharing = (it != cacheLines.end() && it->second.hasFalseSharing);
        size_t accessCount = (it != cacheLines.end()) ? it->second.accessCount : 0;
        
        // Calculate color based on access count
        std::string color;
        if (hasFalseSharing) {
            color = "#d50000"; // Red for false sharing
        } else if (accessCount == 0) {
            color = "#e0f7fa"; // Very light blue for no access
        } else if (accessCount < 10) {
            color = "#4fc3f7"; // Light blue for low access
        } else if (accessCount < 50) {
            color = "#0277bd"; // Medium blue for medium access
        } else {
            color = "#01579b"; // Dark blue for high access
        }
        
        // Display cache line number and access count
        out << "                <div class='cell' style='background-color: " << color << ";' "
            << "title='Cache line " << lineNum << ": " << accessCount << " accesses";
        
        if (hasFalseSharing) {
            out << " (Potential false sharing)";
        }
        
        out << "'>" << lineNum << "</div>\n";
        out << "            </div>\n";
    }
    
    out << "        </div>\n";
}

// Generate access timeline
void MemoryAccessTracker::generateAccessTimeline(std::ostream& out) const {
    if (m_accesses.empty()) {
        out << "        <p>No access data available for timeline.</p>\n";
        return;
    }
    
    // Find time range
    auto startTime = m_startTime;
    auto endTime = startTime;
    
    for (const auto& access : m_accesses) {
        if (access.timestamp > endTime) {
            endTime = access.timestamp;
        }
    }
    
    // Add a small buffer to the end time
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
    duration += duration * 0.1; // Add 10% buffer
    
    // Generate timeline
    out << "        <div class='timeline' id='timeline'>\n";
    
    // Generate access points
    int maxThreads = 0;
    for (const auto& access : m_accesses) {
        if (access.threadId > maxThreads) {
            maxThreads = access.threadId;
        }
    }
    maxThreads++; // Convert to count
    
    for (const auto& access : m_accesses) {
        auto timeOffset = std::chrono::duration_cast<std::chrono::microseconds>(
            access.timestamp - startTime).count();
        
        double xPercent = (static_cast<double>(timeOffset) / duration) * 100.0;
        double yPercent = (static_cast<double>(access.threadId) / maxThreads) * 100.0;
        
        std::string accessClass = access.isWrite ? "write" : "read";
        std::string regionName = getRegionName(access.address);
        
        out << "            <div class='access " << accessClass << "' "
            << "style='left: " << xPercent << "%; top: " << yPercent << "%;' "
            << "title='Thread " << access.threadId << " " 
            << (access.isWrite ? "write" : "read") << " to " << regionName << "'></div>\n";
    }
    
    out << "        </div>\n";
    
    // Add timeline labels
    out << "        <div style='display: flex; justify-content: space-between; margin-top: 5px;'>\n"
        << "            <div>Start</div>\n"
        << "            <div>Time (total: " << (duration / 1000.0) << " ms)</div>\n"
        << "            <div>End</div>\n"
        << "        </div>\n";
    
    // Add thread labels
    out << "        <div style='position: absolute; left: 0; margin-top: -400px;'>\n";
    for (int i = 0; i < maxThreads; i++) {
        double yPercent = (static_cast<double>(i) / maxThreads) * 100.0;
        out << "            <div style='position: absolute; top: " << yPercent << "%; transform: translateY(-50%);'>"
            << "Thread " << i << "</div>\n";
    }
    out << "        </div>\n";
}

// Generate summary
void MemoryAccessTracker::generateSummary(std::ostream& out, const std::map<size_t, CacheLineInfo>& cacheLines) const {
    // Count total accesses and writes
    size_t totalAccesses = m_accesses.size();
    size_t totalWrites = 0;
    std::set<int> uniqueThreads;
    
    for (const auto& access : m_accesses) {
        if (access.isWrite) {
            totalWrites++;
        }
        uniqueThreads.insert(access.threadId);
    }
    
    // Count cache lines with potential false sharing
    size_t falseSharingLines = 0;
    for (const auto& entry : cacheLines) {
        if (entry.second.hasFalseSharing) {
            falseSharingLines++;
        }
    }
    
    // Generate summary table
    out << "        <table>\n"
        << "            <tr><th>Metric</th><th>Value</th></tr>\n"
        << "            <tr><td>Total memory regions tracked</td><td>" << m_trackedRegions.size() << "</td></tr>\n"
        << "            <tr><td>Total memory accesses</td><td>" << totalAccesses << "</td></tr>\n"
        << "            <tr><td>Read accesses</td><td>" << (totalAccesses - totalWrites) << "</td></tr>\n"
        << "            <tr><td>Write accesses</td><td>" << totalWrites << "</td></tr>\n"
        << "            <tr><td>Unique threads</td><td>" << uniqueThreads.size() << "</td></tr>\n"
        << "            <tr><td>Cache line size</td><td>" << m_cacheLineSize << " bytes</td></tr>\n"
        << "            <tr><td>Cache lines accessed</td><td>" << cacheLines.size() << "</td></tr>\n";
    
    if (falseSharingLines > 0) {
        out << "            <tr><td class='warning'>Cache lines with potential false sharing</td>"
            << "<td class='warning'>" << falseSharingLines << "</td></tr>\n";
    } else {
        out << "            <tr><td>Cache lines with potential false sharing</td><td>0</td></tr>\n";
    }
    
    out << "        </table>\n";
    
    // Add warnings if false sharing detected
    if (falseSharingLines > 0) {
        out << "        <div class='warning'>\n"
            << "            <p>Warning: Potential false sharing detected in " << falseSharingLines << " cache lines!</p>\n"
            << "            <p>False sharing occurs when multiple threads access different variables that happen to be in the same cache line,\n"
            << "               causing cache invalidation and performance degradation.</p>\n"
            << "            <p>See the heatmap below for details. Cache lines with potential false sharing are highlighted in red.</p>\n"
            << "        </div>\n";
    }
    
    out << "    </div>\n";
}

// Simple demonstration of memory access visualization
int main(int argc, char* argv[]) {
    const int arraySize = 1000;
    int* testArray = new int[arraySize];
    
    // Initialize array
    for (int i = 0; i < arraySize; i++) {
        testArray[i] = i;
    }
    
    // Get the tracker instance
    auto& tracker = MemoryAccessTracker::getInstance();
    
    // Track our test array
    tracker.trackMemoryRegion(testArray, arraySize * sizeof(int), "TestArray");
    
    // Simulate memory accesses with OpenMP
    #pragma omp parallel num_threads(4)
    {
        int threadId = omp_get_thread_num();
        int numThreads = omp_get_num_threads();
        
        // Simulate different access patterns
        
        // Pattern 1: Thread-local access (good)
        int chunkSize = arraySize / numThreads;
        int start = threadId * chunkSize;
        int end = (threadId == numThreads - 1) ? arraySize : (threadId + 1) * chunkSize;
        
        for (int i = start; i < end; i++) {
            // Record read and write
            tracker.recordAccess(&testArray[i], threadId, false); // Read
            testArray[i] = testArray[i] + 1;
            tracker.recordAccess(&testArray[i], threadId, true);  // Write
        }
        
        // Pattern 2: False sharing (bad)
        int index = threadId;
        for (int i = 0; i < 100; i++) {
            tracker.recordAccess(&testArray[index], threadId, false); // Read
            testArray[index] = testArray[index] + 1;
            tracker.recordAccess(&testArray[index], threadId, true);  // Write
            
            // Move to next thread's area to create false sharing
            index = (index + 1) % numThreads;
        }
    }
    
    // Generate visualization
    tracker.generateVisualization("memory_access.html");
    
    // Analyze for false sharing
    tracker.analyzeFalseSharing(std::cout);
    
    // Clean up
    delete[] testArray;
    
    std::cout << "Memory access visualization completed. See memory_access.html for results." << std::endl;
    
    return 0;
}