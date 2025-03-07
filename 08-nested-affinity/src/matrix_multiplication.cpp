#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <random>
#include <omp.h>
#include <string>
#include <algorithm>
#include "../include/system_topology.h"
#include "../include/thread_utils.h"
#include "../include/cli_parser.h"

// Typedefs for matrix types
using Matrix = std::vector<std::vector<double>>;

/**
 * @brief Initialize a matrix with random values
 * @param rows Number of rows
 * @param cols Number of columns
 * @return Initialized matrix
 */
Matrix initializeMatrix(int rows, int cols) {
    Matrix result(rows, std::vector<double>(cols, 0.0));
    
    // Random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            result[i][j] = dis(gen);
        }
    }
    
    return result;
}

/**
 * @brief Sequential matrix multiplication
 * @param A First matrix
 * @param B Second matrix
 * @return Result matrix
 */
Matrix sequentialMultiply(const Matrix& A, const Matrix& B) {
    int rowsA = A.size();
    int colsA = A[0].size();
    int colsB = B[0].size();
    
    Matrix C(rowsA, std::vector<double>(colsB, 0.0));
    
    for (int i = 0; i < rowsA; i++) {
        for (int j = 0; j < colsB; j++) {
            for (int k = 0; k < colsA; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    
    return C;
}

/**
 * @brief Basic parallel matrix multiplication (single level)
 * @param A First matrix
 * @param B Second matrix
 * @return Result matrix
 */
Matrix basicParallelMultiply(const Matrix& A, const Matrix& B) {
    int rowsA = A.size();
    int colsA = A[0].size();
    int colsB = B[0].size();
    
    Matrix C(rowsA, std::vector<double>(colsB, 0.0));
    
    #pragma omp parallel for default(none) shared(A, B, C, rowsA, colsA, colsB)
    for (int i = 0; i < rowsA; i++) {
        for (int j = 0; j < colsB; j++) {
            for (int k = 0; k < colsA; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    
    return C;
}

/**
 * @brief Nested parallel matrix multiplication (two levels)
 * @param A First matrix
 * @param B Second matrix
 * @param outerThreads Number of threads for outer parallel region
 * @param innerThreads Number of threads for inner parallel region
 * @return Result matrix
 */
Matrix nestedParallelMultiply(const Matrix& A, const Matrix& B, 
                             int outerThreads, int innerThreads) {
    int rowsA = A.size();
    int colsA = A[0].size();
    int colsB = B[0].size();
    
    Matrix C(rowsA, std::vector<double>(colsB, 0.0));
    
    // Enable nested parallelism
    omp_set_nested(1);
    
    #pragma omp parallel num_threads(outerThreads) default(none) shared(A, B, C, rowsA, colsA, colsB, innerThreads)
    {
        #pragma omp for
        for (int i = 0; i < rowsA; i++) {
            #pragma omp parallel num_threads(innerThreads) default(none) shared(A, B, C, i, colsA, colsB)
            {
                #pragma omp for
                for (int j = 0; j < colsB; j++) {
                    for (int k = 0; k < colsA; k++) {
                        C[i][j] += A[i][k] * B[k][j];
                    }
                }
            }
        }
    }
    
    return C;
}

/**
 * @brief Cache-optimized blocked matrix multiplication
 * @param A First matrix
 * @param B Second matrix
 * @param blockSize Block size for cache optimization
 * @return Result matrix
 */
Matrix blockedParallelMultiply(const Matrix& A, const Matrix& B, int blockSize) {
    int rowsA = A.size();
    int colsA = A[0].size();
    int colsB = B[0].size();
    
    Matrix C(rowsA, std::vector<double>(colsB, 0.0));
    
    // Enable nested parallelism
    omp_set_nested(1);
    
    #pragma omp parallel default(none) shared(A, B, C, rowsA, colsA, colsB, blockSize)
    {
        #pragma omp for
        for (int ii = 0; ii < rowsA; ii += blockSize) {
            for (int jj = 0; jj < colsB; jj += blockSize) {
                for (int kk = 0; kk < colsA; kk += blockSize) {
                    // Process a block
                    int iLimit = std::min(ii + blockSize, rowsA);
                    int jLimit = std::min(jj + blockSize, colsB);
                    int kLimit = std::min(kk + blockSize, colsA);
                    
                    for (int i = ii; i < iLimit; i++) {
                        for (int j = jj; j < jLimit; j++) {
                            double sum = 0.0;
                            for (int k = kk; k < kLimit; k++) {
                                sum += A[i][k] * B[k][j];
                            }
                            #pragma omp atomic
                            C[i][j] += sum;
                        }
                    }
                }
            }
        }
    }
    
    return C;
}

/**
 * @brief Check if two matrices are approximately equal
 * @param A First matrix
 * @param B Second matrix
 * @param epsilon Tolerance
 * @return true if matrices are approximately equal
 */
bool matricesEqual(const Matrix& A, const Matrix& B, double epsilon = 1e-10) {
    if (A.size() != B.size() || A[0].size() != B[0].size()) {
        return false;
    }
    
    for (size_t i = 0; i < A.size(); i++) {
        for (size_t j = 0; j < A[0].size(); j++) {
            if (std::abs(A[i][j] - B[i][j]) > epsilon) {
                return false;
            }
        }
    }
    
    return true;
}

// Entry point
int main(int argc, char* argv[]) {
    // Parse command line arguments
    CliParser parser(argc, argv);
    
    // Check if OpenMP is available
    #ifdef _OPENMP
        std::cout << "OpenMP is supported! Version: " << _OPENMP << std::endl;
    #else
        std::cerr << "OpenMP is not supported!" << std::endl;
        return 1;
    #endif
    
    // Initialize system topology detector
    SystemTopology topology;
    topology.detectTopology();
    
    // Print basic system info
    std::cout << "Detected " << topology.getLogicalProcessorCount() << " logical processors" << std::endl;
    std::cout << "NUMA nodes: " << topology.getNumaNodeCount() << std::endl;
    
    // Matrix size from command line or default
    int matrixSize = parser.getIntOption("matrix_size", 1000);
    std::cout << "Matrix size: " << matrixSize << "x" << matrixSize << std::endl;
    
    // Get thread counts from command line or default
    int outerThreads = parser.getIntOption("outer_threads", 
                                         std::min(4, topology.getLogicalProcessorCount()));
    int innerThreads = parser.getIntOption("inner_threads", 2);
    
    // Get block size from command line or default
    int blockSize = parser.getIntOption("block_size", 32);
    
    std::cout << "Thread configuration: " << outerThreads << " outer, " 
              << innerThreads << " inner" << std::endl;
    std::cout << "Block size: " << blockSize << std::endl;
    
    // Initialize matrices
    std::cout << "\nInitializing matrices..." << std::endl;
    auto startInit = std::chrono::high_resolution_clock::now();
    
    Matrix A = initializeMatrix(matrixSize, matrixSize);
    Matrix B = initializeMatrix(matrixSize, matrixSize);
    
    auto endInit = std::chrono::high_resolution_clock::now();
    auto initTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        endInit - startInit).count();
    
    std::cout << "Initialization completed in " << initTime << " ms" << std::endl;
    
    // Sequential multiplication
    std::cout << "\nPerforming sequential multiplication..." << std::endl;
    auto startSeq = std::chrono::high_resolution_clock::now();
    
    Matrix Cseq = sequentialMultiply(A, B);
    
    auto endSeq = std::chrono::high_resolution_clock::now();
    auto seqTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        endSeq - startSeq).count();
    
    std::cout << "Sequential multiplication completed in " << seqTime << " ms" << std::endl;
    
    // Basic parallel multiplication
    std::cout << "\nPerforming basic parallel multiplication..." << std::endl;
    auto startBasic = std::chrono::high_resolution_clock::now();
    
    Matrix Cbasic = basicParallelMultiply(A, B);
    
    auto endBasic = std::chrono::high_resolution_clock::now();
    auto basicTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        endBasic - startBasic).count();
    
    std::cout << "Basic parallel multiplication completed in " << basicTime << " ms" << std::endl;
    std::cout << "Speedup: " << std::fixed << std::setprecision(2) 
              << static_cast<double>(seqTime) / basicTime << "x" << std::endl;
    
    // Verify result
    std::cout << "Result verification: " 
              << (matricesEqual(Cseq, Cbasic) ? "PASSED" : "FAILED") << std::endl;
    
    // Nested parallel multiplication
    std::cout << "\nPerforming nested parallel multiplication..." << std::endl;
    auto startNested = std::chrono::high_resolution_clock::now();
    
    Matrix Cnested = nestedParallelMultiply(A, B, outerThreads, innerThreads);
    
    auto endNested = std::chrono::high_resolution_clock::now();
    auto nestedTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        endNested - startNested).count();
    
    std::cout << "Nested parallel multiplication completed in " << nestedTime << " ms" << std::endl;
    std::cout << "Speedup vs sequential: " << std::fixed << std::setprecision(2) 
              << static_cast<double>(seqTime) / nestedTime << "x" << std::endl;
    std::cout << "Speedup vs basic parallel: " << std::fixed << std::setprecision(2) 
              << static_cast<double>(basicTime) / nestedTime << "x" << std::endl;
    
    // Verify result
    std::cout << "Result verification: " 
              << (matricesEqual(Cseq, Cnested) ? "PASSED" : "FAILED") << std::endl;
    
    // Blocked parallel multiplication
    std::cout << "\nPerforming blocked parallel multiplication..." << std::endl;
    auto startBlocked = std::chrono::high_resolution_clock::now();
    
    Matrix Cblocked = blockedParallelMultiply(A, B, blockSize);
    
    auto endBlocked = std::chrono::high_resolution_clock::now();
    auto blockedTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        endBlocked - startBlocked).count();
    
    std::cout << "Blocked parallel multiplication completed in " << blockedTime << " ms" << std::endl;
    std::cout << "Speedup vs sequential: " << std::fixed << std::setprecision(2) 
              << static_cast<double>(seqTime) / blockedTime << "x" << std::endl;
    std::cout << "Speedup vs nested parallel: " << std::fixed << std::setprecision(2) 
              << static_cast<double>(nestedTime) / blockedTime << "x" << std::endl;
    
    // Verify result
    std::cout << "Result verification: " 
              << (matricesEqual(Cseq, Cblocked) ? "PASSED" : "FAILED") << std::endl;
    
    // Print performance summary
    std::cout << "\n=== Performance Summary ===" << std::endl;
    std::cout << "Matrix size: " << matrixSize << "x" << matrixSize << std::endl;
    std::cout << "Thread configuration: " << outerThreads << " outer, " 
              << innerThreads << " inner" << std::endl;
    std::cout << "Block size: " << blockSize << std::endl;
    std::cout << std::endl;
    std::cout << std::setw(30) << "Algorithm" << std::setw(15) << "Time (ms)" 
              << std::setw(15) << "Speedup" << std::endl;
    std::cout << std::string(60, '-') << std::endl;
    std::cout << std::setw(30) << "Sequential" << std::setw(15) << seqTime 
              << std::setw(15) << "1.00x" << std::endl;
    std::cout << std::setw(30) << "Basic Parallel" << std::setw(15) << basicTime 
              << std::setw(15) << std::fixed << std::setprecision(2) 
              << static_cast<double>(seqTime) / basicTime << "x" << std::endl;
    std::cout << std::setw(30) << "Nested Parallel" << std::setw(15) << nestedTime 
              << std::setw(15) << std::fixed << std::setprecision(2) 
              << static_cast<double>(seqTime) / nestedTime << "x" << std::endl;
    std::cout << std::setw(30) << "Blocked Parallel" << std::setw(15) << blockedTime 
              << std::setw(15) << std::fixed << std::setprecision(2) 
              << static_cast<double>(seqTime) / blockedTime << "x" << std::endl;
    
    return 0;
}