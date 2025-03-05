Create a comprehensive C++ OpenMP project demonstrating nested parallelism and thread affinity for Visual Studio 2022 on Windows 11 with these detailed specifications:

1. PROJECT STRUCTURE:
   
   - Project name: "OpenMP_NestedParallelism_Affinity"
   - Complete directory structure:
     - src/ for source files
     - include/ for headers
     - scripts/ for batch files
     - docs/ for documentation
     - benchmarks/ for performance tests
     - utils/ for utility functions
     - system/ for system topology detection

2. CMAKE CONFIGURATION:
   
   - Create a sophisticated CMakeLists.txt that:
     - Requires CMake 3.20 or newer
     - Sets C++17 as required standard
     - Properly configures OpenMP with find_package
     - Sets MSVC-specific compiler flags (/openmp, /O2, /Oi, /arch:AVX2)
     - Creates multiple targets for different aspects of nested parallelism
     - Configures Debug/Release builds with appropriate settings
     - Sets up proper include directories and dependencies
     - Handles potential VS2022 version differences

3. HARDWARE TOPOLOGY DETECTION:
   
   - system_topology.cpp:
     - Detect CPU topology (cores, physical packages, NUMA nodes)
     - Identify logical to physical core mapping
     - Generate visual representation of system topology
     - Store hardware information for affinity optimization

4. IMPLEMENTATION FILES:
   
   - main.cpp:
     
     - Command-line interface with comprehensive options
     - Configuration for thread counts and affinity settings
     - Menu system for selecting different demonstrations
   
   - Create comprehensive example files:
     
     - basic_nested.cpp: Introduction to nested parallel regions
     - matrix_multiplication.cpp: Nested parallel matrix operations:
       * Cache-aware blocking technique
       * Row-major vs. column-major optimizations
       * Performance analysis with different nesting strategies
     - nested_level_control.cpp: Managing parallel nesting depth
     - thread_num_control.cpp: Controlling thread counts at each level
     - proc_bind_examples.cpp: Demonstrating all proc_bind types:
       * primary, close, spread, master binding strategies
       * Performance impact of each binding type
     - affinity_customization.cpp: Custom thread placement strategies:
       * Manual thread binding with MSVC specific approaches
       * Thread migration prevention
       * NUMA-aware data placement

5. VISUALIZATION COMPONENTS:
   
   - thread_placement_visualizer.cpp:
     - Visual representation of thread placement on cores
     - Thread migration tracking
     - Cache hierarchy visualization
     - Thread binding effectiveness analysis

6. BATCH FILES:
   
   - Environment preparation:
     
     - check_prerequisites.bat: Verify VS2022 and required components
     - detect_system.bat: Detect system topology and capabilities
   
   - Build process:
     
     - configure.bat:
       * Create build directories
       * Run CMake with Visual Studio 17 2022 generator
       * Configure with system-specific optimizations
     - build_all.bat:
       * Build all examples in Debug and Release configurations
       * Create optimized builds with different settings
   
   - Execution:
     
     - run_topology_detection.bat: Generate system topology report
     - set_affinity_tests.bat: Run with different affinity settings
     - run_performance_comparison.bat: Execute benchmarks with various configurations
     - generate_reports.bat: Create comprehensive performance reports

7. DOCUMENTATION:
   
   - README.md:
     
     - Project overview and educational objectives
     - Detailed build and execution instructions
     - System requirements and recommendations
     - Output interpretation guide
   
   - NESTED_PARALLELISM_GUIDE.md:
     
     - In-depth explanation of OpenMP nested parallelism
     - Best practices and common pitfalls
     - When to use nested parallelism
     - Performance implications and overhead
   
   - THREAD_AFFINITY_GUIDE.md:
     
     - Comprehensive explanation of thread affinity concepts
     - Windows-specific affinity management techniques
     - NUMA considerations for Windows systems
     - Visual Studio and OpenMP affinity interaction

8. PERFORMANCE ANALYSIS:
   
   - benchmark_suite.cpp:
     - Comprehensive performance tests for different configurations:
       * Single level vs. nested parallelism
       * Various affinity strategies
       * Impact of thread counts and nesting levels
       * Memory access patterns and cache effects
     - Generate detailed performance comparison reports
     - Create ASCII-based charts for visualization

9. ENVIRONMENT VARIABLES MANAGEMENT:
   
   - Create scripts to manage OpenMP environment variables:
     - set_nested_env.bat: Configure OMP_NESTED, OMP_MAX_ACTIVE_LEVELS
     - set_affinity_env.bat: Configure OMP_PROC_BIND, OMP_PLACES
     - set_thread_env.bat: Configure OMP_NUM_THREADS for different levels

Ensure all code is thoroughly documented with detailed comments, includes proper error handling, and works seamlessly on Windows 11 with Visual Studio 2022 Community Edition installed.
