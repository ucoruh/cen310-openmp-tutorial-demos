cmake_minimum_required(VERSION 3.20)
project(OpenMP_SIMD_Vectorization VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Define _USE_MATH_DEFINES for M_PI constant
add_definitions(-D_USE_MATH_DEFINES)

# Find OpenMP package - ensure compatibility with OpenMP 2.0
find_package(OpenMP REQUIRED)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release)

# Add compiler-specific flags for MSVC
if(MSVC)
    # Add MSVC-specific compiler flags
    add_compile_options(/W4 /MP)
    
    # Remove default OpenMP flags to avoid conflicts
    set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS})
    set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    
    # Ensure OpenMP is enabled for MSVC with experimental support for SIMD
    # But don't add it as a global option to avoid conflicts
    set(SIMD_OPENMP_FLAG "/openmp:experimental")
    
    # Enable more detailed vectorization diagnostics
    add_compile_options(/Qvec-report:2)
    
    # Add optimization flags for Release configuration
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /fp:fast")
    
    # Enable AVX2 instructions for compatible CPUs in Release mode
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /arch:AVX2")
    
    # Add additional optimization options to help with vectorization
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Oi /GL")
    
    # Add Debug-specific flags
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od /Zi /RTC1")
    
    # Disable specific warnings that might be too noisy
    add_compile_options(/wd4514 /wd4710 /wd4711)
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Add the executable
add_executable(${PROJECT_NAME} 
    src/main.cpp
    src/basic_simd.cpp
    src/array_operations.cpp
    src/complex_math.cpp
    src/simd_alignment.cpp
    src/simd_width.cpp
    src/mixed_precision.cpp
    src/simd_parallelism.cpp
    src/cpu_features.cpp
    src/benchmark_suite.cpp
    src/simd_verifier.cpp
    src/asm_analyzer.cpp
)

# Link OpenMP and add target-specific options
if(OpenMP_CXX_FOUND)
    target_link_libraries(${PROJECT_NAME} PUBLIC OpenMP::OpenMP_CXX)
    # Add the SIMD experimental flag specifically to this target
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE ${SIMD_OPENMP_FLAG})
    endif()
else()
    message(FATAL_ERROR "OpenMP not found!")
endif()

# Set specific output file for this executable to avoid potential conflicts
set_target_properties(${PROJECT_NAME} PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release"
    PDB_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/Debug"
    PDB_NAME_DEBUG "${PROJECT_NAME}_debug"
)

# Create a custom target for generating assembly output
add_custom_target(generate_asm
    COMMAND ${CMAKE_COMMAND} -E echo "Generating assembly output for ${PROJECT_NAME}"
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/asm_output
    COMMAND ${CMAKE_CXX_COMPILER} /FA /c /O2 ${SIMD_OPENMP_FLAG} /arch:AVX2 ${CMAKE_SOURCE_DIR}/src/basic_simd.cpp /Fo${CMAKE_SOURCE_DIR}/asm_output/basic_simd.obj /Fa${CMAKE_SOURCE_DIR}/asm_output/basic_simd.asm
    COMMAND ${CMAKE_CXX_COMPILER} /FA /c /O2 ${SIMD_OPENMP_FLAG} /arch:AVX2 ${CMAKE_SOURCE_DIR}/src/array_operations.cpp /Fo${CMAKE_SOURCE_DIR}/asm_output/array_operations.obj /Fa${CMAKE_SOURCE_DIR}/asm_output/array_operations.asm
    COMMAND ${CMAKE_CXX_COMPILER} /FA /c /O2 ${SIMD_OPENMP_FLAG} /arch:AVX2 ${CMAKE_SOURCE_DIR}/src/complex_math.cpp /Fo${CMAKE_SOURCE_DIR}/asm_output/complex_math.obj /Fa${CMAKE_SOURCE_DIR}/asm_output/complex_math.asm
    COMMENT "Generating assembly output for SIMD examples"
)

# Add post-build message
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Build completed successfully!"
)