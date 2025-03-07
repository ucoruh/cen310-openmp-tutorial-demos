@echo off
echo === OpenMP Nested Parallelism and Affinity Configure Script ===

:: Create build directory if it doesn't exist
if not exist build (
    echo Creating build directory...
    mkdir build
)

:: Navigate to build directory
cd build

:: Run CMake configuration with Visual Studio 2022 generator
echo Running CMake configuration...
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_CXX_STANDARD=17

:: Check if CMake configuration was successful
if %ERRORLEVEL% EQU 0 (
    echo.
    echo Configuration completed successfully!
    echo.
    echo Next steps:
    echo 1. Run build_all.bat to compile the project
    echo 2. Run run.bat to execute the program
) else (
    echo.
    echo Configuration failed. Please check the error messages above.
)

cd ..

:: Pause to see the output
pause