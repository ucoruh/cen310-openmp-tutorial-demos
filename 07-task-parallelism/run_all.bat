@echo off
echo === OpenMP Task Parallelism - Running All Examples ===
echo.

echo This script will run all task parallelism demos in sequence with different configurations.
echo Note: Executables will be temporarily copied to the working directory for proper execution.
echo Press Ctrl+C to abort or any key to continue...
pause > nul

echo === 1. Running Basic Task Examples ===
call run.bat --demo basic --threads 4
echo.

echo === 2. Running Recursive Task Examples ===
call run.bat --demo recursive --threads 4
echo.

echo === 3. Running Task Dependency Examples ===
call run.bat --demo dependency --threads 4
echo.

echo === 4. Running Task Priority Examples ===
call run.bat --demo priority --threads 4
echo.

echo === 5. Running Performance Benchmarks ===
echo --- Running with different thread counts ---
echo --- 2 Threads ---
call run.bat --benchmark --threads 2
echo.
echo --- 4 Threads ---
call run.bat --benchmark --threads 4
echo.
echo --- 8 Threads (if your CPU supports it) ---
call run.bat --benchmark --threads 8
echo.

echo === All demonstrations completed ===
echo.