@echo off
echo === OpenMP Thread Affinity Tests ===

:: Define path for executable
set EXE_PATH=..\build\Release\OpenMP_NestedParallelism_Affinity.exe

:: Check if executable exists
if not exist "%EXE_PATH%" (
    echo.
    echo Executable not found: %EXE_PATH%
    echo Please build the project first by running build.bat from the project root.
    pause
    exit /b 1
)

echo.
echo This script will run the main program with different thread configurations
echo to demonstrate thread placement behavior on your system.
echo.

:: Run thread affinity demo
echo.
echo ===============================================
echo === Running Thread Affinity Demo ===
echo ===============================================
echo.

:: Set environment variables
set OMP_NESTED=1
set OMP_NUM_THREADS=4

echo OpenMP Environment Settings:
echo OMP_NESTED=%OMP_NESTED%
echo OMP_NUM_THREADS=%OMP_NUM_THREADS%
echo.

:: Run the program with affinity demo (demo 3)
"%EXE_PATH%" --demo=3

echo.
echo Next, we'll run the program with custom thread placement (demo 4)
echo to explicitly set thread affinities...
echo.
pause

echo.
echo ===============================================
echo === Running Custom Thread Placement Demo ===
echo ===============================================
echo.

:: Run the program with custom thread placement demo (demo 4)
"%EXE_PATH%" --demo=4

echo.
echo All affinity tests completed.
echo.
echo Note: In OpenMP 2.0, fine-grained control over thread affinity is limited.
echo For better control in newer OpenMP versions, use the -openmp:llvm
echo compiler option if you have Visual Studio 2019 version 16.9 or later.
echo.

pause