#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <omp.h>
#include <fstream>
#include <random>
#include <algorithm>
#include <thread>
#include "../../include/cli_parser.h"
#include "../../include/profiler.h"

/**
 * @file intel_vtune.cpp
 * @brief Example demonstrating integration with Intel VTune Profiler
 * 
 * This example shows how to:
 * 1. Use the Intel ITT API for custom instrumentation
 * 2. Set up user markers and regions for VTune analysis
 * 3. Enable collection of custom hardware events
 * 4. Analyze different types of threading issues with VTune
 * 
 * Note: This requires Intel VTune Profiler to be installed.
 * Intel VTune is part of the oneAPI toolkit and can be downloaded from:
 * https://www.intel.com/content/www/us/en/developer/tools/oneapi/toolkits.html
 */

// Include Intel ITT API if available
#ifdef INTEL_VTUNE_AVAILABLE
#include <ittnotify.h>
#endif

// Structure to hold a sample workload
struct WorkItem {
    int id;
    int complexity;
    double result;
    
    WorkItem(int id_, int complexity_) : id(id_), complexity(complexity_), result(0.0) {}
};

// Create a sample workload with varying complexity
std::vector<WorkItem> createWorkload(int size, int maxComplexity) {
    std::vector<WorkItem> workload;
    workload.reserve(size);
    
    std::mt19937 rng(42);  // Fixed seed for reproducibility
    std::uniform_int_distribution<int> dist(1, maxComplexity);
    
    for (int i = 0; i < size; i++) {
        workload.emplace_back(i, dist(rng));
    }
    
    return workload;
}

// Function to perform some work based on complexity
double doWork(int complexity) {
    double result = 0.0;
    
    // Simulate work by performing calculations
    for (int i = 0; i < complexity * 10000; i++) {
        result += std::sin(i) * std::cos(i);
    }
    
    return result;
}

