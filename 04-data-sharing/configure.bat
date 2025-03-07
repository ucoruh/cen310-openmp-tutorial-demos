@echo off
echo === OpenMP Data Sharing Demo Configuration ===

:: Check if CMake is installed
where cmake >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo Error: CMake is not found in your PATH.
    echo Please install CMake from https://cmake.org/download/
    echo Ensure it is added to your PATH during installation.
    pause
    exit /b 1
)

:: Create build directory if it doesn't exist
if not exist build (
    echo Creating build directory...
    mkdir build
) else (
    echo Build directory already exists.
)

:: Navigate to build directory
cd build

:: Run CMake configuration with Visual Studio generator
echo Configuring project with CMake...
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_CXX_STANDARD=17

:: Check if CMake configuration was successful
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo Error: CMake configuration failed!
    echo Please check the error messages above.
    cd ..
    pause
    exit /b 1
)

:: Return to project root
cd ..

echo.
echo Configuration completed successfully!
echo.
echo Next steps:
echo 1. Run build_all.bat to compile the project
echo 2. Run run.bat to execute examples with various options
echo 3. Run run_all.bat to execute all examples in sequence
echo.

:: Pause to see the output
pause 