cmake_minimum_required(VERSION 3.20)
project(OpenMP_SchedulingStrategies VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find OpenMP package
find_package(OpenMP REQUIRED)

# Add compiler-specific flags for MSVC
if(MSVC)
    # Add MSVC-specific compiler flags
    add_compile_options(/W4 /MP)
    
    # Ensure OpenMP is enabled for MSVC
    add_compile_options(/openmp)
    
    # Disable specific warnings that might be too noisy
    add_compile_options(/wd4514 /wd4710 /wd4711)
    
    # Disable conversion warnings (C4244)
    add_compile_options(/wd4244)
    
    # Disable unused variable warnings (C4189)
    add_compile_options(/wd4189)
    
    # Add optimization flags for Release mode
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /Ob2 /Oi /Ot /GL")
endif()

# Add the executable
add_executable(${PROJECT_NAME} src/main.cpp)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Link OpenMP
if(OpenMP_CXX_FOUND)
    target_link_libraries(${PROJECT_NAME} PUBLIC OpenMP::OpenMP_CXX)
else()
    message(FATAL_ERROR "OpenMP not found!")
endif()

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/Release)

# Add post-build message
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Build completed successfully!"
) 