#include "../include/cpu_features.h"
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>

// Windows-specific headers for CPU detection
#ifdef _WIN32
#include <windows.h>
#include <intrin.h>
#endif

// Static instance of CPU features
static CPUFeatures g_cpuFeatures;

// Implementation of CPU detection logic
void detectCPUFeatures() {
    // Initialize CPU features to false
    g_cpuFeatures.hasSSE = false;
    g_cpuFeatures.hasSSE2 = false;
    g_cpuFeatures.hasSSE3 = false;
    g_cpuFeatures.hasSSSE3 = false;
    g_cpuFeatures.hasSSE41 = false;
    g_cpuFeatures.hasSSE42 = false;
    g_cpuFeatures.hasAVX = false;
    g_cpuFeatures.hasAVX2 = false;
    g_cpuFeatures.hasAVX512F = false;
    g_cpuFeatures.maxSIMDWidth = 0;
    
    // Use CPUID to detect CPU features
#ifdef _WIN32
    int cpuInfo[4] = {0};
    
    // Get vendor ID
    __cpuid(cpuInfo, 0);
    char vendor[13];
    *reinterpret_cast<int*>(vendor) = cpuInfo[1];
    *reinterpret_cast<int*>(vendor+4) = cpuInfo[3];
    *reinterpret_cast<int*>(vendor+8) = cpuInfo[2];
    vendor[12] = '\0';
    g_cpuFeatures.cpuVendor = vendor;
    
    // Get brand string
    char brand[49];
    __cpuid(cpuInfo, 0x80000000);
    unsigned int maxExtendedId = cpuInfo[0];
    if (maxExtendedId >= 0x80000004) {
        brand[0] = '\0';
        for (int i = 0x80000002; i <= 0x80000004; ++i) {
            __cpuid(cpuInfo, i);
            memcpy(brand + (i - 0x80000002) * 16, cpuInfo, sizeof(cpuInfo));
        }
        brand[48] = '\0';
        g_cpuFeatures.cpuBrand = brand;
    } else {
        g_cpuFeatures.cpuBrand = "Unknown";
    }
    
    // Check for basic SIMD features
    __cpuid(cpuInfo, 1);
    g_cpuFeatures.hasSSE = (cpuInfo[3] & (1 << 25)) != 0;
    g_cpuFeatures.hasSSE2 = (cpuInfo[3] & (1 << 26)) != 0;
    g_cpuFeatures.hasSSE3 = (cpuInfo[2] & (1 << 0)) != 0;
    g_cpuFeatures.hasSSSE3 = (cpuInfo[2] & (1 << 9)) != 0;
    g_cpuFeatures.hasSSE41 = (cpuInfo[2] & (1 << 19)) != 0;
    g_cpuFeatures.hasSSE42 = (cpuInfo[2] & (1 << 20)) != 0;
    g_cpuFeatures.hasAVX = (cpuInfo[2] & (1 << 28)) != 0;
    
    // Check for AVX2
    if (maxExtendedId >= 7) {
        __cpuidex(cpuInfo, 7, 0);
        g_cpuFeatures.hasAVX2 = (cpuInfo[1] & (1 << 5)) != 0;
        g_cpuFeatures.hasAVX512F = (cpuInfo[1] & (1 << 16)) != 0;
    }
#endif
    
    // Determine maximum SIMD width based on detected features
    if (g_cpuFeatures.hasAVX512F) {
        g_cpuFeatures.maxSIMDWidth = 512;
    } else if (g_cpuFeatures.hasAVX2 || g_cpuFeatures.hasAVX) {
        g_cpuFeatures.maxSIMDWidth = 256;
    } else if (g_cpuFeatures.hasSSE2) {
        g_cpuFeatures.maxSIMDWidth = 128;
    } else if (g_cpuFeatures.hasSSE) {
        g_cpuFeatures.maxSIMDWidth = 128; // SSE also works with 128-bit
    } else {
        g_cpuFeatures.maxSIMDWidth = 64; // MMX or scalar
    }
}

