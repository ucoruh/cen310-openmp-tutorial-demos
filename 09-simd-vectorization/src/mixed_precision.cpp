#include "../include/simd_examples.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <random>
#include <omp.h>

// Mixed type operations without SIMD
void scalarMixedTypeOperations(const std::vector<float>& floatVec, 
                              const std::vector<int>& intVec, 
                              std::vector<float>& result) {
    const size_t size = floatVec.size();
    for (size_t i = 0; i < size; ++i) {
        // Mix float and int operations
        result[i] = floatVec[i] * static_cast<float>(intVec[i]) + floatVec[i] / (static_cast<float>(intVec[i]) + 1.0f);
    }
}

// Mixed type operations with SIMD
void simdMixedTypeOperations(const std::vector<float>& floatVec, 
                            const std::vector<int>& intVec, 
                            std::vector<float>& result) {
    const size_t size = floatVec.size();
    
    #pragma omp simd
    for (size_t i = 0; i < size; ++i) {
        // Same operation as above, with SIMD directive
        result[i] = floatVec[i] * static_cast<float>(intVec[i]) + floatVec[i] / (static_cast<float>(intVec[i]) + 1.0f);
    }
}

// Int to float conversion without SIMD
void scalarIntToFloatConversion(const std::vector<int>& intVec, 
                               std::vector<float>& floatVec) {
    const size_t size = intVec.size();
    for (size_t i = 0; i < size; ++i) {
        floatVec[i] = static_cast<float>(intVec[i]);
    }
}

// Int to float conversion with SIMD
void simdIntToFloatConversion(const std::vector<int>& intVec, 
                             std::vector<float>& floatVec) {
    const size_t size = intVec.size();
    
    #pragma omp simd
    for (size_t i = 0; i < size; ++i) {
        floatVec[i] = static_cast<float>(intVec[i]);
    }
}

// Float to int conversion (with rounding) without SIMD
void scalarFloatToIntConversion(const std::vector<float>& floatVec, 
                               std::vector<int>& intVec) {
    const size_t size = floatVec.size();
    for (size_t i = 0; i < size; ++i) {
        intVec[i] = static_cast<int>(floatVec[i] + 0.5f); // Round to nearest
    }
}

// Float to int conversion (with rounding) with SIMD
void simdFloatToIntConversion(const std::vector<float>& floatVec, 
                             std::vector<int>& intVec) {
    const size_t size = floatVec.size();
    
    #pragma omp simd
    for (size_t i = 0; i < size; ++i) {
        intVec[i] = static_cast<int>(floatVec[i] + 0.5f); // Round to nearest
    }
}

// Mixed precision operations function exposed to other modules
void mixedTypeOperations(std::vector<float>& floatVec, std::vector<int>& intVec, 
                        std::vector<float>& result, bool useSimd) {
    if (useSimd) {
        simdMixedTypeOperations(floatVec, intVec, result);
    } else {
        scalarMixedTypeOperations(floatVec, intVec, result);
    }
}

