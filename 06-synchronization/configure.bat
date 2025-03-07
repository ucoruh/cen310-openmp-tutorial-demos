@echo off
echo ===================================
echo   OpenMP Synchronization - Configure
echo ===================================

:: Check if build directory exists, create if not
if not exist build (
    echo Creating build directory...
    mkdir build
)

:: Navigate to build directory
cd build

:: Configure CMake
echo Configuring project with CMake...
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_CXX_STANDARD=17

:: Check if CMake was successful
if %ERRORLEVEL% NEQ 0 (
    echo Error: CMake configuration failed.
    cd ..
    exit /b %ERRORLEVEL%
)

:: Return to the original directory
cd ..

echo.
echo Configuration complete!
echo.
echo Next steps:
echo - Run 'build_all.bat' to compile the project
echo - After building, run 'run.bat' to execute the program with options
echo - Or run 'run_all.bat' to execute all examples in sequence
echo.
