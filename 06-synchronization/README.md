# OpenMP Synchronization Mechanisms

This project demonstrates various synchronization mechanisms available in OpenMP, a parallel programming API for C/C++. Proper synchronization is essential for correct and efficient parallel programs to avoid race conditions and ensure proper coordination between threads.

## Overview

Parallel programming introduces challenges when multiple threads access shared resources simultaneously. OpenMP provides several synchronization constructs to coordinate thread execution and protect shared data. This project includes comprehensive examples and benchmarks for each synchronization mechanism.

## Synchronization Mechanisms Covered

### 1. Race Conditions

Race conditions occur when multiple threads access and modify shared data without proper synchronization. The examples demonstrate:
- Counter race conditions
- Array update race conditions  
- Sum reduction race conditions

### 2. Critical Sections

Critical sections ensure exclusive access to shared resources. Examples include:
- Basic critical sections
- Named critical sections for fine-grained control
- Nested critical sections
- Performance impact analysis

### 3. Atomic Operations

Atomic operations provide lightweight synchronization for simple operations. Examples include:
- Different atomic operations (update, read, write, capture)
- Performance comparison with critical sections

### 4. Locks

OpenMP provides more flexible lock mechanisms than critical sections. Examples include:
- Simple locks (omp_set_lock, omp_unset_lock)
- Nested locks (omp_set_nest_lock, omp_unset_nest_lock)
- Lock hints for performance optimization
- Implementing reader-writer locks

### 5. Barriers

Barriers synchronize all threads at specific points. Examples include:
- Implicit barriers (at the end of parallel regions and worksharing constructs)
- Explicit barriers (#pragma omp barrier)
- Performance impact of barriers

### 6. Ordered Construct

The ordered construct ensures sequential execution within a parallel loop. Examples include:
- Basic ordered execution
- Performance comparison with unordered execution

### 7. Master and Single Constructs

These constructs restrict execution to one thread. Examples include:
- Master construct (executed only by the master thread)
- Single construct (executed by exactly one thread)
- Performance comparison

### 8. Flush Directive

The flush directive ensures memory consistency. Examples include:
- Memory consistency examples
- Visualization of memory state

## Performance Analysis

The project includes comprehensive performance analysis tools:
- Overhead measurement for each synchronization mechanism
- Scalability analysis with different thread counts
- Visualization of lock contention and thread activity

## Prerequisites

- Windows 11
- Visual Studio 2022 Community Edition (or newer)
- CMake 3.20 or higher
- C++17 compatible compiler

## Building and Running

### Step 1: Configure

Run the configuration script:
```
configure.bat
```

### Step 2: Build

Build the project in Debug and Release configurations:
```
build_all.bat
```

### Step 3: Run

Use the unified `run.bat` script with various options:

```
run.bat                          # Run with default settings (Release mode)
run.bat --debug                  # Run in Debug mode with additional diagnostics
run.bat --release                # Run in Release mode (optimized performance)
run.bat --threads 8              # Run with 8 threads
run.bat --sync critical          # Run only critical section examples
run.bat --verbose                # Run with verbose output
run.bat --help                   # Show all available options
```

For the most comprehensive experience, you can use:

```
run_all.bat                      # Run all demonstrations in sequence
```

This will execute all synchronization examples with various thread counts and performance comparisons.

For specific synchronization mechanisms, we also provide:

```
run.bat --sync critical          # Demonstrate critical sections
run.bat --sync atomic            # Demonstrate atomic operations
run.bat --sync ordered           # Demonstrate ordered constructs
run.bat --sync barrier           # Demonstrate barriers
run.bat --sync nowait            # Demonstrate nowait clause
```

**Note**: For accurate performance measurements, always use the Release build.

### Step 4: Clean

If you want to clean the build files and start from scratch:

```
clean.bat
```

This will remove all build artifacts and temporary files.

### Command-Line Arguments

The program supports various command-line arguments:

```
Usage: OpenMP_Synchronization [options]

Options:
  -h, --help                  Show this help message
  -b, --benchmark             Run in benchmark mode
  -p, --performance           Run performance analysis
  -t, --threads <num>         Specify number of threads (0 for default)
  -w, --workload <size>       Specify workload size
  -d, --demo <n>              Run a specific demo by name

Examples:
  OpenMP_Synchronization                              Run interactive menu
  OpenMP_Synchronization --benchmark                  Run all benchmarks
  OpenMP_Synchronization --threads 4 --workload 1000000  Run with 4 threads and specified workload
  OpenMP_Synchronization --demo "Basic Critical Sections"  Run a specific demo
```

## Visualization Tools

The project includes visualization tools to help understand thread behavior and synchronization:

- Thread timeline visualization
- Lock contention visualization
- Memory consistency visualization

## Understanding the Results

### Performance Overhead

Each synchronization mechanism introduces different levels of overhead:

1. **Critical Sections**: Highest overhead but most flexible
2. **Atomic Operations**: Lower overhead than critical sections, limited functionality
3. **Barriers**: High overhead, used for coordinating all threads
4. **Master/Single**: Medium overhead, used when a section should be executed by one thread

### Scalability

The examples demonstrate how different synchronization approaches scale with:
- Increasing thread counts
- Increasing workload sizes

## Best Practices

### 1. Minimize Synchronization

- Use as little synchronization as possible
- Synchronization creates bottlenecks and reduces parallelism

### 2. Use the Right Mechanism

- Use atomic operations for simple updates to shared variables
- Use critical sections when more complex code needs protection
- Use locks for more fine-grained control

### 3. Reduce Contention

- Use named critical sections to allow non-competing code to run in parallel
- Minimize the code inside critical sections
- Use thread-local storage when possible

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- OpenMP Architecture Review Board for the OpenMP specification
- Microsoft Visual Studio and Intel teams for their OpenMP implementations

## Contact

If you have any questions or feedback, please open an issue on the project repository. 