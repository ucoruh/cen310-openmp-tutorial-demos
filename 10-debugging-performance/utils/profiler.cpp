#include "../include/profiler.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <sstream>
#include <thread>
#include <map>

Profiler& Profiler::getInstance() {
    static Profiler instance;
    return instance;
}

Profiler::Profiler() : m_nextId(0), m_enabled(true) {
}

Profiler::~Profiler() {
}

int Profiler::startSection(const std::string& name) {
    if (!m_enabled) {
        return -1;
    }

    int id = -1;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        id = m_nextId++;
        
        ProfilePoint point;
        point.name = name;
        point.startTime = std::chrono::high_resolution_clock::now();
        point.threadId = omp_get_thread_num();
        point.level = omp_in_parallel() ? 1 : 0;  // Simplified level tracking
        
        m_profilePoints.push_back(point);
        m_activePoints[point.threadId] = id;
    }
    
    return id;
}

void Profiler::endSection(int id) {
    if (!m_enabled || id < 0) {
        return;
    }

    auto endTime = std::chrono::high_resolution_clock::now();

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (id < static_cast<int>(m_profilePoints.size())) {
            m_profilePoints[id].endTime = endTime;
            
            // Calculate duration in milliseconds
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
                m_profilePoints[id].endTime - m_profilePoints[id].startTime).count() / 1000.0;
            
            m_profilePoints[id].duration = duration;
            
            // Remove from active points
            m_activePoints.erase(m_profilePoints[id].threadId);
        }
    }
}

const std::vector<Profiler::ProfilePoint>& Profiler::getProfilePoints() const {
    return m_profilePoints;
}

bool Profiler::saveToCSV(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    // Write header
    file << "Name,Thread,Level,Duration (ms)" << std::endl;
    
    // Write data
    for (const auto& point : m_profilePoints) {
        if (point.endTime != std::chrono::high_resolution_clock::time_point()) {
            file << point.name << ","
                 << point.threadId << ","
                 << point.level << ","
                 << std::fixed << std::setprecision(3) << point.duration << std::endl;
        }
    }
    
    return true;
}

void Profiler::reset() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_profilePoints.clear();
    m_activePoints.clear();
    m_nextId = 0;
}

void Profiler::printSummary(bool sortByTime) const {
    std::cout << "=== Profile Summary ===" << std::endl;
    
    // Group by name and calculate total and average time
    std::map<std::string, std::vector<double>> timesByName;
    
    for (const auto& point : m_profilePoints) {
        if (point.endTime != std::chrono::high_resolution_clock::time_point()) {
            timesByName[point.name].push_back(point.duration);
        }
    }
    
    // Create a vector of summary entries for sorting
    struct SummaryEntry {
        std::string name;
        int count;
        double totalTime;
        double avgTime;
        double minTime;
        double maxTime;
    };
    
    std::vector<SummaryEntry> summaries;
    
    for (const auto& entry : timesByName) {
        const auto& name = entry.first;
        const auto& times = entry.second;
        
        double totalTime = std::accumulate(times.begin(), times.end(), 0.0);
        double avgTime = totalTime / times.size();
        double minTime = *std::min_element(times.begin(), times.end());
        double maxTime = *std::max_element(times.begin(), times.end());
        
        summaries.push_back({name, static_cast<int>(times.size()), totalTime, avgTime, minTime, maxTime});
    }
    
    // Sort by total time or name
    if (sortByTime) {
        std::sort(summaries.begin(), summaries.end(),
                 [](const SummaryEntry& a, const SummaryEntry& b) {
                     return a.totalTime > b.totalTime;
                 });
    } else {
        std::sort(summaries.begin(), summaries.end(),
                 [](const SummaryEntry& a, const SummaryEntry& b) {
                     return a.name < b.name;
                 });
    }
    
    // Print the results
    std::cout << std::left << std::setw(30) << "Section"
              << std::right << std::setw(10) << "Count"
              << std::right << std::setw(15) << "Total (ms)"
              << std::right << std::setw(15) << "Avg (ms)"
              << std::right << std::setw(15) << "Min (ms)"
              << std::right << std::setw(15) << "Max (ms)"
              << std::endl;
    
    std::cout << std::string(100, '-') << std::endl;
    
    for (const auto& entry : summaries) {
        std::cout << std::left << std::setw(30) << entry.name
                  << std::right << std::setw(10) << entry.count
                  << std::right << std::setw(15) << std::fixed << std::setprecision(3) << entry.totalTime
                  << std::right << std::setw(15) << std::fixed << std::setprecision(3) << entry.avgTime
                  << std::right << std::setw(15) << std::fixed << std::setprecision(3) << entry.minTime
                  << std::right << std::setw(15) << std::fixed << std::setprecision(3) << entry.maxTime
                  << std::endl;
    }
}

void Profiler::setEnabled(bool enabled) {
    m_enabled = enabled;
}

bool Profiler::isEnabled() const {
    return m_enabled;
}

// ScopedProfile implementation
ScopedProfile::ScopedProfile(const std::string& name) {
    m_id = Profiler::getInstance().startSection(name);
}

ScopedProfile::~ScopedProfile() {
    Profiler::getInstance().endSection(m_id);
}