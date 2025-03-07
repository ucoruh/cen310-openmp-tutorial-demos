#ifndef CPU_FEATURES_H
#define CPU_FEATURES_H

#include <string>
#include <vector>

// CPU feature detection structure
struct CPUFeatures {
    bool hasSSE;
    bool hasSSE2;
    bool hasSSE3;
    bool hasSSSE3;
    bool hasSSE41;
    bool hasSSE42;
    bool hasAVX;
    bool hasAVX2;
    bool hasAVX512F;
    int maxSIMDWidth;
    std::string cpuVendor;
    std::string cpuBrand;
};

// Main detection function
void detectCPUFeatures();

// Display detected features
void displayCPUFeatures();

// Get optimal SIMD width based on CPU capabilities
int getOptimalSIMDWidth();

// Get CPU features structure
CPUFeatures& getCPUFeatures();

#endif // CPU_FEATURES_H