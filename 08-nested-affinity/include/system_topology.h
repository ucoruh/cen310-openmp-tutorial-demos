#pragma once

#include <vector>
#include <string>
#include <map>
#include <set>

/**
 * @class SystemTopology
 * @brief Detects and stores system hardware topology information
 * 
 * This class provides functionality to detect the CPU topology including:
 * - Number of physical processors/packages
 * - Number of cores per physical processor
 * - Number of logical processors (hardware threads)
 * - NUMA node information
 * - Core to NUMA node mapping
 */
class SystemTopology {
public:
    SystemTopology();
    ~SystemTopology();

    /**
     * @brief Detect system topology using Windows API
     * @return true if detection was successful
     */
    bool detectTopology();

    /**
     * @brief Get the total number of logical processors
     * @return Number of logical processors
     */
    int getLogicalProcessorCount() const;

    /**
     * @brief Get the number of physical packages (sockets)
     * @return Number of physical packages
     */
    int getPhysicalPackageCount() const;

    /**
     * @brief Get the number of cores per physical package
     * @return Number of cores per package
     */
    int getCoresPerPackage() const;

    /**
     * @brief Get the number of NUMA nodes
     * @return Number of NUMA nodes
     */
    int getNumaNodeCount() const;

    /**
     * @brief Display a map of processors showing their relationships
     */
    void displayProcessorMap() const;

    /**
     * @brief Display NUMA node information
     */
    void displayNumaInfo() const;

    /**
     * @brief Get the NUMA node for a given logical processor
     * @param logicalProcessor Logical processor ID
     * @return NUMA node ID, or -1 if unknown
     */
    int getNumaNodeForProcessor(int logicalProcessor) const;

    /**
     * @brief Get the physical core ID for a logical processor
     * @param logicalProcessor Logical processor ID
     * @return Physical core ID, or -1 if unknown
     */
    int getPhysicalCoreId(int logicalProcessor) const;

    /**
     * @brief Get the physical package ID for a logical processor
     * @param logicalProcessor Logical processor ID
     * @return Physical package ID, or -1 if unknown
     */
    int getPackageId(int logicalProcessor) const;

    /**
     * @brief Get the list of logical processors for a specific NUMA node
     * @param numaNode NUMA node ID
     * @return Vector of logical processor IDs
     */
    std::vector<int> getProcessorsForNumaNode(int numaNode) const;

private:
    // Topology data
    int m_logicalProcessorCount;
    int m_physicalPackageCount;
    int m_coresPerPackage;
    int m_numaNodeCount;

    // Mappings
    std::vector<int> m_logicalToPackage;      // Maps logical CPU ID -> package ID
    std::vector<int> m_logicalToPhysicalCore; // Maps logical CPU ID -> physical core ID
    std::vector<int> m_logicalToNumaNode;     // Maps logical CPU ID -> NUMA node
    std::map<int, std::vector<int>> m_numaToLogical; // Maps NUMA node -> logical CPU IDs

    // Helper methods for Windows-specific detection
    bool detectWindowsTopology();
    void detectNumaTopology();
};