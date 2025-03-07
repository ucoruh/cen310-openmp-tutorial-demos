#pragma once

#include <vector>
#include <string>

/**
 * @brief Get the current thread's core ID
 * @return Core ID where the thread is currently running
 */
int GetThreadCoreID();

/**
 * @brief Set thread affinity to a specific core
 * @param coreId Core ID to bind the thread to
 * @return true if successful
 */
bool SetThreadAffinity(int coreId);

/**
 * @brief Set thread affinity to a set of cores
 * @param coreIds Vector of core IDs to bind the thread to
 * @return true if successful
 */
bool SetThreadAffinityMask(const std::vector<int>& coreIds);

/**
 * @brief Get the thread affinity mask as a vector of core IDs
 * @return Vector of core IDs that the thread can run on
 */
std::vector<int> GetThreadAffinityMask();

/**
 * @brief Check if nested parallelism is enabled
 * @return true if nested parallelism is enabled
 */
bool IsNestedParallelismEnabled();

/**
 * @brief Enable or disable nested parallelism
 * @param enable true to enable, false to disable
 * @return Previous state
 */
bool SetNestedParallelism(bool enable);

/**
 * @brief Check if currently executing in a parallel region
 * @return true if in a parallel region
 */
bool InParallelRegion();

/**
 * @brief Get the ancestor thread number for a specific level
 * @param level Level to query (0 for the current level)
 * @return Thread number at the specified level, -1 if not available in this OpenMP version
 */
int GetAncestorThreadNum(int level);