// Display detected CPU features
void displayCPUFeatures() {
    std::cout << "\n=== CPU SIMD Capabilities ===" << std::endl;
    std::cout << "CPU Vendor: " << g_cpuFeatures.cpuVendor << std::endl;
    std::cout << "CPU Brand: " << g_cpuFeatures.cpuBrand << std::endl;
    std::cout << "\nSIMD Support:" << std::endl;
    
    // Create a table format
    std::cout << std::left << std::setw(15) << "Feature" << std::setw(10) << "Support" << std::endl;
    std::cout << std::string(25, '-') << std::endl;
    
    std::cout << std::left << std::setw(15) << "SSE" << std::setw(10) << (g_cpuFeatures.hasSSE ? "Yes" : "No") << std::endl;
    std::cout << std::left << std::setw(15) << "SSE2" << std::setw(10) << (g_cpuFeatures.hasSSE2 ? "Yes" : "No") << std::endl;
    std::cout << std::left << std::setw(15) << "SSE3" << std::setw(10) << (g_cpuFeatures.hasSSE3 ? "Yes" : "No") << std::endl;
    std::cout << std::left << std::setw(15) << "SSSE3" << std::setw(10) << (g_cpuFeatures.hasSSSE3 ? "Yes" : "No") << std::endl;
    std::cout << std::left << std::setw(15) << "SSE4.1" << std::setw(10) << (g_cpuFeatures.hasSSE41 ? "Yes" : "No") << std::endl;
    std::cout << std::left << std::setw(15) << "SSE4.2" << std::setw(10) << (g_cpuFeatures.hasSSE42 ? "Yes" : "No") << std::endl;
    std::cout << std::left << std::setw(15) << "AVX" << std::setw(10) << (g_cpuFeatures.hasAVX ? "Yes" : "No") << std::endl;
    std::cout << std::left << std::setw(15) << "AVX2" << std::setw(10) << (g_cpuFeatures.hasAVX2 ? "Yes" : "No") << std::endl;
    std::cout << std::left << std::setw(15) << "AVX-512F" << std::setw(10) << (g_cpuFeatures.hasAVX512F ? "Yes" : "No") << std::endl;
    std::cout << std::string(25, '-') << std::endl;
    std::cout << "Maximum SIMD Width: " << g_cpuFeatures.maxSIMDWidth << " bits" << std::endl;
    
    // Additional information
    std::cout << "\nSIMD Instruction Set Notes:" << std::endl;
    std::cout << "- SSE/SSE2: 128-bit registers (XMM0-XMM7)" << std::endl;
    std::cout << "- AVX/AVX2: 256-bit registers (YMM0-YMM15)" << std::endl;
    if (g_cpuFeatures.hasAVX512F) {
        std::cout << "- AVX-512: 512-bit registers (ZMM0-ZMM31)" << std::endl;
    }
    
    // Recommendations based on CPU capabilities
    std::cout << "\nRecommendations for this CPU:" << std::endl;
    if (g_cpuFeatures.hasAVX2) {
        std::cout << "- Optimal SIMD width: 256 bits (8 floats or 4 doubles per operation)" << std::endl;
        std::cout << "- Compiler flags: /arch:AVX2" << std::endl;
    } else if (g_cpuFeatures.hasAVX) {
        std::cout << "- Optimal SIMD width: 256 bits (8 floats or 4 doubles per operation)" << std::endl;
        std::cout << "- Compiler flags: /arch:AVX" << std::endl;
    } else if (g_cpuFeatures.hasSSE41) {
        std::cout << "- Optimal SIMD width: 128 bits (4 floats or 2 doubles per operation)" << std::endl;
        std::cout << "- Compiler flags: /arch:SSE2" << std::endl;
    } else {
        std::cout << "- Limited SIMD capabilities, performance may be suboptimal" << std::endl;
    }
}

// Get the optimal SIMD width for the current CPU
int getOptimalSIMDWidth() {
    return g_cpuFeatures.maxSIMDWidth;
}

// Get a reference to the CPU features structure
CPUFeatures& getCPUFeatures() {
    return g_cpuFeatures;
}