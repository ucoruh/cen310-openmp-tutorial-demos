@echo off
echo === OpenMP Parallel For Loops Configure Script ===

:: Create the build directory if it doesn't exist
if not exist build (
    echo Creating build directory...
    mkdir build
) else (
    echo Build directory already exists.
)

:: Navigate to the build directory
cd build

:: Run CMake configuration
echo Running CMake configuration...
:: Use consistent CMake approach
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_CXX_STANDARD=17

:: Check if CMake configuration was successful
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo Error: CMake configuration failed.
    cd ..
    exit /b 1
)

cd ..

echo.
echo Configuration completed successfully!
echo.
echo Next steps:
echo 1. Run build_all.bat to compile the project
echo 2. Run run.bat to execute the program
