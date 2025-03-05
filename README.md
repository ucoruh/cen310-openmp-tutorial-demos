# OpenMP Tutorial Demos

This repository contains a collection of comprehensive OpenMP examples demonstrating various parallel programming concepts using C++ with Visual Studio 2022 on Windows 11. Each demo is a self-contained project with full CMake support, detailed documentation, and batch files for easy building and execution.

## Repository Structure

The repository is organized into the following sections:

1. **Basic Parallel Regions** - Introduction to OpenMP parallel regions
2. **Parallel For Loops** - Parallelizing loops with different approaches
3. **Scheduling Strategies** - Comparison of different work distribution strategies
4. **Data Sharing** - Demonstration of variable scoping and data sharing clauses
5. **Reduction Operations** - Various reduction operations and custom reductions
6. **Synchronization** - Different synchronization mechanisms with performance comparisons
7. **Task Parallelism** - Task-based programming for irregular parallelism
8. **Nested Parallelism & Affinity** - Nested parallel regions and thread affinity controls
9. **SIMD Vectorization** - Data parallelism with SIMD instructions
10. **Debugging & Performance** - Techniques for finding and fixing common OpenMP issues

Each folder contains its own README with detailed explanations and instructions.

## Prerequisites

- Windows 11
- Visual Studio 2022 Community Edition
- CMake 3.20 or higher
- C++17 compatible compiler

## Getting Started

Each project includes:

- A complete CMakeLists.txt configuration
- Source code with extensive comments
- Batch files for configuration, building, and running
- Documentation explaining concepts and implementation

To run any example:

1. Navigate to the example directory
2. Run `configure.bat` to set up the CMake project
3. Run `build.bat` to compile the example
4. Run `run.bat` to execute the example

## Learning Path

For best results, follow the examples in numerical order as they build upon concepts introduced in previous demos.

## Additional Resources

- [OpenMP Official Documentation](https://www.openmp.org/resources/refguides/)
- [Microsoft OpenMP Documentation](https://docs.microsoft.com/en-us/cpp/parallel/openmp/openmp-in-visual-cpp)
- [Using OpenMP Book](https://mitpress.mit.edu/books/using-openmp)

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- OpenMP Architecture Review Board for the OpenMP specification
- Microsoft for Visual Studio and MSVC OpenMP implementation

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.
