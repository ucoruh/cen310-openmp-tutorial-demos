@echo off
echo === OpenMP SIMD Vectorization Run Script ===

:: Define paths for executables
set DEBUG_EXE=build\Debug\OpenMP_SIMD_Vectorization.exe
set RELEASE_EXE=build\Release\OpenMP_SIMD_Vectorization.exe

:: Check if build directory exists
if not exist build (
    echo Build directory does not exist. Please run configure.bat and build.bat first.
    pause
    exit /b 1
)

:: Prompt user to select configuration
echo.
echo Select configuration to run:
echo 1. Debug
echo 2. Release
echo.
set /p CONFIG_CHOICE="Enter your choice (1 or 2): "

if "%CONFIG_CHOICE%"=="1" (
    set "EXE_PATH=%DEBUG_EXE%"
    echo.
    echo Running Debug configuration...
) else if "%CONFIG_CHOICE%"=="2" (
    set "EXE_PATH=%RELEASE_EXE%"
    echo.
    echo Running Release configuration...
) else (
    echo.
    echo Invalid choice. Please enter 1 or 2.
    pause
    exit /b 1
)

:: Check if executable exists
if not exist "%EXE_PATH%" (
    echo.
    echo Executable not found: %EXE_PATH%
    echo Please build the project first by running build.bat.
    pause
    exit /b 1
)

:: Prompt for OpenMP environment variables
echo.
echo === OpenMP Environment Variables ===
echo (Leave blank to use default values)
echo.

:: Number of threads
set /p OMP_NUM_THREADS="Number of threads (default=auto): "
if not "%OMP_NUM_THREADS%"=="" (
    set "OMP_NUM_THREADS=%OMP_NUM_THREADS%"
)

echo.
echo === OpenMP Environment Settings ===
if not "%OMP_NUM_THREADS%"=="" (
    echo OMP_NUM_THREADS=%OMP_NUM_THREADS%
) else (
    echo Using default thread count
)
echo.

:: Run the executable
echo.
echo === Program Output ===
echo.

:: Record start time
set start_time=%time%

:: Run the executable with the environment variables set
"%EXE_PATH%"

:: Record end time and show total runtime
set end_time=%time%
echo.
echo Program finished at %end_time%
echo Started at %start_time%

:: Check if program ran successfully
if %ERRORLEVEL% EQU 0 (
    echo.
    echo Program completed successfully!
) else (
    echo.
    echo Program exited with error code: %ERRORLEVEL%
)

:: Pause to see the output
pause