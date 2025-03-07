# OpenMP Debugging and Performance Analysis

This project provides a comprehensive suite of tools and examples for debugging and analyzing the performance of OpenMP parallel programs on Windows with Visual Studio 2022.

## Project Overview

The OpenMP Debugging and Performance Analysis project demonstrates common parallel programming issues and provides tools to detect, analyze, and fix these problems. It includes:

- Example implementations of common OpenMP issues:
  - Race conditions
  - False sharing
  - Load imbalance
  - Excessive synchronization
  - Memory access patterns and NUMA effects

- Diagnostic tools integration:
  - Visual Studio diagnostic tools
  - Intel VTune Profiler
  - Custom profiling infrastructure

- Performance visualization:
  - Thread timeline visualization
  - Memory access patterns and cache line visualization

- Automated analysis tools:
  - Race condition detection
  - Performance regression testing
  - Pattern recognition for common issues

## System Requirements

- Windows 11 (or Windows 10 with latest updates)
- Visual Studio 2022 Community Edition or higher
- CMake 3.20 or higher
- OpenMP-compatible compiler (MSVC)
- Optional: Intel VTune Profiler for advanced performance analysis

## Directory Structure

- `src/` - Implementation files for examples and tools
  - `fixes/` - Fixed versions of the example issues
  - `diagnostics/` - Diagnostic tool integrations
  - `visualization/` - Performance visualization tools
- `include/` - Header files
- `utils/` - Utility functions and shared code
- `scripts/` - Batch files for environment setup and tool execution
- `reports/` - Generated performance reports (created at runtime)
- `docs/` - Documentation files

## Building the Project

The project uses CMake for building. Follow these steps to build the project:

1. Make sure you have Visual Studio 2022 and CMake 3.20+ installed
2. Run the configuration script:
   ```
   configure.bat
   ```
3. Build the project:
   ```
   build_all.bat
   ```

This will create executables for each example in Debug, Release, and Profile configurations.

## Running the Examples

To run the examples, use the provided batch files:

- `run.bat` - Run all examples with default options
- `run_debug.bat` - Run examples with detailed debugging output
- `run_release.bat` - Run optimized release builds
- `scripts/run_issue_examples.bat` - Run specific issue examples
- `scripts/run_diagnostics.bat` - Run the diagnostics tools

Each example can also be run individually from the `build/bin/{Config}` directories.

## Viewing Results

When you run the visualization tools or diagnostic examples, they will generate HTML reports in the `reports/` directory. Open these in any web browser to view the visualizations and analysis results.

## Debugging OpenMP Programs

For comprehensive guidance on debugging OpenMP programs, see the [Debugging Guide](DEBUGGING_GUIDE.md). This includes:

- Setting up Visual Studio for OpenMP debugging
- Thread debugging techniques
- Race condition detection
- Common issues and solutions

## Performance Analysis

For performance analysis tips and methodologies, refer to the [Performance Analysis Guide](PERFORMANCE_ANALYSIS_GUIDE.md). This guide covers:

- Performance measurement tools
- Profiling techniques
- Performance bottleneck identification
- Optimization strategies

## Common Issue Patterns

For a catalog of common OpenMP bugs and performance issues, see the [Issue Patterns](ISSUE_PATTERNS.md) document. This includes:

- Detailed descriptions of each issue
- Detection techniques
- Root cause analysis
- Solution patterns

## Visual Studio Extensions

For information on helpful Visual Studio extensions for OpenMP development, see the [VS Extensions](vs_extensions.md) document.

## License

This project is provided for educational purposes and is available under the MIT License.

## Acknowledgements

This project uses several open-source libraries and tools, and leverages the extensive documentation provided by the OpenMP Architecture Review Board, Intel, and Microsoft. 