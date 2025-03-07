cmake_minimum_required(VERSION 3.20)
project(OpenMP_TaskParallelism VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find OpenMP package
find_package(OpenMP REQUIRED)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release)

# Add compiler-specific flags for MSVC
if(MSVC)
    # Add MSVC-specific compiler flags
    add_compile_options(/W4 /MP)
    
    # Set OpenMP flag directly (only once)
    # This prevents the D9025 warning by ensuring only one OpenMP flag is present
    if(NOT CMAKE_CXX_FLAGS MATCHES "/openmp")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /openmp:llvm")
    endif()
    
    # Add optimization flags only in Release configuration
    # This avoids conflict with /RTC1 which is added automatically by Visual Studio for Debug builds
    string(REPLACE "/O2" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    string(REPLACE "/Ob1" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    
    # Add optimizations only for Release builds
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /arch:AVX2")
    
    # Disable specific warnings that might be too noisy
    add_compile_options(/wd4514 /wd4710 /wd4711)
    
    # Disable additional warnings based on build output
    add_compile_options(/wd4267) # conversion from 'size_t' to 'int', possible loss of data
    add_compile_options(/wd4101) # unreferenced local variable
    add_compile_options(/wd4334) # result of 32-bit shift implicitly converted to 64 bits
endif()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/utils
)

# Add the main executable
add_executable(${PROJECT_NAME} 
    src/main.cpp
)

# Link OpenMP - but DON'T use the default OpenMP flags
if(OpenMP_CXX_FOUND)
    # Create a custom OpenMP target without compiler flags (to avoid duplicating OpenMP flags)
    add_library(CustomOpenMP INTERFACE)
    target_link_libraries(CustomOpenMP INTERFACE ${OpenMP_CXX_LIBRARIES})
    target_link_libraries(${PROJECT_NAME} PUBLIC CustomOpenMP)
else()
    message(FATAL_ERROR "OpenMP not found!")
endif()

# Function to add example executables
function(add_example name source_file)
    add_executable(${name} ${source_file})
    # Use our custom OpenMP target instead of OpenMP::OpenMP_CXX
    target_link_libraries(${name} PUBLIC CustomOpenMP)
    set_target_properties(${name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release
    )
endfunction()

# Add example executables
add_example(basic_tasks examples/basic_tasks.cpp)
add_example(fibonacci examples/fibonacci.cpp)
add_example(quicksort examples/quicksort.cpp)
add_example(tree_traversal examples/tree_traversal.cpp)
add_example(graph_processing examples/graph_processing.cpp)
add_example(task_dependencies examples/task_dependencies.cpp)
add_example(task_priority examples/task_priority.cpp)
add_example(taskloop examples/taskloop.cpp)
add_example(taskgroup examples/taskgroup.cpp)

# Add visualization tool
add_example(task_visualizer examples/task_visualizer.cpp)

# Add benchmark suite
add_example(benchmark_suite benchmarks/benchmark_suite.cpp)

# Add advanced examples
add_example(task_stealing examples/task_stealing.cpp)
add_example(nested_tasks examples/nested_tasks.cpp)

# DISABLED: heterogeneous_tasks.cpp causes compiler crashes
# Note: This file is excluded from build due to compiler issues
# It can be built separately or manually if needed
# add_example(heterogeneous_tasks examples/heterogeneous_tasks.cpp)

add_example(task_throttling examples/task_throttling.cpp)

# Add post-build message
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Build completed successfully!"
)