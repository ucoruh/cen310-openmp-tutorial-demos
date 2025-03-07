# OpenMP Nested Parallelism & Thread Affinity Demo

This project demonstrates nested parallelism and basic thread affinity control using OpenMP on Windows with Visual Studio. The code is designed to work with OpenMP 2.0 which is the default version supported by Visual Studio.

## Project Overview

Modern processors offer multiple levels of parallelism - from multiple CPU sockets to multiple cores per socket, and often multiple hardware threads per core. This project demonstrates basic techniques to utilize this hierarchical parallelism through nested parallel regions and simple thread placement strategies.

### Key Features

- **Nested Parallelism**: Demonstrates multi-level parallel execution
- **System Topology Detection**: Detects and visualizes your hardware's processor topology
- **Thread Placement**: Shows basic techniques for thread placement on specific cores
- **Matrix Multiplication Performance**: Compares different parallelization strategies 
- **Cache-Aware Algorithms**: Demonstrates techniques for cache-friendly parallel processing

## System Requirements

- **Operating System**: Windows 10 or Windows 11
- **Development Environment**: Visual Studio 2022 (any edition)
- **Build System**: CMake 3.20 or higher
- **C++ Standard**: C++17 compatible compiler
- **Hardware**: Multi-core processor (4+ cores recommended for best demonstration)

## OpenMP Version Compatibility

This project is specifically designed to work with OpenMP 2.0, which is the default version supported by Visual Studio. If you have Visual Studio 2019 version 16.9 or later, you can use the `-openmp:llvm` compiler option to enable support for newer OpenMP features.

## Project Structure

```
OpenMP_NestedParallelism_Affinity/
├── include/               # Header files
│   ├── system_topology.h  # System hardware detection
│   ├── thread_utils.h     # Thread management utilities
│   └── cli_parser.h       # Command-line interface
├── scripts/               # Utility scripts
│   ├── check_prerequisites.bat
│   └── set_affinity_tests.bat
├── src/                   # Source files
│   ├── main.cpp           # Main application
│   └── matrix_multiplication.cpp  # Matrix operations
├── system/                # System detection implementation
│   └── system_topology.cpp
├── utils/                 # Utilities implementation
│   ├── thread_utils.cpp
│   └── cli_parser.cpp
├── CMakeLists.txt         # CMake build configuration
├── configure.bat          # Project configuration script
├── build.bat              # Build script
├── run.bat                # Run script
├── clean.bat              # Cleanup script
└── README.md              # This file
```

## Getting Started

### Step 1: Verify Prerequisites

Before starting, verify that your system meets all requirements by running:

```
scripts\check_prerequisites.bat
```

This script checks for:
- Compatible Windows version
- Visual Studio installation
- CMake version
- C++ compiler availability
- Processor capabilities

### Step 2: Configure the Project

Run the configuration script to generate the Visual Studio project files:

```
configure.bat
```

This script:
- Creates a `build` directory if needed
- Runs CMake to generate Visual Studio project files
- Configures the project with appropriate settings for your system

### Step 3: Build the Project

Build both Debug and Release configurations:

```
build.bat
```

This script builds:
- The main application executable
- The matrix multiplication example

### Step 4: Run the Application

Run the main application with:

```
run.bat
```

The run script allows you to:
- Choose between Debug and Release configurations
- Configure OpenMP environment variables
- Set the number of threads for parallel execution

### Step 5: Explore Specific Demonstrations

The project includes specialized demonstrations that can be run from the main menu or independently.

#### System Topology

This demo displays information about your system's processor topology, including:
- Number of logical processors
- Physical packages (sockets)
- Cores per package
- NUMA nodes

#### Nested Parallelism

Demonstrates multi-level parallel execution with:
- Outer parallel region with configurable thread count
- Inner parallel region with configurable thread count
- Thread information at each nesting level

#### Thread Affinity

Shows thread placement on cores and includes a demo with manual control of thread affinity.

#### Matrix Multiplication Performance

Run the dedicated matrix multiplication executable:

```
build\Release\matrix_multiplication.exe --matrix_size=1000 --outer_threads=4 --inner_threads=2
```

This demonstrates:
- Sequential vs. parallel matrix multiplication
- Single-level vs. nested parallelism
- Cache-blocked algorithm implementation

## Understanding Nested Parallelism

Nested parallelism allows for multiple levels of parallel execution. This can be useful in cases where:

1. A task has multiple levels of parallelism (e.g., processing multiple files, each with multiple data records)
2. Different parallel regions require different degrees of parallelism
3. Different parallel regions have different granularities of work

To enable nested parallelism in OpenMP 2.0:

```cpp
omp_set_nested(1); // Enable nested parallelism
```

Then create nested parallel regions:

```cpp
#pragma omp parallel num_threads(outer_threads)
{
    // Outer parallel region code
    
    #pragma omp parallel num_threads(inner_threads)
    {
        // Inner parallel region code
    }
}
```

## Thread Placement

In OpenMP 2.0, thread placement control is limited but can be implemented using Windows API functions:

```cpp
#ifdef _WIN32
    HANDLE hThread = GetCurrentThread();
    DWORD_PTR mask = 1ULL << coreId;  // Set bit for desired core
    SetThreadAffinityMask(hThread, mask);
#endif
```

The project demonstrates how to:
1. Get the current thread's core ID
2. Set thread affinity to a specific core
3. Create custom thread placement strategies

## Performance Tuning

For optimal performance:

- Use Release builds for all performance testing
- Experiment with different thread counts at each nesting level
- Try different block sizes for matrix multiplication
- Consider your system's NUMA topology when distributing work

## Troubleshooting

### Common Issues

- **Build Fails**: Ensure Visual Studio with C++ desktop development workload is installed
- **OpenMP Not Found**: Check that the appropriate Visual Studio components are installed
- **Poor Performance**: Try different thread counts and block sizes
- **Inconsistent Results**: Ensure no background processes are competing for resources

### Environment Variables

Key OpenMP environment variables used in this project:

- `OMP_NESTED`: Set to 1 to enable nested parallelism
- `OMP_NUM_THREADS`: Number of threads to use

## Upgrading to Newer OpenMP Versions

If you have Visual Studio 2019 version 16.9 or later, you can enable support for newer OpenMP features by:

1. Using the `-openmp:llvm` compiler option instead of `-openmp`
2. Updating the code to use newer OpenMP features (e.g., `proc_bind` clauses, `omp_get_max_active_levels()`)

To modify the CMakeLists.txt for this:

```cmake
if(MSVC)
    # For newer OpenMP features
    add_compile_options(/openmp:llvm)
endif()
```

## License

This project is provided as-is for educational and research purposes.