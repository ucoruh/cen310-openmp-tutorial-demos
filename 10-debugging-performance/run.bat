@echo off
setlocal enabledelayedexpansion

echo === OpenMP Debugging and Performance Analysis Run Script ===
echo.

:: Process command line parameters with defaults
set CONFIG=Release
set MODE=normal
set OMP_NUM_THREADS=4
set SPECIFIC_EXAMPLE=all

:: Parse command line arguments
:parse_args
if "%~1"=="" goto continue_execution
if /i "%~1"=="--debug" (
    set CONFIG=Debug
    shift
    goto parse_args
)
if /i "%~1"=="--release" (
    set CONFIG=Release
    shift
    goto parse_args
)
if /i "%~1"=="--profile" (
    set CONFIG=Profile
    shift
    goto parse_args
)
if /i "%~1"=="--verbose" (
    set MODE=verbose
    shift
    goto parse_args
)
if /i "%~1"=="--quick" (
    set MODE=quick
    shift
    goto parse_args
)
if /i "%~1"=="--benchmark" (
    set MODE=benchmark
    shift
    goto parse_args
)
if /i "%~1"=="--threads" (
    set OMP_NUM_THREADS=%~2
    shift
    shift
    goto parse_args
)
if /i "%~1"=="--example" (
    set SPECIFIC_EXAMPLE=%~2
    shift
    shift
    goto parse_args
)
if /i "%~1"=="--help" (
    goto show_help
)
echo Unknown parameter: %~1
echo Use --help for usage information.
exit /b 1

:show_help
echo Usage: run.bat [options]
echo.
echo Options:
echo   --debug               Run Debug configuration
echo   --release             Run Release configuration (default)
echo   --profile             Run Profile configuration
echo   --verbose             Run with verbose output
echo   --quick               Run with quick mode
echo   --benchmark           Run performance benchmarks
echo   --threads N           Set number of OpenMP threads (default: 4)
echo   --example NAME        Run specific example (race_conditions, false_sharing, etc.)
echo                         Use 'all' for all examples (default)
echo   --help                Show this help message
echo.
echo Examples:
echo   run.bat --debug --verbose
echo   run.bat --release --benchmark --threads 8
echo   run.bat --example race_conditions --benchmark
exit /b 0

:continue_execution
rem Check if build directory exists
if not exist build (
    echo Error: Build directory not found.
    echo Please run configure.bat and build_all.bat first.
    exit /b 1
)

rem Check if binaries exist
if not exist build\bin\%CONFIG% (
    echo Error: %CONFIG% binaries not found.
    echo Please run build_all.bat first.
    exit /b 1
)

rem Set environment variables for OpenMP
set OMP_PROC_BIND=spread

if /i "%CONFIG%"=="Debug" (
    set OMP_DISPLAY_ENV=TRUE
    set OMP_DISPLAY_AFFINITY=TRUE
) else (
    set OMP_DISPLAY_ENV=FALSE
    set OMP_DISPLAY_AFFINITY=FALSE
)

rem Create reports directory if it doesn't exist
if not exist reports (
    mkdir reports
    echo Created reports directory.
)

echo Running with %OMP_NUM_THREADS% OpenMP threads (%CONFIG% build)
if /i "%MODE%"=="verbose" echo Running in verbose mode
if /i "%MODE%"=="quick" echo Running in quick mode
if /i "%MODE%"=="benchmark" echo Running in benchmark mode
echo.

set SUFFIX=
if /i "%MODE%"=="verbose" set SUFFIX=--verbose
if /i "%MODE%"=="quick" set SUFFIX=--quick
if /i "%MODE%"=="benchmark" set SUFFIX=--benchmark

:: Function to run an example if it matches the selection
call :run_if_selected "race_conditions" "Race Conditions"
call :run_if_selected "race_conditions_fixed" "Race Conditions (Fixed)"
call :run_if_selected "false_sharing" "False Sharing"
call :run_if_selected "false_sharing_fixed" "False Sharing (Fixed)"
call :run_if_selected "load_imbalance" "Load Imbalance"
call :run_if_selected "load_imbalance_fixed" "Load Imbalance (Fixed)"
call :run_if_selected "excessive_synchronization" "Excessive Synchronization"
call :run_if_selected "excessive_synchronization_fixed" "Excessive Synchronization (Fixed)"
call :run_if_selected "memory_issues" "Memory Issues"
call :run_if_selected "memory_issues_fixed" "Memory Issues (Fixed)"
call :run_if_selected "vs_diagnostics" "VS Diagnostics"
call :run_if_selected "intel_vtune" "Intel VTune Integration"
call :run_if_selected "custom_profiler" "Custom Profiler"
call :run_if_selected "thread_timeline_visualizer" "Thread Timeline Visualizer" "--threads %OMP_NUM_THREADS% --output thread_timeline_%CONFIG%.html %SUFFIX%"
call :run_if_selected "memory_access_visualizer" "Memory Access Visualizer" "--threads %OMP_NUM_THREADS% --array-size 100 --cache-line 64 --output memory_access_%CONFIG%.html %SUFFIX%"
call :run_if_selected "analysis_tools" "Analysis Tools" "--demo all --threads %OMP_NUM_THREADS% %SUFFIX%"

:: Performance comparison section (only in benchmark mode and Release configuration)
if /i "%MODE%"=="benchmark" if /i "%CONFIG%"=="Release" (
    echo.
    echo === Performance Comparison ===
    echo.
    
    if "%SPECIFIC_EXAMPLE%"=="all" (
        set COMPARE_EXAMPLES=race_conditions false_sharing load_imbalance excessive_synchronization memory_issues
    ) else (
        set COMPARE_EXAMPLES=%SPECIFIC_EXAMPLE%
    )
    
    for %%e in (!COMPARE_EXAMPLES!) do (
        echo ---------------------------------------------------------
        echo %%e - Performance Comparison
        echo ---------------------------------------------------------
        echo Original implementation:
        build\bin\%CONFIG%\%%e.exe --benchmark
        echo.
        echo Fixed implementation:
        build\bin\%CONFIG%\%%e_fixed.exe --benchmark
        echo.
    )
)

echo === Examples Completed ===
echo.
echo Check the 'reports' directory for generated visualizations and analysis results.
echo.
exit /b 0

:run_if_selected
set example_name=%~1
set display_name=%~2
set custom_args=%~3

if "%custom_args%"=="" set custom_args=%SUFFIX%

if /i "%SPECIFIC_EXAMPLE%"=="all" (
    goto run_example
) else if /i "%SPECIFIC_EXAMPLE%"=="%example_name%" (
    goto run_example
) else (
    exit /b 0
)

:run_example
echo Running %display_name% Example...
build\bin\%CONFIG%\%example_name%.exe %custom_args%
echo.
exit /b 0 