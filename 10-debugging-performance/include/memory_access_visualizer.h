#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <map>
#include <set>
#include <chrono>
#include <omp.h>
#include <iostream>

/**
 * @class MemoryAccessTracker
 * @brief Tracks memory accesses for visualization and analysis
 * 
 * This class provides functionality to track memory accesses in OpenMP
 * parallel regions and generate visualizations to help identify patterns
 * and potential issues like false sharing.
 */

// Structure to track memory access information
struct MemoryAccess {
    void* address;
    int threadId;
    bool isWrite;
    std::chrono::steady_clock::time_point timestamp;
};

// Structure to track cache line information
struct CacheLineInfo {
    size_t lineNumber;
    size_t accessCount;
    size_t writeCount;
    std::set<int> threads;
    bool hasFalseSharing;
};

class MemoryAccessTracker {
public:
    /**
     * @brief Get the singleton instance
     * @return Reference to the global tracker instance
     */
    static MemoryAccessTracker& getInstance();

    /**
     * @brief Prevent copying due to mutex
     * @param other Other instance to copy from
     */
    MemoryAccessTracker(const MemoryAccessTracker&) = delete;

    /**
     * @brief Prevent assignment due to mutex
     * @param other Other instance to assign from
     * @return Reference to this instance
     */
    MemoryAccessTracker& operator=(const MemoryAccessTracker&) = delete;

    /**
     * @brief Allow move semantics
     * @param other Other instance to move from
     */
    MemoryAccessTracker(MemoryAccessTracker&&) = default;

    /**
     * @brief Allow move assignment
     * @param other Other instance to move from
     * @return Reference to this instance
     */
    MemoryAccessTracker& operator=(MemoryAccessTracker&&) = default;

    /**
     * @brief Track an array for visualization
     * @param array Pointer to the start of the array
     * @param size Size of the array in bytes
     * @param name Name for this array (for reporting)
     */
    template<typename T>
    void trackArray(T* array, size_t size, const std::string& name) {
        trackMemoryRegion(static_cast<void*>(array), size * sizeof(T), name);
    }

    /**
     * @brief Track a memory region for visualization
     * @param baseAddress Pointer to the start of the region
     * @param size Size of the region in bytes
     * @param name Name for this region (for reporting)
     */
    void trackMemoryRegion(void* baseAddress, size_t size, const std::string& name);

    /**
     * @brief Record a read access to a memory location
     * @param address Pointer to the memory location
     * @param threadId Optional thread ID (defaults to current thread)
     */
    template<typename T>
    void recordRead(T* address, int threadId = -1) {
        recordAccess(static_cast<void*>(address), threadId, false);
    }

    /**
     * @brief Record a write access to a memory location
     * @param address Pointer to the memory location
     * @param threadId Optional thread ID (defaults to current thread)
     */
    template<typename T>
    void recordWrite(T* address, int threadId = -1) {
        recordAccess(static_cast<void*>(address), threadId, true);
    }

    /**
     * @brief Record a memory access
     * @param address Pointer to the memory location
     * @param threadId Thread ID for this access
     * @param isWrite Whether this is a write access
     */
    void recordAccess(void* address, int threadId, bool isWrite);

    /**
     * @brief Generate an HTML visualization of memory access patterns
     * @param filename Filename to save the visualization (if empty, uses default)
     */
    void generateVisualization(const std::string& filename = "memory_access.html");

    /**
     * @brief Analyze for false sharing
     * @param out Output stream for analysis results
     */
    void analyzeFalseSharing(std::ostream& out = std::cout);

    /**
     * @brief Reset the tracker, clearing all tracked arrays and accesses
     */
    void reset();

    /**
     * @brief Set the cache line size for analysis
     * @param size Cache line size in bytes
     */
    void setCacheLineSize(size_t size);

    /**
     * @brief Enable or disable tracking
     * @param enabled Whether tracking is enabled
     */
    void setEnabled(bool enabled);

    /**
     * @brief Get tracking status
     * @return Whether tracking is enabled
     */
    bool isEnabled() const;

private:
    // Private constructor for singleton pattern
    MemoryAccessTracker();
    ~MemoryAccessTracker() = default;

    struct TrackedRegion {
        void* baseAddress;
        size_t size;
        std::string name;
    };

    std::vector<TrackedRegion> m_trackedRegions;
    std::vector<MemoryAccess> m_accesses;
    mutable std::mutex m_mutex;
    size_t m_cacheLineSize;
    bool m_enabled;
    std::chrono::steady_clock::time_point m_startTime;

    // Helper methods
    bool isAddressInRegion(void* address, const TrackedRegion& region) const;
    size_t getCacheLineNumber(void* address) const;
    std::string getRegionName(void* address) const;
    std::map<size_t, CacheLineInfo> analyzeCacheLineUsage() const;
    void generateHeatmap(std::ostream& out, const TrackedRegion& region, 
                         const std::map<size_t, CacheLineInfo>& cacheLines) const;
    void generateAccessTimeline(std::ostream& out) const;
    void generateSummary(std::ostream& out, const std::map<size_t, CacheLineInfo>& cacheLines) const;
};

// Helper macros for memory access tracking
#ifdef DEBUG_MEMORY_ACCESS
    #define TRACK_ARRAY(arr, size, name) MemoryAccessTracker::getInstance().trackArray(arr, size, name)
    #define RECORD_READ(addr) MemoryAccessTracker::getInstance().recordRead(addr)
    #define RECORD_WRITE(addr) MemoryAccessTracker::getInstance().recordWrite(addr)
#else
    #define TRACK_ARRAY(arr, size, name)
    #define RECORD_READ(addr)
    #define RECORD_WRITE(addr)
#endif 