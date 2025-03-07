Create a complete C++ OpenMP demo project for Visual Studio 2022 on Windows with the following:

1. PROJECT STRUCTURE:
   
   - Create a project named "OpenMP_BasicParallel"
   - Include all necessary folder structure for a CMake project
   - Create a src folder with main.cpp as the entry point

2. CMAKE CONFIGURATION:
   
   - Create a CMakeLists.txt file that:
     - Sets minimum CMake version to 3.20
     - Sets C++ standard to C++17
     - Properly configures OpenMP with find_package
     - Links OpenMP libraries correctly
     - Adds compiler-specific flags for MSVC
     - Properly configures output directories (note: with Visual Studio generator, executables will be in build/Debug and build/Release)

3. MAIN CODE:
   
   - Include all necessary headers (#include <omp.h>, etc.)
   - Create a main function demonstrating a basic parallel region with:
     - Code that sets the number of threads to 4
     - A parallel region where each thread prints its ID and the total thread count
     - Code sections showing thread-private vs shared execution
     - Timing code to measure parallel execution speed
     - Vector operations demonstrating parallel vs sequential performance
     - Important: Use signed integer types (like long long) for OpenMP loop indices, not size_t

4. BATCH FILES:
   
   - Create configure.bat that:
     - Creates a build directory
     - Runs CMake configuration with Visual Studio 2022 generator
     - Provides clear feedback and next steps
   
   - Create build.bat that:
     - Runs MSBuild to compile the project in both Debug and Release modes
     - Displays the correct paths to the executables
     - Provides proper error handling
   
   - Create run.bat that:
     - Executes the compiled program from the correct output directory
     - Allows user to select Debug or Release configuration
     - Includes proper error handling if executables don't exist
   
   - Create clean.bat that:
     - Removes build directory and temporary files
     - Handles errors gracefully if files are locked
     - Provides instructions for rebuilding

5. DOCUMENTATION:
   
   - Add README.md explaining:
     - Project purpose and what it demonstrates
     - Step-by-step instructions for building and running
     - Expected output and explanation
     - Common issues and troubleshooting
     - Include a PlantUML activity diagram visualizing the parallel workflow
     - Document proper paths and execution flow

6. CODE QUALITY:
   
   - Include comprehensive comments explaining each OpenMP directive
   - Add error checking for OpenMP initialization
   - Handle potential threading issues
   - Use critical sections appropriately for thread-safe operations
   - Ensure signed integers are used for OpenMP loop indices
   - Implement meaningful performance measurements with speedup and efficiency calculations

7. ADVANCED FEATURES:
   
   - Implement a vector addition demo showing clear performance benefits
   - Add efficiency calculation (speedup / number of threads * 100%)
   - Include proper error handling and informative messages
   - Make sure the demo works consistently across different Windows systems
   - Optimize for both Debug and Release configurations

Ensure all files work out-of-the-box on a standard Windows system with Visual Studio 2022 Community Edition. The project should demonstrate clear benefits of parallel processing and provide educational value for learning OpenMP concepts.
