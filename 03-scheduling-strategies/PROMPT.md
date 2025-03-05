Create a comprehensive C++ OpenMP project demonstrating scheduling strategies for Visual Studio 2022 on Windows 11 with the following components:

1. PROJECT STRUCTURE:
   
   - Project name: "OpenMP_SchedulingStrategies"
   - Create a complete CMake-based structure with:
     - src folder for implementation files
     - include folder for headers
     - scripts folder for batch files
     - docs folder for documentation

2. CMAKE CONFIGURATION:
   
   - CMakeLists.txt with:
     - Minimum CMake version 3.20
     - C++17 standard requirement
     - OpenMP detection and configuration
     - MSVC-specific compiler flags (/openmp, /O2 for release)
     - Debug and Release configurations
     - Output directory configuration

3. IMPLEMENTATION FILES:
   
   - main.cpp:
     - Create a workload with variable computation time per iteration
     - Implement a function that calculates prime numbers up to a variable limit
     - Create an array of different limits to create uneven workload
     - Implement parallel for loops with all scheduling types:
       * Static scheduling with different chunk sizes
       * Dynamic scheduling with different chunk sizes
       * Guided scheduling with different chunk sizes
       * Auto scheduling
     - Add detailed timing for each scheduling type
     - Visualize work distribution across threads using a text-based chart
     - Create a summary showing which scheduling worked best for this workload

4. BATCH FILES:
   
   - configure.bat:
     - Detect Visual Studio installation
     - Create build directories
     - Run CMake with proper generator
   - build.bat:
     - Build both Debug and Release configurations
     - Report build status
   - run_comparisons.bat:
     - Run the executable multiple times with different thread counts
     - Collect and display comparative results

5. DOCUMENTATION:
   
   - README.md:
     - Project overview and educational objectives
     - Detailed explanation of each scheduling strategy
     - Build and execution instructions
     - Output interpretation guide
     - Performance analysis methodology
   - SCHEDULING.md:
     - In-depth explanation of when to use each scheduling type
     - Real-world scenarios for different scheduling strategies
     - Performance considerations

6. VISUALIZATION:
   
   - Implement console-based visualization showing:
     - Thread assignment for each iteration
     - Work distribution balance
     - Timeline of task completion
   - Create a summary table comparing all strategies

Ensure the code handles system differences and works seamlessly on Windows 11 with Visual Studio 2022 Community Edition installed.
