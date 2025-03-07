# OpenMP Scheduling Strategies Demo

This project demonstrates different OpenMP scheduling strategies and their impact on performance, load balancing, and thread utilization when applied to non-uniform workloads.

## Overview

OpenMP provides several scheduling strategies to distribute loop iterations among threads, each with distinct advantages for different workload types:

| Strategy | Description | Best For |
|----------|-------------|----------|
| **Static** | Pre-divides iterations equally at compile time | Uniform workloads |
| **Dynamic** | Assigns iterations on-demand as threads complete work | Non-uniform workloads |
| **Guided** | Similar to dynamic but with decreasing chunk sizes | Mixed workloads |
| **Auto** | Runtime system selects the scheduling method | Unknown characteristics |

This demo visualizes and compares these strategies across several key performance metrics:
- Execution time and speedup vs. sequential code
- Load balancing across available threads
- Thread utilization patterns
- Iteration distribution visualization

## Prerequisites

- Windows 10/11
- Visual Studio 2022 with C++ development tools
- CMake 3.20 or higher
- OpenMP support (included with Visual Studio)

## Building and Running

### Step 1: Configure
Run `configure.bat` to generate the Visual Studio project files:
```
configure.bat
```

### Step 2: Build
Run `build_all.bat` to compile Debug, Release, and Profile configurations:
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
run.bat --schedule dynamic       # Use dynamic scheduling strategy
run.bat --verbose                # Run with verbose output
run.bat --help                   # Show all available options
```

For the most comprehensive experience, you can use:

```
run_all.bat                      # Run all demonstrations in sequence
```

This will execute all scheduling strategies with various thread counts and performance comparisons.

For specific scheduling strategy comparisons, we also provide:

```
run.bat --schedule static        # Use static scheduling
run.bat --schedule dynamic       # Use dynamic scheduling
run.bat --schedule guided        # Use guided scheduling
run.bat --schedule auto          # Use auto scheduling
```

**Note**: For accurate performance measurements, always use the Release build.

### Step 4: Clean
If you want to clean the build files and start from scratch:

```
clean.bat
```

This will remove all build artifacts and temporary files.

## Program Workflow

1. **Workload Generation**:
   - Creates a synthetic workload of computing prime numbers with varying computational intensity
   - Ensures non-uniform task distribution to highlight scheduling differences

2. **Execution and Measurement**:
   - Sequential execution (baseline)
   - Parallel execution with multiple scheduling strategies:
     - Static (default and with chunk sizes)
     - Dynamic (default and with chunk sizes)
     - Guided (default and with chunk sizes)
     - Runtime scheduling

3. **Performance Analysis**:
   - Measures execution time for each strategy
   - Calculates speedup relative to sequential execution
   - Analyzes load distribution among threads
   - Determines optimal scheduling for the workload

4. **Visualization**:
   - Displays formatted performance summary
   - Visualizes thread work distribution
   - Shows iteration assignment patterns for each strategy

## Code Components

- `src/main.cpp` - Main implementation with all scheduling strategies
- `CMakeLists.txt` - CMake configuration for building the project
- Batch files:
  - `configure.bat` - Sets up the CMake project
  - `build_all.bat` - Builds Debug, Release, and Profile configurations
  - `run.bat` - Unified script for running the program with various options
  - `run_all.bat` - Runs all scheduling strategies with various thread counts
  - `clean.bat` - Removes build artifacts

## Scheduling Strategy Visualization

The program provides a visual representation of how iterations are distributed:

```
Static Scheduling (chunk=default):
Thread 0: [##########............................]
Thread 1: [..........##########...................]
Thread 2: [....................##########.........]
Thread 3: [..............................##########]

Dynamic Scheduling (chunk=1):
Thread 0: [#....#..#....#..#....#..#....#..#....#..]
Thread 1: [.#....#..#....#..#....#..#....#..#....#.]
Thread 2: [..#....#..#....#..#....#..#....#..#.....]
Thread 3: [...#....#..#....#..#....#..#....#..#....]
```

These visualizations help understand how different scheduling strategies assign work to threads:
- **Static**: Large contiguous blocks of iterations
- **Dynamic (chunk=1)**: Interleaved pattern showing fine-grained distribution
- **Guided**: Large chunks initially, smaller chunks later
- **Runtime**: Depends on the environment variable OMP_SCHEDULE

## Common Issues and Troubleshooting

### Performance Variations
- System load can significantly impact results
- Hyperthreading effects may cause unexpected scaling behavior
- For consistent results:
  - Close other applications when benchmarking
  - Run multiple trials and average the results
  - Use `run_release.bat` with optimizations enabled

### Build Issues
- If CMake configuration fails:
  - Ensure Visual Studio 2022 is properly installed with C++ components
  - Verify CMake 3.20+ is installed and in your PATH
  - Run the batch files from a Visual Studio Developer Command Prompt

### Runtime Issues
- If the executable fails to launch:
  - Check that the build completed successfully
  - Verify the executable path in the run scripts matches your build output
  - Ensure OpenMP runtime libraries are available

## Advanced Usage

### Modifying Workload Parameters
You can adjust these parameters in `main.cpp`:
- `NUM_WORKLOAD_ITEMS` - Number of tasks/iterations to process
- `MAX_NUMBER` - Upper limit for prime calculation workload
- Chunk sizes for different scheduling strategies

### Experimenting with Custom Schedules
Add new scheduling configurations by:
1. Adding new `SchedulingResult` instances in the main function
2. Implementing additional `#pragma omp` schedules with custom parameters
3. Adding the results to the performance comparison table

### Advanced Analysis
For deeper understanding of scheduling behavior:
1. Modify the code to export timing data to CSV
2. Add variance calculations across multiple runs
3. Implement more complex visualizations of thread behavior

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Additional Resources

- [OpenMP Documentation](https://www.openmp.org/resources/)
- [OpenMP 5.1 Specification](https://www.openmp.org/spec-html/5.1/openmp.html)
- See the accompanying `SCHEDULING.md` for detailed explanations of each scheduling strategy 