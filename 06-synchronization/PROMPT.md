Create a comprehensive C++ OpenMP project demonstrating synchronization mechanisms for Visual Studio 2022 on Windows 11 with these detailed specifications:

1. PROJECT STRUCTURE:
   
   - Project name: "OpenMP_Synchronization"
   - Complete directory structure:
     - src/ for implementation files
     - include/ for header files
     - scripts/ for batch files
     - docs/ for documentation
     - data/ for output data and results
     - utils/ for utility functions

2. CMAKE CONFIGURATION:
   
   - Create a sophisticated CMakeLists.txt that:
     - Requires CMake 3.20 or higher
     - Mandates C++17 standard
     - Properly detects and configures OpenMP
     - Sets MSVC-specific flags (/openmp, /W4, /EHsc)
     - Creates separate targets for each synchronization mechanism
     - Configures appropriate Debug/Release settings
     - Sets up proper include paths and dependencies
     - Handles potential platform-specific configurations

3. IMPLEMENTATION FILES:
   
   - main.cpp:
     
     - Command-line argument handling
     - Menu system for selecting demos
     - Configuration options for thread counts and workloads
   
   - Create comprehensive demo files for each synchronization mechanism:
     
     - race_conditions.cpp: Demonstrates unsynchronized access problems
     - critical_sections.cpp: Using #pragma omp critical with:
       * Named critical sections
       * Nested critical sections
       * Performance analysis
     - atomic_operations.cpp: Using #pragma omp atomic with:
       * Different atomic operations (update, read, write, capture)
       * Performance comparison with critical sections
     - locks.cpp: OpenMP lock mechanisms:
       * Simple locks
       * Nested locks
       * Lock hints
       * Reader-writer locks
     - barriers.cpp: Thread synchronization points:
       * Implicit barriers
       * Explicit barriers
       * Impact on performance
     - ordered.cpp: Ordered execution within parallel regions
     - master_single.cpp: Compare master and single constructs
     - flush.cpp: Memory consistency with flush directive

4. VISUALIZATION:
   
   - Create visualization utilities:
     - thread_timeline.cpp: Visual representation of thread execution
     - lock_contention.cpp: Visualization of lock contention periods
     - memory_consistency.cpp: Visualization of memory state

5. BATCH FILES:
   
   - Environment setup:
     
     - check_prerequisites.bat: Verifies VS2022 installation and components
     - setup_environment.bat: Sets required environment variables
   
   - Build process:
     
     - configure.bat: Run CMake with proper VS2022 generator
     - build_all.bat: Builds all demos in Debug and Release
     - clean.bat: Removes build artifacts
   
   - Execution:
     
     - run_demos.bat: Executes all synchronization demos sequentially
     - run_performance_tests.bat: Runs and captures performance metrics
     - generate_report.bat: Creates consolidated report with results

6. DOCUMENTATION:
   
   - README.md:
     
     - Project overview and learning objectives
     - Detailed build and execution instructions
     - Output interpretation guide
     - Troubleshooting common issues
   
   - SYNCHRONIZATION_GUIDE.md:
     
     - Comprehensive explanations of each synchronization mechanism
     - Performance characteristics and trade-offs
     - Best practices for each mechanism
     - Common pitfalls and solutions

7. PERFORMANCE ANALYSIS:
   
   - Implement detailed performance comparison:
     - Measure overhead of each synchronization method
     - Compare scalability with different thread counts
     - Analyze lock contention scenarios
     - Create graphs using ASCII art in console output

8. DEBUGGING HELPERS:
   
   - Include utilities for debugging threading issues:
     - race_detector.cpp: Simple race condition detection
     - deadlock_detector.cpp: Detecting potential deadlocks
     - thread_analyzer.cpp: Thread behavior analysis

Ensure all code is thoroughly documented, error-checked, and works seamlessly on Windows 11 with Visual Studio 2022 Community Edition installed.
