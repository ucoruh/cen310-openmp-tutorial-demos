@echo off
echo === OpenMP SIMD Vectorization Performance Report Generator ===

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

:: Select report type
echo.
echo Select performance report type:
echo 1. Basic Report (quick benchmarks)
echo 2. Comprehensive Report (all benchmarks, may take longer)
echo 3. Thread Scaling Report (tests with different thread counts)
echo.
set /p REPORT_TYPE="Enter your choice (1-3, default=2): "

:: Set appropriate command-line arguments based on report type
if "%REPORT_TYPE%"=="1" (
    set ARGS=--benchmark --quick
    echo Generating Basic Performance Report...
) else if "%REPORT_TYPE%"=="3" (
    set ARGS=--benchmark --thread-scaling
    echo Generating Thread Scaling Report...
) else (
    set ARGS=--benchmark
    echo Generating Comprehensive Performance Report...
)

:: Run the executable with the appropriate flags
echo.
echo Running benchmarks... (This may take several minutes)
echo Results will be saved to benchmarks/performance_report.txt

:: Record start time
set start_time=%time%

:: Run the executable
"%RELEASE_EXE%" %ARGS%

:: Record end time and show total runtime
set end_time=%time%
echo.
echo Report generation finished at %end_time%
echo Started at %start_time%

:: Check if the report generation ran successfully
if %ERRORLEVEL% EQU 0 (
    echo.
    echo Performance report generated successfully!
    
    :: Check if performance report was generated
    if exist benchmarks\performance_report.txt (
        echo.
        echo Performance report saved to: benchmarks\performance_report.txt
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
    echo Report generation exited with error code: %ERRORLEVEL%
)

:: Create timestamp for comparison reports
for /f "tokens=1-4 delims=/ " %%a in ('date /t') do (
    set DATE=%%c%%b%%a
)
for /f "tokens=1-2 delims=: " %%a in ('time /t') do (
    set TIME=%%a%%b
)

:: Offer to save a timestamped copy of the report for comparison
if exist benchmarks\performance_report.txt (
    echo.
    echo Would you like to save a timestamped copy of this report for future comparison? (Y/N)
    set /p SAVE_COPY="Your choice: "
    
    if /i "%SAVE_COPY%"=="Y" (
        copy benchmarks\performance_report.txt "benchmarks\performance_report_%DATE%_%TIME%.txt" > nul
        echo.
        echo Report copied to benchmarks\performance_report_%DATE%_%TIME%.txt
    )
)

pause