Create a complete C++ OpenMP project demonstrating parallel for loops for Visual Studio 2022 on Windows 11 with the following:

1. PROJECT STRUCTURE:
   
   - Create a project named "OpenMP_ParallelForLoops"
   - Include complete folder structure for a CMake project
   - Create a src folder containing main.cpp
   - Create an include folder for any header files

2. CMAKE CONFIGURATION:
   
   - Create a CMakeLists.txt that:
     - Sets minimum CMake version to 3.20
     - Sets C++ standard to C++17
     - Uses find_package for OpenMP with required flag
     - Configures MSVC-specific flags for OpenMP (/openmp)
     - Creates executable with proper linking to OpenMP
     - Sets appropriate optimization flags for Release mode

3. MAIN CODE IMPLEMENTATION:
   
   - Create an array of 100 million integers
   - Implement and compare three versions:
     - Sequential array initialization and sum calculation
     - Parallel array initialization using "#pragma omp parallel for"
     - Parallel sum calculation with proper reduction
   - Add precise timing for each version using high-resolution timers
   - Display performance comparison results
   - Visualize thread workload distribution

4. BATCH FILES:
   
   - configure.bat: 
     - Creates build directory
     - Runs CMake with Visual Studio 17 2022 generator
     - Configures both Debug and Release modes
   - build.bat:
     - Compiles both Debug and Release configurations
     - Shows compilation progress
   - run_debug.bat and run_release.bat:
     - Execute the appropriate build with timing information

5. DOCUMENTATION:
   
   - README.md with:
     - Project description and learning objectives
     - Build and execution instructions
     - Expected output explanation
     - Performance analysis guidance
     - Common issues and solutions

6. CODE QUALITY:
   
   - Exception handling for memory allocation
   - Detailed comments for each OpenMP directive
   - Thread-safety analysis
   - Memory cleanup and resource management

Ensure all code is fully functional on Windows 11 with Visual Studio 2022 Community Edition with minimal configuration required from the user.
