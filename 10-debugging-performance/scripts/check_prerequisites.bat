@echo off
echo === OpenMP Debugging and Performance Analysis Prerequisites Check ===

set PREREQS_MET=true

echo.
echo Checking system requirements...

:: Check Windows version
ver | findstr /i "Version 10\." > nul
if %ERRORLEVEL% NEQ 0 (
    ver | findstr /i "Version 11\." > nul
    if %ERRORLEVEL% NEQ 0 (
        echo [FAIL] This project requires Windows 10 or Windows 11.
        set PREREQS_MET=false
    ) else (
        echo [PASS] Windows 11 detected.
    )
) else (
    echo [PASS] Windows 10 detected.
)

:: Check if Visual Studio 2022 is installed
reg query "HKLM\SOFTWARE\Microsoft\VisualStudio\SxS\VS7" /v "17.0" > nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [FAIL] Visual Studio 2022 is not installed or not detected.
    set PREREQS_MET=false
) else (
    echo [PASS] Visual Studio 2022 is installed.
    
    :: Check for required components
    reg query "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\VisualStudio\17.0\VC\Runtimes\x64" > nul 2>&1
    if %ERRORLEVEL% NEQ 0 (
        echo [WARN] Visual C++ components might not be installed correctly.
    ) else (
        echo [PASS] Visual C++ components detected.
    )
)

:: Check for CMake
cmake --version > nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [FAIL] CMake is not installed or not in the PATH.
    set PREREQS_MET=false
) else (
    for /f "tokens=3" %%i in ('cmake --version ^| findstr /i "version"') do (
        set CMAKE_VERSION=%%i
    )
    echo [PASS] CMake version %CMAKE_VERSION% detected.
    
    :: Check CMake version
    for /f "tokens=1 delims=." %%a in ("%CMAKE_VERSION%") do (
        if %%a LSS 3 (
            echo [FAIL] CMake version 3.20 or higher is required.
            set PREREQS_MET=false
        ) else (
            for /f "tokens=2 delims=." %%b in ("%CMAKE_VERSION%") do (
                if %%a EQU 3 (
                    if %%b LSS 20 (
                        echo [FAIL] CMake version 3.20 or higher is required.
                        set PREREQS_MET=false
                    )
                )
            )
        )
    )
)

:: Check for C++ compiler
where cl.exe > nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [FAIL] Microsoft C++ compiler (cl.exe) not found in PATH.
    echo        You may need to run the "Developer Command Prompt for VS 2022"
    echo        or run vcvarsall.bat to set up the environment.
    set PREREQS_MET=false
) else (
    echo [PASS] Microsoft C++ compiler (cl.exe) is accessible.
)

:: Check for VSPerf (Visual Studio Performance Tools)
where vsinstr.exe > nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [WARN] Visual Studio Performance Tools (vsinstr.exe) not found in PATH.
    echo        Some diagnostic features may be limited.
) else (
    echo [PASS] Visual Studio Performance Tools detected.
)

:: Check for Windows Performance Toolkit (optional)
where xperf.exe > nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [INFO] Windows Performance Toolkit (xperf.exe) not found in PATH.
    echo        This is optional but recommended for ETW tracing.
) else (
    echo [PASS] Windows Performance Toolkit detected.
)

:: Check system processor capabilities
echo.
echo Checking processor capabilities...

:: Check number of logical processors
for /f "tokens=2 delims==" %%i in ('wmic cpu get NumberOfLogicalProcessors /value ^| findstr NumberOfLogicalProcessors') do (
    set NUM_LOGICAL_PROCESSORS=%%i
)

if %NUM_LOGICAL_PROCESSORS% LEQ 1 (
    echo [WARN] Only %NUM_LOGICAL_PROCESSORS% logical processor detected.
    echo        This project is designed to demonstrate parallel execution
    echo        and will not show the benefits of parallelism on a single-core system.
) else (
    echo [PASS] %NUM_LOGICAL_PROCESSORS% logical processors detected.
)

:: Check for NUMA architecture (optional)
wmic cpu get NumaNodeCount 2>nul | findstr /r "[1-9]" > nul
if %ERRORLEVEL% NEQ 0 (
    echo [INFO] NUMA architecture not detected. Some memory locality examples 
    echo        may not demonstrate expected behavior on this system.
) else (
    echo [PASS] NUMA architecture detected. Memory locality examples will work correctly.
)

echo.
if "%PREREQS_MET%"=="true" (
    echo All critical prerequisites are met! You can proceed with the project.
) else (
    echo Some prerequisites are not met. Please address the issues above before proceeding.
)

echo.