@echo off
setlocal enabledelayedexpansion

echo === OpenMP Scheduling Strategies Run Script ===
echo.

:: Process command line parameters with defaults
set CONFIG=Release
set MODE=normal
set OMP_NUM_THREADS=4
set SCHEDULE=default

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
if /i "%~1"=="--schedule" (
    set SCHEDULE=%~2
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
echo   --schedule TYPE       Set scheduling type (static, dynamic, guided, auto)
echo   --help                Show this help message
echo.
echo Examples:
echo   run.bat --debug --verbose
echo   run.bat --release --benchmark --threads 8
echo   run.bat --schedule dynamic --threads 4
exit /b 0

:continue_execution
:: Check if build directory exists
if not exist build (
    echo Error: Build directory not found.
    echo Please run configure.bat and build_all.bat first.
    exit /b 1
)

:: Define executable path based on configuration
set "EXE_PATH=build\%CONFIG%\OpenMP_SchedulingStrategies.exe"

:: Check if executable exists
if not exist "%EXE_PATH%" (
    echo.
    echo Executable not found: %EXE_PATH%
    echo Please build the project first by running build_all.bat.
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
if /i "%MODE%"=="quick" set ARGS=%ARGS% --quick
if /i "%MODE%"=="benchmark" set ARGS=%ARGS% --benchmark
if not "%SCHEDULE%"=="default" set ARGS=%ARGS% --schedule %SCHEDULE%

echo Running %CONFIG% configuration with %OMP_NUM_THREADS% threads...
if not "%ARGS%"=="" echo Mode: %MODE%
if not "%SCHEDULE%"=="default" echo Schedule: %SCHEDULE%
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

echo.
exit /b 0 