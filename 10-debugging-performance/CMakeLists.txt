cmake_minimum_required(VERSION 3.20)
project(OpenMP_Debugging_Performance VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find OpenMP package
find_package(OpenMP REQUIRED)

# Define output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Set specific output directories for different configurations
foreach(CONFIG_TYPE ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${CONFIG_TYPE} CONFIG_TYPE_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} ${CMAKE_BINARY_DIR}/bin/${CONFIG_TYPE})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} ${CMAKE_BINARY_DIR}/lib/${CONFIG_TYPE})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} ${CMAKE_BINARY_DIR}/lib/${CONFIG_TYPE})
endforeach()

# Add compiler flags for MSVC
if(MSVC)
    # Common flags for all configurations
    add_compile_options(/W4 /MP)
    
    # Disable specific warnings
    add_compile_options(/wd4244) # Disable conversion warnings
    add_compile_options(/wd4100) # Disable unreferenced formal parameter warnings
    add_compile_options(/wd4457) # Disable declaration hides function parameter warnings
    add_compile_options(/wd4189) # Disable 'local variable is initialized but not referenced' warnings
    add_compile_options(/wd4267) # Disable 'conversion from size_t' warnings
    
    # Debug mode flags - Fix /O1 and /RTC1 incompatibility
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /openmp /Zi /MDd /D_DEBUG")
    
    # Release mode flags
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /openmp /O2 /GL")
    
    # Create Profile configuration
    set(CMAKE_CXX_FLAGS_PROFILE "/openmp /O2 /Zi /DPROFILE /MD")
    set(CMAKE_EXE_LINKER_FLAGS_PROFILE "${CMAKE_EXE_LINKER_FLAGS_RELEASE}")
    set(CMAKE_SHARED_LINKER_FLAGS_PROFILE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE}")
    
    # Define VSPerf and ETW if available
    include(CheckIncludeFileCXX)
    check_include_file_cxx("windows.h" HAVE_WINDOWS_H)
    if(HAVE_WINDOWS_H)
        add_definitions(-DHAVE_WINDOWS_SUPPORT)
        check_include_file_cxx("evntprov.h" HAVE_ETW)
        if(HAVE_ETW)
            add_definitions(-DHAVE_ETW_SUPPORT)
        endif()
    endif()
endif()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
)

# Find all source files in the system and utils directories
file(GLOB_RECURSE SYSTEM_SOURCES "${CMAKE_SOURCE_DIR}/utils/*.cpp")

# Add a library for common utilities
add_library(common_lib STATIC ${SYSTEM_SOURCES})

# Link OpenMP to common library
if(OpenMP_CXX_FOUND)
    target_link_libraries(common_lib PUBLIC OpenMP::OpenMP_CXX)
else()
    message(FATAL_ERROR "OpenMP not found!")
endif()

# Define the issue category examples
set(ISSUE_EXAMPLES
    race_conditions
    false_sharing
    load_imbalance
    excessive_synchronization
    memory_issues
)

# Add executables for each issue category
foreach(EXAMPLE ${ISSUE_EXAMPLES})
    # Original problem example
    add_executable(${EXAMPLE} src/${EXAMPLE}.cpp)
    target_link_libraries(${EXAMPLE} PRIVATE common_lib)
    
    # Fixed implementation
    add_executable(${EXAMPLE}_fixed src/fixes/${EXAMPLE}_fixed.cpp)
    target_link_libraries(${EXAMPLE}_fixed PRIVATE common_lib)
    
    # Set output directories
    set_target_properties(${EXAMPLE} ${EXAMPLE}_fixed PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
endforeach()

# Add diagnostic tool integration examples
set(DIAGNOSTIC_EXAMPLES
    vs_diagnostics
    intel_vtune
    custom_profiler
)

foreach(EXAMPLE ${DIAGNOSTIC_EXAMPLES})
    add_executable(${EXAMPLE} src/diagnostics/${EXAMPLE}.cpp)
    target_link_libraries(${EXAMPLE} PRIVATE common_lib)
    set_target_properties(${EXAMPLE} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
endforeach()

# Add visualization tools
set(VISUALIZATION_TOOLS
    thread_timeline_visualizer
    memory_access_visualizer
)

foreach(TOOL ${VISUALIZATION_TOOLS})
    add_executable(${TOOL} src/visualization/${TOOL}.cpp)
    target_link_libraries(${TOOL} PRIVATE common_lib)
    set_target_properties(${TOOL} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
endforeach()

# Add analysis tools
add_executable(analysis_tools src/analysis_tools.cpp)
target_link_libraries(analysis_tools PRIVATE common_lib)
set_target_properties(analysis_tools PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Add post-build message
add_custom_command(TARGET race_conditions POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Build completed successfully!"
)