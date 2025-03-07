#include "../include/debug_utils.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <algorithm>
#include <cstdint>
#include <map>
#include <set>
#include <unordered_map>
#include <utility>
#include <limits>

// Undefine Windows macros to avoid conflicts
#undef min
#undef max

// DebugLogger implementation

DebugLogger& DebugLogger::getInstance() {
    static DebugLogger instance;
    return instance;
}

DebugLogger::DebugLogger() 
    : m_logLevel(INFO), 
      m_fileOutput(false), 
      m_consoleOutput(true),
      m_includeThreadId(true),
      m_filename("debug.log") {
}

DebugLogger::~DebugLogger() {
    // Nothing to clean up
}

void DebugLogger::log(LogLevel level, const std::string& message, int threadId) {
    if (level < m_logLevel) {
        return;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Get current time
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf;
    localtime_s(&tm_buf, &time);
    
    std::stringstream ss;
    ss << "[" << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S") << "] ";
    ss << "[" << levelToString(level) << "] ";
    
    if (m_includeThreadId) {
        int tid = (threadId == -1) ? omp_get_thread_num() : threadId;
        ss << "[Thread " << tid << "] ";
    }
    
    ss << message;
    
    std::string logEntry = ss.str();
    m_logEntries.push_back(logEntry);
    
    if (m_consoleOutput) {
        std::cout << logEntry << std::endl;
    }
    
    if (m_fileOutput) {
        std::ofstream file(m_filename, std::ios::app);
        if (file.is_open()) {
            file << logEntry << std::endl;
            file.close();
        }
    }
}

void DebugLogger::debug(const std::string& message) {
    log(DEBUG, message);
}

void DebugLogger::info(const std::string& message) {
    log(INFO, message);
}

void DebugLogger::warning(const std::string& message) {
    log(WARNING, message);
}

void DebugLogger::error(const std::string& message) {
    log(ERROR_LEVEL, message);
}

void DebugLogger::critical(const std::string& message) {
    log(CRITICAL, message);
}

void DebugLogger::setLogLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_logLevel = level;
}

void DebugLogger::setFileOutput(bool enabled, const std::string& filename) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_fileOutput = enabled;
    if (!filename.empty()) {
        m_filename = filename;
    }
}

void DebugLogger::setConsoleOutput(bool enabled) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_consoleOutput = enabled;
}

void DebugLogger::setIncludeThreadId(bool enabled) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_includeThreadId = enabled;
}

void DebugLogger::clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_logEntries.clear();
    
    if (m_fileOutput) {
        std::ofstream file(m_filename, std::ios::trunc);
        file.close();
    }
}

std::vector<std::string> DebugLogger::getLogEntries() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_logEntries;
}

std::string DebugLogger::levelToString(LogLevel level) const {
    switch (level) {
        case DEBUG:    return "DEBUG";
        case INFO:     return "INFO";
        case WARNING:  return "WARNING";
        case ERROR_LEVEL:   return "ERROR";
        case CRITICAL: return "CRITICAL";
        default:       return "UNKNOWN";
    }
}

// RaceDetector implementation

RaceDetector& RaceDetector::getInstance() {
    static RaceDetector instance;
    return instance;
}

RaceDetector::RaceDetector() : m_enabled(true) {
}

RaceDetector::~RaceDetector() {
}

void RaceDetector::watchAddress(void* addr, const std::string& name, size_t size) {
    if (!m_enabled) return;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    MemoryRegion region = {addr, size, name};
    m_watchedRegions.push_back(region);
}

void RaceDetector::recordReadAddress(void* addr, int threadId) {
    if (!m_enabled) return;
    
    if (threadId == -1) {
        threadId = omp_get_thread_num();
    }
    
    std::lock_guard<std::mutex> lock(m_mutex);
    MemoryAccess access = {addr, threadId, false};
    m_accesses.push_back(access);
}

void RaceDetector::recordWriteAddress(void* addr, int threadId) {
    if (!m_enabled) return;
    
    if (threadId == -1) {
        threadId = omp_get_thread_num();
    }
    
    std::lock_guard<std::mutex> lock(m_mutex);
    MemoryAccess access = {addr, threadId, true};
    m_accesses.push_back(access);
}

void RaceDetector::generateReport(const std::string& filename) {
    if (!m_enabled) return;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::ofstream file;
    std::ostream* out = &std::cout;
    
    if (!filename.empty()) {
        file.open(filename);
        if (file.is_open()) {
            out = &file;
        }
    }
    
    *out << "=== Race Detector Report ===" << std::endl;
    *out << "Watched memory regions: " << m_watchedRegions.size() << std::endl;
    *out << "Recorded accesses: " << m_accesses.size() << std::endl;
    
    if (detectRaces()) {
        *out << "WARNING: Potential race conditions detected!" << std::endl;
    } else {
        *out << "No race conditions detected." << std::endl;
    }
    
    // Map of address to thread accesses
    std::map<void*, std::map<int, std::pair<bool, bool>>> addressAccesses;
    
    for (const auto& access : m_accesses) {
        addressAccesses[access.address][access.threadId].first |= !access.isWrite;  // Read
        addressAccesses[access.address][access.threadId].second |= access.isWrite;  // Write
    }
    
    *out << std::endl << "Access details by memory region:" << std::endl;
    
    for (const auto& region : m_watchedRegions) {
        *out << "Region: " << region.name << " at " << region.address << " (size: " << region.size << ")" << std::endl;
        
        for (const auto& addrAccess : addressAccesses) {
            if (isInRegion(region, addrAccess.first)) {
                *out << "  Address: " << addrAccess.first << " (" 
                     << (static_cast<char*>(addrAccess.first) - static_cast<char*>(region.address)) 
                     << " bytes offset)" << std::endl;
                
                for (const auto& threadAccess : addrAccess.second) {
                    *out << "    Thread " << threadAccess.first << ": ";
                    if (threadAccess.second.first) *out << "Read ";
                    if (threadAccess.second.second) *out << "Write";
                    *out << std::endl;
                }
            }
        }
    }
    
    if (file.is_open()) {
        file.close();
    }
}

void RaceDetector::reset() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_watchedRegions.clear();
    m_accesses.clear();
}

void RaceDetector::setEnabled(bool enabled) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_enabled = enabled;
}

std::string RaceDetector::getRegionName(void* addr) const {
    for (const auto& region : m_watchedRegions) {
        if (isInRegion(region, addr)) {
            return region.name;
        }
    }
    return "Unknown";
}

bool RaceDetector::isInRegion(const MemoryRegion& region, void* addr) const {
    char* start = static_cast<char*>(region.address);
    char* end = start + region.size;
    char* ptr = static_cast<char*>(addr);
    return (ptr >= start && ptr < end);
}

bool RaceDetector::detectRaces() const {
    std::map<void*, std::set<int>> writers;
    std::map<void*, std::set<int>> readers;
    
    for (const auto& access : m_accesses) {
        if (access.isWrite) {
            writers[access.address].insert(access.threadId);
        } else {
            readers[access.address].insert(access.threadId);
        }
    }
    
    // Check for write-write conflicts
    for (const auto& writer : writers) {
        if (writer.second.size() > 1) {
            return true;  // Multiple threads writing to the same location
        }
    }
    
    // Check for read-write conflicts
    for (const auto& writer : writers) {
        auto readerIt = readers.find(writer.first);
        if (readerIt != readers.end()) {
            for (int threadId : readerIt->second) {
                if (writer.second.find(threadId) == writer.second.end()) {
                    return true;  // One thread reading, another writing
                }
            }
        }
    }
    
    return false;
}