@echo off
echo === OpenMP SIMD Vectorization Prerequisite Check ===

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

:: Check if the processor supports necessary SIMD instructions
set SUPPORTS_SIMD=false

:: Create a temporary program to detect CPU features
echo #include ^<iostream^> > cpu_check.cpp
echo #include ^<vector^> >> cpu_check.cpp
echo #include ^<string^> >> cpu_check.cpp
echo int main() { >> cpu_check.cpp
echo     std::vector^<std::string^> features; >> cpu_check.cpp
echo #ifdef __SSE__ >> cpu_check.cpp
echo     features.push_back("SSE"); >> cpu_check.cpp
echo #endif >> cpu_check.cpp
echo #ifdef __SSE2__ >> cpu_check.cpp
echo     features.push_back("SSE2"); >> cpu_check.cpp
echo #endif >> cpu_check.cpp
echo #ifdef __AVX__ >> cpu_check.cpp
echo     features.push_back("AVX"); >> cpu_check.cpp
echo #endif >> cpu_check.cpp
echo #ifdef __AVX2__ >> cpu_check.cpp
echo     features.push_back("AVX2"); >> cpu_check.cpp
echo #endif >> cpu_check.cpp
echo     for(const auto^& feature : features) { >> cpu_check.cpp
echo         std::cout ^<^< feature ^<^< std::endl; >> cpu_check.cpp
echo     } >> cpu_check.cpp
echo     return 0; >> cpu_check.cpp
echo } >> cpu_check.cpp

:: Try to compile and run the CPU check program
where cl.exe > nul 2>&1
if %ERRORLEVEL% EQU 0 (
    cl.exe /EHsc /O2 /arch:AVX2 cpu_check.cpp /Fecpu_check.exe > nul 2>&1
    if exist cpu_check.exe (
        cpu_check.exe > cpu_features.txt
        
        :: Check for SSE2 (minimum requirement)
        findstr /i "SSE2" cpu_features.txt > nul
        if %ERRORLEVEL% EQU 0 (
            set SUPPORTS_SIMD=true
            echo [PASS] CPU supports SSE2 instructions (minimum requirement).
        ) else (
            echo [WARN] CPU may not support SSE2 instructions.
            echo        Performance of SIMD operations may be limited.
        )
        
        :: Check for AVX
        findstr /i "AVX" cpu_features.txt > nul
        if %ERRORLEVEL% EQU 0 (
            echo [PASS] CPU supports AVX instructions.
        ) else (
            echo [INFO] CPU does not support AVX instructions.
            echo        Some SIMD operations will use SSE2 instead.
        )
        
        :: Check for AVX2
        findstr /i "AVX2" cpu_features.txt > nul
        if %ERRORLEVEL% EQU 0 (
            echo [PASS] CPU supports AVX2 instructions.
        ) else (
            echo [INFO] CPU does not support AVX2 instructions.
            echo        Performance of SIMD operations may be limited.
        )
        
        :: Clean up temporary files
        del cpu_check.exe cpu_features.txt
    ) else (
        echo [WARN] Could not compile CPU check program.
        echo        Unable to verify CPU SIMD support.
    )
) else (
    echo [WARN] Could not check CPU SIMD support without compiler.
)

:: Clean up temporary files
if exist cpu_check.cpp del cpu_check.cpp
if exist cpu_check.obj del cpu_check.obj

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