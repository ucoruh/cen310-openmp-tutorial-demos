cmake_minimum_required(VERSION 3.10)
project(OpenMP_ReductionOperations VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find OpenMP package
find_package(OpenMP REQUIRED)

# Set compiler-specific flags
if(MSVC)
    # Visual Studio flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od /Zi")
else()
    # GCC/Clang flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
endif()

# Define the executable target
add_executable(OpenMP_ReductionOperations src/main.cpp)

# Include directories
target_include_directories(OpenMP_ReductionOperations PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Link against OpenMP
if(OpenMP_CXX_FOUND)
    target_link_libraries(OpenMP_ReductionOperations PRIVATE OpenMP::OpenMP_CXX)
endif()

# Set output directories
set_target_properties(OpenMP_ReductionOperations PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release"
)

# Add post-build message for the executable
add_custom_command(TARGET OpenMP_ReductionOperations POST_BUILD
    COMMENT "OpenMP_ReductionOperations executable is now available at ${CMAKE_BINARY_DIR}/[Debug|Release]/OpenMP_ReductionOperations${CMAKE_EXECUTABLE_SUFFIX}"
) 