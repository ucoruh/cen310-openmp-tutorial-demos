#include "../include/system_topology.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <sstream>

// Windows-specific includes
#ifdef _WIN32
#include <windows.h>
#endif

SystemTopology::SystemTopology()
    : m_logicalProcessorCount(0),
      m_physicalPackageCount(0),
      m_coresPerPackage(0),
      m_numaNodeCount(0) {
}

SystemTopology::~SystemTopology() {
}

bool SystemTopology::detectTopology() {
#ifdef _WIN32
    return detectWindowsTopology();
#else
    std::cerr << "Topology detection is only supported on Windows." << std::endl;
    return false;
#endif
}

#ifdef _WIN32
bool SystemTopology::detectWindowsTopology() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    
    m_logicalProcessorCount = sysInfo.dwNumberOfProcessors;
    
    // Initialize mappings
    m_logicalToPackage.resize(m_logicalProcessorCount, -1);
    m_logicalToPhysicalCore.resize(m_logicalProcessorCount, -1);
    m_logicalToNumaNode.resize(m_logicalProcessorCount, -1);
    
    // Get processor information using GetLogicalProcessorInformation
    DWORD returnLength = 0;
    GetLogicalProcessorInformation(NULL, &returnLength);
    
    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        std::cerr << "Failed to get processor information. Error code: " 
                  << GetLastError() << std::endl;
        return false;
    }
    
    DWORD processorInfoCount = returnLength / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
    SYSTEM_LOGICAL_PROCESSOR_INFORMATION* buffer = 
        new SYSTEM_LOGICAL_PROCESSOR_INFORMATION[processorInfoCount];
    
    if (!GetLogicalProcessorInformation(buffer, &returnLength)) {
        std::cerr << "Failed to get processor information. Error code: " 
                  << GetLastError() << std::endl;
        delete[] buffer;
        return false;
    }
    
    // Process the information
    std::set<ULONG_PTR> packages;
    std::map<ULONG_PTR, int> coresPerPackage;
    
    for (DWORD i = 0; i < processorInfoCount; i++) {
        switch (buffer[i].Relationship) {
            case RelationProcessorCore: {
                // This is a physical processor core
                ULONG_PTR processorMask = buffer[i].ProcessorMask;
                
                // Count the number of logical processors in this core
                int logicalPerCore = 0;
                ULONG_PTR mask = processorMask;
                
                while (mask) {
                    if (mask & 1) {
                        logicalPerCore++;
                    }
                    mask >>= 1;
                }
                
                // Determine the package ID and core ID
                // This is simplified - in a real implementation, you'd need to
                // cross-reference with RelationProcessorPackage
                static int nextCoreId = 0;
                int coreId = nextCoreId++;
                
                // Map each logical processor to this core
                mask = processorMask;
                for (int lp = 0; lp < m_logicalProcessorCount; lp++) {
                    if (processorMask & (1ULL << lp)) {
                        m_logicalToPhysicalCore[lp] = coreId;
                    }
                }
                break;
            }
            
            case RelationNumaNode: {
                // NUMA node information
                ULONG_PTR processorMask = buffer[i].ProcessorMask;
                DWORD nodeNumber = buffer[i].NumaNode.NodeNumber;
                
                // Map each logical processor to this NUMA node
                for (int lp = 0; lp < m_logicalProcessorCount; lp++) {
                    if (processorMask & (1ULL << lp)) {
                        m_logicalToNumaNode[lp] = nodeNumber;
                        m_numaToLogical[nodeNumber].push_back(lp);
                    }
                }
                break;
            }
            
            case RelationProcessorPackage: {
                // This is a physical processor package
                ULONG_PTR processorMask = buffer[i].ProcessorMask;
                
                // Add this package to the set of packages
                packages.insert(packages.size());
                ULONG_PTR packageId = packages.size() - 1;
                
                // Map each logical processor to this package
                for (int lp = 0; lp < m_logicalProcessorCount; lp++) {
                    if (processorMask & (1ULL << lp)) {
                        m_logicalToPackage[lp] = packageId;
                    }
                }
                
                // Count cores in this package
                coresPerPackage[packageId] = 0;
                for (int lp = 0; lp < m_logicalProcessorCount; lp++) {
                    if ((processorMask & (1ULL << lp)) && 
                        (lp == 0 || m_logicalToPhysicalCore[lp] != m_logicalToPhysicalCore[lp-1])) {
                        coresPerPackage[packageId]++;
                    }
                }
                break;
            }
        }
    }
    
    // Calculate the final values
    m_physicalPackageCount = packages.size();
    
    if (m_physicalPackageCount > 0) {
        // Calculate average cores per package
        int totalCores = 0;
        for (const auto& pair : coresPerPackage) {
            totalCores += pair.second;
        }
        m_coresPerPackage = totalCores / m_physicalPackageCount;
    }
    
    m_numaNodeCount = m_numaToLogical.size();
    
    // Clean up
    delete[] buffer;
    
    // If we didn't find NUMA information with GetLogicalProcessorInformation,
    // try to get it using the NUMA API
    if (m_numaNodeCount == 0) {
        detectNumaTopology();
    }
    
    return true;
}
#endif

