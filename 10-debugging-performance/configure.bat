@echo off
echo === OpenMP Debugging and Performance Analysis Configure Script ===

:: Check Windows version
ver | find "10.0." > nul
if %ERRORLEVEL% EQU 0 (
    echo [PASS] Windows 10 detected.
) else (
    ver | find "6.3." > nul
    if %ERRORLEVEL% EQU 0 (
        echo [PASS] Windows 8.1 detected.
    ) else (
        ver | find "6.2." > nul
        if %ERRORLEVEL% EQU 0 (
            echo [PASS] Windows 8 detected.
        ) else (
            ver | find "6.1." > nul
            if %ERRORLEVEL% EQU 0 (
                echo [PASS] Windows 7 detected.
            ) else (
                echo [WARN] Unknown Windows version. Setup may not work correctly.
            )
        )
    )
)

:: Check for Visual Studio
set VS_DETECTED=0
set VS_VERSION=

:: Check for Visual Studio 2022
reg query "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\VisualStudio\17.0" /ve >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    set VS_DETECTED=1
    set VS_VERSION=Visual Studio 17 2022
    echo [PASS] Visual Studio 2022 detected.
    goto VS_FOUND
)

:: Check for Visual Studio 2019
reg query "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\VisualStudio\16.0" /ve >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    set VS_DETECTED=1
    set VS_VERSION=Visual Studio 16 2019
    echo [PASS] Visual Studio 2019 detected.
    goto VS_FOUND
)

:: Check for Visual Studio 2017
reg query "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\VisualStudio\15.0" /ve >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    set VS_DETECTED=1
    set VS_VERSION=Visual Studio 15 2017
    echo [PASS] Visual Studio 2017 detected.
    goto VS_FOUND
)

:VS_NOT_FOUND
echo [FAIL] Visual Studio is not installed or not detected.
echo If you have Visual Studio installed, you can try specifying the generator manually:
echo cmake .. -G "Visual Studio XX YYYY" -A x64
goto CHECK_CMAKE

:VS_FOUND
:: Check for CMake
:CHECK_CMAKE
cmake --version > nul 2>&1
if %ERRORLEVEL% EQU 0 (
    for /f "tokens=3" %%i in ('cmake --version ^| find "version"') do set CMAKE_VERSION=%%i
    echo [PASS] CMake version %CMAKE_VERSION% detected.
) else (
    echo [FAIL] CMake is not installed or not detected.
    echo Please install CMake from https://cmake.org/download/
    exit /b 1
)

:: Create build directories
if not exist build (
    echo Creating build directory...
    mkdir build
)

cd build

:: Run CMake configuration
echo.
echo Running CMake configuration...

if %VS_DETECTED% EQU 1 (
    cmake .. -G "%VS_VERSION%" -A x64 -DCMAKE_CONFIGURATION_TYPES="Debug;Release;Profile" -DCMAKE_CXX_STANDARD=17
) else (
    :: Fallback to let CMake choose the generator
    echo Using default CMake generator since Visual Studio was not detected.
    cmake .. -DCMAKE_CONFIGURATION_TYPES="Debug;Release;Profile" -DCMAKE_CXX_STANDARD=17
)

:: Check if CMake configuration was successful
if %ERRORLEVEL% EQU 0 (
    echo.
    echo Configuration completed successfully!
    echo.
    echo Creating VS solution file with diagnostic options...
    
    :: Create a .natvis file for Visual Studio debugging
    echo Creating visualization files...
    if not exist OpenMP_Debugging_Performance.natvis (
        echo // Creating empty natvis file > OpenMP_Debugging_Performance.natvis
        echo [PASS] Created empty visualization file.
    )
    
    echo.
    echo Next steps:
    echo 1. Run build_all.bat to compile all configurations
    echo 2. Run run.bat to execute the example programs with options
    echo 3. Run run_all.bat to execute all examples in sequence
) else (
    echo.
    echo Configuration failed. Please check the error messages above.
)

cd ..