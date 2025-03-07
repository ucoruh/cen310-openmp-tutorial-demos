@echo off
echo === OpenMP Task Parallelism Benchmarks Runner ===

:: Check if build directory exists
if not exist build (
    echo Build directory does not exist. Please run configure.bat and build_all.bat first.
    pause
    exit /b 1
)

:: Define paths
set RELEASE_DIR=build\Release
set BENCHMARK_EXE=%RELEASE_DIR%\benchmark_suite.exe

:: Check if executable exists
if not exist "%BENCHMARK_EXE%" (
    echo.
    echo Benchmark executable not found: %BENCHMARK_EXE%
    echo Please build the project first by running build_all.bat.
    pause
    exit /b 1
)

echo.
echo Running performance benchmarks (Release mode)...
echo This may take several minutes, please be patient.
echo.
echo === Benchmark Output ===
echo.

:: Run the benchmark executable
"%BENCHMARK_EXE%"

:: Check if benchmarks ran successfully
if %ERRORLEVEL% EQU 0 (
    echo.
    echo Benchmarks completed successfully!
    echo.
    echo Results can be found in the 'benchmark_results' directory.
    echo Run generate_reports.bat to create comparative performance reports.
) else (
    echo.
    echo Benchmarks exited with error code: %ERRORLEVEL%
)

pause
exit /b 0