@echo off
echo === OpenMP Debugging and Performance Analysis Diagnostics Runner ===

:: Define paths for executables
set DEBUG_PATH=..\build\bin\Debug
set RELEASE_PATH=..\build\bin\Release
set PROFILE_PATH=..\build\bin\Profile
set INSTRUMENTED_PATH=..\bin\instrumented
set REPORTS_PATH=..\reports

:: Check if executables exist
if not exist "%RELEASE_PATH%\race_conditions.exe" (
    echo Executables not found. Please run build_all.bat first.
    exit /b 1
)

:: Create reports directory if it doesn't exist
if not exist "%REPORTS_PATH%" mkdir "%REPORTS_PATH%"

:: Display menu for diagnostic tools
echo.
echo Select diagnostic tool to use:
echo 1. Visual Studio Performance Profiler (VSPerf)
echo 2. ETW Tracing (Windows Performance Recorder)
echo 3. Custom Profiler
echo 4. Concurrency Visualizer
echo 5. All Tools (Sequential Run)
echo.
set /p TOOL_CHOICE="Enter your choice (1-5): "

:: Run with Visual Studio Performance Profiler
if "%TOOL_CHOICE%"=="1" (
    echo.
    echo Running with Visual Studio Performance Profiler...
    
    :: Check if VSPerf is available
    where VSPerfCmd.exe > nul 2>&1
    if %ERRORLEVEL% NEQ 0 (
        echo [FAIL] VSPerfCmd.exe not found. Make sure you're running from a Developer Command Prompt.
        exit /b 1
    )
    
    :: Create output directory
    set VSPERF_OUTPUT=..\profiling\vsperf
    if not exist "%VSPERF_OUTPUT%" mkdir "%VSPERF_OUTPUT%"
    
    :: Start profiling
    echo Starting VSPerf Collection...
    VSPerfCmd.exe /start:sample /output:"%VSPERF_OUTPUT%\race_conditions.vsp"
    
    :: Run the application
    echo Running race_conditions with profiling...
    "%INSTRUMENTED_PATH%\race_conditions.exe" --threads=4 --iterations=1000
    
    :: Stop profiling
    VSPerfCmd.exe /shutdown
    
    echo.
    echo VSPerf collection completed. Results saved to %VSPERF_OUTPUT%\race_conditions.vsp
    echo To analyze results, open this file in Visual Studio Performance Analyzer.
)

:: Run with ETW Tracing
if "%TOOL_CHOICE%"=="2" (
    echo.
    echo Running with ETW Tracing (Windows Performance Recorder)...
    
    :: Check if WPR is available
    where wpr.exe > nul 2>&1
    if %ERRORLEVEL% NEQ 0 (
        echo [FAIL] wpr.exe not found. Make sure Windows Performance Toolkit is installed.
        exit /b 1
    )
    
    :: Create output directory
    set ETW_OUTPUT=..\profiling\etw
    if not exist "%ETW_OUTPUT%" mkdir "%ETW_OUTPUT%"
    
    :: Start ETW tracing
    echo Starting ETW Collection...
    wpr.exe -start CPU -start FileIO -start DiskIO
    
    :: Run the application
    echo Running race_conditions with ETW tracing...
    "%RELEASE_PATH%\race_conditions.exe" --threads=4 --iterations=1000
    
    :: Stop ETW tracing
    wpr.exe -stop "%ETW_OUTPUT%\race_conditions.etl"
    
    echo.
    echo ETW collection completed. Results saved to %ETW_OUTPUT%\race_conditions.etl
    echo To analyze results, open this file in Windows Performance Analyzer (WPA).
)

:: Run with Custom Profiler
if "%TOOL_CHOICE%"=="3" (
    echo.
    echo Running with Custom Profiler...
    
    :: Create output directory
    set CUSTOM_OUTPUT=..\profiling\custom
    if not exist "%CUSTOM_OUTPUT%" mkdir "%CUSTOM_OUTPUT%"
    
    :: Set environment variable for custom profiler output
    set OPENMP_PROFILER_OUTPUT=%CUSTOM_OUTPUT%\race_conditions.json
    
    :: Run the profiler example
    echo Running custom_profiler with race_conditions example...
    "%RELEASE_PATH%\custom_profiler.exe" --target=race_conditions --threads=4 --iterations=1000
    
    echo.
    echo Custom profiling completed. Results saved to %CUSTOM_OUTPUT%\race_conditions.json
    echo A summary report will be generated if available.
)