void SystemTopology::detectNumaTopology() {
#ifdef _WIN32
    ULONG highestNodeNumber;
    if (GetNumaHighestNodeNumber(&highestNodeNumber)) {
        m_numaNodeCount = highestNodeNumber + 1;
        
        for (ULONG i = 0; i <= highestNodeNumber; i++) {
            ULONGLONG processorMask;
            if (GetNumaNodeProcessorMask(i, &processorMask)) {
                for (int lp = 0; lp < m_logicalProcessorCount; lp++) {
                    if (processorMask & (1ULL << lp)) {
                        m_logicalToNumaNode[lp] = i;
                        m_numaToLogical[i].push_back(lp);
                    }
                }
            }
        }
    }
#endif
}

int SystemTopology::getLogicalProcessorCount() const {
    return m_logicalProcessorCount;
}

int SystemTopology::getPhysicalPackageCount() const {
    return m_physicalPackageCount;
}

int SystemTopology::getCoresPerPackage() const {
    return m_coresPerPackage;
}

int SystemTopology::getNumaNodeCount() const {
    return m_numaNodeCount;
}

void SystemTopology::displayProcessorMap() const {
    std::cout << "\n=== Processor Map ===" << std::endl;
    
    std::cout << std::setw(10) << "Logical" << std::setw(10) << "Package" 
              << std::setw(10) << "Core" << std::setw(10) << "NUMA Node" << std::endl;
    std::cout << std::string(40, '-') << std::endl;
    
    for (int i = 0; i < m_logicalProcessorCount; i++) {
        std::cout << std::setw(10) << i 
                  << std::setw(10) << m_logicalToPackage[i]
                  << std::setw(10) << m_logicalToPhysicalCore[i]
                  << std::setw(10) << m_logicalToNumaNode[i]
                  << std::endl;
    }
}

void SystemTopology::displayNumaInfo() const {
    if (m_numaNodeCount <= 0) {
        std::cout << "No NUMA information available." << std::endl;
        return;
    }
    
    std::cout << "\n=== NUMA Topology ===" << std::endl;
    
    for (const auto& pair : m_numaToLogical) {
        int numaNode = pair.first;
        const std::vector<int>& processors = pair.second;
        
        std::cout << "NUMA Node " << numaNode << " contains " 
                  << processors.size() << " logical processors: ";
        
        for (size_t i = 0; i < processors.size(); i++) {
            std::cout << processors[i];
            if (i < processors.size() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << std::endl;
    }
}

int SystemTopology::getNumaNodeForProcessor(int logicalProcessor) const {
    if (logicalProcessor >= 0 && logicalProcessor < m_logicalProcessorCount) {
        return m_logicalToNumaNode[logicalProcessor];
    }
    return -1;
}

int SystemTopology::getPhysicalCoreId(int logicalProcessor) const {
    if (logicalProcessor >= 0 && logicalProcessor < m_logicalProcessorCount) {
        return m_logicalToPhysicalCore[logicalProcessor];
    }
    return -1;
}

int SystemTopology::getPackageId(int logicalProcessor) const {
    if (logicalProcessor >= 0 && logicalProcessor < m_logicalProcessorCount) {
        return m_logicalToPackage[logicalProcessor];
    }
    return -1;
}

std::vector<int> SystemTopology::getProcessorsForNumaNode(int numaNode) const {
    auto it = m_numaToLogical.find(numaNode);
    if (it != m_numaToLogical.end()) {
        return it->second;
    }
    return std::vector<int>();
}