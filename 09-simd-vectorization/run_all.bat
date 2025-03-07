@echo off
echo === OpenMP SIMD Vectorization Benchmark Runner ===

:: Define paths for release executable
set RELEASE_EXE=build\Release\OpenMP_SIMD_Vectorization.exe

:: Check if build directory exists
if not exist build (
    echo Build directory does not exist. Please run configure.bat and build.bat first.
    pause
    exit /b 1
)

:: Check if executable exists
if not exist "%RELEASE_EXE%" (
    echo.
    echo Release executable not found: %RELEASE_EXE%
    echo Please build the project first by running build.bat.
    pause
    exit /b 1
)

:: Create benchmarks directory if it doesn't exist
if not exist benchmarks (
    echo Creating benchmarks directory...
    mkdir benchmarks
)

:: Set optimal OpenMP environment variables for benchmarking
echo.
echo Setting OpenMP environment variables for optimal benchmarking...
echo.

:: Clear any existing OpenMP environment variables
set OMP_NUM_THREADS=
set OMP_PROC_BIND=
set OMP_SCHEDULE=

:: Prompt for number of threads
echo === Benchmark Setup ===
echo.
echo Enter the number of threads to use for benchmarks
echo (Leave blank to use all available cores):
set /p THREAD_COUNT=

if not "%THREAD_COUNT%"=="" (
    set OMP_NUM_THREADS=%THREAD_COUNT%
    echo Setting OMP_NUM_THREADS=%THREAD_COUNT%
) else (
    echo Using default thread count (all available cores)
)

:: Prompt for thread binding
echo.
echo Select thread binding policy:
echo 1. close (maximize cache reuse)
echo 2. spread (maximize core utilization)
echo 3. master (bind to master thread's core)
echo 4. none (let system decide)
echo.
set /p BINDING_CHOICE="Enter your choice (1-4, default=1): "

if "%BINDING_CHOICE%"=="1" (
    set OMP_PROC_BIND=close
    echo Setting OMP_PROC_BIND=close
) else if "%BINDING_CHOICE%"=="2" (
    set OMP_PROC_BIND=spread
    echo Setting OMP_PROC_BIND=spread
) else if "%BINDING_CHOICE%"=="3" (
    set OMP_PROC_BIND=master
    echo Setting OMP_PROC_BIND=master
) else if "%BINDING_CHOICE%"=="4" (
    set OMP_PROC_BIND=false
    echo Setting OMP_PROC_BIND=false
) else (
    set OMP_PROC_BIND=close
    echo Setting OMP_PROC_BIND=close (default)
)

echo.
echo === Running Benchmarks ===
echo.
echo Running all benchmarks... (This may take several minutes)
echo Results will be saved to benchmarks/performance_report.txt

:: Record start time
set start_time=%time%

:: Run the executable with the benchmark flag
"%RELEASE_EXE%" --benchmark

:: Record end time and show total runtime
set end_time=%time%
echo.
echo Benchmarks finished at %end_time%
echo Started at %start_time%

:: Check if benchmarks ran successfully
if %ERRORLEVEL% EQU 0 (
    echo.
    echo Benchmarks completed successfully!
    
    :: Check if performance report was generated
    if exist benchmarks\performance_report.txt (
        echo.
        echo Performance report generated: benchmarks\performance_report.txt
        echo.
        echo Would you like to view the report now? (Y/N)
        set /p VIEW_REPORT="Your choice: "
        
        if /i "%VIEW_REPORT%"=="Y" (
            notepad benchmarks\performance_report.txt
        )
    ) else (
        echo.
        echo Warning: Performance report file not found.
    )
) else (
    echo.
    echo Benchmarks exited with error code: %ERRORLEVEL%
)

pause