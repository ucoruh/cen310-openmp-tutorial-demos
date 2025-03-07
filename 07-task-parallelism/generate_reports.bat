@echo off
echo === OpenMP Task Parallelism Report Generator ===

:: Check if benchmark results directory exists
if not exist benchmark_results (
    echo Benchmark results directory not found.
    echo Please run run_benchmarks.bat first to generate benchmark data.
    pause
    exit /b 1
)

:: Create reports directory if it doesn't exist
if not exist reports (
    echo Creating reports directory...
    mkdir reports
)

:: Check if executable exists
set REPORT_GEN_EXE=build\Release\OpenMP_TaskParallelism.exe

if not exist "%REPORT_GEN_EXE%" (
    echo.
    echo Report generator executable not found: %REPORT_GEN_EXE%
    echo Please build the project first by running build_all.bat.
    pause
    exit /b 1
)

echo.
echo Generating performance comparison reports...
echo.

:: Run the executable with report generation flag
"%REPORT_GEN_EXE%" --generate-reports

:: Check if report generation was successful
if %ERRORLEVEL% EQU 0 (
    echo.
    echo Report generation completed successfully!
    echo.
    echo Reports can be found in the 'reports' directory:
    echo - reports\task_performance_comparison.txt
    echo - reports\thread_scaling_analysis.txt
    echo - reports\granularity_impact_analysis.txt
) else (
    echo.
    echo Report generation failed with error code: %ERRORLEVEL%
)

pause
exit /b 0