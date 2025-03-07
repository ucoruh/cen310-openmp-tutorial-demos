@echo off
echo === OpenMP Nested Parallelism ^& Affinity Prerequisite Check ===

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

echo.
if "%PREREQS_MET%"=="true" (
    echo All prerequisites are met! You can proceed with the project.
) else (
    echo Some prerequisites are not met. Please address the issues above before proceeding.
)

echo.
echo Press any key to exit...
pause > nul