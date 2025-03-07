@echo off
setlocal enabledelayedexpansion

echo === OpenMP Task Parallelism Run Script ===
echo.

:: Process command line parameters with defaults
set CONFIG=Release
set MODE=normal
set OMP_NUM_THREADS=4
set DEMO_TYPE=all

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
if /i "%~1"=="--verbose" (
    set MODE=verbose
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
if /i "%~1"=="--demo" (
    set DEMO_TYPE=%~2
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
echo   --verbose             Run with verbose output
echo   --benchmark           Run performance benchmarks
echo   --threads N           Set number of OpenMP threads (default: 4)
echo   --demo TYPE           Run specific demo type (basic, recursive, dependency, priority, all)
echo   --help                Show this help message
echo.
echo Examples:
echo   run.bat --debug --verbose
echo   run.bat --release --benchmark --threads 8
echo   run.bat --demo dependency --threads 4
exit /b 0

:continue_execution
:: Check if build directory exists
if not exist build (
    echo Error: Build directory not found.
    echo Please run configure.bat and build_all.bat first.
    exit /b 1
)

:: Define the base executable directory
set "EXEC_DIR=build\%CONFIG%"

:: Check if the bin directory exists as alternative
if exist bin\%CONFIG% (
    set "EXEC_DIR=bin\%CONFIG%"
)

:: Define executable path based on configuration
set "EXE_PATH=%EXEC_DIR%\OpenMP_TaskParallelism.exe"

:: Copy all necessary executables to the working directory
echo Copying executables to the working directory...
copy "%EXEC_DIR%\*.exe" . > nul 2>&1
if errorlevel 1 (
    echo Warning: Could not copy all executables. Some examples may not run correctly.
) else (
    echo Executables copied successfully.
)

:: Check if executable exists
if not exist "%EXE_PATH%" (
    echo.
    echo Main executable not found: %EXE_PATH%
    echo Please build the project first by running build_all.bat.
    echo.
    echo Available executables in %EXEC_DIR%:
    dir /b "%EXEC_DIR%\*.exe" 2>nul
    exit /b 1
)

:: Set environment variables for OpenMP
set OMP_PROC_BIND=spread

if /i "%CONFIG%"=="Debug" (
    set OMP_DISPLAY_ENV=TRUE
    set OMP_DISPLAY_AFFINITY=TRUE
) else (
    set OMP_DISPLAY_ENV=FALSE
    set OMP_DISPLAY_AFFINITY=FALSE
)

:: Create reports directory if it doesn't exist
if not exist reports (
    mkdir reports
    echo Created reports directory.
)

:: Build command line arguments
set ARGS=
if /i "%MODE%"=="verbose" set ARGS=%ARGS% --verbose
if /i "%MODE%"=="benchmark" set ARGS=%ARGS% --benchmark
if not "%DEMO_TYPE%"=="all" set ARGS=%ARGS% --demo %DEMO_TYPE%

echo Running %CONFIG% configuration with %OMP_NUM_THREADS% threads...
if not "%ARGS%"=="" echo Mode: %MODE%
if not "%DEMO_TYPE%"=="all" echo Demo type: %DEMO_TYPE%
echo.

echo === Program Output ===
echo.

:: Run the executable with environment variable set
"%EXE_PATH%" %ARGS%

:: Check if program ran successfully
if %ERRORLEVEL% EQU 0 (
    echo.
    echo Program completed successfully!
) else (
    echo.
    echo Program exited with error code: %ERRORLEVEL%
)

:: Clean up copied executables
echo.
echo Cleaning up copied executables...
for %%F in (basic_tasks.exe fibonacci.exe quicksort.exe tree_traversal.exe graph_processing.exe task_dependencies.exe task_priority.exe taskloop.exe taskgroup.exe task_visualizer.exe task_stealing.exe nested_tasks.exe task_throttling.exe heterogeneous_tasks.exe) do (
    if exist "%%F" del "%%F" > nul 2>&1
)

echo.
exit /b 0