#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <omp.h>
#include <fstream>
#include <random>
#include <thread>
#include "../../include/cli_parser.h"
#include "../../include/profiler.h"

/**
 * @file vs_diagnostics.cpp
 * @brief Example demonstrating integration with Visual Studio Diagnostics Tools
 * 
 * This example shows how to:
 * 1. Configure and use Visual Studio's Concurrency Visualizer
 * 2. Instrument code for ETW events
 * 3. Generate custom markers for VS diagnostics
 * 4. Analyze results using VS Diagnostics Tools
 * 
 * Note: This file specifically targets Visual Studio 2022 on Windows
 */

// Include Windows and ETW headers if available
#ifdef _WIN32
#include <windows.h>

// Check for Event Tracing for Windows (ETW) support
#ifdef HAVE_ETW_SUPPORT
#include <evntprov.h>
#include <evntrace.h>
#endif // HAVE_ETW_SUPPORT

// Concurrency Visualizer markers (only available with VS installed)
#ifdef _MSC_VER
// Try to include cvmarkersobj.h if available
#if __has_include(<cvmarkersobj.h>)
#include <cvmarkersobj.h>
using namespace Concurrency::diagnostic;

// Create a marker series provider for our application
marker_series g_markerSeries(L"OpenMP_Debugging_Performance");
#else
// Define dummy marker_series class if header is not available
class marker_series {
public:
    enum flag {
        none = 0,
        profile = 1,
        flagged = 2,
        importance_high = 4
    };
    
    marker_series(const wchar_t* name) {}
    void write_flag(const wchar_t* text, flag f = none) {}
    void write_message(const wchar_t* text) {}
};

// Define dummy span_context class if header is not available
class span_context {
public:
    span_context(marker_series& series, const wchar_t* name) {}
    ~span_context() {}
};

// Create a marker series provider for our application
marker_series g_markerSeries(L"OpenMP_Debugging_Performance");
#endif // __has_include
#endif // _MSC_VER

#endif // _WIN32

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
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(1, maxComplexity);
    
    for (int i = 0; i < size; i++) {
        workload.push_back(WorkItem(i, dist(gen)));
    }
    
    return workload;
}

// Simulate work with varying complexity
double doWork(int complexity) {
    double result = 0.0;
    for (int i = 0; i < complexity * 1000000; i++) {
        result += std::sin(static_cast<double>(i));
    }
    return result;
}

// Demonstrate Visual Studio Concurrency Visualizer markers
void demoVSConcurrencyMarkers(const std::vector<WorkItem>& workload, int numThreads) {
#ifdef _MSC_VER
    std::cout << "Running VS Concurrency Visualizer markers demo..." << std::endl;
    
    // Create a span for the entire function
    span_context functionSpan(g_markerSeries, L"demoVSConcurrencyMarkers");
    
    // Flag the start of the demo
    g_markerSeries.write_flag(L"Starting VS Concurrency Visualizer demo", marker_series::flagged);
    
    // Process workload in parallel with OpenMP
    #pragma omp parallel num_threads(numThreads)
    {
        int threadId = omp_get_thread_num();
        
        // Create a thread-specific marker series
        wchar_t threadName[32];
        swprintf(threadName, 32, L"Thread %d", threadId);
        marker_series threadMarkerSeries(threadName);
        
        // Create a span for the thread's execution
        span_context threadSpan(threadMarkerSeries, threadName);
        
        // Process items in parallel
        #pragma omp for schedule(dynamic)
        for (int i = 0; i < static_cast<int>(workload.size()); i++) {
            // Create a span for this work item
            wchar_t itemName[64];
            swprintf(itemName, 64, L"Item %d (complexity: %d)", workload[i].id, workload[i].complexity);
            span_context itemSpan(threadMarkerSeries, itemName);
            
            // Flag high complexity items
            if (workload[i].complexity > 7) {
                threadMarkerSeries.write_flag(itemName, marker_series::importance_high);
            }
            
            // Do the actual work
            double result = doWork(workload[i].complexity);
            
            // Write a message with the result
            wchar_t resultMsg[128];
            swprintf(resultMsg, 128, L"Item %d completed with result: %.2f", workload[i].id, result);
            threadMarkerSeries.write_message(resultMsg);
        }
    }
    
    // Flag the end of the demo
    g_markerSeries.write_flag(L"Completed VS Concurrency Visualizer demo", marker_series::flagged);
#else
    std::cout << "Visual Studio Concurrency Visualizer not available on this platform." << std::endl;
#endif
}

