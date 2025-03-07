=== OpenMP Debugging and Performance Analysis ===

This project demonstrates common OpenMP performance issues and debugging techniques.

=== Quick Start ===

1. Run the following commands in order:

   configure.bat   - Configure the project
   build_all.bat   - Build all configurations
   run.bat         - Run the examples with default settings

=== Available Scripts ===

1. configure.bat
   - Configures the project with CMake
   - Only needs to be run once initially or after a clean

2. build_all.bat
   - Builds Debug, Release, and Profile configurations
   - Copies executables to bin directory

3. clean.bat
   - Removes build, bin, and reports directories
   - Use before reconfiguring the project

4. run.bat
   - Unified script to run all or specific examples
   - Supports multiple options (see below)

=== Run Script Options ===

run.bat [options]

Options:
  --debug               Run Debug configuration
  --release             Run Release configuration (default)
  --profile             Run Profile configuration
  --verbose             Run with verbose output
  --quick               Run with quick mode
  --benchmark           Run performance benchmarks
  --threads N           Set number of OpenMP threads (default: 4)
  --example NAME        Run specific example (race_conditions, false_sharing, etc.)
                        Use 'all' for all examples (default)
  --help                Show this help message

Examples:
  run.bat --debug --verbose
  run.bat --release --benchmark --threads 8
  run.bat --example race_conditions --benchmark

=== Common Examples ===

1. Run all examples in quick mode:
   run.bat --quick

2. Run just the race_conditions example in debug configuration:
   run.bat --debug --example race_conditions

3. Run performance benchmarks with 8 threads:
   run.bat --benchmark --threads 8

4. Generate visualizations in profile mode:
   run.bat --profile --example thread_timeline_visualizer

=== Output Files ===

Check the 'reports' directory for generated visualizations and analysis results. 