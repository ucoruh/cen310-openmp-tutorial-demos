#include "../include/simd_verifier.h"
#include "../include/cpu_features.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <chrono>
#include <functional>
#include <map>
#include <cmath>
#include <omp.h>

// Structure to hold verification results
struct VerificationResult {
    std::string name;        // Name of the operation
    bool isVectorized;       // Indicates if vectorization was detected
    double efficiency;       // Estimated SIMD efficiency (0.0 to 1.0)
    std::string bottleneck;  // Identified bottleneck, if any
    double scalarTime;       // Execution time without SIMD
    double simdTime;         // Execution time with SIMD
    double speedup;          // Speedup factor
};

// Global map for storing verification results
std::map<std::string, VerificationResult> g_verificationResults;

// Simple timer for measuring function execution time
class Timer {
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_end;
    bool m_running;

public:
    Timer() : m_running(false) {}

    void start() {
        m_start = std::chrono::high_resolution_clock::now();
        m_running = true;
    }

    void stop() {
        m_end = std::chrono::high_resolution_clock::now();
        m_running = false;
    }

    double elapsedMilliseconds() const {
        auto duration = m_running 
            ? std::chrono::high_resolution_clock::now() - m_start
            : m_end - m_start;
        return std::chrono::duration<double, std::milli>(duration).count();
    }
};

// Vectorization test case for simple array addition
bool testSimpleVectorization() {
    const size_t size = 10000000; // 10 million elements
    std::vector<float> a(size, 1.0f);
    std::vector<float> b(size, 2.0f);
    std::vector<float> c(size, 0.0f);
    
    Timer timer;
    
    // First run: non-vectorized version (prevent auto-vectorization with volatile)
    timer.start();
    for (size_t i = 0; i < size; ++i) {
        volatile float av = a[i];
        volatile float bv = b[i];
        c[i] = av + bv;
    }
    timer.stop();
    double scalarTime = timer.elapsedMilliseconds();
    
    // Second run: vectorized version
    timer.start();
    #pragma omp simd
    for (size_t i = 0; i < size; ++i) {
        c[i] = a[i] + b[i];
    }
    timer.stop();
    double simdTime = timer.elapsedMilliseconds();
    
    // Calculate speedup
    double speedup = scalarTime / simdTime;
    
    // Store the result
    VerificationResult result;
    result.name = "Simple Array Addition";
    result.isVectorized = (speedup > 1.2); // Assume vectorized if decent speedup
    result.efficiency = std::min(1.0, speedup / 4.0); // Assume ideal speedup is 4x for SSE
    result.bottleneck = (speedup < 1.2) ? "Insufficient speedup" : "";
    result.scalarTime = scalarTime;
    result.simdTime = simdTime;
    result.speedup = speedup;
    
    g_verificationResults["SimpleAddition"] = result;
    
    return result.isVectorized;
}

// Vectorization test case for complex array operations
bool testComplexVectorization() {
    const size_t size = 1000000; // 1 million elements
    std::vector<double> a(size);
    std::vector<double> b(size);
    std::vector<double> c(size);
    
    // Initialize with some values
    for (size_t i = 0; i < size; ++i) {
        a[i] = std::sin(static_cast<double>(i) * 0.01);
        b[i] = std::cos(static_cast<double>(i) * 0.01);
    }
    
    Timer timer;
    
    // First run: non-vectorized version
    timer.start();
    for (size_t i = 0; i < size; ++i) {
        c[i] = std::sin(a[i]) * std::cos(b[i]) + std::sqrt(std::abs(a[i] * b[i]));
    }
    timer.stop();
    double scalarTime = timer.elapsedMilliseconds();
    
    // Second run: vectorized version
    timer.start();
    #pragma omp simd
    for (size_t i = 0; i < size; ++i) {
        c[i] = std::sin(a[i]) * std::cos(b[i]) + std::sqrt(std::abs(a[i] * b[i]));
    }
    timer.stop();
    double simdTime = timer.elapsedMilliseconds();
    
    // Calculate speedup
    double speedup = scalarTime / simdTime;
    
    // Store the result
    VerificationResult result;
    result.name = "Complex Math Operations";
    result.isVectorized = (speedup > 1.2);
    result.efficiency = std::min(1.0, speedup / 4.0);
    result.bottleneck = (speedup < 1.2) ? "Complex math functions may limit vectorization" : "";
    result.scalarTime = scalarTime;
    result.simdTime = simdTime;
    result.speedup = speedup;
    
    g_verificationResults["ComplexMath"] = result;
    
    return result.isVectorized;
}

