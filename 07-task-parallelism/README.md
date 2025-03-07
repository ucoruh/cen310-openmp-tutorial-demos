# OpenMP Task Parallelism

This project demonstrates task-based parallelism using OpenMP in C++. It is designed for Visual Studio 2022 on Windows 11 and uses CMake for build configuration.

## Project Overview

OpenMP tasks provide a flexible way to express parallelism, particularly for irregular parallel patterns that are difficult to express with parallel loops. This project includes several examples demonstrating various aspects of OpenMP tasks:

- Basic task creation and execution
- Recursive algorithms (Fibonacci, Quicksort)
- Tree and graph traversal with tasks
- Task dependencies and synchronization
- Task priorities and scheduling
- Taskloop and taskgroup constructs
- Performance analysis and visualization

## Prerequisites

- Windows 11 (should also work on Windows 10)
- Visual Studio 2022 Community Edition or higher
- CMake 3.20 or higher
- C++17 compatible compiler

## Project Structure

```
OpenMP_TaskParallelism/
├── src/                    # Main implementation files
│   └── main.cpp            # Command-line interface
├── include/                # Header files
├── examples/               # Task parallelism examples
│   ├── basic_tasks.cpp     # Introduction to OpenMP tasks
│   ├── fibonacci.cpp       # Recursive Fibonacci using tasks
│   ├── quicksort.cpp       # Parallel quicksort with tasks
│   ├── tree_traversal.cpp  # Binary tree traversal with tasks
│   ├── graph_processing.cpp # Graph algorithms with tasks
│   ├── task_dependencies.cpp # Using depends(in,out) clauses
│   ├── task_priority.cpp   # Using priority clause for tasks
│   ├── taskloop.cpp        # Demonstrating taskloop construct
│   ├── taskgroup.cpp       # Using taskgroup for synchronization
│   ├── task_visualizer.cpp # Visualize task execution
│   ├── task_stealing.cpp   # Analyze task stealing behavior
│   ├── nested_tasks.cpp    # Demonstrate nested task creation
│   ├── heterogeneous_tasks.cpp # Tasks with different computational needs
│   └── task_throttling.cpp # Control task creation rate
├── utils/                  # Helper functions
├── benchmarks/             # Performance testing
│   └── benchmark_suite.cpp # Compare implementations
├── scripts/                # Batch files
│   ├── check_vs2022.bat    # Verify Visual Studio installation
│   ├── setup_environment.bat # Set environment variables
│   ├── configure.bat       # Run CMake configuration
│   ├── build_all.bat       # Build all examples
│   ├── run_examples.bat    # Execute examples
│   ├── run_benchmarks.bat  # Run performance tests
│   └── generate_reports.bat # Create performance reports
└── docs/                   # Documentation
    ├── TASK_PARALLELISM_GUIDE.md  # Guide to OpenMP tasks
    └── PERFORMANCE_TUNING.md      # Performance optimization guide
```

## Building and Running

### Step 1: Check Prerequisites

Run the `check_vs2022.bat` script to verify that Visual Studio 2022 is properly installed with the required components:

```
scripts\check_vs2022.bat
```

### Step 2: Set Up Environment

Run the `setup_environment.bat` script to set up the necessary environment variables:

```
scripts\setup_environment.bat
```

### Step 3: Configure the Project

Run the `configure.bat` script to generate the Visual Studio project files:

```
configure.bat
```

