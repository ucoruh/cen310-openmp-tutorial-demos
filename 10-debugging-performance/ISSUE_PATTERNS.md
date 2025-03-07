# OpenMP Issue Patterns Catalog

This document catalogs common bugs and performance issues encountered in OpenMP parallel programming, providing detection techniques, root cause analysis, and solution patterns.

## Table of Contents

1. [Race Conditions](#race-conditions)
2. [False Sharing](#false-sharing)
3. [Load Imbalance](#load-imbalance)
4. [Excessive Synchronization](#excessive-synchronization)
5. [Memory Issues](#memory-issues)
6. [Thread Management](#thread-management)
7. [Compiler-Related Issues](#compiler-related-issues)

## Race Conditions

Race conditions occur when multiple threads access shared data without proper synchronization, leading to unpredictable behavior.

### Types of Race Conditions

1. **Read-Write Conflicts**: One thread reads a value while another thread is writing to it
2. **Write-Write Conflicts**: Multiple threads write to the same memory location
3. **Atomicity Violations**: Operations that require multiple steps are interrupted by another thread

### Detection Techniques

1. **Thread Sanitizers**: Use compiler-based sanitizers or dedicated tools
2. **Log-Based Analysis**: Use timestamps and thread IDs to track access patterns
3. **Race Detector**: Use the project's race detection tool 

### Root Cause Analysis

1. Identify the shared variables involved in the race
2. Determine which threads access these variables and when
3. Check if accesses are properly synchronized
4. Examine if assumptions about atomic operations are correct

### Solution Patterns

1. **Critical Sections**:
   ```cpp
   #pragma omp critical
   {
       // Protected code accessing shared data
   }
   ```

2. **Atomic Operations**:
   ```cpp
   #pragma omp atomic
   shared_var += local_value;
   ```

3. **Private Variables**:
   ```cpp
   #pragma omp parallel for private(temp_var)
   for (int i = 0; i < n; i++) {
       temp_var = compute(i);
       // Use temp_var without race conditions
   }
   ```

4. **Reduction**:
   ```cpp
   int sum = 0;
   #pragma omp parallel for reduction(+:sum)
   for (int i = 0; i < n; i++) {
       sum += array[i];
   }
   ```

5. **Thread-Local Storage**:
   ```cpp
   #pragma omp threadprivate(thread_local_var)
   // Each thread has its own copy of thread_local_var
   ```

## False Sharing

False sharing occurs when threads on different cores write to variables that share the same cache line, causing cache coherence traffic and performance degradation.

### Symptoms

1. Performance degradation as thread count increases
2. High coherence traffic visible in hardware counters
3. Performance improves when data is reorganized or padded

### Detection Techniques

1. **Memory Access Visualizer**: Use our memory access visualization tool 
2. **Performance Counters**: Monitor cache line invalidations
3. **Scaling Tests**: Compare performance with thread-local vs. shared data structures

### Root Cause Analysis

1. Identify data structures accessed by multiple threads
2. Check memory layout and alignment of these structures
3. Analyze access patterns to identify potential false sharing
4. Use tools to visualize cache line access patterns

### Solution Patterns

1. **Structure Padding**:
   ```cpp
   struct alignas(64) ThreadData {
       int counter;
       char padding[60]; // Pad to 64 bytes (cache line size)
   };
   ThreadData thread_data[MAX_THREADS];
   ```

2. **Array Transformation**:
   ```cpp
   // Instead of:
   int counters[MAX_THREADS]; // Adjacent elements

   // Use:
   int counters[MAX_THREADS * 16]; // Access every 16th element
   // Thread i uses counters[i * 16]
   ```

3. **Thread-Local Variables**:
   ```cpp
   #pragma omp parallel
   {
       // Thread-local counter avoids false sharing
       int local_counter = 0;
       
       // Work with local_counter
       
       // Only combine results at the end
       #pragma omp critical
       {
           global_counter += local_counter;
       }
   }
   ```

4. **Algorithms with Independent Data**:
   Redesign algorithms to work on thread-local data as much as possible.

## Load Imbalance

Load imbalance occurs when work is unevenly distributed across threads, causing some threads to finish early while others continue working.

### Symptoms

1. Some threads finish much earlier than others
2. Overall performance limited by the slowest thread
3. Underutilization of processor cores

### Detection Techniques

1. **Thread Timeline Visualizer**: Use our thread timeline visualization tool
2. **Thread Profiling**: Measure work time per thread
3. **Visual Studio Concurrency Visualizer**: Shows thread activity over time

### Root Cause Analysis

1. Analyze work distribution patterns
2. Identify sources of workload variability
3. Check for dependencies that force sequential execution
4. Evaluate granularity of parallel tasks

### Solution Patterns

1. **Dynamic Scheduling**:
   ```cpp
   #pragma omp parallel for schedule(dynamic, chunk_size)
   for (int i = 0; i < n; i++) {
       // Work that may vary in duration
   }
   ```

2. **Guided Scheduling**:
   ```cpp
   #pragma omp parallel for schedule(guided)
   for (int i = 0; i < n; i++) {
       // Work with decreasing chunk sizes
   }
   ```

3. **Manual Load Balancing**:
   ```cpp
   // Custom partitioning based on workload characteristics
   std::vector<WorkItem> items_per_thread[num_threads];
   
   // Pre-distribute work based on estimated cost
   for (const auto& item : all_items) {
       int thread_id = getOptimalThreadForItem(item);
       items_per_thread[thread_id].push_back(item);
   }
   
   #pragma omp parallel
   {
       int tid = omp_get_thread_num();
       processItems(items_per_thread[tid]);
   }
   ```

4. **Task-Based Parallelism**:
   ```cpp
   #pragma omp parallel
   {
       #pragma omp single
       {
           for (int i = 0; i < n; i++) {
               #pragma omp task
               {
                   // Process task i
               }
           }
       }
   }
   ```

## Excessive Synchronization

Excessive synchronization limits parallel performance by forcing threads to wait for each other frequently.

### Symptoms

1. Poor scaling with increased thread count
2. High percentage of time spent in synchronization operations
3. Threads frequently waiting at barriers or critical sections

### Detection Techniques

1. **Concurrency Visualizer**: Identify synchronization bottlenecks
2. **Custom Profiler**: Measure time spent in synchronization operations
3. **Thread State Analysis**: Monitor thread wait states

### Root Cause Analysis

1. Identify frequently used synchronization constructs
2. Measure time spent in each synchronization operation
3. Analyze algorithm design for unnecessary dependencies
4. Check for overly conservative synchronization

### Solution Patterns

1. **Reduce Barrier Frequency**:
   ```cpp
   // Instead of multiple barriers:
   for (int phase = 0; phase < num_phases; phase++) {
       #pragma omp parallel for
       for (int i = 0; i < n; i++) {
           // Work for this phase
       }
       // Implicit barrier here
   }
   
   // Combine phases when possible:
   #pragma omp parallel
   {
       for (int phase = 0; phase < num_phases; phase++) {
           #pragma omp for nowait
           for (int i = 0; i < n; i++) {
               // Work for this phase
           }
           
           // Only synchronize when absolutely necessary
           #pragma omp barrier
       }
   }
   ```

2. **Finer-Grained Synchronization**:
   ```cpp
   // Instead of one large critical section:
   #pragma omp critical
   {
       // Large section of code
   }
   
   // Use multiple smaller critical sections:
   // Only protect what needs protection
   computation_outside_critical_section();
   
   #pragma omp critical(resource1)
   {
       // Minimal code that needs exclusive access to resource1
   }
   
   more_computation_outside_critical_section();
   
   #pragma omp critical(resource2)
   {
       // Minimal code that needs exclusive access to resource2
   }
   ```

3. **Lock-Free Techniques**:
   ```cpp
   // Instead of critical sections for counters:
   #pragma omp atomic
   counter++;
   
   // For more complex operations, consider atomic capture:
   int old_value;
   #pragma omp atomic capture
   {
       old_value = counter;
       counter++;
   }
   ```

4. **Algorithm Redesign**:
   Reorganize algorithms to minimize dependencies and synchronization requirements.

## Memory Issues

Memory-related issues can severely impact performance, especially in NUMA (Non-Uniform Memory Access) systems.

### Symptoms

1. Performance varies based on data allocation patterns
2. Decreased performance with large datasets
3. Different behavior on NUMA vs. non-NUMA systems

### Detection Techniques

1. **Memory Access Visualizer**: Analyze memory access patterns
2. **Performance Counters**: Monitor memory transactions and bandwidth
3. **NUMA Tools**: Use Windows performance tools with NUMA metrics

### Root Cause Analysis

1. Analyze memory allocation patterns
2. Identify thread-to-memory affinity
3. Measure memory bandwidth utilization
4. Check for remote NUMA node accesses

### Solution Patterns

1. **NUMA-Aware Allocation**:
   ```cpp
   // Allocate memory with awareness of NUMA topology
   #include <windows.h>
   #include <memoryapi.h>
   
   // Get NUMA node for current thread
   USHORT numa_node = 0;
   GetNumaProcessorNodeEx(GetCurrentProcessorNumber(), &numa_node);
   
   // Allocate on specific NUMA node
   void* data = VirtualAllocExNuma(
       GetCurrentProcess(),
       NULL,
       size,
       MEM_RESERVE | MEM_COMMIT,
       PAGE_READWRITE,
       numa_node
   );
   ```

2. **First-Touch Policy**:
   ```cpp
   // Allocate memory
   double* data = new double[size];
   
   // Initialize in parallel to leverage first-touch policy
   #pragma omp parallel for
   for (int i = 0; i < size; i++) {
       data[i] = 0.0; // Memory page will be allocated on the NUMA node of the initializing thread
   }
   ```

3. **Memory Blocking**:
   ```cpp
   // Process data in cache-friendly blocks
   const int block_size = 64; // Adjust based on cache size
   
   #pragma omp parallel for
   for (int i = 0; i < n; i += block_size) {
       for (int j = 0; j < m; j += block_size) {
           // Process block (i,j) to (i+block_size, j+block_size)
           for (int ii = i; ii < std::min(i + block_size, n); ii++) {
               for (int jj = j; jj < std::min(j + block_size, m); jj++) {
                   process(data[ii][jj]);
               }
           }
       }
   }
   ```

4. **Memory Layout Optimization**:
   ```cpp
   // For row-major access patterns (C/C++ default)
   for (int i = 0; i < rows; i++) {
       for (int j = 0; j < cols; j++) {
           matrix[i][j] = compute(i, j);
       }
   }
   
   // NOT:
   for (int j = 0; j < cols; j++) {
       for (int i = 0; i < rows; i++) {
           matrix[i][j] = compute(i, j); // Poor locality
       }
   }
   ```

## Thread Management

Improper thread management can cause performance issues, overhead, or unexpected behavior.

### Symptoms

1. Performance degradation with increased thread count
2. Excessive thread creation and destruction
3. Thread contention with other applications

### Detection Techniques

1. **Thread Analysis Tools**: Monitor thread creation and destruction
2. **System Resource Monitor**: Check CPU utilization across cores
3. **Custom Thread Metrics**: Use Profiler for thread behavior

### Root Cause Analysis

1. Analyze thread creation patterns
2. Measure overhead of thread management
3. Identify contention with system or other application threads
4. Check thread affinity and processor binding

### Solution Patterns

1. **Thread Pool Reuse**:
   ```cpp
   // Instead of creating parallel regions repeatedly:
   for (int i = 0; i < iterations; i++) {
       #pragma omp parallel
       {
           // Work for iteration i
       }
   }
   
   // Use a persistent parallel region:
   #pragma omp parallel
   {
       for (int i = 0; i < iterations; i++) {
           #pragma omp single
           {
               // Master thread setup for iteration i
           }
           
           #pragma omp for
           for (int j = 0; j < n; j++) {
               // Parallel work
           }
           
           #pragma omp barrier
           // All threads synchronized between iterations
       }
   }
   ```

2. **Appropriate Thread Count**:
   ```cpp
   // Adjust thread count based on system and workload
   int num_threads = omp_get_num_procs(); // Start with core count
   
   // Adjust for memory-bound workloads
   if (is_memory_bound) {
       num_threads = std::min(num_threads, memory_nodes * 2);
   }
   
   omp_set_num_threads(num_threads);
   ```

3. **Thread Affinity**:
   ```cpp
   // Set environment variables for thread affinity
   // In code or batch file:
   // SET OMP_PROC_BIND=spread
   // SET OMP_PLACES=cores
   
   // Or programmatically:
   omp_set_proc_bind(omp_proc_bind_spread);
   ```

4. **Nested Parallelism Control**:
   ```cpp
   // Control nested parallelism
   omp_set_nested(1); // Enable nested parallelism
   omp_set_max_active_levels(2); // Limit to 2 levels
   
   #pragma omp parallel num_threads(4)
   {
       // First level with 4 threads
       
       #pragma omp parallel num_threads(2)
       {
           // Second level with 2 threads each (8 total)
       }
   }
   ```

## Compiler-Related Issues

Compiler behavior can significantly affect OpenMP performance and correctness.

### Symptoms

1. Different behavior between debug and release builds
2. Unexpected performance with optimizations
3. Missing vectorization or parallelization

### Detection Techniques

1. **Compiler Reports**: Enable optimization reports
2. **Assembly Inspection**: Check generated code
3. **Comparative Builds**: Test different compilers and options

### Root Cause Analysis

1. Analyze compiler optimization decisions
2. Check for optimization barriers in code
3. Verify vectorization and parallelization
4. Compare generated assembly for different build options

### Solution Patterns

1. **Optimization Pragmas**:
   ```cpp
   // Guide compiler optimizations
   #pragma optimize("gt", on)
   void performance_critical_function() {
       // Function that needs aggressive optimization
   }
   
   // Help vectorization
   #pragma omp simd
   for (int i = 0; i < n; i++) {
       result[i] = compute(data[i]);
   }
   ```

2. **Compiler-Specific Flags**:
   ```cpp
   // In CMake or project settings:
   // /Qpar - Auto-parallelization (MSVC)
   // /Qvec-report:2 - Vectorization reporting
   // /fp:fast - Fast floating-point model
   ```

3. **Function Attributes**:
   ```cpp
   __declspec(noinline) void prevent_inlining() {
       // Function that should not be inlined
   }
   
   __declspec(novtable) class OptimizedBase {
       // Class with optimized vtable
   };
   ```

4. **Optimized Memory Alignment**:
   ```cpp
   // Aligned memory allocation for SIMD
   alignas(32) float vector[16]; // 32-byte alignment for AVX
   
   // Or dynamic allocation
   float* aligned_data = (float*)_aligned_malloc(size * sizeof(float), 32);
   ```

## Conclusion

Understanding these common OpenMP issue patterns, their detection techniques, and solution approaches will help you develop more robust and efficient parallel applications. Use the tools provided in this project to identify and fix these issues in your code. 