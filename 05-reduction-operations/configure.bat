@echo off
echo ===================================
echo   OpenMP Reduction - Configure
echo ===================================

:: Check if CMake is installed
where cmake >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo Error: CMake is not installed or not in the PATH.
    echo Please install CMake from https://cmake.org/download/
    exit /b 1
)

:: Create build directory if it doesn't exist
if not exist build (
    mkdir build
    if %ERRORLEVEL% neq 0 (
        echo Error: Failed to create build directory.
        exit /b 1
    )
)

:: Configure the project with CMake
cd build
echo Configuring the project with CMake...
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_CXX_STANDARD=17
if %ERRORLEVEL% neq 0 (
    echo Error: CMake configuration failed.
    cd ..
    exit /b 1
)

:: Return to project root
cd ..

echo.
echo Configuration successful!
echo.
echo Next steps:
echo 1. Run 'build_all.bat' to compile the project
echo 2. Run 'run.bat' to execute the program with options
echo 3. Run 'run_all.bat' to execute all examples in sequence
echo.
echo === Configuration Complete === 