// Demonstrate ETW events for performance analysis
void demoETWEvents(const std::vector<WorkItem>& workload, int numThreads) {
#if defined(_WIN32) && defined(HAVE_ETW_SUPPORT)
    std::cout << "Running ETW events demo..." << std::endl;
    
    // Define ETW provider GUID (replace with your own)
    static const GUID PROVIDER_GUID = 
        { 0x12345678, 0x1234, 0x1234, { 0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF } };
    
    // Register ETW provider
    REGHANDLE hProvider = 0;
    ULONG result = EventRegister(&PROVIDER_GUID, NULL, NULL, &hProvider);
    
    if (result != ERROR_SUCCESS) {
        std::cerr << "Failed to register ETW provider. Error code: " << result << std::endl;
        return;
    }
    
    // Process workload in parallel with OpenMP
    #pragma omp parallel num_threads(numThreads)
    {
        int threadId = omp_get_thread_num();
        
        // Process items in parallel
        #pragma omp for schedule(dynamic)
        for (int i = 0; i < static_cast<int>(workload.size()); i++) {
            // Start event for this work item
            if (EventEnabled(hProvider, EVENT_LEVEL_VERBOSE, EVENT_KEYWORD_ALL)) {
                // Define event data structure
                struct {
                    int threadId;
                    int itemId;
                    int complexity;
                } eventInfo = { threadId, workload[i].id, workload[i].complexity };
                
                // Write start event
                EventWrite(hProvider, &EVENT_DESCRIPTOR{1, 1, 0, 1, 0, 0, 0}, NULL, &eventInfo);
            }
            
            // Do the actual work
            double result = doWork(workload[i].complexity);
            
            // End event for this work item
            if (EventEnabled(hProvider, EVENT_LEVEL_VERBOSE, EVENT_KEYWORD_ALL)) {
                // Define event data structure
                struct {
                    int threadId;
                    int itemId;
                    double result;
                } eventInfo = { threadId, workload[i].id, result };
                
                // Write end event
                EventWrite(hProvider, &EVENT_DESCRIPTOR{2, 1, 0, 1, 0, 0, 0}, NULL, &eventInfo);
            }
        }
    }
    
    // Unregister ETW provider
    EventUnregister(hProvider);
#else
    std::cout << "ETW events not available on this platform." << std::endl;
#endif
}

