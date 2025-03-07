# OpenMP SIMD Vectorization Demo

This project demonstrates SIMD (Single Instruction, Multiple Data) vectorization techniques using OpenMP directives in C++. It provides comprehensive examples, benchmarks, and analysis tools to help you understand and utilize SIMD vectorization for performance optimization.

## Project Purpose

This demo showcases:

1. How to use OpenMP SIMD directives to enable vectorization
2. Performance benefits of SIMD vectorization for various operations
3. Analysis of vectorized vs. non-vectorized assembly code
4. Best practices for maximizing SIMD performance
5. Integration of SIMD with multi-threading (OpenMP parallel)

## Prerequisites

- Windows 10 or Windows 11
- Visual Studio 2022 Community Edition or higher
- CMake 3.20 or higher
- C++17 compatible compiler
- CPU with SIMD support (at minimum SSE2, ideally AVX2)

## Project Structure

- `src/`: Implementation files
- `include/`: Header files
- `scripts/`: Batch files for automation
- `benchmarks/`: Performance test results
- `asm_output/`: Assembly output for analysis
- `docs/`: Documentation files

## Getting Started

### Step 1: Check Prerequisites

Run the `check_prerequisites.bat` script to verify that your system meets all requirements:

```
check_prerequisites.bat
```

### Step 2: Set Up Environment

Run the environment setup script to configure optimal settings for SIMD development:

```
setup_environment.bat
```

### Step 3: Configure the Project

Run the configure script to generate the Visual Studio project files:

```
configure.bat
```

This will:
- Create a `build` directory (if it doesn't exist)
- Run CMake with the Visual Studio 2022 generator

### Step 4: Build the Project

Run the build script to compile the project in both Debug and Release configurations:

```
build.bat
```

This will build:
- Debug configuration: `build\Debug\OpenMP_SIMD_Vectorization.exe`
- Release configuration: `build\Release\OpenMP_SIMD_Vectorization.exe`

### Step 5: Run the Program

Run the execution script to select and run either the debug or release version:

```
run.bat
```

## SIMD Examples Included

The project includes the following SIMD vectorization examples:

1. **Basic SIMD Introduction**  
   Simple vector addition with and without SIMD directives.

2. **Array Operations with SIMD**  
   Element-wise arithmetic and reductions with SIMD optimizations.

3. **Complex Math Functions with SIMD**  
   Vectorization of transcendental math functions (sin, cos, exp).

4. **Memory Alignment Optimization**  
   Impact of aligned vs. unaligned memory access on SIMD performance.

5. **SIMD Width Adaptation**  
   Handling different SIMD widths (SSE, AVX, AVX2).

6. **Mixed Precision Operations**  
   SIMD operations with different data types (float, int).

7. **SIMD with Thread Parallelism**  
   Combining SIMD vectorization with OpenMP multi-threading.

## Analysis Tools

### Assembly Output Analysis

Generate and analyze assembly code to understand how the compiler vectorizes your code:

```
generate_asm_report.bat
```

This will create annotated assembly files in the `asm_output` directory that highlight SIMD instructions.

### Performance Benchmarking

Run comprehensive benchmarks to measure the impact of SIMD vectorization:

```
run_benchmarks.bat
```

Generate detailed performance reports:

```
generate_performance_report.bat
```

## OpenMP 2.0 Compatibility

All examples in this project are compatible with OpenMP 2.0, ensuring broad compatibility with different compilers and systems. The SIMD directives used are part of the OpenMP 4.0+ specification but are implemented in a way that works with OpenMP 2.0 when available.

## Windows 11 Optimization

This project has been optimized for Windows 11, utilizing its improved thread scheduling and memory management for better SIMD performance. It also works well on Windows 10.

## Common Issues and Troubleshooting

### SIMD Not Supported

If you see "SIMD instructions not detected" when running the verifier:

- Make sure your CPU supports at least SSE2 instructions
- Check that compiler flags are correctly set in CMakeLists.txt

### Build Errors

- Ensure Visual Studio 2022 is properly installed
- Make sure you have the C++ desktop development workload installed
- Try running the scripts as Administrator if you encounter permission issues

### Performance Issues

- The SIMD speedup may vary depending on your hardware
- For best results, run the Release configuration
- Try different thread binding policies in the `setup_environment.bat` script

## Advanced Usage

### Custom Vector Sizes

You can modify the vector sizes used in the benchmarks by editing the constants in the source files or by passing command-line arguments:

```
build\Release\OpenMP_SIMD_Vectorization.exe --benchmark --size=100000000
```

### Custom Assembly Analysis

You can generate assembly for your own code by adding it to the project and modifying the `generate_asm_report.bat` script.

## License

This project is provided as-is for educational purposes.

## Acknowledgments

- Intel and AMD for developing SIMD instruction sets
- The OpenMP ARB for creating the OpenMP specification
- Microsoft for Visual Studio and the MSVC compiler

## Further Reading

- [OpenMP 4.5 Specifications](https://www.openmp.org/specifications/)
- [Intel Intrinsics Guide](https://software.intel.com/sites/landingpage/IntrinsicsGuide/)
- [MSVC Compiler Options](https://docs.microsoft.com/en-us/cpp/build/reference/compiler-options-listed-alphabetically)