@echo off
echo === OpenMP Nested Parallelism and Affinity Build Script ===

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
echo Debug executables:
echo - Main application: build\Debug\OpenMP_NestedParallelism_Affinity.exe
echo - Matrix multiplication: build\Debug\matrix_multiplication.exe
echo.
echo Release executables:
echo - Main application: build\Release\OpenMP_NestedParallelism_Affinity.exe
echo - Matrix multiplication: build\Release\matrix_multiplication.exe
echo.
echo Next step: Run run.bat to execute the program

:: Pause to see the output
pause