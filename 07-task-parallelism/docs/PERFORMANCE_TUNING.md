# OpenMP Task Parallelism Performance Tuning Guide

This guide provides strategies and best practices for optimizing the performance of OpenMP task-based parallel programs. It focuses on practical approaches to maximize efficiency and scalability of task-parallel applications.

## Table of Contents

1. [Understanding Task Performance Factors](#understanding-task-performance-factors)
2. [Task Granularity Optimization](#task-granularity-optimization)
3. [Cut-off Strategies for Recursive Tasks](#cut-off-strategies-for-recursive-tasks)
4. [Memory Locality Considerations](#memory-locality-considerations)
5. [Thread Scaling Analysis](#thread-scaling-analysis)
6. [Task Scheduling Optimization](#task-scheduling-optimization)
7. [Performance Measurement Techniques](#performance-measurement-techniques)
8. [Advanced Optimization Strategies](#advanced-optimization-strategies)

## Understanding Task Performance Factors

Task-based parallelism performance is influenced by several key factors:

### Task Creation and Scheduling Overhead

Creating and scheduling tasks incurs overhead. Each task creation requires:
- Allocating task data structures
- Capturing variables (especially for firstprivate variables)
- Scheduling decisions by the runtime

This overhead is typically in the microsecond range, which means very small tasks may not provide a performance benefit.

### Load Balancing

Effective load balancing is essential for optimal performance:
- Tasks should have similar computational requirements when possible
- The runtime's work-stealing mechanism helps balance irregular workloads
- Too few tasks can lead to idle threads
- Too many tasks can create excessive overhead

### Synchronization Points

Synchronization points like `taskwait` and `taskgroup` can become bottlenecks:
- Each synchronization point may cause threads to wait
- Excessive synchronization reduces parallelism
- Using task dependencies can often reduce the need for explicit synchronization

### Resource Contention

Tasks executing in parallel compete for system resources:
- Memory bandwidth
- Cache space
- CPU resources

Proper task design considers resource requirements to minimize contention.

## Task Granularity Optimization

Task granularity—the amount of work per task—is a critical performance factor.

### Finding the Optimal Granularity

The optimal task granularity balances two competing factors:
- **Overhead minimization**: Larger tasks amortize creation/scheduling overhead
- **Parallelism maximization**: Smaller tasks provide more opportunities for parallelism

General guidelines:
- Tasks should perform work equivalent to at least thousands of CPU cycles
- Aim for task execution time in the millisecond range for most applications
- Create more tasks than available threads (typically 2-4x) to enable load balancing

### Measuring Granularity Impact

To optimize granularity:

```cpp
// Test different granularity levels
void test_granularity() {
    const size_t data_size = 10000000;
    std::vector<double> data(data_size, 1.0);
    
    std::cout << "Testing different granularity levels:" << std::endl;
    std::cout << "Granularity | Time (ms) | Speedup" << std::endl;
    std::cout << "------------------------------------" << std::endl;
    
    // Baseline: process everything in one task
    double baseline_time = measure_time([&]() {
        process_data(data, 0, data_size);
    });
    
    // Test different granularity levels
    for (size_t grain = 100; grain <= 1000000; grain *= 10) {
        double time = measure_time([&]() {
            #pragma omp parallel
            {
                #pragma omp single
                {
                    for (size_t i = 0; i < data_size; i += grain) {
                        size_t end = std::min(i + grain, data_size);
                        #pragma omp task firstprivate(i, end)
                        {
                            process_data(data, i, end);
                        }
                    }
                }
            }
        });
        
        double speedup = baseline_time / time;
        std::cout << std::setw(10) << grain << " | " 
                  << std::setw(8) << time << " | "
                  << std::setw(6) << speedup << std::endl;
    }
}
```

### Adaptive Granularity

For irregular workloads, adaptive granularity can be more effective:

```cpp
void process_adaptive(std::vector<WorkItem>& items) {
    #pragma omp parallel
    {
        #pragma omp single
        {
            process_chunk(items, 0, items.size(), 0);
        }
    }
}

void process_chunk(std::vector<WorkItem>& items, int start, int end, int depth) {
    // Base case 1: Small enough chunk to process directly
    if (end - start <= MIN_GRANULARITY) {
        for (int i = start; i < end; i++) {
            process_item(items[i]);
        }
        return;
    }
    
    // Base case 2: Maximum recursion depth reached
    if (depth >= MAX_DEPTH) {
        for (int i = start; i < end; i++) {
            process_item(items[i]);
        }
        return;
    }
    
    // Estimate work in this chunk
    int work_estimate = estimate_work(items, start, end);
    
    // If work is below threshold, process directly
    if (work_estimate < WORK_THRESHOLD) {
        for (int i = start; i < end; i++) {
            process_item(items[i]);
        }
        return;
    }
    
    // Otherwise, split and create tasks
    int mid = start + (end - start) / 2;
    
    #pragma omp task
    process_chunk(items, start, mid, depth + 1);
    
    #pragma omp task
    process_chunk(items, mid, end, depth + 1);
}
```

## Cut-off Strategies for Recursive Tasks

Recursive algorithms using tasks need effective cut-off strategies to limit task creation overhead.

### Static Cut-off Based on Problem Size

The simplest approach uses a fixed size threshold:

```cpp
void quicksort(int* arr, int low, int high) {
    if (low < high) {
        int pivot = partition(arr, low, high);
        
        // Use problem size as the cut-off criterion
        if (high - low > CUTOFF) {
            #pragma omp task
            quicksort(arr, low, pivot - 1);
            
            #pragma omp task
            quicksort(arr, pivot + 1, high);
            
            #pragma omp taskwait
        } else {
            // Sequential execution for small problem sizes
            quicksort_seq(arr, low, pivot - 1);
            quicksort_seq(arr, pivot + 1, high);
        }
    }
}
```

### Recursion Depth-Based Cut-off

Limiting the recursion depth for task creation:

```cpp
void process_tree(Node* node, int depth) {
    if (!node) return;
    
    // Process current node
    process_node(node);
    
    // Use depth as the cut-off criterion
    if (depth < MAX_TASK_DEPTH) {
        if (node->left) {
            #pragma omp task
            process_tree(node->left, depth + 1);
        }
        
        if (node->right) {
            #pragma omp task
            process_tree(node->right, depth + 1);
        }
        
        #pragma omp taskwait
    } else {
        // Sequential processing for deeper levels
        if (node->left) process_tree(node->left, depth + 1);
        if (node->right) process_tree(node->right, depth + 1);
    }
}
```

### Dynamic Cut-off Based on Available Resources

Adapt cut-off based on system load:

```cpp
int fibonacci(int n, bool use_tasks = true) {
    if (n < 2) return n;
    
    // Base case: small values or too many active tasks
    if (n <= 20 || !use_tasks || omp_in_final()) {
        return fibonacci(n-1, false) + fibonacci(n-2, false);
    }
    
    int x, y;
    
    #pragma omp task shared(x) final(n <= 30)
    x = fibonacci(n-1, use_tasks);
    
    #pragma omp task shared(y) final(n <= 30)
    y = fibonacci(n-2, use_tasks);
    
    #pragma omp taskwait
    return x + y;
}
```

### Determining Optimal Cut-off Values

To find optimal cut-offs for your specific hardware and problem:

1. Start with a reasonable default (e.g., 1000 elements for sorting, recursion depth of 3-4)
2. Benchmark with different cut-off values
3. Look for the "sweet spot" where performance peaks
4. Consider creating a parameterized cut-off that can be tuned at runtime

```cpp
// Example of benchmarking different cut-off values
void benchmark_cutoffs() {
    const int ARRAY_SIZE = 10000000;
    std::vector<int> original_data = generate_random_vector(ARRAY_SIZE);
    
    std::cout << "Testing different cut-off values:" << std::endl;
    std::cout << "Cut-off | Time (ms) | Speedup" << std::endl;
    std::cout << "----------------------------------" << std::endl;
    
    // Baseline: sequential quicksort
    auto sequential_data = original_data;
    double baseline_time = measure_time([&]() {
        quicksort_seq(sequential_data.data(), 0, sequential_data.size() - 1);
    });
    
    // Test different cut-off values
    for (int cutoff = 10; cutoff <= 100000; cutoff *= 5) {
        auto parallel_data = original_data;
        
        double par_time = measure_time([&]() {
            #pragma omp parallel
            {
                #pragma omp single
                {
                    quicksort_par(parallel_data.data(), 0, parallel_data.size() - 1, cutoff);
                }
            }
        });
        
        double speedup = baseline_time / par_time;
        std::cout << std::setw(7) << cutoff << " | " 
                  << std::setw(8) << par_time << " | "
                  << std::setw(6) << speedup << std::endl;
    }
}
```

## Memory Locality Considerations

Memory access patterns significantly impact task performance, especially for memory-bound applications.

### Data Layout for Task Locality

Organize data to maximize cache utilization:

- Use contiguous memory layouts (arrays instead of linked structures)
- Group data that will be accessed by the same task
- Consider cache line size (typically 64 bytes) when designing data structures

```cpp
// Example: Structure of Arrays vs. Array of Structures
// Better for task parallelism (each task works on a contiguous block)
struct SoA {
    std::vector<float> x;
    std::vector<float> y;
    std::vector<float> z;
};

// May be less efficient for task parallelism due to strided access
struct AoS {
    struct Point {
        float x, y, z;
    };
    std::vector<Point> points;
};
```

### Task Data Affinity

When possible, keep tasks working on data that's likely to be in cache:

```cpp
#pragma omp parallel
{
    #pragma omp single
    {
        for (int block = 0; block < num_blocks; block++) {
            #pragma omp task firstprivate(block)
            {
                // Each task processes an entire block of contiguous data
                int start = block * block_size;
                int end = start + block_size;
                
                for (int i = start; i < end; i++) {
                    process(data[i]);
                }
            }
        }
    }
}
```

### NUMA Awareness

For multi-socket systems, consider Non-Uniform Memory Access (NUMA) effects:

- Initialize data in parallel to distribute it across NUMA nodes (first-touch policy)
- Keep threads close to their data with thread affinity
- Consider explicit NUMA-aware allocation for large data structures

```cpp
// Initialize data with first-touch policy to distribute across NUMA nodes
#pragma omp parallel for
for (int i = 0; i < data_size; i++) {
    data[i] = initial_value;
}

// Then process data with tasks, using proc_bind to keep threads close to their data
#pragma omp parallel proc_bind(close)
{
    #pragma omp single
    {
        // Create tasks to process the data
        // ...
    }
}
```

### Cache Optimizations

Consider the cache hierarchy when designing tasks:

- L1 cache: 32-64 KB per core, very fast access
- L2 cache: 256-512 KB per core, fast access
- L3 cache: Several MB shared among cores, slower access

Ideally, task working sets should fit in cache when possible:

```cpp
// Example: Blocking for cache efficiency
void matrix_multiply_blocked(double* A, double* B, double* C, int N) {
    // Choose block size to fit in L1/L2 cache
    const int BLOCK_SIZE = 64;
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            for (int i = 0; i < N; i += BLOCK_SIZE) {
                for (int j = 0; j < N; j += BLOCK_SIZE) {
                    for (int k = 0; k < N; k += BLOCK_SIZE) {
                        #pragma omp task firstprivate(i, j, k)
                        {
                            // Process a block that fits in cache
                            for (int ii = i; ii < i + BLOCK_SIZE && ii < N; ii++) {
                                for (int jj = j; jj < j + BLOCK_SIZE && jj < N; jj++) {
                                    for (int kk = k; kk < k + BLOCK_SIZE && kk < N; kk++) {
                                        C[ii*N + jj] += A[ii*N + kk] * B[kk*N + jj];
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
```

## Thread Scaling Analysis

Understanding how your application scales with thread count is crucial for performance optimization.

### Performance Scaling Metrics

Key metrics to measure:

- **Speedup**: S(p) = T(1) / T(p), where T(p) is the execution time with p threads
- **Efficiency**: E(p) = S(p) / p, represents how effectively additional threads are used
- **Scalability limit**: The point where adding more threads provides diminishing returns

### Measuring Thread Scaling

To analyze scaling performance:

```cpp
void analyze_thread_scaling(int max_threads) {
    // Problem size should be large enough to show parallel benefits
    const size_t problem_size = 100000000;
    std::vector<double> data(problem_size, 1.0);
    
    std::cout << "Thread Scaling Analysis:" << std::endl;
    std::cout << "Threads | Time (ms) | Speedup | Efficiency" << std::endl;
    std::cout << "--------------------------------------------" << std::endl;
    
    // Measure sequential performance (baseline)
    omp_set_num_threads(1);
    double baseline_time = measure_time([&]() {
        process_parallel(data);
    });
    
    // Measure performance with increasing thread counts
    for (int threads = 1; threads <= max_threads; threads *= 2) {
        omp_set_num_threads(threads);
        
        double time = measure_time([&]() {
            process_parallel(data);
        });
        
        double speedup = baseline_time / time;
        double efficiency = speedup / threads * 100;
        
        std::cout << std::setw(7) << threads << " | " 
                  << std::setw(9) << time << " | "
                  << std::setw(7) << std::fixed << std::setprecision(2) << speedup << " | "
                  << std::setw(6) << std::fixed << std::setprecision(1) << efficiency << "%" 
                  << std::endl;
    }
}
```

### Identifying Scaling Bottlenecks

Common reasons for poor scaling:

1. **Insufficient parallelism**: Not enough tasks to keep all threads busy
   - Solution: Create more tasks or redesign the algorithm

2. **Load imbalance**: Uneven work distribution
   - Solution: Use smaller tasks, improve work distribution

3. **Synchronization overhead**: Too many taskwait/barrier points
   - Solution: Reduce synchronization, use task dependencies instead

4. **Memory bottlenecks**: Saturating memory bandwidth
   - Solution: Improve data locality, reduce memory traffic

5. **False sharing**: Multiple threads accessing the same cache line
   - Solution: Pad data structures, reorganize data access patterns

```cpp
// Example of identifying synchronization overhead:
double measure_synchronization_impact() {
    const int NUM_TASKS = 10000;
    const int WORK_SIZE = 1000;
    
    // Version with excessive synchronization
    double time_with_sync = measure_time([]() {
        #pragma omp parallel
        {
            #pragma omp single
            {
                for (int i = 0; i < NUM_TASKS; i++) {
                    #pragma omp task
                    {
                        do_work(WORK_SIZE);
                    }
                    #pragma omp taskwait  // Excessive synchronization
                }
            }
        }
    });
    
    // Version with optimal synchronization
    double time_optimal = measure_time([]() {
        #pragma omp parallel
        {
            #pragma omp single
            {
                for (int i = 0; i < NUM_TASKS; i++) {
                    #pragma omp task
                    {
                        do_work(WORK_SIZE);
                    }
                }
                // Single taskwait at the end
                #pragma omp taskwait
            }
        }
    });
    
    return time_with_sync / time_optimal;  // Synchronization overhead factor
}
```

## Task Scheduling Optimization

Understanding and optimizing task scheduling can significantly improve performance.

### Task Scheduling Policies

Modern OpenMP implementations use work-stealing task schedulers:
- Each thread maintains a local task queue
- When a thread's queue is empty, it "steals" tasks from other threads
- Task creation follows either a work-first or breadth-first policy

To optimize for efficient scheduling:

1. Create enough tasks to keep all threads busy
2. Balance task granularity to amortize scheduling overhead
3. Consider task priorities for critical path optimization
4. Be aware of task execution order impacts on data locality

### Thread Affinity Control

Control thread placement to optimize for memory access patterns:

```cpp
// Set thread affinity policy
void set_thread_affinity() {
    // Options: close, spread, master
    omp_set_proc_bind(omp_proc_bind_close);  // Keep threads close in the hardware
    
    #pragma omp parallel
    {
        // Threads will have processor affinity according to the policy
        int thread_id = omp_get_thread_num();
        
        #pragma omp critical
        {
            std::cout << "Thread " << thread_id << " running on CPU " 
                      << sched_getcpu() << std::endl;
        }
    }
}
```

### Task Priorities

Use the priority clause (OpenMP 4.5+) to hint at task execution order:

```cpp
void critical_path_optimization() {
    #pragma omp parallel
    {
        #pragma omp single
        {
            // High priority task (critical path)
            #pragma omp task priority(100)
            {
                critical_path_work();
            }
            
            // Create many lower priority tasks
            for (int i = 0; i < 1000; i++) {
                #pragma omp task priority(0)
                {
                    regular_work();
                }
            }
        }
    }
}
```

### Task Dependencies

Use task dependencies to express complex execution flows:

```cpp
void pipeline_processing() {
    double input[N], intermediate[N], output[N];
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            for (int i = 0; i < N; i++) {
                // Stage 1
                #pragma omp task depend(out: input[i])
                {
                    read_input(&input[i], i);
                }
                
                // Stage 2 depends on Stage 1
                #pragma omp task depend(in: input[i]) depend(out: intermediate[i])
                {
                    process_data(input[i], &intermediate[i]);
                }
                
                // Stage 3 depends on Stage 2
                #pragma omp task depend(in: intermediate[i]) depend(out: output[i])
                {
                    finalize_output(intermediate[i], &output[i]);
                }
            }
        }
    }
}
```

## Performance Measurement Techniques

Accurate performance measurement is essential for effective optimization.

### Basic Timing

For reliable time measurements:

```cpp
#include <chrono>

template<typename Func>
double measure_time(Func func) {
    // Warm-up run
    func();
    
    // Timed runs
    const int NUM_RUNS = 5;
    double total_time = 0.0;
    
    for (int i = 0; i < NUM_RUNS; i++) {
        auto start = std::chrono::high_resolution_clock::now();
        func();
        auto end = std::chrono::high_resolution_clock::now();
        
        total_time += std::chrono::duration<double, std::milli>(end - start).count();
    }
    
    return total_time / NUM_RUNS;
}
```

### Profiling Tool Integration

Use profiling tools for deeper analysis:

- **Windows Performance Analyzer (WPA)**: Built into Windows
- **Intel VTune Profiler**: Detailed CPU performance analysis
- **AMD μProf**: For AMD processors
- **Visual Studio Performance Profiler**: Integrated in Visual Studio

These tools can identify:
- Hotspots (time-consuming sections)
- Task scheduling and synchronization issues
- Memory access patterns and cache behavior
- Thread load imbalance

### Custom Performance Metrics

Develop task-specific performance metrics:

```cpp
struct TaskMetrics {
    int task_id;
    int thread_id;
    double creation_time;
    double start_time;
    double end_time;
    double work_amount;
    
    double waiting_time() const { return start_time - creation_time; }
    double execution_time() const { return end_time - start_time; }
};

std::vector<TaskMetrics> task_metrics;
std::mutex metrics_mutex;

void log_task_metrics(int task_id, double work_amount) {
    TaskMetrics metrics;
    metrics.task_id = task_id;
    metrics.thread_id = omp_get_thread_num();
    metrics.work_amount = work_amount;
    
    double now = omp_get_wtime();
    metrics.creation_time = now;
    
    #pragma omp task firstprivate(metrics)
    {
        metrics.start_time = omp_get_wtime();
        
        // Do the actual work
        do_work(metrics.work_amount);
        
        metrics.end_time = omp_get_wtime();
        
        // Record the metrics
        std::lock_guard<std::mutex> lock(metrics_mutex);
        task_metrics.push_back(metrics);
    }
}

void analyze_task_metrics() {
    // Analyze collected metrics
    double total_wait_time = 0.0;
    double total_exec_time = 0.0;
    
    std::map<int, double> thread_exec_time;
    
    for (const auto& m : task_metrics) {
        total_wait_time += m.waiting_time();
        total_exec_time += m.execution_time();
        thread_exec_time[m.thread_id] += m.execution_time();
    }
    
    // Calculate statistics
    double avg_wait_time = total_wait_time / task_metrics.size();
    double avg_exec_time = total_exec_time / task_metrics.size();
    
    // Calculate load imbalance
    double max_thread_time = 0;
    for (const auto& [thread_id, time] : thread_exec_time) {
        max_thread_time = std::max(max_thread_time, time);
    }
    
    double avg_thread_time = total_exec_time / thread_exec_time.size();
    double imbalance = max_thread_time / avg_thread_time;
    
    // Report results
    std::cout << "Task Performance Metrics:" << std::endl;
    std::cout << "Total tasks: " << task_metrics.size() << std::endl;
    std::cout << "Avg wait time: " << avg_wait_time * 1000 << " ms" << std::endl;
    std::cout << "Avg exec time: " << avg_exec_time * 1000 << " ms" << std::endl;
    std::cout << "Load imbalance: " << imbalance << std::endl;
}
```

## Advanced Optimization Strategies

These techniques can further enhance task parallelism performance.

### Task Throttling

Control task creation rate to avoid overwhelming the system:

```cpp
void throttled_task_creation() {
    const int MAX_TASKS = omp_get_max_threads() * 4;
    std::atomic<int> active_tasks(0);
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            for (int i = 0; i < LARGE_NUMBER; i++) {
                // Simple throttling mechanism
                while (active_tasks.load() >= MAX_TASKS) {
                    #pragma omp taskyield
                }
                
                active_tasks++;
                
                #pragma omp task firstprivate(i)
                {
                    process_item(i);
                    active_tasks--;
                }
            }
        }
    }
}
```

### Task Stealing Analysis

Understanding and optimizing task stealing behavior:

```cpp
void analyze_task_stealing() {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        int num_local_tasks = 0;
        int num_stolen_tasks = 0;
        
        #pragma omp single
        {
            // Create tasks with affinity to specific threads
            for (int i = 0; i < 1000; i++) {
                int target_thread = i % omp_get_num_threads();
                
                #pragma omp task firstprivate(i, target_thread, thread_id)
                {
                    int executing_thread = omp_get_thread_num();
                    
                    if (executing_thread == target_thread) {
                        #pragma omp atomic
                        num_local_tasks++;
                    } else {
                        #pragma omp atomic
                        num_stolen_tasks++;
                    }
                    
                    // Simulate work with varying duration
                    do_work(i % 10);
                }
            }
        }
        
        #pragma omp critical
        {
            std::cout << "Thread " << thread_id 
                      << ": Local tasks: " << num_local_tasks
                      << ", Stolen tasks: " << num_stolen_tasks 
                      << std::endl;
        }
    }
}
```

### Combined Task and Data Parallelism

For maximum performance, combine task parallelism with data parallelism:

```cpp
void hybrid_parallelism() {
    #pragma omp parallel
    {
        #pragma omp single
        {
            // Outer level: task parallelism for irregular workloads
            for (int block = 0; block < num_blocks; block++) {
                #pragma omp task firstprivate(block)
                {
                    int start = block * block_size;
                    int end = start + block_size;
                    
                    // Inner level: data parallelism for regular workloads
                    #pragma omp parallel for
                    for (int i = start; i < end; i++) {
                        process_element(data[i]);
                    }
                }
            }
        }
    }
}
```

### Advanced Memory Optimizations

For memory-bound tasks, consider these techniques:

1. **Software Prefetching**: Explicitly prefetch data before it's needed
   ```cpp
   #include <xmmintrin.h>  // For _mm_prefetch
   
   void prefetch_example(double* data, int size) {
       const int PREFETCH_DISTANCE = 16;
       
       for (int i = 0; i < size; i++) {
           // Prefetch data that will be needed soon
           if (i + PREFETCH_DISTANCE < size) {
               _mm_prefetch((const char*)&data[i + PREFETCH_DISTANCE], _MM_HINT_T0);
           }
           
           process(data[i]);
       }
   }
   ```

2. **Streaming Stores**: Bypass cache when writing data that won't be read soon
   ```cpp
   #include <immintrin.h>
   
   void streaming_store_example(float* dest, int size) {
       for (int i = 0; i < size; i += 4) {
           __m128 data = _mm_set_ps(i+3, i+2, i+1, i);
           _mm_stream_ps(&dest[i], data);  // Non-temporal store
       }
   }
   ```

3. **False Sharing Mitigation**: Avoid multiple threads modifying the same cache line
   ```cpp
   // Prevent false sharing with padding
   struct alignas(64) PaddedCounter {
       std::atomic<int> value;
       char padding[64 - sizeof(std::atomic<int>)];
   };
   
   // Use in parallel code
   std::vector<PaddedCounter> thread_counters(omp_get_max_threads());
   
   #pragma omp parallel
   {
       int tid = omp_get_thread_num();
       thread_counters[tid].value++;  // No false sharing
   }
   ```

### Compiler Optimization

Take advantage of compiler optimizations for maximum performance:

1. **Profile-Guided Optimization (PGO)**: Use runtime profiles to guide optimization
   ```
   // For MSVC: Compile with PGO
   cl /GL /LTCG:PGINSTRUMENT program.cpp  // Instrument build
   program.exe                            // Generate profile data
   cl /GL /LTCG:PGOPTIMIZE program.cpp    // Optimize using profile
   ```

2. **Vectorization Hints**: Help the compiler vectorize code in tasks
   ```cpp
   #pragma omp task
   {
       #pragma omp simd
       for (int i = 0; i < N; i++) {
           result[i] = a[i] * b[i] + c[i];
       }
   }
   ```

3. **Auto-vectorization Reports**: Use compiler flags to understand vectorization
   ```
   // For MSVC: Get vectorization reports
   cl /O2 /Qvec-report:2 program.cpp
   ```