// Run mixed precision operations demo
void runMixedPrecision() {
    std::cout << "\n=== Mixed Precision Operations Demo ===" << std::endl;
    
    // Vector size
    const size_t size = 50000000; // 50 million elements
    
    std::cout << "Initializing vectors with " << size << " elements..." << std::endl;
    
    // Initialize random number generators
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> floatDis(0.1f, 100.0f);
    std::uniform_int_distribution<int> intDis(1, 100);
    
    // Initialize input vectors
    std::vector<float> floatVec(size);
    std::vector<int> intVec(size);
    std::vector<float> result_scalar(size, 0.0f);
    std::vector<float> result_simd(size, 0.0f);
    
    // Fill vectors with random values
    for (size_t i = 0; i < size; ++i) {
        floatVec[i] = floatDis(gen);
        intVec[i] = intDis(gen);
    }
    
    // Measure mixed type operations performance
    std::cout << "\n--- Mixed Type Arithmetic Operations ---" << std::endl;
    
    // Scalar mixed operations
    auto start = std::chrono::high_resolution_clock::now();
    scalarMixedTypeOperations(floatVec, intVec, result_scalar);
    auto end = std::chrono::high_resolution_clock::now();
    double timeScalarMixed = std::chrono::duration<double, std::milli>(end - start).count();
    
    // SIMD mixed operations
    start = std::chrono::high_resolution_clock::now();
    simdMixedTypeOperations(floatVec, intVec, result_simd);
    end = std::chrono::high_resolution_clock::now();
    double timeSimdMixed = std::chrono::duration<double, std::milli>(end - start).count();
    
    // Calculate speedup
    double speedupMixed = timeScalarMixed / timeSimdMixed;
    
    // Verify results
    bool mixedResultsMatch = true;
    for (size_t i = 0; i < size; ++i) {
        if (std::abs(result_scalar[i] - result_simd[i]) > 1e-5f) {
            mixedResultsMatch = false;
            std::cout << "Mixed operations mismatch at index " << i << ": "
                      << result_scalar[i] << " vs " << result_simd[i] << std::endl;
            break;
        }
    }
    
    // Print mixed operations results
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "Scalar mixed operations time: " << timeScalarMixed << " ms" << std::endl;
    std::cout << "SIMD mixed operations time: " << timeSimdMixed << " ms" << std::endl;
    std::cout << "Speedup: " << speedupMixed << "x" << std::endl;
    std::cout << "Results verification: " << (mixedResultsMatch ? "PASSED" : "FAILED") << std::endl;
    
    // Temporary vectors for conversion tests
    std::vector<float> floatResult(size);
    std::vector<int> intResult(size);
    
    // Measure int to float conversion performance
    std::cout << "\n--- Integer to Float Conversion ---" << std::endl;
    
    // Scalar int to float conversion
    start = std::chrono::high_resolution_clock::now();
    scalarIntToFloatConversion(intVec, floatResult);
    end = std::chrono::high_resolution_clock::now();
    double timeScalarIntToFloat = std::chrono::duration<double, std::milli>(end - start).count();
    
    // SIMD int to float conversion
    start = std::chrono::high_resolution_clock::now();
    simdIntToFloatConversion(intVec, floatResult);
    end = std::chrono::high_resolution_clock::now();
    double timeSimdIntToFloat = std::chrono::duration<double, std::milli>(end - start).count();
    
    // Calculate speedup
    double speedupIntToFloat = timeScalarIntToFloat / timeSimdIntToFloat;
    
    // Print int to float conversion results
    std::cout << "Scalar int to float conversion time: " << timeScalarIntToFloat << " ms" << std::endl;
    std::cout << "SIMD int to float conversion time: " << timeSimdIntToFloat << " ms" << std::endl;
    std::cout << "Speedup: " << speedupIntToFloat << "x" << std::endl;
    
    // Measure float to int conversion performance
    std::cout << "\n--- Float to Integer Conversion ---" << std::endl;
    
    // Scalar float to int conversion
    start = std::chrono::high_resolution_clock::now();
    scalarFloatToIntConversion(floatVec, intResult);
    end = std::chrono::high_resolution_clock::now();
    double timeScalarFloatToInt = std::chrono::duration<double, std::milli>(end - start).count();
    
    // SIMD float to int conversion
    start = std::chrono::high_resolution_clock::now();
    simdFloatToIntConversion(floatVec, intResult);
    end = std::chrono::high_resolution_clock::now();
    double timeSimdFloatToInt = std::chrono::duration<double, std::milli>(end - start).count();
    
    // Calculate speedup
    double speedupFloatToInt = timeScalarFloatToInt / timeSimdFloatToInt;
    
    // Print float to int conversion results
    std::cout << "Scalar float to int conversion time: " << timeScalarFloatToInt << " ms" << std::endl;
    std::cout << "SIMD float to int conversion time: " << timeSimdFloatToInt << " ms" << std::endl;
    std::cout << "Speedup: " << speedupFloatToInt << "x" << std::endl;
    
    // Mixed precision explanation
    std::cout << "\n=== Mixed Precision SIMD Explained ===" << std::endl;
    
    std::cout << "1. Mixed Type Operations:" << std::endl;
    std::cout << "   - SIMD can handle operations between different data types" << std::endl;
    std::cout << "   - Type conversions happen automatically in hardware" << std::endl;
    std::cout << "   - Generally, operations convert to the higher precision type" << std::endl;
    
    std::cout << "\n2. Type Conversion Performance:" << std::endl;
    std::cout << "   - Int to float conversion: " << (speedupIntToFloat > 1.2 ? "Significant" : "Modest") << " speedup" << std::endl;
    std::cout << "   - Float to int conversion: " << (speedupFloatToInt > 1.2 ? "Significant" : "Modest") << " speedup" << std::endl;
    
    std::cout << "\n3. Key Considerations for Mixed Precision:" << std::endl;
    std::cout << "   - Type conversions can partially offset SIMD benefits" << std::endl;
    std::cout << "   - Modern CPUs have dedicated SIMD conversion instructions" << std::endl;
    std::cout << "   - Minimize conversions by using consistent types where possible" << std::endl;
    std::cout << "   - Different rounding modes can affect float-to-int conversions" << std::endl;
    
    std::cout << "\nBest Practices:" << std::endl;
    std::cout << "- Use the same data type within a SIMD loop when possible" << std::endl;
    std::cout << "- If conversions are necessary, batch them together" << std::endl;
    std::cout << "- For float-to-int, be explicit about rounding mode" << std::endl;
    std::cout << "- Consider using intrinsics for more control over conversions" << std::endl;
}