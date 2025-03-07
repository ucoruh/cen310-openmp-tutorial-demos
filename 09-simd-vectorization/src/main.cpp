#include <iostream>
#include <string>
#include <vector>
#include <omp.h>
#include "../include/simd_examples.h"
#include "../include/cpu_features.h"
#include "../include/benchmark_suite.h"
#include "../include/simd_verifier.h"
#include "../include/asm_analyzer.h"

void printMenu() {
    std::cout << "\n=== OpenMP SIMD Vectorization Demo Menu ===\n";
    std::cout << "1. Basic SIMD Introduction\n";
    std::cout << "2. Array Operations with SIMD\n";
    std::cout << "3. Complex Math Functions with SIMD\n";
    std::cout << "4. Memory Alignment Optimization\n";
    std::cout << "5. SIMD Width Adaptation\n";
    std::cout << "6. Mixed Precision Operations\n";
    std::cout << "7. SIMD with Thread Parallelism\n";
    std::cout << "8. Run Benchmark Suite\n";
    std::cout << "9. Assembly Output Analysis\n";
    std::cout << "10. SIMD Verification\n";
    std::cout << "11. Show CPU SIMD Capabilities\n";
    std::cout << "0. Exit\n";
    std::cout << "Enter your choice: ";
}

void processCommandLine(int argc, char* argv[]) {
    if (argc <= 1) return; // No arguments provided
    
    std::string arg = argv[1];
    if (arg == "--benchmark" || arg == "-b") {
        std::cout << "Running benchmark suite...\n";
        runBenchmarkSuite();
        exit(0);
    }
    else if (arg == "--verify" || arg == "-v") {
        std::cout << "Running SIMD verification...\n";
        verifySIMD();
        exit(0);
    }
    else if (arg == "--asm-analysis" || arg == "-a") {
        std::cout << "Running assembly analysis...\n";
        runASMAnalysis();
        exit(0);
    }
    else if (arg == "--cpu-info" || arg == "-c") {
        std::cout << "Displaying CPU information...\n";
        displayCPUFeatures();
        exit(0);
    }
    else if (arg == "--help" || arg == "-h") {
        std::cout << "Usage: " << argv[0] << " [option]\n";
        std::cout << "Options:\n";
        std::cout << "  --benchmark, -b    Run benchmark suite\n";
        std::cout << "  --verify, -v       Run SIMD verification\n";
        std::cout << "  --asm-analysis, -a Run assembly analysis\n";
        std::cout << "  --cpu-info, -c     Display CPU information\n";
        std::cout << "  --help, -h         Display this help message\n";
        exit(0);
    }
}

int main(int argc, char* argv[]) {
    // Check if OpenMP is available
    #ifdef _OPENMP
        std::cout << "OpenMP is supported! Version: " << _OPENMP << std::endl;
        
        // Ensure the version is compatible with OpenMP 2.0 or later
        if (_OPENMP < 200203) {
            std::cout << "Warning: OpenMP version may be too old for optimal SIMD support.\n";
            std::cout << "OpenMP 2.0 (200203) or later is recommended.\n";
        }
    #else
        std::cerr << "OpenMP is not supported!" << std::endl;
        return 1;
    #endif
    
    // Detect CPU capabilities at startup
    detectCPUFeatures();
    std::cout << "CPU detected. Maximum SIMD width: " << getOptimalSIMDWidth() << " bits\n";
    
    // Process command line arguments if provided
    processCommandLine(argc, argv);
    
    // Interactive menu-driven interface
    int choice = -1;
    while (choice != 0) {
        printMenu();
        std::cin >> choice;
        
        switch (choice) {
            case 0:
                std::cout << "Exiting program.\n";
                break;
            case 1:
                runBasicSIMD();
                break;
            case 2:
                runArrayOperations();
                break;
            case 3:
                runComplexMath();
                break;
            case 4:
                runSIMDAlignment();
                break;
            case 5:
                runSIMDWidth();
                break;
            case 6:
                runMixedPrecision();
                break;
            case 7:
                runSIMDParallelism();
                break;
            case 8:
                runBenchmarkSuite();
                break;
            case 9:
                runASMAnalysis();
                break;
            case 10:
                verifySIMD();
                break;
            case 11:
                displayCPUFeatures();
                break;
            default:
                std::cout << "Invalid choice. Please try again.\n";
        }
    }
    
    return 0;
}