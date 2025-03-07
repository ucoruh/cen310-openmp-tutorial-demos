cmake_minimum_required(VERSION 3.20)
project(OpenMP_BasicParallel VERSION 1.0.0 LANGUAGES CXX)

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
endif()

# Add the executable
add_executable(${PROJECT_NAME} src/main.cpp)

# Link OpenMP
if(OpenMP_CXX_FOUND)
    target_link_libraries(${PROJECT_NAME} PUBLIC OpenMP::OpenMP_CXX)
else()
    message(FATAL_ERROR "OpenMP not found!")
endif()

# Note: Output will be in build/Debug and build/Release directories
# by default with Visual Studio generator

# Add post-build message
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Build completed successfully!"
) 