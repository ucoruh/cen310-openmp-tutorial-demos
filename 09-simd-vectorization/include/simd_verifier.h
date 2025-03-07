#ifndef SIMD_VERIFIER_H
#define SIMD_VERIFIER_H

#include <string>

// SIMD verification functions
void verifySIMD();

// Check if a loop is being vectorized
bool verifyVectorization(const char* loopDescription);

// Detect if SIMD instructions are being used at runtime
bool detectSIMDUsage();

// Estimate SIMD efficiency for a particular operation
double estimateSIMDEfficiency(const char* operationName);

// Identify potential vectorization bottlenecks
void identifyVectorizationBottlenecks(const char* functionName);

// Display vectorization report
void displayVectorizationReport();

#endif // SIMD_VERIFIER_H