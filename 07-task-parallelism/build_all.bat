@echo off
echo === OpenMP Task Parallelism Build Script ===

:: Check if build directory exists
if not exist build (
    echo Build directory does not exist. Please run configure.bat first.
    pause
    exit /b 1
)

cd build

:: Build Debug configuration
echo.
echo Building Debug configuration...
cmake --build . --config Debug

:: Check if Debug build was successful
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo Debug build failed. Please check the error messages above.
    cd ..
    pause
    exit /b 1
)

:: Build Release configuration
echo.
echo Building Release configuration...
cmake --build . --config Release

:: Check if Release build was successful
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo Release build failed. Please check the error messages above.
    cd ..
    pause
    exit /b 1
)

cd ..

echo.
echo Build completed successfully!
echo.
echo Debug executables can be found in: build\Debug\
echo Release executables can be found in: build\Release\
echo.
echo Main executable: OpenMP_TaskParallelism.exe
echo Example executables: basic_tasks.exe, fibonacci.exe, etc.
echo.
echo Next step: Run run_examples.bat to execute the examples

:: Pause to see the output
pause