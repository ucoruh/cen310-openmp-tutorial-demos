#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <mutex>
#include <unordered_map>
#include <omp.h>

/**
 * @class Profiler
 * @brief Lightweight thread-safe profiling utility for OpenMP programs
 * 
 * This class provides simple timing and profiling functionality for tracking
 * performance of code regions, particularly in multi-threaded OpenMP applications.
 */
class Profiler {
public:
    struct ProfilePoint {
        std::string name;                          // Name of the profile point
        std::chrono::high_resolution_clock::time_point startTime;  // Start time
        std::chrono::high_resolution_clock::time_point endTime;    // End time
        double duration;                           // Duration in milliseconds
        int threadId;                              // Thread ID
        int level;                                 // Nesting level
    };

    /**
     * @brief Get the singleton instance
     * @return Reference to the global profiler instance
     */
    static Profiler& getInstance();

    /**
     * @brief Start a profiling section
     * @param name Name of the section to profile
     * @return ID of the profile point (for matching with endSection)
     */
    int startSection(const std::string& name);

    /**
     * @brief End a profiling section
     * @param id ID returned from the matching startSection call
     */
    void endSection(int id);

    /**
     * @brief Start an event for profiling
     * @param name Name of the event
     * @param threadId ID of the thread
     */
    void startEvent(const std::string& name, int threadId);

    /**
     * @brief End an event for profiling
     * @param name Name of the event
     * @param threadId ID of the thread
     * @param durationMicros Duration in microseconds
     */
    void endEvent(const std::string& name, int threadId, double durationMicros);

    /**
     * @brief Start collecting system metrics
     * @param intervalMs Interval in milliseconds
     */
    void startSystemMetricCollection(int intervalMs = 500);

    /**
     * @brief Stop collecting system metrics
     */
    void stopSystemMetricCollection();

    /**
     * @brief Generate a report of profiling data
     * @param filename Filename to save the report
     */
    void generateReport(const std::string& filename);

    /**
     * @brief Profile a function or code block with automatic timing
     * @param name Name of the section to profile
     * @param function Function to profile (typically a lambda)
     * @return The result of the function call
     */
    template<typename Func, typename... Args>
    auto profileFunction(const std::string& name, Func&& function, Args&&... args) 
        -> decltype(function(std::forward<Args>(args)...)) {
        int id = startSection(name);
        auto result = function(std::forward<Args>(args)...);
        endSection(id);
        return result;
    }

    /**
     * @brief Get all collected profile points
     * @return Vector of all profile points
     */
    const std::vector<ProfilePoint>& getProfilePoints() const;

    /**
     * @brief Save collected profile data to a CSV file
     * @param filename Name of the CSV file to create
     * @return true if successful
     */
    bool saveToCSV(const std::string& filename) const;

    /**
     * @brief Reset all profile data
     */
    void reset();

    /**
     * @brief Print a summary of profile results to stdout
     * @param sortByTime Whether to sort by total time (true) or by name (false)
     */
    void printSummary(bool sortByTime = true) const;

    /**
     * @brief Enable or disable the profiler
     * @param enabled Whether profiling is enabled
     */
    void setEnabled(bool enabled);

    /**
     * @brief Check if profiling is enabled
     * @return true if profiling is enabled
     */
    bool isEnabled() const;

private:
    // Private constructor for singleton pattern
    Profiler();
    ~Profiler();

    // Prevent copying and assignment
    Profiler(const Profiler&) = delete;
    Profiler& operator=(const Profiler&) = delete;

    std::vector<ProfilePoint> m_profilePoints;
    std::mutex m_mutex;
    int m_nextId;
    bool m_enabled;
    std::unordered_map<int, int> m_activePoints;  // Thread ID -> Profile Point ID
    
    // Implementation details
    class ProfilerImpl;
    ProfilerImpl* impl;
};

/**
 * @class ScopedProfile
 * @brief RAII class for automatic profiling of a scope
 * 
 * This class allows for automatic profiling of code scopes using RAII.
 * When the object goes out of scope, the profiling section is automatically ended.
 */
class ScopedProfile {
public:
    /**
     * @brief Construct a new Scoped Profile object
     * @param name Name of the profile section
     */
    ScopedProfile(const std::string& name);
    
    /**
     * @brief Destroy the Scoped Profile object and end profiling
     */
    ~ScopedProfile();

private:
    int m_id;
};

// Convenience macro for scoped profiling
#ifdef PROFILE
    #define PROFILE_SCOPE(name) ScopedProfile profiler_##__LINE__(name)
    #define PROFILE_FUNCTION() ScopedProfile profiler_function(__FUNCTION__)
#else
    #define PROFILE_SCOPE(name)
    #define PROFILE_FUNCTION()
#endif