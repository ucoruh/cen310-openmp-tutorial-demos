#ifndef BENCHMARK_SUITE_H
#define BENCHMARK_SUITE_H

#include <string>
#include <vector>
#include <utility>
#include <functional>

// Define a benchmark result structure
struct BenchmarkResult {
    std::string name;
    double timeScalar;    // Time in milliseconds for scalar version
    double timeVectorized; // Time in milliseconds for vectorized version
    double speedup;       // Computed speedup
    bool verified;        // Result verification status
};

// Benchmark function for performance measurement
void runBenchmarkSuite();

// Specific benchmark functions
BenchmarkResult benchmarkVectorAddition(size_t vectorSize);
BenchmarkResult benchmarkArrayMultiply(size_t vectorSize);
BenchmarkResult benchmarkTranscendental(size_t vectorSize);
BenchmarkResult benchmarkAlignedAccess(size_t vectorSize);
BenchmarkResult benchmarkMixedPrecision(size_t vectorSize);
BenchmarkResult benchmarkSIMDParallelism(size_t vectorSize, int numThreads);

// Utility functions for benchmarking
double measureExecutionTime(std::function<void()> func);
void displayBenchmarkResults(const std::vector<BenchmarkResult>& results);
void generatePerformanceReport(const std::vector<BenchmarkResult>& results, const std::string& filename);

// ASCII visualization of benchmark results
void displayPerformanceGraph(const std::vector<BenchmarkResult>& results);

#endif // BENCHMARK_SUITE_H