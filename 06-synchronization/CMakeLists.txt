cmake_minimum_required(VERSION 3.20)
project(OpenMP_Synchronization VERSION 1.0 LANGUAGES CXX)

# Require C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find OpenMP package
find_package(OpenMP REQUIRED)

# Set compiler flags
if(MSVC)
    # Visual Studio specific flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /EHsc")
    
    # Atomic operations için LLVM OpenMP desteği gerekli
    # MSVC'de advanced OpenMP özellikleri (atomic read/write/update/capture) için llvm backend gerekli
    # Use /openmp:llvm directly in CMAKE_CXX_FLAGS to avoid overriding warnings
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /openmp:llvm")
    
    # Add MSVC-specific compiler flags
    add_compile_options(/W4 /MP /FS)
    
    # Disable specific warnings that might be too noisy
    add_compile_options(/wd4514)  # Unreferenced inline function has been removed
    add_compile_options(/wd4710)  # Function not inlined
    add_compile_options(/wd4711)  # Function selected for automatic inline expansion
    
    # Disable conversion warnings
    add_compile_options(/wd4244)  # Conversion from 'type1' to 'type2', possible loss of data
    
    # Disable unused variable warnings
    add_compile_options(/wd4189)  # Local variable is initialized but not referenced
    
    # Disable unreferenced formal parameter warning (seen in locks.cpp)
    add_compile_options(/wd4100)  # Unreferenced formal parameter
    
    # Fix for invalid warning number format - use correct format for warning disable
    add_compile_options(/wd9025)  # Command-line warning D9025: overriding '/openmp' with '/openmp:llvm'
    
    # Disable D9014 warning about invalid warning numbers
    add_compile_options(/wd9014)  # Command-line warning D9014: invalid value for '/wd'
    
    # Add optimization flags for Release mode
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /Ob2 /Oi /Ot /GL")
else()
    # GCC/Clang flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
    # OpenMP is handled by find_package
endif()

# Debug configuration
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

# Include directories
include_directories(include)

# Define source files
set(SOURCES 
    src/main.cpp
    src/race_conditions.cpp
    src/critical_sections.cpp
    src/atomic_operations.cpp
    src/locks.cpp
    src/barriers.cpp
    src/ordered.cpp
    src/master_single.cpp
    src/flush.cpp
    src/thread_timeline.cpp
    src/lock_contention.cpp
    src/memory_consistency.cpp
    src/utils.cpp
    src/performance.cpp
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link with OpenMP - OpenMP_CXX_FOUND kontrolü ekliyoruz
if(OpenMP_CXX_FOUND)
    # Use target_compile_options instead of directly linking to avoid OpenMP flag conflicts
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenMP::OpenMP_CXX)
    
    # Ensure OpenMP flags don't conflict with our manual settings
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:/wd9025>)
        target_compile_options(${PROJECT_NAME} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:/wd9014>)
    endif()
endif()

# Set output directories
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release"
)

# Install targets
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

# Output information
message(STATUS "Configured ${PROJECT_NAME} with OpenMP version ${OpenMP_CXX_VERSION}") 