// Demo using Intel ITT API for custom instrumentation
void demoIntelITTInstrumentation(const std::vector<WorkItem>& workload, int numThreads) {
    std::cout << "Running Intel ITT instrumentation demo with "
              << numThreads << " threads and " << workload.size() << " work items..." << std::endl;
    
#ifdef INTEL_VTUNE_AVAILABLE
    // Create a domain for our application
    __itt_domain* domain = __itt_domain_create("OpenMP_Debugging_Performance");
    
    // Create string handles for different tasks
    __itt_string_handle* shTaskMain = __itt_string_handle_create("Process Workload");
    __itt_string_handle* shTaskThread = __itt_string_handle_create("Thread Processing");
    __itt_string_handle* shTaskItem = __itt_string_handle_create("Process Work Item");
    
    // Start the main task
    __itt_task_begin(domain, __itt_null, __itt_null, shTaskMain);
#endif
    
    // Process the workload in parallel with ITT instrumentation
    #pragma omp parallel num_threads(numThreads)
    {
        int threadId = omp_get_thread_num();
        
#ifdef INTEL_VTUNE_AVAILABLE
        // Create a thread-specific string handle
        char threadName[32];
        sprintf(threadName, "Thread %d", threadId);
        __itt_string_handle* shThread = __itt_string_handle_create(threadName);
        
        // Start a task for this thread's work
        __itt_task_begin(domain, __itt_null, __itt_null, shThread);
#endif
        
        #pragma omp for schedule(dynamic, 10)
        for (int i = 0; i < static_cast<int>(workload.size()); i++) {
#ifdef INTEL_VTUNE_AVAILABLE
            // Create a string handle for this work item
            char itemName[64];
            sprintf(itemName, "Item %d (Complexity %d)", workload[i].id, workload[i].complexity);
            __itt_string_handle* shItem = __itt_string_handle_create(itemName);
            
            // Start a task for this work item
            __itt_task_begin(domain, __itt_null, __itt_null, shItem);
#endif
            
            // Do the actual work
            double result = doWork(workload[i].complexity);
            
#ifdef INTEL_VTUNE_AVAILABLE
            // End the task for this work item
            __itt_task_end(domain);
#endif
            
            // Simulate thread synchronization for demonstration
            if (i % 50 == 0) {
#ifdef INTEL_VTUNE_AVAILABLE
                // Mark this synchronization point
                __itt_sync_prepare(domain);
#endif
                
                #pragma omp critical
                {
                    // Just a dummy critical section for visualization
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
                
#ifdef INTEL_VTUNE_AVAILABLE
                // Mark the end of synchronization
                __itt_sync_acquired(domain);
#endif
            }
        }
        
#ifdef INTEL_VTUNE_AVAILABLE
        // End the task for this thread's work
        __itt_task_end(domain);
#endif
    }
    
#ifdef INTEL_VTUNE_AVAILABLE
    // End the main task
    __itt_task_end(domain);
#endif
    
    std::cout << "Intel ITT instrumentation demo completed." << std::endl;
    std::cout << "View results in Intel VTune Profiler." << std::endl;
}

// Demo focusing on memory bandwidth and cache issues
void demoMemoryAnalysis(int numThreads, int matrixSize) {
    std::cout << "Running memory analysis demo with "
              << numThreads << " threads and matrix size " << matrixSize << "..." << std::endl;
    
#ifdef INTEL_VTUNE_AVAILABLE
    // Create a domain for memory analysis
    __itt_domain* memoryDomain = __itt_domain_create("Memory_Analysis");
    
    // Create string handles for different tasks
    __itt_string_handle* shMatrixOps = __itt_string_handle_create("Matrix Operations");
    __itt_string_handle* shRowMajor = __itt_string_handle_create("Row-Major Access");
    __itt_string_handle* shColMajor = __itt_string_handle_create("Column-Major Access");
    __itt_string_handle* shRandomAccess = __itt_string_handle_create("Random Access");
    
    // Start the main task
    __itt_task_begin(memoryDomain, __itt_null, __itt_null, shMatrixOps);
#endif
    
    // Allocate matrices
    std::vector<double> matrixA(matrixSize * matrixSize, 1.0);
    std::vector<double> matrixB(matrixSize * matrixSize, 0.0);
    
    // First test: Row-major access (cache-friendly)
#ifdef INTEL_VTUNE_AVAILABLE
    __itt_task_begin(memoryDomain, __itt_null, __itt_null, shRowMajor);
#endif
    
    #pragma omp parallel for num_threads(numThreads)
    for (int i = 0; i < matrixSize; i++) {
        for (int j = 0; j < matrixSize; j++) {
            matrixB[i * matrixSize + j] = matrixA[i * matrixSize + j] * 2.0;
        }
    }
    
#ifdef INTEL_VTUNE_AVAILABLE
    __itt_task_end(memoryDomain);
#endif
    
    // Second test: Column-major access (cache-unfriendly)
#ifdef INTEL_VTUNE_AVAILABLE
    __itt_task_begin(memoryDomain, __itt_null, __itt_null, shColMajor);
#endif
    
    #pragma omp parallel for num_threads(numThreads)
    for (int j = 0; j < matrixSize; j++) {
        for (int i = 0; i < matrixSize; i++) {
            matrixB[i * matrixSize + j] = matrixA[i * matrixSize + j] * 2.0;
        }
    }
    
#ifdef INTEL_VTUNE_AVAILABLE
    __itt_task_end(memoryDomain);
#endif
    
    // Third test: Random access (cache-unfriendly)
#ifdef INTEL_VTUNE_AVAILABLE
    __itt_task_begin(memoryDomain, __itt_null, __itt_null, shRandomAccess);
#endif
    
    // Generate random indices
    std::vector<int> indices(matrixSize * matrixSize);
    for (int i = 0; i < matrixSize * matrixSize; i++) {
        indices[i] = i;
    }
    std::mt19937 rng(42);
    std::shuffle(indices.begin(), indices.end(), rng);
    
    #pragma omp parallel for num_threads(numThreads)
    for (int i = 0; i < matrixSize * matrixSize; i++) {
        matrixB[indices[i]] = matrixA[indices[i]] * 2.0;
    }
    
#ifdef INTEL_VTUNE_AVAILABLE
    __itt_task_end(memoryDomain);
#endif
    
#ifdef INTEL_VTUNE_AVAILABLE
    // End the main task
    __itt_task_end(memoryDomain);
#endif
    
    std::cout << "Memory analysis demo completed." << std::endl;
    std::cout << "View results in Intel VTune Profiler." << std::endl;
}

// Demo focusing on threading and locking issues
void demoThreadingIssues(const std::vector<WorkItem>& workload, int numThreads) {
    std::cout << "Running threading issues demo with "
              << numThreads << " threads and " << workload.size() << " work items..." << std::endl;
    
#ifdef INTEL_VTUNE_AVAILABLE
    // Create a domain for threading analysis
    __itt_domain* threadDomain = __itt_domain_create("Threading_Analysis");
    
    // Create string handles for different tasks
    __itt_string_handle* shLockContention = __itt_string_handle_create("Lock Contention");
    __itt_string_handle* shLoadImbalance = __itt_string_handle_create("Load Imbalance");
    __itt_string_handle* shFalseSharing = __itt_string_handle_create("False Sharing");
    
    // Start the main task
    __itt_task_begin(threadDomain, __itt_null, __itt_null, shLockContention);
#endif
    
    // First test: Lock contention
    std::mutex mutex;
    std::vector<double> results(workload.size(), 0.0);
    
    #pragma omp parallel num_threads(numThreads)
    {
#ifdef INTEL_VTUNE_AVAILABLE
        __itt_sync_prepare(threadDomain);
#endif
        
        #pragma omp for schedule(dynamic, 1)
        for (int i = 0; i < static_cast<int>(workload.size()); i++) {
            double result = doWork(workload[i].complexity / 10);
            
            // Introduce excessive locking
            mutex.lock();
            results[i] = result;
            mutex.unlock();
        }
        
#ifdef INTEL_VTUNE_AVAILABLE
        __itt_sync_acquired(threadDomain);
#endif
    }
    
#ifdef INTEL_VTUNE_AVAILABLE
    __itt_task_end(threadDomain);
#endif
    
    // Second test: Load imbalance
#ifdef INTEL_VTUNE_AVAILABLE
    __itt_task_begin(threadDomain, __itt_null, __itt_null, shLoadImbalance);
#endif
    
    #pragma omp parallel num_threads(numThreads)
    {
        int threadId = omp_get_thread_num();
        
        // Create artificial load imbalance - higher thread IDs do more work
        #pragma omp for schedule(static)
        for (int i = 0; i < static_cast<int>(workload.size()); i++) {
            // Scale complexity by thread ID to create imbalance
            int scaledComplexity = workload[i].complexity * (1 + threadId);
            results[i] = doWork(scaledComplexity);
        }
    }
    
#ifdef INTEL_VTUNE_AVAILABLE
    __itt_task_end(threadDomain);
#endif
    
    // Third test: False sharing
#ifdef INTEL_VTUNE_AVAILABLE
    __itt_task_begin(threadDomain, __itt_null, __itt_null, shFalseSharing);
#endif
    
    // Array with elements sharing the same cache line
    double sharedArray[16] = {0};
    
    #pragma omp parallel num_threads(numThreads)
    {
        int threadId = omp_get_thread_num();
        int index = threadId % 16;
        
        // Each thread updates its own element, but elements are adjacent
        for (int i = 0; i < 1000000; i++) {
            sharedArray[index] += 0.1;
        }
    }
    
#ifdef INTEL_VTUNE_AVAILABLE
    __itt_task_end(threadDomain);
#endif
    
    std::cout << "Threading issues demo completed." << std::endl;
    std::cout << "View results in Intel VTune Profiler." << std::endl;
}

// Demo SIMD and vectorization opportunities
void demoVectorizationOpportunities(int numThreads, int arraySize) {
    std::cout << "Running vectorization opportunities demo with "
              << numThreads << " threads and array size " << arraySize << "..." << std::endl;
    
#ifdef INTEL_VTUNE_AVAILABLE
    // Create a domain for vectorization analysis
    __itt_domain* vectDomain = __itt_domain_create("Vectorization_Analysis");
    
    // Create string handles for different tasks
    __itt_string_handle* shNonVectorized = __itt_string_handle_create("Non-Vectorized Loop");
    __itt_string_handle* shVectorizable = __itt_string_handle_create("Vectorizable Loop");
    __itt_string_handle* shDependency = __itt_string_handle_create("Loop with Dependency");
    
    // Start the main task
    __itt_task_begin(vectDomain, __itt_null, __itt_null, shNonVectorized);
#endif
    
    // Allocate arrays
    std::vector<float> arrayA(arraySize, 1.0f);
    std::vector<float> arrayB(arraySize, 2.0f);
    std::vector<float> arrayC(arraySize, 0.0f);
    
    // First test: Non-vectorized loop with complex operations
    #pragma omp parallel for num_threads(numThreads)
    for (int i = 0; i < arraySize; i++) {
        arrayC[i] = std::sin(arrayA[i]) * std::cos(arrayB[i]);
    }
    
#ifdef INTEL_VTUNE_AVAILABLE
    __itt_task_end(vectDomain);
#endif
    
    // Second test: Vectorizable simple loop
#ifdef INTEL_VTUNE_AVAILABLE
    __itt_task_begin(vectDomain, __itt_null, __itt_null, shVectorizable);
#endif
    
    #pragma omp parallel for num_threads(numThreads)
    for (int i = 0; i < arraySize; i++) {
        arrayC[i] = arrayA[i] + arrayB[i];
    }
    
#ifdef INTEL_VTUNE_AVAILABLE
    __itt_task_end(vectDomain);
#endif
    
    // Third test: Loop with dependency (harder to vectorize)
#ifdef INTEL_VTUNE_AVAILABLE
    __itt_task_begin(vectDomain, __itt_null, __itt_null, shDependency);
#endif
    
    #pragma omp parallel for num_threads(numThreads)
    for (int i = 1; i < arraySize; i++) {
        arrayC[i] = arrayC[i-1] + arrayA[i];  // Dependency on previous iteration
    }
    
#ifdef INTEL_VTUNE_AVAILABLE
    __itt_task_end(vectDomain);
#endif
    
    std::cout << "Vectorization opportunities demo completed." << std::endl;
    std::cout << "View results in Intel VTune Profiler." << std::endl;
}

// Show how to run VTune analysis from command line
void displayVTuneCommandLineInfo() {
    std::cout << "\nTo analyze this application with VTune from the command line:" << std::endl;
    std::cout << "--------------------------------------------------------------" << std::endl;
    
    // Hotspots analysis
    std::cout << "1. For CPU Hotspots Analysis:" << std::endl;
    std::cout << "   vtune -collect hotspots -knob sampling-mode=hw ./bin/intel_vtune --mode=all" << std::endl;
    
    // Threading analysis
    std::cout << "\n2. For Threading Analysis:" << std::endl;
    std::cout << "   vtune -collect threading ./bin/intel_vtune --mode=threading" << std::endl;
    
    // Memory analysis
    std::cout << "\n3. For Memory Access Analysis:" << std::endl;
    std::cout << "   vtune -collect memory-access ./bin/intel_vtune --mode=memory" << std::endl;
    
    // Microarchitecture analysis
    std::cout << "\n4. For Microarchitecture Analysis:" << std::endl;
    std::cout << "   vtune -collect uarch-exploration ./bin/intel_vtune --mode=vectorization" << std::endl;
    
    // View results
    std::cout << "\nTo view the results:" << std::endl;
    std::cout << "   vtune -report summary" << std::endl;
    
    std::cout << "\nNote: VTune commands may vary based on your installation and version." << std::endl;
    std::cout << "      Refer to the Intel VTune documentation for your specific version." << std::endl;
}

// Main function
int main(int argc, char* argv[]) {
    // Parse command line arguments
    CliParser parser(argc, argv);
    
    // Get parameters
    int threads = parser.getIntOption("threads", std::min(4, omp_get_max_threads()));
    int workItems = parser.getIntOption("items", 1000);
    int complexity = parser.getIntOption("complexity", 100);
    int matrixSize = parser.getIntOption("matrix-size", 1000);
    int arraySize = parser.getIntOption("array-size", 100000);
    std::string mode = parser.getStringOption("mode", "all");
    bool showCommands = parser.getBoolOption("show-commands", false);
    
    std::cout << "=== Intel VTune Profiler Integration Demo ===" << std::endl;
    
    // Show command-line info if requested
    if (showCommands) {
        displayVTuneCommandLineInfo();
        return 0;
    }
    
    // Display configuration
    std::cout << "Threads: " << threads << std::endl;
    std::cout << "Work Items: " << workItems << std::endl;
    std::cout << "Complexity: " << complexity << std::endl;
    std::cout << std::endl;
    
    // Check for Intel VTune availability
#ifndef INTEL_VTUNE_AVAILABLE
    std::cout << "WARNING: Intel VTune instrumentation API (ITT) is not available." << std::endl;
    std::cout << "The demo will run without instrumentation. To enable instrumentation:" << std::endl;
    std::cout << "1. Install Intel VTune Profiler" << std::endl;
    std::cout << "2. Add -DINTEL_VTUNE_AVAILABLE to compiler flags" << std::endl;
    std::cout << "3. Link with the ITT library" << std::endl;
    std::cout << std::endl;
#endif
    
    // Create workload if needed
    std::vector<WorkItem> workload;
    if (mode == "all" || mode == "itt" || mode == "threading") {
        workload = createWorkload(workItems, complexity);
    }
    
    // Run selected demo mode
    if (mode == "all" || mode == "itt") {
        demoIntelITTInstrumentation(workload, threads);
        std::cout << std::endl;
    }
    
    if (mode == "all" || mode == "memory") {
        demoMemoryAnalysis(threads, matrixSize);
        std::cout << std::endl;
    }
    
    if (mode == "all" || mode == "threading") {
        demoThreadingIssues(workload, threads);
        std::cout << std::endl;
    }
    
    if (mode == "all" || mode == "vectorization") {
        demoVectorizationOpportunities(threads, arraySize);
        std::cout << std::endl;
    }
    
    // Print summary
    std::cout << "Intel VTune integration demos completed." << std::endl;
    std::cout << "To analyze with Intel VTune Profiler:" << std::endl;
    std::cout << "1. Use the standalone VTune Profiler GUI and select this executable" << std::endl;
    std::cout << "2. Or use the command line interface with 'vtune -collect [analysis-type] ./intel_vtune'" << std::endl;
    std::cout << "3. Run with --show-commands for detailed command-line examples" << std::endl;
    
    return 0;
}