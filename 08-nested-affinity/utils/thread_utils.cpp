#include "../include/thread_utils.h"
#include <omp.h>
#include <string>
#include <vector>
#include <iostream>

// Windows-specific includes
#ifdef _WIN32
#include <windows.h>
#include <processthreadsapi.h>
#endif

int GetThreadCoreID() {
#ifdef _WIN32
    // Get the current thread ID
    HANDLE hThread = GetCurrentThread();
    
    // Get processor number where this thread is running
    return GetCurrentProcessorNumber();
#else
    // For non-Windows platforms, return -1
    return -1;
#endif
}

bool SetThreadAffinity(int coreId) {
#ifdef _WIN32
    // Get the current thread handle
    HANDLE hThread = GetCurrentThread();
    
    // Create a mask with only the specified core bit set
    DWORD_PTR mask = 1ULL << coreId;
    
    // Set the thread affinity mask
    DWORD_PTR prevMask = SetThreadAffinityMask(hThread, mask);
    
    return (prevMask != 0);
#else
    // For non-Windows platforms, return false
    return false;
#endif
}

bool SetThreadAffinityMask(const std::vector<int>& coreIds) {
#ifdef _WIN32
    // Get the current thread handle
    HANDLE hThread = GetCurrentThread();
    
    // Create a mask with bits set for each specified core
    DWORD_PTR mask = 0;
    for (int coreId : coreIds) {
        mask |= (1ULL << coreId);
    }
    
    // Set the thread affinity mask
    DWORD_PTR prevMask = SetThreadAffinityMask(hThread, mask);
    
    return (prevMask != 0);
#else
    // For non-Windows platforms, return false
    return false;
#endif
}

std::vector<int> GetThreadAffinityMask() {
    std::vector<int> result;
    
#ifdef _WIN32
    // Get the current thread handle
    HANDLE hThread = GetCurrentThread();
    
    // Get the system affinity mask for comparison
    DWORD_PTR processMask, systemMask;
    if (GetProcessAffinityMask(GetCurrentProcess(), &processMask, &systemMask)) {
        // Get the thread affinity mask
        DWORD_PTR threadMask = 0;
        GROUP_AFFINITY groupAffinity;
        
        if (GetThreadGroupAffinity(hThread, &groupAffinity)) {
            threadMask = groupAffinity.Mask;
        } else {
            threadMask = processMask; // Fallback to process mask
        }
        
        // Add each core ID where the bit is set
        for (int i = 0; i < 64; i++) {  // Assuming 64-bit DWORD_PTR
            if ((threadMask >> i) & 1) {
                result.push_back(i);
            }
        }
    }
#endif

    return result;
}

bool IsNestedParallelismEnabled() {
    return omp_get_nested() != 0;
}

bool SetNestedParallelism(bool enable) {
    int previous = omp_get_nested();
    omp_set_nested(enable ? 1 : 0);
    return previous != 0;
}

bool InParallelRegion() {
    return omp_in_parallel() != 0;
}

int GetAncestorThreadNum(int level) {
    // In OpenMP 2.0, we can only get the current thread ID
    if (level == 0) {
        return omp_get_thread_num();
    }
    return -1; // Not supported in this OpenMP version
}