// Vectorization test case for operations with conditionals
bool testConditionalVectorization() {
    const size_t size = 10000000; // 10 million elements
    std::vector<float> a(size);
    std::vector<float> b(size);
    std::vector<float> c(size);
    
    // Initialize with some values
    for (size_t i = 0; i < size; ++i) {
        a[i] = static_cast<float>(i % 256) / 256.0f;
        b[i] = static_cast<float>((i + 128) % 256) / 256.0f;
    }
    
    Timer timer;
    
    // First run: non-vectorized version
    timer.start();
    for (size_t i = 0; i < size; ++i) {
        if (a[i] > 0.5f) {
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
    timer.stop();
    double scalarTime = timer.elapsedMilliseconds();
    
    // Second run: vectorized version
    timer.start();
    #pragma omp simd
    for (size_t i = 0; i < size; ++i) {
        if (a[i] > 0.5f) {
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
    timer.stop();
    double simdTime = timer.elapsedMilliseconds();
    
    // Calculate speedup
    double speedup = scalarTime / simdTime;
    
    // Store the result
    VerificationResult result;
    result.name = "Operations with Conditionals";
    result.isVectorized = (speedup > 1.2);
    result.efficiency = std::min(1.0, speedup / 3.0); // Lower expected efficiency due to conditionals
    result.bottleneck = (speedup < 1.2) ? "Conditionals may be limiting vectorization" : "";
    result.scalarTime = scalarTime;
    result.simdTime = simdTime;
    result.speedup = speedup;
    
    g_verificationResults["Conditionals"] = result;
    
    return result.isVectorized;
}

// Vectorization test case for memory gather/scatter operations
bool testMemoryGatherScatter() {
    const size_t size = 1000000; // 1 million elements
    std::vector<float> a(size);
    std::vector<float> b(size);
    std::vector<int> indices(size);
    
    // Initialize with some values
    for (size_t i = 0; i < size; ++i) {
        a[i] = static_cast<float>(i);
        indices[i] = (i * 17) % size; // Create non-sequential access pattern
    }
    
    Timer timer;
    
    // First run: non-vectorized indirect access
    timer.start();
    for (size_t i = 0; i < size; ++i) {
        b[i] = a[indices[i]];
    }
    timer.stop();
    double scalarTime = timer.elapsedMilliseconds();
    
    // Second run: attempt vectorized indirect access
    timer.start();
    #pragma omp simd
    for (size_t i = 0; i < size; ++i) {
        b[i] = a[indices[i]];
    }
    timer.stop();
    double simdTime = timer.elapsedMilliseconds();
    
    // Calculate speedup
    double speedup = scalarTime / simdTime;
    
    // Store the result
    VerificationResult result;
    result.name = "Memory Gather/Scatter";
    result.isVectorized = (speedup > 1.2);
    result.efficiency = std::min(1.0, speedup / 2.0); // Lower expected efficiency due to memory access pattern
    result.bottleneck = (speedup < 1.2) ? "Indirect memory access patterns are difficult to vectorize" : "";
    result.scalarTime = scalarTime;
    result.simdTime = simdTime;
    result.speedup = speedup;
    
    g_verificationResults["GatherScatter"] = result;
    
    return result.isVectorized;
}

// Verify if a specific loop can be vectorized
bool verifyVectorization(const char* loopDescription) {
    std::string desc(loopDescription);
    
    // Check if we already have results for this loop
    auto it = g_verificationResults.find(desc);
    if (it != g_verificationResults.end()) {
        return it->second.isVectorized;
    }
    
    // If not found, run a generic test and return a default value
    std::cout << "Verification for '" << desc << "' not implemented." << std::endl;
    std::cout << "Running generic test instead..." << std::endl;
    
    return testSimpleVectorization();
}

// Detect if SIMD instructions are being used at runtime
bool detectSIMDUsage() {
    // This is a simplified approximation - in reality, detecting actual SIMD
    // instruction usage at runtime requires more sophisticated methods like
    // hardware performance counters or dynamic binary instrumentation
    
    // We'll use a simple heuristic: if a basic vectorizable loop shows significant
    // speedup, we'll assume SIMD instructions are being used
    
    const size_t size = 10000000; // 10 million elements
    std::vector<float> a(size, 1.0f);
    std::vector<float> b(size, 2.0f);
    std::vector<float> c(size, 0.0f);
    
    Timer timer;
    
    // First run: non-vectorized version with added barriers to prevent auto-vectorization
    timer.start();
    for (size_t i = 0; i < size; ++i) {
        volatile float av = a[i];
        volatile float bv = b[i];
        c[i] = av + bv;
    }
    timer.stop();
    double scalarTime = timer.elapsedMilliseconds();
    
    // Second run: vectorized version
    timer.start();
    #pragma omp simd
    for (size_t i = 0; i < size; ++i) {
        c[i] = a[i] + b[i];
    }
    timer.stop();
    double simdTime = timer.elapsedMilliseconds();
    
    // Calculate speedup - if significant, SIMD is likely being used
    double speedup = scalarTime / simdTime;
    
    return (speedup > 1.5); // Threshold for determining SIMD usage
}

// Estimate SIMD efficiency for a particular operation
double estimateSIMDEfficiency(const char* operationName) {
    std::string op(operationName);
    
    // Check if we have results for this operation
    auto it = g_verificationResults.find(op);
    if (it != g_verificationResults.end()) {
        return it->second.efficiency;
    }
    
    // If not found, run a test based on the operation name
    if (op == "Addition" || op == "SimpleAddition") {
        testSimpleVectorization();
        return g_verificationResults["SimpleAddition"].efficiency;
    } else if (op == "ComplexMath" || op == "Transcendental") {
        testComplexVectorization();
        return g_verificationResults["ComplexMath"].efficiency;
    } else if (op == "Conditional" || op == "Conditionals") {
        testConditionalVectorization();
        return g_verificationResults["Conditionals"].efficiency;
    } else if (op == "GatherScatter" || op == "IndirectAccess") {
        testMemoryGatherScatter();
        return g_verificationResults["GatherScatter"].efficiency;
    }
    
    // Default: run simple test
    testSimpleVectorization();
    return g_verificationResults["SimpleAddition"].efficiency;
}

// Identify potential vectorization bottlenecks
void identifyVectorizationBottlenecks(const char* functionName) {
    std::string func(functionName);
    
    std::cout << "Analyzing vectorization bottlenecks for: " << func << std::endl;
    
    // Run all tests if not already run
    if (g_verificationResults.empty()) {
        testSimpleVectorization();
        testComplexVectorization();
        testConditionalVectorization();
        testMemoryGatherScatter();
    }
    
    // Check if we have specific results for this function
    auto it = g_verificationResults.find(func);
    if (it != g_verificationResults.end()) {
        auto& result = it->second;
        if (!result.bottleneck.empty()) {
            std::cout << "Identified bottleneck: " << result.bottleneck << std::endl;
        } else {
            std::cout << "No specific bottlenecks identified. Vectorization efficiency: " 
                      << std::fixed << std::setprecision(1) << (result.efficiency * 100.0) << "%" << std::endl;
        }
        return;
    }
    
    // General analysis based on all tests
    std::cout << "General vectorization bottlenecks analysis:" << std::endl;
    
    for (const auto& pair : g_verificationResults) {
        const auto& result = pair.second;
        std::cout << "- " << result.name << ": ";
        if (result.isVectorized) {
            std::cout << "Vectorized with " << std::fixed << std::setprecision(1) 
                      << (result.efficiency * 100.0) << "% efficiency, speedup: " 
                      << std::fixed << std::setprecision(2) << result.speedup << "x";
        } else {
            std::cout << "Not effectively vectorized";
        }
        
        if (!result.bottleneck.empty()) {
            std::cout << " - " << result.bottleneck;
        }
        std::cout << std::endl;
    }
    
    // Overall recommendations
    std::cout << "\nRecommendations for improving vectorization:" << std::endl;
    std::cout << "1. Ensure data is properly aligned in memory" << std::endl;
    std::cout << "2. Minimize conditional branches within loops" << std::endl;
    std::cout << "3. Use linear memory access patterns when possible" << std::endl;
    std::cout << "4. Consider data layout transformations (Array of Structures -> Structure of Arrays)" << std::endl;
    std::cout << "5. Break complex loops into simpler, more vectorizable components" << std::endl;
}

// Display a comprehensive vectorization report
void displayVectorizationReport() {
    std::cout << "\n=== SIMD Vectorization Verification Report ===" << std::endl;
    
    // CPU capabilities
    std::cout << "\nCPU SIMD Capabilities:" << std::endl;
    std::cout << "----------------------" << std::endl;
    const CPUFeatures& features = getCPUFeatures();
    std::cout << "Maximum SIMD width: " << features.maxSIMDWidth << " bits" << std::endl;
    std::cout << "SSE: " << (features.hasSSE ? "Yes" : "No") << std::endl;
    std::cout << "SSE2: " << (features.hasSSE2 ? "Yes" : "No") << std::endl;
    std::cout << "AVX: " << (features.hasAVX ? "Yes" : "No") << std::endl;
    std::cout << "AVX2: " << (features.hasAVX2 ? "Yes" : "No") << std::endl;
    std::cout << "AVX-512: " << (features.hasAVX512F ? "Yes" : "No") << std::endl;
    
    // Runtime SIMD usage check
    std::cout << "\nRuntime SIMD Usage Check:" << std::endl;
    std::cout << "------------------------" << std::endl;
    bool simdUsed = detectSIMDUsage();
    std::cout << "SIMD instructions detected: " << (simdUsed ? "Yes" : "No") << std::endl;
    if (!simdUsed) {
        std::cout << "Warning: SIMD instructions do not seem to be used effectively." << std::endl;
        std::cout << "Possible reasons:" << std::endl;
        std::cout << "- Compiler may not be vectorizing the code" << std::endl;
        std::cout << "- OpenMP SIMD directives may not be effective" << std::endl;
        std::cout << "- The system might not support the required SIMD instructions" << std::endl;
    }
    
    // Vectorization test results
    std::cout << "\nVectorization Test Results:" << std::endl;
    std::cout << "---------------------------" << std::endl;
    
    // Run tests if they haven't been run yet
    if (g_verificationResults.empty()) {
        testSimpleVectorization();
        testComplexVectorization();
        testConditionalVectorization();
        testMemoryGatherScatter();
    }
    
    // Display table header
    std::cout << std::left 
              << std::setw(30) << "Test"
              << std::right 
              << std::setw(15) << "Scalar (ms)" 
              << std::setw(15) << "SIMD (ms)" 
              << std::setw(15) << "Speedup" 
              << std::setw(15) << "Vectorized?" 
              << std::endl;
    
    std::cout << std::string(90, '-') << std::endl;
    
    // Display results
    std::cout << std::fixed << std::setprecision(3);
    for (const auto& pair : g_verificationResults) {
        const auto& result = pair.second;
        std::cout << std::left 
                  << std::setw(30) << result.name
                  << std::right 
                  << std::setw(15) << result.scalarTime
                  << std::setw(15) << result.simdTime
                  << std::setw(15) << result.speedup
                  << std::setw(15) << (result.isVectorized ? "Yes" : "No")
                  << std::endl;
    }
    
    // Bottlenecks and recommendations
    std::cout << "\nVectorization Bottlenecks:" << std::endl;
    std::cout << "-------------------------" << std::endl;
    
    bool bottlenecksFound = false;
    for (const auto& pair : g_verificationResults) {
        const auto& result = pair.second;
        if (!result.bottleneck.empty()) {
            std::cout << "- " << result.name << ": " << result.bottleneck << std::endl;
            bottlenecksFound = true;
        }
    }
    
    if (!bottlenecksFound) {
        std::cout << "No significant vectorization bottlenecks detected." << std::endl;
    }
    
    // Efficiency analysis
    std::cout << "\nVectorization Efficiency Analysis:" << std::endl;
    std::cout << "---------------------------------" << std::endl;
    
    double totalEfficiency = 0.0;
    int count = 0;
    for (const auto& pair : g_verificationResults) {
        const auto& result = pair.second;
        std::cout << "- " << result.name << ": " 
                  << std::fixed << std::setprecision(1) << (result.efficiency * 100.0) << "% efficient" << std::endl;
        totalEfficiency += result.efficiency;
        count++;
    }
    
    double avgEfficiency = (count > 0) ? (totalEfficiency / count) : 0.0;
    std::cout << "\nOverall vectorization efficiency: " 
              << std::fixed << std::setprecision(1) << (avgEfficiency * 100.0) << "%" << std::endl;
    
    // Overall assessment
    std::cout << "\nOverall Assessment:" << std::endl;
    std::cout << "------------------" << std::endl;
    
    if (avgEfficiency > 0.7) {
        std::cout << "Excellent vectorization: Your code is being effectively vectorized." << std::endl;
    } else if (avgEfficiency > 0.4) {
        std::cout << "Good vectorization: Your code is being vectorized, but there's room for improvement." << std::endl;
    } else if (avgEfficiency > 0.1) {
        std::cout << "Limited vectorization: Your code shows some vectorization, but significant optimization is needed." << std::endl;
    } else {
        std::cout << "Poor vectorization: Your code is not being effectively vectorized." << std::endl;
    }
    
    // Recommendations
    std::cout << "\nRecommendations for Improving Vectorization:" << std::endl;
    std::cout << "-------------------------------------------" << std::endl;
    std::cout << "1. Ensure data is properly aligned in memory" << std::endl;
    std::cout << "2. Minimize conditional branches within loops" << std::endl;
    std::cout << "3. Use linear memory access patterns when possible" << std::endl;
    std::cout << "4. Consider data layout transformations (Array of Structures -> Structure of Arrays)" << std::endl;
    std::cout << "5. Break complex loops into simpler, more vectorizable components" << std::endl;
    std::cout << "6. Use explicit SIMD directives for critical loops" << std::endl;
    std::cout << "7. For complex math, consider using specialized SIMD libraries" << std::endl;
}

// Main verification function
void verifySIMD() {
    std::cout << "\n=== SIMD Vectorization Verification ===" << std::endl;
    
    // Run all verification tests
    bool testSimple = testSimpleVectorization();
    bool testComplex = testComplexVectorization();
    bool testConditional = testConditionalVectorization();
    bool testGatherScatter = testMemoryGatherScatter();
    
    // Display results summary
    std::cout << "\nBasic Tests:" << std::endl;
    std::cout << "- Simple Array Addition: " << (testSimple ? "Vectorized" : "Not Vectorized") << std::endl;
    std::cout << "- Complex Math Operations: " << (testComplex ? "Vectorized" : "Not Vectorized") << std::endl;
    std::cout << "- Operations with Conditionals: " << (testConditional ? "Vectorized" : "Not Vectorized") << std::endl;
    std::cout << "- Memory Gather/Scatter: " << (testGatherScatter ? "Vectorized" : "Not Vectorized") << std::endl;
    
    // Display comprehensive report
    displayVectorizationReport();
}