// Demonstrate Visual Studio Performance Tools
void demoVSPerformanceTools(const std::vector<WorkItem>& workload, int numThreads) {
    std::cout << "Running VS Performance Tools demo..." << std::endl;
    
    // Simulate different performance scenarios
    
    // Scenario 1: Balanced workload
    std::cout << "Scenario 1: Balanced workload" << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    
    #pragma omp parallel num_threads(numThreads)
    {
        #pragma omp for schedule(dynamic, 1)
        for (int i = 0; i < static_cast<int>(workload.size()); i++) {
            // Simulate balanced work
            double result = doWork(5); // Fixed complexity
            
            // Prevent optimization
            if (result < 0) {
                std::cout << "Unexpected result" << std::endl;
            }
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Balanced workload completed in " << elapsed.count() << " seconds" << std::endl;
    
    // Scenario 2: Imbalanced workload
    std::cout << "Scenario 2: Imbalanced workload" << std::endl;
    start = std::chrono::high_resolution_clock::now();
    
    #pragma omp parallel num_threads(numThreads)
    {
        int threadId = omp_get_thread_num();
        
        #pragma omp for schedule(static)
        for (int i = 0; i < static_cast<int>(workload.size()); i++) {
            // Simulate imbalanced work
            double result = doWork(workload[i].complexity);
            
            // Prevent optimization
            if (result < 0) {
                std::cout << "Unexpected result" << std::endl;
            }
        }
    }
    
    end = std::chrono::high_resolution_clock::now();
    elapsed = end - start;
    std::cout << "Imbalanced workload completed in " << elapsed.count() << " seconds" << std::endl;
    
    // Scenario 3: Memory-bound workload
    std::cout << "Scenario 3: Memory-bound workload" << std::endl;
    start = std::chrono::high_resolution_clock::now();
    
    // Create a large array to work with
    const int arraySize = 100000000;
    std::vector<double> data(arraySize, 1.0);
    
    #pragma omp parallel num_threads(numThreads)
    {
        #pragma omp for schedule(static)
        for (int i = 0; i < static_cast<int>(data.size()); i++) {
            // Memory-bound operation
            data[i] = std::sqrt(data[i]);
        }
    }
    
    end = std::chrono::high_resolution_clock::now();
    elapsed = end - start;
    std::cout << "Memory-bound workload completed in " << elapsed.count() << " seconds" << std::endl;
}

// Setup custom ETW provider for more detailed analysis
void setupCustomETWProvider() {
#if defined(_WIN32) && defined(HAVE_ETW_SUPPORT)
    std::cout << "Setting up custom ETW provider..." << std::endl;
    
    // Define ETW provider GUID (replace with your own)
    static const GUID PROVIDER_GUID = 
        { 0x12345678, 0x1234, 0x1234, { 0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF } };
    
    // Create manifest file for the provider
    std::ofstream manifestFile("OpenMPDiagnostics.man");
    
    if (manifestFile.is_open()) {
        manifestFile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        manifestFile << "<instrumentationManifest\n";
        manifestFile << "    xmlns=\"http://schemas.microsoft.com/win/2004/08/events\"\n";
        manifestFile << "    xmlns:win=\"http://manifests.microsoft.com/win/2004/08/windows/events\"\n";
        manifestFile << "    xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">\n";
        manifestFile << "  <instrumentation>\n";
        manifestFile << "    <events>\n";
        manifestFile << "      <provider name=\"OpenMP-Diagnostics\"\n";
        manifestFile << "                guid=\"{12345678-1234-1234-1234-567890ABCDEF}\"\n";
        manifestFile << "                symbol=\"OpenMP_Diagnostics\"\n";
        manifestFile << "                resourceFileName=\"OpenMPDiagnostics.dll\"\n";
        manifestFile << "                messageFileName=\"OpenMPDiagnostics.dll\">\n";
        manifestFile << "        <events>\n";
        manifestFile << "          <event value=\"1\" level=\"5\" symbol=\"WorkItemStart\"\n";
        manifestFile << "                 template=\"WorkItemStartTemplate\" message=\"Work item started\"/>\n";
        manifestFile << "          <event value=\"2\" level=\"5\" symbol=\"WorkItemEnd\"\n";
        manifestFile << "                 template=\"WorkItemEndTemplate\" message=\"Work item completed\"/>\n";
        manifestFile << "        </events>\n";
        manifestFile << "        <templates>\n";
        manifestFile << "          <template tid=\"WorkItemStartTemplate\">\n";
        manifestFile << "            <data name=\"ThreadId\" inType=\"win:Int32\"/>\n";
        manifestFile << "            <data name=\"ItemId\" inType=\"win:Int32\"/>\n";
        manifestFile << "            <data name=\"Complexity\" inType=\"win:Int32\"/>\n";
        manifestFile << "          </template>\n";
        manifestFile << "          <template tid=\"WorkItemEndTemplate\">\n";
        manifestFile << "            <data name=\"ThreadId\" inType=\"win:Int32\"/>\n";
        manifestFile << "            <data name=\"ItemId\" inType=\"win:Int32\"/>\n";
        manifestFile << "            <data name=\"Result\" inType=\"win:Double\"/>\n";
        manifestFile << "          </template>\n";
        manifestFile << "        </templates>\n";
        manifestFile << "      </provider>\n";
        manifestFile << "    </events>\n";
        manifestFile << "  </instrumentation>\n";
        manifestFile << "</instrumentationManifest>\n";
        
        manifestFile.close();
        std::cout << "Created ETW manifest file: OpenMPDiagnostics.man" << std::endl;
        std::cout << "To register the provider, run: wevtutil im OpenMPDiagnostics.man" << std::endl;
    } else {
        std::cerr << "Failed to create ETW manifest file" << std::endl;
    }
#else
    std::cout << "ETW provider setup not available on this platform." << std::endl;
#endif
}

int main(int argc, char* argv[]) {
    // Parse command line arguments
    CliParser parser(argc, argv);
    
    int numThreads = parser.getIntOption("threads", omp_get_max_threads());
    int workloadSize = parser.getIntOption("size", 100);
    int maxComplexity = parser.getIntOption("complexity", 10);
    
    std::cout << "=== Visual Studio Diagnostics Tools Demo ===" << std::endl;
    std::cout << "Threads: " << numThreads << std::endl;
    std::cout << "Workload size: " << workloadSize << std::endl;
    std::cout << "Max complexity: " << maxComplexity << std::endl;
    
    // Create workload
    std::vector<WorkItem> workload = createWorkload(workloadSize, maxComplexity);
    
    // Run demos
    demoVSConcurrencyMarkers(workload, numThreads);
    demoETWEvents(workload, numThreads);
    demoVSPerformanceTools(workload, numThreads);
    
    // Setup custom ETW provider
    if (parser.getBoolOption("setup-etw", false)) {
        setupCustomETWProvider();
    }
    
    std::cout << "All demos completed successfully!" << std::endl;
    
    return 0;
}