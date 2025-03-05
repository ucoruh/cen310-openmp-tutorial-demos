Create a comprehensive C++ OpenMP project demonstrating task-based parallelism for Visual Studio 2022 on Windows 11 with the following detailed specifications:

1. PROJECT STRUCTURE:
   
   - Project name: "OpenMP_TaskParallelism"
   - Complete directory hierarchy:
     - src/ for main implementation files
     - include/ for headers
     - examples/ with different task paradigms
     - scripts/ for batch files
     - docs/ for documentation
     - utils/ for helper functions
     - benchmarks/ for performance testing

2. CMAKE CONFIGURATION:
   
   - Create a sophisticated CMakeLists.txt that:
     - Requires CMake 3.20 or higher
     - Mandates C++17 standard
     - Properly configures OpenMP with find_package
     - Sets MSVC-specific compiler flags (/openmp, /O2, /arch:AVX2)
     - Creates multiple targets for different task examples
     - Sets up proper Debug/Release configurations
     - Configures output directories and install rules
     - Handles potential VS2022 version differences

3. IMPLEMENTATION FILES:
   
   - main.cpp:
     
     - Command-line interface for selecting examples
     - Configuration options for task granularity and thread counts
     - Performance measurement infrastructure
   
   - Create comprehensive example files:
     
     - basic_tasks.cpp: Introduction to OpenMP tasks
     - fibonacci.cpp: Recursive Fibonacci using tasks
     - quicksort.cpp: Parallel quicksort with tasks
     - tree_traversal.cpp: Binary tree traversal with tasks
     - graph_processing.cpp: Graph algorithms with task parallelism
     - task_dependencies.cpp: Demonstrating depends(in,out) clauses
     - task_priority.cpp: Using priority clause for critical tasks
     - taskloop.cpp: Demonstrating taskloop construct
     - taskgroup.cpp: Using taskgroup for synchronization

4. VISUALIZATION:
   
   - task_visualizer.cpp:
     - Generate task dependency graphs
     - Visualize thread assignment
     - Show task execution timeline
     - Output in console-based graphics

5. BATCH FILES:
   
   - Environment setup:
     
     - check_vs2022.bat: Verifies Visual Studio 2022 and components
     - setup_environment.bat: Sets path and environment variables
   
   - Build process:
     
     - configure.bat: 
       * Creates build directory
       * Runs CMake with Visual Studio 17 2022 generator
       * Configures optional features
     - build_all.bat:
       * Builds all examples in both Debug and Release
       * Reports build status
   
   - Execution:
     
     - run_examples.bat: Executes all task examples with varying parameters
     - run_benchmarks.bat: Performs performance measurements
     - generate_reports.bat: Creates performance comparison reports

6. DOCUMENTATION:
   
   - README.md:
     
     - Project overview and educational goals
     - Detailed build instructions
     - Example execution guide
     - Expected output explanation
   
   - TASK_PARALLELISM_GUIDE.md:
     
     - In-depth explanation of OpenMP tasks
     - Best practices for task granularity
     - Task dependency management
     - Common pitfalls and solutions
   
   - PERFORMANCE_TUNING.md:
     
     - Task scheduling optimization
     - Cut-off strategies for recursive tasks
     - Memory locality considerations
     - Thread scaling analysis

7. PERFORMANCE ANALYSIS:
   
   - benchmark_suite.cpp:
     
     - Compare sequential, simple parallel, and task-based implementations
     - Measure scaling efficiency with different thread counts
     - Analyze task creation and scheduling overhead
     - Test different task granularity settings
   
   - Create comprehensive comparison visualizations:
     
     - Speedup graphs using ASCII art
     - Thread utilization metrics
     - Task granularity impact analysis

8. ADVANCED FEATURES:
   
   - task_stealing.cpp: Analyze OpenMP's task stealing behavior
   - nested_tasks.cpp: Demonstrate nested task creation
   - heterogeneous_tasks.cpp: Tasks with different computational requirements
   - task_throttling.cpp: Techniques to control task creation rate

Ensure all code is thoroughly tested, documented with detailed comments, and works seamlessly on Windows 11 with Visual Studio 2022 Community Edition installed.
