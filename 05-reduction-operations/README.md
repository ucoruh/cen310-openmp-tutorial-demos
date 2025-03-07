# OpenMP Reduction Operations Demo

This project demonstrates various OpenMP reduction operations and different implementation techniques for parallel reductions. It compares the performance of various approaches and validates their correctness.

## Project Overview

Reduction operations in parallel computing combine multiple values into a single result using an associative operator (like `+`, `*`, `min`, `max`, etc.). In OpenMP, reductions are a common parallel pattern that can greatly benefit from proper implementation.

This demo includes:

1. **Multiple reduction operators**: sum, product, min/max, logical operations (AND/OR), and bitwise operations
2. **Different implementation approaches**:
   - Sequential (baseline)
   - Parallel with critical sections (inefficient)
   - Parallel with atomic operations
   - Manual reduction with thread-local variables
   - OpenMP reduction clause (most efficient)
3. **Performance comparisons** across different data sizes
4. **Result validation** to ensure correctness

## Prerequisites

- C++17 compatible compiler (MSVC, GCC, or Clang)
- CMake 3.10 or higher
- OpenMP support in your compiler

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
run.bat --reduction sum          # Run only the sum reduction example
run.bat --verbose                # Run with verbose output
run.bat --help                   # Show all available options
```

For the most comprehensive experience, you can use:

```
run_all.bat                      # Run all demonstrations in sequence
```

This will execute all reduction examples with various thread counts and performance comparisons.

For specific reduction types, we also provide:

```
run.bat --reduction sum          # Demonstrate sum reduction
run.bat --reduction product      # Demonstrate product reduction
run.bat --reduction min          # Demonstrate min reduction
run.bat --reduction max          # Demonstrate max reduction
run.bat --reduction custom       # Demonstrate custom reduction
```

**Note**: For accurate performance measurements, always use the Release build.

### Step 4: Clean

If you want to clean the build files and start from scratch:

```
clean.bat
```

This will remove all build artifacts and temporary files.

## Implementation Approaches

The project demonstrates several approaches to implementing reductions:

### 1. Sequential (Baseline)

A simple sequential implementation for reference:

```cpp
double sum = 0.0;
for (const auto& val : data) {
    sum += val;
}
```

### 2. Parallel with Critical Section

The simplest (but inefficient) parallel approach:

```cpp
double sum = 0.0;
#pragma omp parallel for
for (size_t i = 0; i < data.size(); ++i) {
    #pragma omp critical
    {
        sum += data[i];
    }
}
```

### 3. Parallel with Atomic Operations

Better than critical sections for simple operations:

```cpp
double sum = 0.0;
#pragma omp parallel for
for (size_t i = 0; i < data.size(); ++i) {
    #pragma omp atomic
    sum += data[i];
}
```

### 4. Manual Reduction

Each thread maintains a local copy, combining at the end:

```cpp
double sum = 0.0;
#pragma omp parallel
{
    double local_sum = 0.0;
    
    #pragma omp for
    for (size_t i = 0; i < data.size(); ++i) {
        local_sum += data[i];
    }
    
    #pragma omp critical
    {
        sum += local_sum;
    }
}
```

### 5. OpenMP Reduction Clause

The most elegant and efficient approach:

```cpp
double sum = 0.0;
#pragma omp parallel for reduction(+:sum)
for (size_t i = 0; i < data.size(); ++i) {
    sum += data[i];
}
```

## Supported Reduction Operations

This demo includes examples of all standard OpenMP reduction operations:

| Operator | Description | Example |
|----------|-------------|---------|
| `+` | Addition | `reduction(+:sum)` |
| `*` | Multiplication | `reduction(*:product)` |
| `-` | Subtraction | `reduction(-:diff)` |
| `&` | Bitwise AND | `reduction(&:bit_and)` |
| `\|` | Bitwise OR | `reduction(\|:bit_or)` |
| `^` | Bitwise XOR | `reduction(^:bit_xor)` |
| `&&` | Logical AND | `reduction(&&:logical_and)` |
| `\|\|` | Logical OR | `reduction(\|\|:logical_or)` |
| `min` | Minimum | `reduction(min:min_val)` |
| `max` | Maximum | `reduction(max:max_val)` |

## Performance Analysis

The program tests each approach with different data sizes:
- Small (10,000 elements)
- Medium (1,000,000 elements) 
- Large (10,000,000 elements)

Expected performance patterns:
1. Sequential: Baseline performance
2. Critical section: Often slower than sequential due to contention
3. Atomic operations: Better than critical but still has overhead
4. Manual reduction: Good performance with minimal synchronization
5. OpenMP reduction: Best performance, often comparable to manual reduction

## Validation

The program validates that all parallel implementations produce the same results as the sequential version, within a small tolerance for floating-point operations.

## Advanced Features

The project also demonstrates:
- Multiple reduction operations in a single loop
- Custom reduction operations (sum of squares)
- Dynamic thread management

## Additional Resources

For more detailed information about OpenMP reductions, see the companion guide:
[OpenMP Reduction Guide](REDUCTION_GUIDE.md)

## License

This project is provided as open source under the MIT License.

## Author

[Your Name/Organization]

## Acknowledgments

- OpenMP Architecture Review Board for the OpenMP specification
- The C++ Standards Committee for C++17 features 