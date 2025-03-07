@echo off
echo === OpenMP Debugging and Performance Analysis Tools Setup ===

set SETUP_SUCCESS=true

:: Create required environment variables
echo.
echo Setting up environment variables for diagnostic tools...

:: Set up symbol paths for debugging
setx _NT_SYMBOL_PATH "srv*C:\Symbols*https://msdl.microsoft.com/download/symbols" /M
if %ERRORLEVEL% NEQ 0 (
    echo [FAIL] Could not set system symbol path.
    set SETUP_SUCCESS=false
) else (
    echo [PASS] Symbol path configured successfully.
)

:: Check Visual Studio installation path
for /f "tokens=2*" %%a in ('reg query "HKLM\SOFTWARE\Microsoft\VisualStudio\SxS\VS7" /v "17.0" 2^>nul') do set "VS_PATH=%%b"

if not defined VS_PATH (
    echo [FAIL] Visual Studio 2022 installation path not found.
    set SETUP_SUCCESS=false
) else (
    echo [PASS] Visual Studio 2022 found at: %VS_PATH%
    
    :: Check for Concurrency Visualizer
    if exist "%VS_PATH%\Team Tools\Performance Tools\vsconcert.exe" (
        echo [PASS] Concurrency Visualizer found.
    ) else (
        echo [WARN] Concurrency Visualizer not found.
        echo        Consider installing it from Visual Studio Installer under Individual Components.
    )
    
    :: Check for Diagnostics Tools
    if exist "%VS_PATH%\Team Tools\DiagnosticsHub\DiagnosticsHub.exe" (
        echo [PASS] Diagnostics Hub found.
    ) else (
        echo [WARN] Diagnostics Hub not found.
        echo        Some diagnostics features might be unavailable.
    )
)

:: Check Intel VTune (optional)
if exist "C:\Program Files (x86)\Intel\oneAPI" (
    echo [PASS] Intel oneAPI detected. VTune integration may be available.
) else (
    echo [INFO] Intel oneAPI not detected. VTune integration will not be available.
    echo        This is optional but recommended for detailed CPU performance analysis.
)

:: Configure Windows Performance Toolkit (WPT)
where xperf.exe > nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [INFO] Windows Performance Toolkit not found.
    echo        Consider installing the Windows Assessment and Deployment Kit (Windows ADK)
    echo        to get access to Windows Performance Toolkit for ETW tracing.
) else (
    echo [PASS] Windows Performance Toolkit found.
    
    :: Set up ETW session
    echo Setting up ETW session for OpenMP diagnostics...
    xperf -start OpenMPSession -on Latency+PROC_THREAD+LOADER+DISK_IO+HARD_FAULTS+CSWITCH+INTERRUPT+DPC+PROFILE
    if %ERRORLEVEL% NEQ 0 (
        echo [WARN] Could not create ETW session. Run this script as Administrator.
    ) else (
        xperf -stop OpenMPSession
        echo [PASS] ETW session configuration tested successfully.
    )
)

:: Setup common directory locations
echo.
echo Setting up directory structure for diagnostic outputs...

:: Create required folders
if not exist "..\profiling" mkdir "..\profiling"
if not exist "..\reports" mkdir "..\reports"
if not exist "..\profiling\vsperf" mkdir "..\profiling\vsperf"
if not exist "..\profiling\etw" mkdir "..\profiling\etw"
if not exist "..\profiling\vtune" mkdir "..\profiling\vtune"
if not exist "..\profiling\custom" mkdir "..\profiling\custom"

echo [PASS] Diagnostic output directories created.

:: Create .natvis file for custom visualization in VS debugger
echo.
echo Creating custom visualizers for OpenMP constructs...

set NATVIS_FILE="..\OpenMP_Debugging_Performance.natvis"
echo ^<?xml version="1.0" encoding="utf-8"?^> > %NATVIS_FILE%
echo ^<AutoVisualizer xmlns="http://schemas.microsoft.com/vstudio/debugger/natvis/2010"^> >> %NATVIS_FILE%
echo   ^<Type Name="omp_lock_t"^> >> %NATVIS_FILE%
echo     ^<DisplayString^>OMP Lock {(void*)_Reserved1}^</DisplayString^> >> %NATVIS_FILE%
echo   ^</Type^> >> %NATVIS_FILE%
echo ^</AutoVisualizer^> >> %NATVIS_FILE%

echo [PASS] Custom visualizer file created at %NATVIS_FILE%

echo.
if "%SETUP_SUCCESS%"=="true" (
    echo Setup completed successfully!
) else (
    echo Setup completed with some issues. Please review the warnings above.
)

echo.
echo Next step: Run configure.bat to configure the build