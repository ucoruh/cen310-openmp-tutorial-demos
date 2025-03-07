# OpenMP Debugging Guide

This guide provides comprehensive techniques and best practices for debugging OpenMP parallel programs, with a focus on Visual Studio 2022 debugging features.

## Table of Contents

1. [Setting Up the Debugging Environment](#setting-up-the-debugging-environment)
2. [Visual Studio Debugger Configuration](#visual-studio-debugger-configuration)
3. [Thread Debugging Techniques](#thread-debugging-techniques)
4. [Race Condition Identification](#race-condition-identification)
5. [Debugging Deadlocks and Hangs](#debugging-deadlocks-and-hangs)
6. [Memory Issues](#memory-issues)
7. [Using Diagnostic Tools](#using-diagnostic-tools)
8. [Working with Debug vs. Release Builds](#working-with-debug-vs-release-builds)

## Setting Up the Debugging Environment

### Debug Configuration

For effective OpenMP debugging, ensure your project is set up with appropriate debug settings:

- Enable OpenMP support with the `/openmp` compiler flag
- Set appropriate debug options: `/Zi /RTC1 /MDd /D_DEBUG`
- Disable optimizations or use minimal optimization (`/O1`) for debugging
- Add debugging symbols and enable incremental linking

### Environment Variables

Set these environment variables to control OpenMP behavior during debugging:

- `OMP_DISPLAY_ENV=TRUE` - Display OpenMP environment variables at startup
- `OMP_DISPLAY_AFFINITY=TRUE` - Show thread affinity information
- `OMP_PROC_BIND=spread` - Control thread binding during debugging
- `OMP_NUM_THREADS=4` (or similar) - Limit thread count for easier debugging

## Visual Studio Debugger Configuration

### Configuring Thread Debugging

1. Open Visual Studio 2022
2. Go to Debug → Options
3. Configure these settings:
   - Enable "Show all threads in source" for thread visualization
   - Enable "Show threads in source" to see thread markers
   - Set "Just My Code" according to your preference (disable for library debugging)

### Parallel Stacks Window

The Parallel Stacks window is invaluable for OpenMP debugging:

1. During debugging, go to Debug → Windows → Parallel Stacks
2. This shows a visual representation of all thread call stacks
3. Switch between "Threads View" and "Tasks View" as needed
4. Use the "Group By" dropdown to organize threads by function or call stack

### Parallel Watch Window

For watching variables across multiple threads:

1. Go to Debug → Windows → Parallel Watch
2. Add variables to monitor across threads
3. Use conditional formatting to highlight values of interest

## Thread Debugging Techniques

### Freezing and Thawing Threads

During debugging, you can freeze and thaw threads to isolate issues:

1. Open the Threads window (Debug → Windows → Threads)
2. Right-click on a thread and select "Freeze" to pause it
3. Examine other threads' behavior while some are frozen
4. "Thaw" threads to resume them

### Thread Naming

For easier identification in the debugger:

```cpp
#pragma omp parallel
{
    char name[32];
    sprintf(name, "Worker-%d", omp_get_thread_num());
    #ifdef _MSC_VER
    // Set thread name for debugging
    const DWORD MS_VC_EXCEPTION = 0x406D1388;
    #pragma pack(push, 8)
    struct THREADNAME_INFO {
        DWORD dwType;
        LPCSTR szName;
        DWORD dwThreadID;
        DWORD dwFlags;
    };
    #pragma pack(pop)
    THREADNAME_INFO info = { 0x1000, name, static_cast<DWORD>(-1), 0 };
    __try {
        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info);
    } __except (EXCEPTION_CONTINUE_EXECUTION) {}
    #endif
    
    // Thread code here
}
```

### Breaking on Specific Thread Events

Set conditional breakpoints based on thread ID:

1. Set a breakpoint in a parallel region
2. Right-click the breakpoint and select "Conditions"
3. Enter a condition like `omp_get_thread_num() == 2` to break only on thread 2

## Race Condition Identification

Race conditions are one of the most common and difficult-to-debug issues in OpenMP programs.

### Using Thread Sanitizers

Visual Studio Thread Sanitizer can identify race conditions:

1. Enable Address Sanitizer with Thread Sanitizer in project properties
2. Run your application to detect race conditions
3. Examine reports of detected race conditions

### Manual Race Detection Techniques

When sanitizers aren't available or sufficient:

1. Use logging with thread IDs and timestamps to track access order
2. Implement artificial delays (like `sleep`) to trigger timing conditions
3. Use the `TRACK_READ` and `TRACK_WRITE` macros from our race detector:

```cpp
// Example of using the race detector
raceDetector.enable();

#pragma omp parallel for
for (int i = 0; i < n; i++) {
    // Track memory reads
    int value = sharedArray[i];
    TRACK_READ(&sharedArray[i], "file.cpp:line123");
    
    // Track memory writes
    sharedArray[i] = value + 1;
    TRACK_WRITE(&sharedArray[i], "file.cpp:line126");
}

raceDetector.disable();
raceDetector.generateReport("race_report.html");
```

## Debugging Deadlocks and Hangs

### Identifying Deadlocks

When your OpenMP program hangs:

1. Break execution in the debugger
2. Examine the Parallel Stacks window to see where threads are blocked
3. Look for threads waiting on locks, critical sections, or barriers
4. Check for circular dependencies between threads

### Using the Concurrency Visualizer

Visual Studio's Concurrency Visualizer helps identify deadlocks:

1. Start a debugging session with Concurrency Visualizer (Debug → Concurrency Visualizer)
2. Run your application until it hangs
3. Examine the thread blocking graph to identify blocking relationships
4. Use the execution timeline to see which operations led to the deadlock

## Memory Issues

### Memory Access Patterns

Use our memory access visualizer to identify problematic patterns:

```cpp
// Example usage
MemoryAccessTracker tracker;
tracker.setArrayInfo(data, size);

#pragma omp parallel for
for (int i = 0; i < n; i++) {
    // Track memory operations
    TRACK_READ(&data[i], sizeof(int));
    data[i] = compute(data[i]);
    TRACK_WRITE(&data[i], sizeof(int));
}

tracker.generateHeatMap("memory_access.html");
```

### NUMA-Related Issues

For NUMA debugging:

1. Use Windows Performance Recorder to capture memory accesses
2. Analyze thread-to-memory access patterns
3. Look for remote NUMA node access patterns
4. Test with different thread affinity settings

## Using Diagnostic Tools

### Visual Studio Diagnostics

1. Open Diagnostic Tools (Debug → Windows → Show Diagnostic Tools)
2. Enable CPU Usage, Memory Usage, and any other relevant tools
3. Run your application with these tools enabled
4. Analyze the diagnostic session to identify hotspots and issues

### Intel VTune Integration

For more advanced analysis:

1. Install Intel VTune Profiler
2. Use our Intel VTune integration examples from `src/diagnostics/intel_vtune.cpp`
3. Focus on threading analysis and memory access patterns
4. Correlate findings with source code using VTune's annotations

### Custom Profiling

Our custom profiler provides targeted insight:

```cpp
// Example of using the custom profiler
ScopedTimer timer("MainLoop");

#pragma omp parallel
{
    ScopedTimer threadTimer("ThreadExecution");
    // Work here
}

Profiler::getInstance().generateReport("profile_report.html");
```

## Working with Debug vs. Release Builds

### Understanding Optimization Effects

Be aware that some OpenMP issues may not appear in release builds due to optimizations:

1. Race conditions that appear in debug may be hidden in release due to different timing
2. False sharing effects are often more pronounced in release builds
3. Some synchronization issues may only appear under load in release builds

### Debug Build Practices

For effective debugging:

1. Start with unoptimized debug builds to find logical errors
2. Use minimal optimization (`/O1`) when needed to reproduce issues
3. Instrument release builds with debug symbols for performance debugging
4. Compare behavior between debug and release to identify optimization-related issues

### Diagnostic Builds

Consider creating special diagnostic builds:

1. Use the Profile configuration from our CMake setup
2. Add diagnostic instrumentation to critical code sections
3. Enable relevant debug options (`_DEBUG`, `DEBUG_SYNCHRONIZATION`, etc.)
4. Compile with optimizations but retain debug information

## Conclusion

Debugging OpenMP programs requires a combination of specialized tools, techniques, and knowledge of parallel programming patterns. This guide provides a foundation for identifying and fixing common issues in your parallel code. Use the example implementations in this project to practice these debugging techniques on real-world parallel programming problems. 