#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <iostream>
#include <sstream>
#include <fstream>
#include <omp.h>
#include <map>
#include <set>

// Define log levels
enum LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR_LEVEL,
    CRITICAL
};

// Debug logger class
class DebugLogger {
public:
    static DebugLogger& getInstance();
    void log(LogLevel level, const std::string& message, int threadId = -1);
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    void critical(const std::string& message);
    void setLogLevel(LogLevel level);
    void setFileOutput(bool enabled, const std::string& filename = "debug.log");
    void setConsoleOutput(bool enabled);
    void setIncludeThreadId(bool enabled);
    void clear();
    std::vector<std::string> getLogEntries() const;

private:
    DebugLogger();
    ~DebugLogger();
    DebugLogger(const DebugLogger&) = delete;
    DebugLogger& operator=(const DebugLogger&) = delete;

    LogLevel m_logLevel;
    bool m_fileOutput;
    bool m_consoleOutput;
    bool m_includeThreadId;
    std::string m_filename;
    std::vector<std::string> m_logEntries;
    mutable std::mutex m_mutex;

    std::string levelToString(LogLevel level) const;
};

// Race detector class
class RaceDetector {
public:
    static RaceDetector& getInstance();
    
    template<typename T>
    void watchLocation(T* ptr, const std::string& name, size_t size = sizeof(T)) {
        watchAddress(reinterpret_cast<void*>(ptr), name, size);
    }
    
    void watchAddress(void* addr, const std::string& name, size_t size);
    
    template<typename T>
    void recordRead(T* ptr, int threadId = -1) {
        recordReadAddress(reinterpret_cast<void*>(ptr), threadId);
    }
    
    void recordReadAddress(void* addr, int threadId = -1);
    
    template<typename T>
    void recordWrite(T* ptr, int threadId = -1) {
        recordWriteAddress(reinterpret_cast<void*>(ptr), threadId);
    }
    
    void recordWriteAddress(void* addr, int threadId = -1);
    void generateReport(const std::string& filename = "");
    void reset();
    void setEnabled(bool enabled);

private:
    RaceDetector();
    ~RaceDetector();
    RaceDetector(const RaceDetector&) = delete;
    RaceDetector& operator=(const RaceDetector&) = delete;

    struct MemoryRegion {
        void* address;
        size_t size;
        std::string name;
    };

    struct MemoryAccess {
        void* address;
        int threadId;
        bool isWrite;
    };

    std::vector<MemoryRegion> m_watchedRegions;
    std::vector<MemoryAccess> m_accesses;
    mutable std::mutex m_mutex;
    bool m_enabled;

    std::string getRegionName(void* addr) const;
    bool isInRegion(const MemoryRegion& region, void* addr) const;
    bool detectRaces() const;
};

// Helper macros for race detection
#ifdef DEBUG_RACE_DETECTION
    #define WATCH_VARIABLE(var, name) RaceDetector::getInstance().watchLocation(&(var), name)
    #define RECORD_READ(var) RaceDetector::getInstance().recordRead(&(var))
    #define RECORD_WRITE(var) RaceDetector::getInstance().recordWrite(&(var))
#else
    #define WATCH_VARIABLE(var, name)
    #define RECORD_READ(var)
    #define RECORD_WRITE(var)
#endif
