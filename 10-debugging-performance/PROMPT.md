Create a comprehensive C++ OpenMP project demonstrating debugging and performance analysis techniques for Visual Studio 2022 on Windows 11 with these detailed specifications:

1. PROJECT STRUCTURE:
   
   - Project name: "OpenMP_Debugging_Performance"
   - Complete directory structure:
     - src/ for implementation files
     - include/ for headers
     - scripts/ for batch files
     - docs/ for documentation
     - issues/ for intentional bug examples
     - profiling/ for performance analysis tools
     - fixes/ for corrected implementations
     - reports/ for generated reports
     - utils/ for utility functions

2. CMAKE CONFIGURATION:
   
   - Create a sophisticated CMakeLists.txt that:
     - Requires CMake 3.20 or higher
     - Sets C++17 as required standard
     - Properly configures OpenMP with find_package
     - Sets MSVC-specific compiler flags:
       * Debug mode: /openmp /O1 /Zi /RTC1 /MDd /D_DEBUG
       * Release mode: /openmp /O2 /GL /LTCG
       * Profile mode: /openmp /O2 /Zi /DPROFILE
     - Adds Windows-specific definitions for VSPerf and ETW if available
     - Creates multiple targets for different bug categories
     - Properly handles include paths and dependencies
     - Sets up build variants with different instrumentation levels
     - Configures output directories for different artifacts

3. COMMON ISSUE IMPLEMENTATIONS:
   
   - Create comprehensive examples for each issue category:
     
     - race_conditions.cpp:
       
       * Multiple thread access patterns causing data races
       * Increment, read-modify-write, and complex data structure races
       * Demonstrate race detection techniques
       * Add instrumentation points for diagnostics
     
     - false_sharing.cpp:
       
       * Create classic false sharing scenario with adjacent data
       * Demonstrate cache line effects with timing
       * Show memory layout visualization
       * Implement progressively improved versions
     
     - load_imbalance.cpp:
       
       * Create workloads with significant imbalance
       * Demonstrate different scheduling strategies
       * Implement workload distribution visualization
       * Show thread idle time analysis
     
     - excessive_synchronization.cpp:
       
       * Implement over-synchronized algorithms
       * Show lock contention effects
       * Measure synchronization overhead
       * Progressive improvements with relaxed synchronization
     
     - memory_issues.cpp:
       
       * Demonstrate NUMA effects on Windows
       * Show inefficient memory access patterns
       * Implement cache-oblivious alternatives
       * Memory bandwidth saturation tests

4. DIAGNOSTIC TOOLS INTEGRATION:
   
   - vs_diagnostics.cpp:
     
     - Integration with Visual Studio Diagnostics Tools
     - Configuring and using Concurrency Visualizer
     - Instrumenting code for ETW events
     - Custom ETW provider implementation
   
   - intel_vtune.cpp:
     
     - Integration instructions for Intel VTune Profiler
     - ITT instrumentation API usage
     - Custom data collection markers
     - Result interpretation guide
   
   - custom_profiler.cpp:
     
     - Lightweight custom profiling infrastructure
     - Thread-safe timing instrumentation
     - Performance counter integration
     - Visual results generation

5. BATCH FILES:
   
   - Environment setup:
     
     - check_prerequisites.bat:
       * Verify Visual Studio 2022 installation
       * Check for required components
       * Set up environment variables
     - setup_debugging_tools.bat:
       * Configure diagnostic tools integration
       * Set up symbol paths
       * Configure Windows Performance Toolkit
   
   - Build process:
     
     - configure.bat:
       * Create build directories for each configuration
       * Run CMake with Visual Studio 17 2022 generator
       * Configure diagnostic options
     - build_all.bat:
       * Build all examples in Debug, Release, and Profile modes
       * Generate PDB files for debugging
       * Create instrumented builds
   
   - Execution:
     
     - run_diagnostics.bat:
       * Execute examples with various diagnostic tools
       * Collect performance data
       * Generate diagnostic reports
     - run_issue_examples.bat:
       * Execute all issue examples with debug instrumentation
       * Collect problem evidence
       * Generate comparison reports with fixes

6. VISUAL STUDIO INTEGRATION:
   
   - vs_extensions.md:
     - Guide for installing useful VS extensions for OpenMP
     - Setup for Concurrency Visualizer
     - Custom visualizer (.natvis) files for OpenMP constructs
     - Debugging shortcuts and techniques

7. DOCUMENTATION:
   
   - README.md:
     
     - Project overview and educational objectives
     - Detailed build and execution instructions
     - System requirements and recommendations
     - Diagnostic tools setup guide
   
   - DEBUGGING_GUIDE.md:
     
     - Comprehensive OpenMP debugging techniques
     - Visual Studio debugger configuration
     - Thread debugging best practices
     - Race condition identification
   
   - PERFORMANCE_ANALYSIS_GUIDE.md:
     
     - Profiling methodology for OpenMP programs
     - Tools comparison (VS Diagnostics, VTune, etc.)
     - Performance metrics interpretation
     - Common bottleneck identification
   
   - ISSUE_PATTERNS.md:
     
     - Catalog of common OpenMP bugs and performance issues
     - Detection techniques for each issue
     - Root cause analysis methodology
     - Solution patterns and best practices

8. PERFORMANCE VISUALIZATION:
   
   - thread_timeline_visualizer.cpp:
     
     - Generate visual thread execution timeline
     - Highlight synchronization points
     - Show idle periods and load imbalance
     - Export to HTML with interactive features
   
   - memory_access_visualizer.cpp:
     
     - Visualize memory access patterns
     - Show cache line conflicts
     - Highlight NUMA effects
     - Generate heat maps of memory access

9. FIXES AND SOLUTIONS:
   
   - For each issue, create corresponding fixed implementations:
     
     - race_conditions_fixed.cpp
     - false_sharing_fixed.cpp
     - load_imbalance_fixed.cpp
     - excessive_synchronization_fixed.cpp
     - memory_issues_fixed.cpp
   
   - Create progressive improvement versions showing:
     
     - Initial problematic implementation
     - Basic fix implementation
     - Optimized implementation
     - Best practice implementation

10. AUTOMATED ANALYSIS:
    
    - analysis_tools.cpp:
      - Automated race detection helper
      - Performance regression testing framework
      - Pattern recognition for common issues
      - Recommendation engine for fixes

Ensure all code is thoroughly documented with detailed comments, includes proper error handling, and works seamlessly on Windows 11 with Visual Studio 2022 Community Edition installed. Include warnings about compiler optimization levels that may hide certain issues in release builds but expose them in debug builds.
