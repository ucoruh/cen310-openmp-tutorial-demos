Create a complete C++ OpenMP demo project for Visual Studio 2022 on Windows 11 with the following:

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

3. MAIN CODE:
   
   - Include all necessary headers (#include <omp.h>, etc.)
   - Create a main function demonstrating a basic parallel region with:
     - Code that sets the number of threads to 4
     - A parallel region where each thread prints its ID and the total thread count
     - Code sections showing thread-private vs shared execution
     - Timing code to measure parallel execution speed

4. BATCH FILES:
   
   - Create configure.bat that:
     - Creates a build directory
     - Runs CMake configuration with Visual Studio 2022 generator
   - Create build.bat that:
     - Runs MSBuild to compile the project in both Debug and Release modes
   - Create run.bat that:
     - Executes the compiled program from the correct output directory

5. DOCUMENTATION:
   
   - Add README.md explaining:
     - Project purpose and what it demonstrates
     - Step-by-step instructions for building and running
     - Expected output and explanation
     - Common issues and troubleshooting

6. CODE QUALITY:
   
   - Include comprehensive comments explaining each OpenMP directive
   - Add error checking for OpenMP initialization
   - Handle potential threading issues

Ensure all files work out-of-the-box on a standard Windows 11 system with Visual Studio 2022 Community Edition.
