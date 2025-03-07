cmake_minimum_required(VERSION 3.20)
project(OpenMP_NestedParallelism_Affinity VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find OpenMP package
find_package(OpenMP REQUIRED)

# Define output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release)

# Add compiler-specific flags for MSVC
if(MSVC)
    # Add MSVC-specific compiler flags
    add_compile_options(/W4 /MP)
    
    # Ensure OpenMP is enabled for MSVC
    add_compile_options(/openmp)
    
    # Add optimization flags for Release configuration
    string(REPLACE "/O2" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /Oi")
    
    # Disable specific warnings that are showing up in your build
    add_compile_options(
        /wd4514  # unreferenced inline function has been removed
        /wd4710  # function not inlined
        /wd4711  # function selected for automatic inline expansion
        /wd4244  # conversion from 'type1' to 'type2', possible loss of data
        /wd4267  # conversion from 'size_t' to 'type', possible loss of data
        /wd4189  # local variable is initialized but not referenced
    )
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Find all source files in the system and utils directories
file(GLOB_RECURSE SYSTEM_SOURCES "system/*.cpp")
file(GLOB_RECURSE UTILS_SOURCES "utils/*.cpp")

# Add a library for common code
add_library(common_lib STATIC
    ${SYSTEM_SOURCES}
    ${UTILS_SOURCES}
)

# Link OpenMP to common library
if(OpenMP_CXX_FOUND)
    target_link_libraries(common_lib PUBLIC OpenMP::OpenMP_CXX)
else()
    message(FATAL_ERROR "OpenMP not found!")
endif()

# Define the main executable
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE common_lib)

# Add matrix multiplication executable
add_executable(matrix_multiplication src/matrix_multiplication.cpp)
target_link_libraries(matrix_multiplication PRIVATE common_lib)

# Add post-build message
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Build completed successfully!"
)