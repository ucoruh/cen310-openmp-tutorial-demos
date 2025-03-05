Create a comprehensive C++ OpenMP project demonstrating SIMD vectorization for Visual Studio 2022 on Windows 11 with these detailed specifications:

1. PROJECT STRUCTURE:
   
   - Project name: "OpenMP_SIMD_Vectorization"
   - Complete directory structure:
     - src/ for implementation files
     - include/ for headers
     - scripts/ for batch files and automation
     - docs/ for documentation
     - benchmarks/ for performance tests
     - asm_output/ for assembly inspection
     - utils/ for utility functions
     - visualizations/ for performance visualizations

2. CMAKE CONFIGURATION:
   
   - Create a sophisticated CMakeLists.txt that:
     - Requires CMake 3.20 or higher
     - Sets C++17 as the required standard
     - Properly configures OpenMP with find_package
     - Sets MSVC-specific compiler flags:
       * /openmp
       * /O2 (optimize for speed)
       * /arch:AVX2 (enable AVX2 instructions)
       * /Qvec-report:2 (vectorization reporting)
       * /fp:fast (fast floating-point model)
     - Creates multiple targets for different SIMD examples
     - Sets up separate configurations:
       * Debug (with minimal optimizations for debugging)
       * Release (fully optimized)
       * ReleaseWithAssembly (generates assembly output)
     - Properly handles include directories and dependencies
     - Configures output directories for all artifacts

3. IMPLEMENTATION FILES:
   
   - main.cpp:
     
     - Command-line interface with comprehensive options
     - Runtime detection of CPU SIMD capabilities
     - Menu system for selecting different vectorization examples
   
   - Create comprehensive example files:
     
     - basic_simd.cpp: Introduction to OpenMP SIMD directives:
       * Simple loop vectorization
       * Vectorization report interpretation
       * Performance comparison with and without SIMD
     - array_operations.cpp: SIMD for array operations:
       * Element-wise arithmetic
       * Reductions with SIMD
       * Alignment considerations
     - complex_math.cpp: Vectorizing math functions:
       * Transcendental functions (sin, cos, exp)
       * Custom math operations
       * SIMD with function calls
     - simd_alignment.cpp: Memory alignment optimization:
       * Aligned vs. unaligned memory access
       * Performance impact of misalignment
       * Proper alignment techniques
     - simd_width.cpp: Working with different SIMD widths:
       * Hardware-specific optimization
       * Handling remainder loops
       * Safelen clause usage
     - mixed_precision.cpp: SIMD with different data types:
       * Integer and floating-point mixed operations
       * Handling type conversions
     - simd_parallelism.cpp: Combining SIMD with thread parallelism:
       * SIMD within parallel for loops
       * Scaling analysis
       * Optimal thread count determination

4. ASSEMBLY OUTPUT ANALYSIS:
   
   - asm_analyzer.cpp:
     - Generate assembly output for vectorized loops
     - Highlight SIMD instructions in the assembly
     - Compare vectorized vs. non-vectorized assembly
     - Create human-readable annotations

5. BATCH FILES:
   
   - Environment setup:
     
     - check_prerequisites.bat: 
       * Verify Visual Studio 2022 installation
       * Check for required components
       * Detect CPU SIMD capabilities
     - setup_environment.bat:
       * Configure environment variables
       * Set optimal OpenMP settings
   
   - Build process:
     
     - configure.bat:
       * Create build directories
       * Run CMake with proper VS2022 generator
       * Configure with system-specific SIMD flags
     - build_all.bat:
       * Build all SIMD examples in multiple configurations
       * Generate assembly output for analysis
     - clean.bat:
       * Remove build artifacts
   
   - Execution:
     
     - run_benchmarks.bat:
       * Execute performance tests for all examples
       * Collect timing information
       * Compare scalar vs. vectorized performance
     - generate_asm_report.bat:
       * Create report with annotated assembly
     - generate_performance_report.bat:
       * Create comprehensive performance analysis

6. SIMD VERIFICATION:
   
   - simd_verifier.cpp:
     - Runtime detection of SIMD instruction usage
     - Verification of vectorization success
     - Measuring SIMD efficiency
     - Identifying vectorization bottlenecks

7. DOCUMENTATION:
   
   - README.md:
     
     - Project overview and educational objectives
     - Detailed build and execution instructions
     - System requirements and recommendations
     - Output interpretation guide
   
   - SIMD_VECTORIZATION_GUIDE.md:
     
     - Comprehensive explanation of SIMD concepts
     - OpenMP SIMD directive usage
     - Vectorization requirements and limitations
     - Performance optimization strategies
   
   - ASSEMBLY_ANALYSIS_GUIDE.md:
     
     - How to interpret vectorized assembly
     - SIMD instruction set reference
     - Common patterns in vectorized code
     - Identifying missed vectorization opportunities

8. PERFORMANCE ANALYSIS:
   
   - benchmark_suite.cpp:
     
     - Comprehensive benchmark suite for SIMD operations
     - Performance comparison across different data sizes
     - Analysis of vectorization speedup factors
     - Impact of alignment on performance
   
   - Create visual performance reports:
     
     - ASCII-based performance graphs
     - Speedup factor visualization
     - Comparison tables for different approaches

9. CPU CAPABILITY DETECTION:
   
   - cpu_features.cpp:
     - Detect available SIMD instruction sets (SSE, AVX, AVX2, AVX-512)
     - Optimal SIMD width determination
     - Runtime adaptation to available instruction sets

Ensure all code is extensively documented with detailed comments, includes proper error handling, and works seamlessly on Windows 11 with Visual Studio 2022 Community Edition installed.
