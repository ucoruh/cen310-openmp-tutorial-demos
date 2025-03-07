#ifndef SIMD_EXAMPLES_H
#define SIMD_EXAMPLES_H

#include <vector>

// Basic SIMD operations
void runBasicSIMD();
double basicVectorAddition(const std::vector<double>& a, const std::vector<double>& b, std::vector<double>& c, bool useSimd);

// Array operations with SIMD
void runArrayOperations();
void arrayElementWiseMultiply(const std::vector<float>& a, const std::vector<float>& b, std::vector<float>& c, bool useSimd);
float arrayReduction(const std::vector<float>& a, bool useSimd);

// Complex math operations
void runComplexMath();
void computeTranscendental(std::vector<double>& input, std::vector<double>& output, bool useSimd);

// SIMD and memory alignment
void runSIMDAlignment();
void alignedVsUnaligned(bool useAligned);

// SIMD width adaptation
void runSIMDWidth();
void vectorizeWithDifferentWidths(int simdWidth);

// Mixed precision operations
void runMixedPrecision();
void mixedTypeOperations(std::vector<float>& floatVec, std::vector<int>& intVec, std::vector<float>& result, bool useSimd);

// SIMD with thread parallelism
void runSIMDParallelism();
void simdWithThreads(int numThreads, size_t vectorSize);

// Assembly analysis
void runASMAnalysis();
void analyzeAssemblyOutput();

// SIMD verification
void verifySIMD();
bool verifyVectorization(const char* loopDescription);

#endif // SIMD_EXAMPLES_H