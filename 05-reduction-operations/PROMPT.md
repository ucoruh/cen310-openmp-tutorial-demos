Create a complete C++ OpenMP project demonstrating reduction operations for Visual Studio 2022 on Windows 11 with the following detailed specifications:

1. PROJECT STRUCTURE:
   
   - Project name: "OpenMP_Reductions"
   - Full project structure:
     - src/ directory with implementation files
     - include/ directory for headers
     - scripts/ directory for batch files
     - docs/ for documentation
     - tests/ for validation tests

2. CMAKE CONFIGURATION:
   
   - Create a comprehensive CMakeLists.txt that:
     - Requires CMake 3.20 or higher
     - Sets C++17 as required standard
     - Properly finds and configures OpenMP
     - Sets MSVC-specific compiler options (/openmp, /O2, /fp:fast for floating-point operations)
     - Creates multiple targets for different reduction examples
     - Configures both Debug and Release builds
     - Sets up proper output directories

3. IMPLEMENTATION FILES:
   
   - main.cpp: 
     
     - Menu system to select different reduction examples
     - Command-line parameter handling
     - Performance measurement infrastructure
   
   - Create separate files for each reduction type:
     
     - sum_reduction.cpp: Integer and floating-point summation
     - product_reduction.cpp: Product reduction operations
     - minmax_reduction.cpp: Finding minimum and maximum values
     - logical_reduction.cpp: Logical AND/OR reductions
     - custom_reduction.cpp: User-defined reduction operation
     - nested_reduction.cpp: Multiple reductions in nested loops
   
   - Create comparison implementations:
     
     - sequential_reductions.cpp: Non-parallel implementations
     - manual_reductions.cpp: Parallel implementations without reduction clause
     - omp_reductions.cpp: Implementations using OpenMP reduction clause

4. BATCH FILES:
   
   - Prerequisites:
     
     - vs_check.bat: Verifies Visual Studio 2022 installation and components
   
   - Build system:
     
     - configure.bat: Creates and configures build directory with Visual Studio 2022 generator
     - build_all.bat: Builds all examples in Debug and Release configurations
   
   - Execution:
     
     - run_benchmarks.bat: Runs performance comparisons between methods
     - validate_results.bat: Confirms numerical accuracy of all methods
   
   - Documentation:
     
     - generate_results.bat: Creates performance report in markdown format

5. DOCUMENTATION:
   
   - README.md:
     
     - Project overview and educational goals
     - Build and execution instructions
     - Output interpretation guide
   
   - REDUCTION_GUIDE.md:
     
     - Detailed explanation of OpenMP reduction clause
     - Performance characteristics
     - Common pitfalls and solutions
     - When to use each reduction operator
   
   - CUSTOM_REDUCTIONS.md:
     
     - Step-by-step guide for creating user-defined reductions
     - Examples for structures and complex types

6. PERFORMANCE ANALYSIS:
   
   - Implement timing measurements for all reduction operations
   - Create a comparison table showing:
     - Sequential vs. Manual Parallel vs. OpenMP Reduction
     - Scaling with different thread counts
     - Impact of data size on performance
   - Visualization of results using ASCII charts in the console

7. VALIDATION:
   
   - Create test cases ensuring numerical correctness
   - Compare results between sequential and parallel implementations
   - Handle potential floating-point accumulation differences

Ensure all code is robustly error-checked and works on Windows 11 with Visual Studio 2022 Community Edition with minimal setup required.