:: Run with Concurrency Visualizer
if "%TOOL_CHOICE%"=="4" (
    echo.
    echo Running with Concurrency Visualizer...
    
    :: Check if Concurrency Visualizer is available
    where CVCollectionCmd.exe > nul 2>&1
    if %ERRORLEVEL% NEQ 0 (
        echo [FAIL] CVCollectionCmd.exe not found.
        echo        Make sure Concurrency Visualizer is installed from Visual Studio Installer.
        exit /b 1
    )
    
    :: Create output directory
    set CV_OUTPUT=..\profiling\concurrency
    if not exist "%CV_OUTPUT%" mkdir "%CV_OUTPUT%"
    
    :: Run with Concurrency Visualizer
    echo Running race_conditions with Concurrency Visualizer...
    CVCollectionCmd.exe /file:"%CV_OUTPUT%\race_conditions.etl" /launch:"%RELEASE_PATH%\race_conditions.exe --threads=4 --iterations=1000"
    
    echo.
    echo Concurrency Visualizer collection completed.
    echo Results saved to %CV_OUTPUT%\race_conditions.etl
    echo To analyze results, open this file in Visual Studio Concurrency Visualizer.
)

:: Run all tools in sequence
if "%TOOL_CHOICE%"=="5" (
    echo.
    echo Running all diagnostic tools sequentially...
    echo This may take some time...
    
    :: Run with each tool (simplified commands)
    echo.
    echo 1/4: Running with VSPerf...
    call :RunVSPerf
    
    echo.
    echo 2/4: Running with ETW...
    call :RunETW
    
    echo.
    echo 3/4: Running with Custom Profiler...
    call :RunCustom
    
    echo.
    echo 4/4: Running with Concurrency Visualizer...
    call :RunConcurrency
    
    echo.
    echo All diagnostic runs completed.
)

:: Generate comparative report if needed
if exist "%REPORTS_PATH%\generate_report.py" (
    echo.
    echo Generating comparative performance report...
    
    where python.exe > nul 2>&1
    if %ERRORLEVEL% EQU 0 (
        python.exe "%REPORTS_PATH%\generate_report.py"
    ) else (
        echo [WARN] Python not found. Skipping report generation.
    )
)

echo.
echo All diagnostic tasks completed.
echo Results are available in the profiling\ directory.
echo.
echo Next step: Analyze the results or run run_issue_examples.bat to test specific issues.

exit /b 0

:: Subroutines for running individual tools
:RunVSPerf
set VSPERF_OUTPUT=..\profiling\vsperf
if not exist "%VSPERF_OUTPUT%" mkdir "%VSPERF_OUTPUT%"
where VSPerfCmd.exe > nul 2>&1
if %ERRORLEVEL% EQU 0 (
    VSPerfCmd.exe /start:sample /output:"%VSPERF_OUTPUT%\race_conditions.vsp"
    "%INSTRUMENTED_PATH%\race_conditions.exe" --threads=4 --iterations=1000 --quiet
    VSPerfCmd.exe /shutdown
    echo VSPerf collection completed.
) else (
    echo [SKIP] VSPerf not available. Skipping.
)
exit /b 0

:RunETW
set ETW_OUTPUT=..\profiling\etw
if not exist "%ETW_OUTPUT%" mkdir "%ETW_OUTPUT%"
where wpr.exe > nul 2>&1
if %ERRORLEVEL% EQU 0 (
    wpr.exe -start CPU -start FileIO -start DiskIO
    "%RELEASE_PATH%\race_conditions.exe" --threads=4 --iterations=1000 --quiet
    wpr.exe -stop "%ETW_OUTPUT%\race_conditions.etl"
    echo ETW collection completed.
) else (
    echo [SKIP] WPR not available. Skipping.
)
exit /b 0

:RunCustom
set CUSTOM_OUTPUT=..\profiling\custom
if not exist "%CUSTOM_OUTPUT%" mkdir "%CUSTOM_OUTPUT%"
set OPENMP_PROFILER_OUTPUT=%CUSTOM_OUTPUT%\race_conditions.json
"%RELEASE_PATH%\custom_profiler.exe" --target=race_conditions --threads=4 --iterations=1000 --quiet
echo Custom profiling completed.
exit /b 0

:RunConcurrency
set CV_OUTPUT=..\profiling\concurrency
if not exist "%CV_OUTPUT%" mkdir "%CV_OUTPUT%"
where CVCollectionCmd.exe > nul 2>&1
if %ERRORLEVEL% EQU 0 (
    CVCollectionCmd.exe /file:"%CV_OUTPUT%\race_conditions.etl" /launch:"%RELEASE_PATH%\race_conditions.exe --threads=4 --iterations=1000 --quiet"
    echo Concurrency Visualizer collection completed.
) else (
    echo [SKIP] Concurrency Visualizer not available. Skipping.
)
exit /b 0