This will:
- Create a `build` directory (if it doesn't exist)
- Run CMake with the Visual Studio 2022 generator

### Step 4: Build the Project

Run the `build_all.bat` script to compile the project in both Debug and Release configurations:

```
build_all.bat
```

This will build all examples and the main program.

### Step 5: Run the Examples

Run the `run_examples.bat` script to execute the examples:

```
run_examples.bat
```

This will display a menu allowing you to select which example to run and whether to use the Debug or Release configuration.

### Step 6: Run Benchmarks (Optional)

For performance testing, run the `run_benchmarks.bat` script:

```
run_benchmarks.bat
```

This will run various benchmarks comparing sequential, simple parallel, and task-based implementations.

### Step 7: Generate Reports (Optional)

To generate performance comparison reports, run the `generate_reports.bat` script:

```
generate_reports.bat
```

## Examples Explained

### Basic Tasks

Demonstrates the fundamental concepts of OpenMP tasks, including task creation, execution, and scheduling.

### Fibonacci

Implements the recursive Fibonacci algorithm using tasks, showcasing how to parallelize recursive algorithms effectively.

### Quicksort

Implements parallel quicksort using tasks, demonstrating recursive divide-and-conquer algorithms with task parallelism.

### Tree Traversal

Shows how to traverse binary trees in parallel using tasks, a pattern that is difficult to express with traditional parallel loops.

### Graph Processing

Demonstrates parallel graph algorithms using tasks, including breadth-first search and connected component analysis.

### Task Dependencies

Showcases the use of the `depends` clause to express dependencies between tasks, enabling more complex parallel patterns.

### Task Priority

Demonstrates the use of the `priority` clause to control the order of task execution, useful for critical path optimization.

### Taskloop

Shows how to use the taskloop construct for loop-based parallelism with the benefits of tasks.

### Taskgroup

Demonstrates the use of taskgroup for synchronization and waiting for a group of tasks to complete.

## Performance Tuning

For optimal performance with OpenMP tasks, consider the following:

1. **Task Granularity**: Choose an appropriate task size to balance parallelism and overhead.
2. **Cutoff Strategies**: Switch to sequential execution for small problem sizes.
3. **Task Dependencies**: Use the depends clause to express dependencies between tasks.
4. **Thread Affinity**: Set thread affinity for better locality and performance.
5. **Task Priorities**: Use the priority clause for critical tasks.
6. **Task Throttling**: Control the rate of task creation to avoid overwhelming the runtime.

For more detailed performance tuning guidance, see the PERFORMANCE_TUNING.md document.

## Common Issues and Troubleshooting

### OpenMP Not Found

If you see "OpenMP is not supported!" when running the program:

- Make sure your compiler supports OpenMP
- Check that Visual Studio has the C++ desktop development workload installed

### Build Errors

- Ensure Visual Studio 2022 is properly installed
- Make sure you have the correct Visual Studio C++ components installed
- Try running the scripts as Administrator if you encounter permission issues

### Performance Issues

- Check that you're using the Release configuration for performance measurements
- Experiment with different task granularity settings
- Ensure you're using an appropriate cutoff value for recursive algorithms
- Consider the number of threads relative to your CPU's core count

## Comprehensive Usage Guide

### Command Line Interface Usage

The `OpenMP_TaskParallelism.exe` program provides the following command-line options for running various examples:

```
OpenMP_TaskParallelism.exe [options]
Options:
  --threads, -t N          Set number of threads (default: max available)
  --granularity, -g N      Set task granularity (default: 100)
  --example, -e NAME       Run specific example. Options:
                           basic, fibonacci, quicksort, tree, graph,
                           dependencies, priority, taskloop, taskgroup,
                           throttling, visualizer, heterogeneous, stealing, nested
  --all, -a                Run all examples
  --generate-reports       Generate performance reports
  --help, -h               Show this help message
```

### Example Usage Patterns

#### Running All Examples

```
.\Debug\OpenMP_TaskParallelism.exe --all
```

This command runs all examples sequentially. Be patient for examples that work with large datasets. The program is designed to retry with smaller parameters if certain examples fail with their default settings.

#### Running a Specific Example

```
.\Debug\OpenMP_TaskParallelism.exe --example fibonacci
```

#### Running with Custom Thread Count

```
.\Debug\OpenMP_TaskParallelism.exe --example taskloop --threads 8
```

#### Running with Custom Granularity

```
.\Debug\OpenMP_TaskParallelism.exe --example basic --granularity 50
```

### Running Individual Examples Directly

Each example can also be run directly using its executable. Here's how to use each one:

#### Basic Tasks Example
```
.\Debug\basic_tasks.exe [work_size] [num_threads]
```
- `work_size`: Amount of work per task (default: 100)
- `num_threads`: Number of threads to use (default: max available)

#### Fibonacci Example
```
.\Debug\fibonacci.exe [n] [cutoff] [num_threads]
```
- `n`: Fibonacci number to compute (default: 40)
- `cutoff`: Cutoff for sequential execution (default: 10)
- `num_threads`: Number of threads to use (default: max available)

#### Quicksort Example
```
.\Debug\quicksort.exe [array_size] [cutoff] [num_threads]
```
- `array_size`: Size of array to sort (default: 10,000,000)
- `cutoff`: Cutoff for sequential execution (default: 1,000)
- `num_threads`: Number of threads to use (default: max available)

Note: For large array sizes (>1,000,000), you may encounter memory issues. Consider starting with smaller values (e.g., 100,000).

#### Tree Traversal Example
```
.\Debug\tree_traversal.exe [depth] [num_threads] [cutoff]
```
- `depth`: Depth of the binary tree (default: 16)
- `num_threads`: Number of threads to use (default: max available)
- `cutoff`: Cutoff depth for parallel execution (default: 8)

Note: For stable execution, use a depth ≤ 10 with a cutoff of around depth/2.

#### Graph Processing Example
```
.\Debug\graph_processing.exe [vertices] [edges] [num_threads]
```
- `vertices`: Number of vertices in the graph (default: 1,000)
- `edges`: Number of edges in the graph (default: 5,000)
- `num_threads`: Number of threads to use (default: max available)

Note: Start with smaller graphs (e.g., 100-500 vertices) to avoid memory issues.

#### Task Dependencies Example
```
.\Debug\task_dependencies.exe [num_tasks] [num_threads]
```
- `num_tasks`: Number of tasks to create (default: 100)
- `num_threads`: Number of threads to use (default: max available)

#### Task Priority Example
```
.\Debug\task_priority.exe [num_tasks] [num_threads]
```
- `num_tasks`: Number of tasks to create (default: 100)
- `num_threads`: Number of threads to use (default: max available)

#### Taskloop Example
```
.\Debug\taskloop.exe [array_size] [grainsize] [num_threads]
```
- `array_size`: Size of array to process (default: 10,000,000)
- `grainsize`: Number of elements per task (default: 100)
- `num_threads`: Number of threads to use (default: max available)

#### Taskgroup Example
```
.\Debug\taskgroup.exe [num_tasks] [num_threads]
```
- `num_tasks`: Number of tasks to create (default: 100)
- `num_threads`: Number of threads to use (default: max available)

#### Task Throttling Example
```
.\Debug\task_throttling.exe
```
This example demonstrates different approaches to throttling task creation to avoid overloading the system.

#### Task Visualizer Example
```
=== OpenMP Task Visualization Example ===
Number of threads: 100
Running simple tasks example...
Running simulated dependency tasks example...
Note: Using manual synchronization for dependencies (OpenMP 2.0 compatible)
Task Execution Timeline:
--------------------------
Time (ms): 0        0        0        0        0        0        0        0        0
          |....:....|....:....|....:....|....:....|....:....|....:....|....:....|....:....
Thread 12: [=====================]
                     0
Thread 31:                       [==============================]
                                               2
Thread 37:                           [======================================]
                                                       1
Thread 66:                                                          [=====================]
                                                                              3
Thread 87:                       [==========================]
                                             1
Thread 88: [=========================[==============================]       [=============]
                       0                           2                              4       

Task Mapping:
--------------------------
Task 400: Parent_1
Task 401: Child_1_1
Task 402: Child_1_2
Task 403: Grandchild_1_2_1
Task 404: Parent_1_Continue
Task 500: Parent_2
Task 501: Child_2_1
Task 502: Child_2_2

Potential Task Dependencies:
--------------------------
Task 500 (Parent_2) on Thread 12
Task 400 (Parent_1) on Thread 88
Task 502 (Child_2_2) on Thread 31 may depend on: 500
Task 501 (Child_2_1) on Thread 87 may depend on: 500
Task 402 (Child_1_2) on Thread 88 may depend on: 500, 400
Task 401 (Child_1_1) on Thread 37 may depend on: 500
Task 403 (Grandchild_1_2_1) on Thread 66 may depend on: 500, 502, 501, 402
Task 404 (Parent_1_Continue) on Thread 88 may depend on: 500, 502, 401, 501, 402
```

The task visualizer example visualizes how OpenMP tasks are distributed and executed. This example facilitates understanding the behavior of task-based parallelization by providing information such as task timeline, task mapping, and potential task dependencies. It also helps analyze how efficiently resources are used by showing thread utilization rates.

### Heterogeneous Tasks Example

```
.\heterogeneous_tasks.exe [cpu_tasks] [io_tasks] [work_size] [num_threads]
```
- `cpu_tasks`: Number of CPU-intensive tasks (default: 20)
- `io_tasks`: Number of I/O-intensive tasks (default: 20)
- `work_size`: Base work amount per task (default: 100)
- `num_threads`: Number of threads to use (default: max available)

### Task Stealing Example

```
.\task_stealing.exe [num_tasks] [num_threads]
```
- `num_tasks`: Number of tasks to create (default: 100)
- `num_threads`: Number of threads to use (default: max available)

### Nested Tasks Example

```
.\nested_tasks.exe [depth] [num_threads]
```
- `depth`: Maximum nesting depth (default: 4)
- `num_threads`: Number of threads to use (default: max available)

## Tips for Optimal Usage

### Performance Optimization

For best performance:

1. **Cutoff Point Optimization**: As shown in the Fibonacci example, an appropriate cutoff point selection can significantly improve performance. Generally, mid-level values (e.g., between 15-25) give the best results rather than very low or very high values.

2. **Thread Count**: Using the maximum number of threads may not always provide the best performance. Determine an appropriate value for your system's physical/logical core count.

3. **Task Size**: Creating too small tasks can reduce performance due to task management overhead. The appropriate task size varies depending on the problem and hardware characteristics.

### Troubleshooting Common Issues

1. **Program Crashes or Hangs**
   - Try reducing the problem size (array size, tree depth, graph size)
   - Reduce thread count to limit resource contention
   - Check system memory availability

2. **Poor Performance**
   - Ensure you're using the Release build for performance measurements
   - Experiment with different task granularity settings
   - Try different cutoff values for recursive algorithms
   - Check if other processes are consuming system resources

3. **Build Issues**
   - Make sure Visual Studio 2022 is properly installed
   - Run CMake configuration again (`scripts\configure.bat`)
   - Rebuild the project from scratch (`scripts\build_all.bat`)

4. **Incorrect Results**
   - All examples include result validation
   - If results don't match, it could indicate a synchronization issue
   - Try with a smaller problem size to see if the issue persists

### Extending the Examples

You can use these examples as a foundation for your own task-parallel applications:

1. Study the code structure in the examples directory
2. Use the utility functions in the include directory
3. Create a new .cpp file based on an existing example
4. Add it to the CMakeLists.txt file using the `add_example()` function
5. Rebuild the project to include your new example

For advanced customization, consider exploring:
- Custom task scheduling strategies
- Advanced synchronization mechanisms
- Task priority optimization
- Performance monitoring and profiling

## License

This project is provided as-is for educational purposes.

## Using OpenMP in Windows and Visual Studio

### Visual Studio Configuration

Visual Studio includes OpenMP support by default, but some settings need to be correctly configured in the project:

1. **Enabling OpenMP Support**: 
   - Right-click on the project and select "Properties"
   - Set "C/C++" → "Language" → "Open MP Support" to "Yes (/openmp)"

2. **Compiler Optimizations**:
   - In Release configuration, set "C/C++" → "Optimization" → "Optimization" to "Maximum Optimization (Favor Speed) (/O2)"
   - Set "C/C++" → "Code Generation" → "Enable Enhanced Instruction Set" appropriately for modern processors (e.g., "Advanced Vector Extensions 2 (/arch:AVX2)")

3. **Multithreading Support**:
   - Make sure that "C/C++" → "Code Generation" → "Runtime Library" uses a multithreaded option (e.g., "Multi-threaded DLL (/MD)" or "Multi-threaded Debug DLL (/MDd)")

### Using OpenMP with CMake

When using OpenMP with CMake, make sure the following lines are in your `CMakeLists.txt` file:

```cmake
find_package(OpenMP REQUIRED)
if(OpenMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()
```

### Analyzing OpenMP Tasks with Debugger

Visual Studio debugger offers some features for analyzing OpenMP tasks:

1. **Parallel Stacks Window**: Opened via "Debug" → "Windows" → "Parallel Stacks" and visualizes threads and tasks executing in parallel.

2. **Parallel Watch Window**: Opened via "Debug" → "Windows" → "Parallel Watch" and allows you to monitor values of specific variables for each thread.

3. **Breakpoint Filtering**: Right-click on breakpoints and select "Filter..." to define breakpoints for specific threads.

### Possible Error Messages and Solutions

1. **"OpenMP is not supported"**:
   - Make sure compiler flags are set correctly
   - Check that the required Visual Studio components are installed

2. **"C1xx : fatal error C1xx: OpenMP header 'omp.h' cannot be found"**:
   - Ensure the include folder path is set correctly
   - Repair your Visual Studio installation using the "Repair" option

3. **"LNKxxxx: Unresolved external symbol referenced by 'function'"**:
   - Check that the OpenMP library is correctly linked to the project

4. **Memory Management Errors**:
   - Ensure that data shared between OpenMP tasks is properly synchronized
   - Particularly check that private, firstprivate, shared, and default keywords are used correctly

### Performance Analysis Tools

Windows and Visual Studio offer various tools for analyzing the performance of OpenMP applications:

1. **Visual Studio Profiler**: Used via "Analyze" → "Performance Profiler" and measures CPU usage, thread activity, and similar metrics.

2. **Intel VTune Profiler**: A more advanced tool for analyzing OpenMP tasks. Provides more detailed information for performance analysis of task-based parallelism.

3. **Windows Performance Toolkit**: You can make system-level measurements using Windows' own performance analysis tools.

## Example Outputs and Results

Below are outputs obtained from running various examples. These examples demonstrate the performance gains provided by OpenMP task-based parallelism.

### Fibonacci Example

```
=== Fibonacci Task Parallelism Example ===
Computing Fibonacci(35)
Number of threads: 32
Cutoff for sequential execution: 10

Sequential Result: 9227465
Sequential Time: 0.0412 seconds

Parallel Result: 9227465
Parallel Time: 0.0168 seconds

Results match! ✓

Analyzing cutoff impact on Fibonacci(35):
--------------------------------------------------
Cutoff | Result | Time (s) | Speedup
--------------------------------------------------
    10 |  9227465 |   0.0139 |   2.94x
    15 |  9227465 |   0.0038 |  10.81x
    20 |  9227465 |   0.0032 |  12.75x
    25 |  9227465 |   0.0032 |  12.72x
    30 |  9227465 |   0.0058 |   7.00x
```

The Fibonacci example demonstrates how recursive algorithms can be accelerated using task parallelization. The impact of different cutoff values on performance is also analyzed. As shown in this example, choosing an appropriate cutoff point is important for optimal performance.

### Basic Tasks Example

```
=== Basic OpenMP Tasks Example ===
Number of threads: 32
Number of elements: 32
Work amount per element: 100

Performance Results:
---------------------------------
Sequential time: 0.0417 seconds
Task-based time: 0.0066 seconds
Parallel for time: 0.0031 seconds
---------------------------------
Task-based speedup: 6.29x
Parallel for speedup: 13.27x
---------------------------------
Task 2 (work=300) executed by thread 13 in 0.0047 seconds
Task 1 (work=200) executed by thread 15 in 0.0053 seconds
Task 4 (work=500) executed by thread 20 in 0.0076 seconds
Task 3 (work=400) executed by thread 14 in 0.0105 seconds
Task 6 (work=700) executed by thread 1 in 0.0110 seconds
Task 7 (work=800) executed by thread 27 in 0.0122 seconds
Task 9 (work=1000) executed by thread 19 in 0.0155 seconds
Task 5 (work=600) executed by thread 12 in 0.0158 seconds
Task 8 (work=900) executed by thread 18 in 0.0238 seconds
```

The basic tasks example demonstrates OpenMP's task creation and execution mechanisms. This example also includes a performance comparison of task-based and parallel loop-based approaches. Additionally, it provides details such as which threads executed which tasks and their completion times.

### Taskloop Example

```
=== OpenMP Task Parallelism Example ===
Note: This is a simplified version using basic tasks instead of taskloop (OpenMP 4.5+)
Data size: 32
Default grainsize: 1000000
Default num_tasks: 100
Number of threads: 32

Comparing different implementation approaches:
-------------------------------------------------------------
Implementation | Time (s) | Speedup | Efficiency
--------------------------------------------------
         1 |   0.0004 |   1.44
        10 |   0.0006 |   1.07
       100 |   0.0010 |   0.60
       500 |   0.0010 |   0.60

Testing impact of num_tasks on task performance:
--------------------------------------------------
Num Tasks | Time (s) | Speedup
--------------------------------------------------
        1 |   0.0011 |   0.60
        2 |   0.0005 |   1.43
        4 |   0.0005 |   1.38
        8 |   0.0003 |   1.97
       16 |   0.0003 |   2.47
       32 |   0.0003 |   1.94
       64 |   0.0005 |   1.36
      128 |   0.0003 |   1.93
```

The taskloop example is an implementation that mimics the taskloop structure introduced in OpenMP 4.5+. This example analyzes the impact of the number of tasks and implementation on performance. It presents time and speedup values for different numbers of tasks and implementation approaches.

### Memory Issues

Some examples may cause memory issues when working with large datasets:

1. **Quicksort**: Memory issues may occur for more than 10 million elements
2. **Tree Traversal**: Very deep trees can cause stack overflow
3. **Graph Processing**: Very large graphs can reach memory limitations

If you encounter these issues, try reducing the example parameters. For example:

```
.\quicksort.exe 1000000 1000 32   # 1 million instead of 10 million
.\tree_traversal.exe 10 32 4      # smaller tree depth
.\graph_processing.exe 500 1000 32 # smaller graph
```

### Task Tracking and Visualization Issues

The task tracking and visualization functionality can sometimes lead to memory access violations or abort calls, especially when running with many threads or tasks. This is often due to race conditions or thread synchronization issues in the TaskTracker class. If you encounter an "abort() has been called" error with the task_priority.exe program, consider:

1. Reducing the number of tasks and threads
2. Using a simpler version without visualization
3. Adding taskwait directives to ensure proper synchronization
4. Adding try-catch blocks to handle exceptions

For example, replace the TaskTracker-based visualization with simple timing and console output:

```cpp
// Instead of using task_tracker for visualization
auto start_time = std::chrono::high_resolution_clock::now();
// ...run your tasks...
#pragma omp taskwait  // Ensure all tasks complete
auto end_time = std::chrono::high_resolution_clock::now();
auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
std::cout << "Tasks completed in " << duration << " ms." << std::endl;
```