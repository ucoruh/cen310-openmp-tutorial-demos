cmake_minimum_required(VERSION 3.20)
project(OpenMP_DataSharing VERSION 1.0.0 LANGUAGES CXX)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find OpenMP package
find_package(OpenMP REQUIRED)

# Set compiler-specific flags
if(MSVC)
    # Visual Studio specific flags
    add_compile_options(/W4 /EHsc)
    
    # Enable OpenMP support for MSVC
    add_compile_options(/openmp)
    
    # Set optimization flags for Release mode
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
else()
    # GCC/Clang flags
    add_compile_options(-Wall -Wextra)
    
    # Set optimization flags for Release mode
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
endif()

# Define the executable
add_executable(OpenMP_DataSharing src/main.cpp)

# Include directories for headers
target_include_directories(OpenMP_DataSharing PRIVATE include)

# Link against OpenMP
if(OpenMP_CXX_FOUND)
    target_link_libraries(OpenMP_DataSharing PRIVATE OpenMP::OpenMP_CXX)
else()
    message(FATAL_ERROR "OpenMP not found")
endif()

# Set output directories for the build
set_target_properties(OpenMP_DataSharing PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release"
)

# Add post-build message
add_custom_command(TARGET OpenMP_DataSharing POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Build completed: $<TARGET_FILE:OpenMP_DataSharing>"
) 