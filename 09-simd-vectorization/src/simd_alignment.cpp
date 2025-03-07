#include "../include/simd_examples.h"
#include "../include/cpu_features.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <random>
#include <omp.h>
#include <memory>

// Aligned memory allocation function - allocates memory aligned to the specified boundary
template<typename T>
T* alignedAlloc(size_t size, size_t alignment) {
#ifdef _WIN32
    return static_cast<T*>(_aligned_malloc(size * sizeof(T), alignment));
#else
    T* ptr = nullptr;
    posix_memalign(reinterpret_cast<void**>(&ptr), alignment, size * sizeof(T));
    return ptr;
#endif
}

// Aligned memory deallocation
template<typename T>
void alignedFree(T* ptr) {
#ifdef _WIN32
    _aligned_free(ptr);
#else
    free(ptr);
#endif
}

// Unaligned vector operation
void unalignedVectorOperation(const float* a, const float* b, float* c, int size) {
    #pragma omp simd
    for (int i = 0; i < size; ++i) {
        c[i] = a[i] * b[i] + a[i];
    }
}

// Aligned vector operation with alignment hint for the compiler
void alignedVectorOperation(const float* __restrict a, const float* __restrict b, float* __restrict c, int size) {
    // Tell the compiler the arrays are aligned (alignment value depends on SIMD width)
    int alignmentBytes = getOptimalSIMDWidth() / 8; // Convert bits to bytes
    
    // Use a simple operation that benefits from aligned loads/stores
    #pragma omp simd aligned(a, b, c: alignmentBytes)
    for (int i = 0; i < size; ++i) {
        c[i] = a[i] * b[i] + a[i];
    }
}

// Function to compare aligned vs unaligned memory access
void alignedVsUnaligned(bool useAligned) {
    const int size = 50000000; // 50 million elements
    const size_t alignment = 64;  // Align to 64-byte boundary (AVX-512 register size)
    
    float *a, *b, *c;
    
    if (useAligned) {
        // Allocate aligned memory
        a = alignedAlloc<float>(size, alignment);
        b = alignedAlloc<float>(size, alignment);
        c = alignedAlloc<float>(size, alignment);
    } else {
        // Use standard allocation (potentially unaligned)
        a = new float[size];
        b = new float[size];
        c = new float[size];
    }
    
    // Initialize with random data
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    
    for (int i = 0; i < size; ++i) {
        a[i] = dis(gen);
        b[i] = dis(gen);
    }
    
    // Warm up
    for (int i = 0; i < 1000; ++i) {
        c[i] = a[i] * b[i] + a[i];
    }
    
    // Measure execution time
    auto start = std::chrono::high_resolution_clock::now();
    
    if (useAligned) {
        alignedVectorOperation(a, b, c, size);
    } else {
        unalignedVectorOperation(a, b, c, size);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    double executionTime = std::chrono::duration<double, std::milli>(end - start).count();
    
    // Print results
    std::cout << "Memory alignment: " << (useAligned ? "Aligned to " + std::to_string(alignment) + " bytes" : "Unaligned") << std::endl;
    std::cout << "Execution time: " << executionTime << " ms" << std::endl;
    
    // Free memory
    if (useAligned) {
        alignedFree(a);
        alignedFree(b);
        alignedFree(c);
    } else {
        delete[] a;
        delete[] b;
        delete[] c;
    }
}

// Run SIMD alignment demo
void runSIMDAlignment() {
    std::cout << "\n=== Memory Alignment Optimization Demo ===" << std::endl;
    
    // Get the optimal SIMD width for the current CPU
    int simdWidth = getOptimalSIMDWidth();
    int alignmentBytes = simdWidth / 8; // Convert bits to bytes
    
    std::cout << "CPU SIMD width: " << simdWidth << " bits (" << alignmentBytes << " bytes)" << std::endl;
    std::cout << "\nPerforming vector operations with different alignment strategies..." << std::endl;
    
    // Run with unaligned memory
    std::cout << "\n--- Unaligned Memory Access ---" << std::endl;
    alignedVsUnaligned(false);
    
    // Run with aligned memory
    std::cout << "\n--- Aligned Memory Access ---" << std::endl;
    alignedVsUnaligned(true);
    
    // Run both versions again for more consistent timing
    std::cout << "\n--- Second Run: Unaligned Memory Access ---" << std::endl;
    alignedVsUnaligned(false);
    
    std::cout << "\n--- Second Run: Aligned Memory Access ---" << std::endl;
    alignedVsUnaligned(true);
    
    // Explanation of memory alignment
    std::cout << "\n=== Understanding Memory Alignment in SIMD ===" << std::endl;
    
    std::cout << "Why alignment matters for SIMD operations:" << std::endl;
    std::cout << "1. SIMD instructions operate on multiple data elements in parallel" << std::endl;
    std::cout << "2. Some SIMD instructions require or perform better with aligned memory" << std::endl;
    std::cout << "   - Aligned loads/stores can be faster than unaligned ones" << std::endl;
    std::cout << "   - Some older processors required alignment for certain instructions" << std::endl;
    
    std::cout << "\nAlignment requirements depend on SIMD width:" << std::endl;
    std::cout << "- SSE (128-bit): 16-byte alignment" << std::endl;
    std::cout << "- AVX (256-bit): 32-byte alignment" << std::endl;
    std::cout << "- AVX-512 (512-bit): 64-byte alignment" << std::endl;
    
    std::cout << "\nHow to ensure proper alignment:" << std::endl;
    std::cout << "1. Use aligned memory allocation (as demonstrated)" << std::endl;
    std::cout << "2. Provide alignment hints to the compiler with directives" << std::endl;
    std::cout << "3. For dynamic arrays, ensure the starting address is properly aligned" << std::endl;
    
    std::cout << "\nNote: Modern CPUs have reduced penalties for unaligned access," << std::endl;
    std::cout << "      but alignment can still provide performance benefits, especially" << std::endl;
    std::cout << "      for bandwidth-limited operations or older hardware." << std::endl;
}