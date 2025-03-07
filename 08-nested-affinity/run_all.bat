@echo off
echo === OpenMP Nested Parallelism and Affinity - Running All Examples ===
echo.

echo This script will run all nested parallelism and affinity demos in sequence with different configurations.
echo Press Ctrl+C to abort or any key to continue...
pause > nul

echo === 1. Running Basic Nested Parallelism Examples ===
call run.bat --demo nested --threads 4
echo.

echo === 2. Running Thread Affinity Examples ===
call run.bat --demo affinity --threads 4
echo.

echo === 3. Running Combined Nested and Affinity Examples ===
call run.bat --demo combined --threads 4
echo.

echo === 4. Running Performance Comparisons ===
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