@echo off
echo === OpenMP Parallel For Loops - Running All Examples ===
echo.

echo This script will run all demonstrations in sequence with different configurations.
echo Press Ctrl+C to abort or any key to continue...
pause > nul

echo === 1. Running Quick Demo (Release) ===
call run.bat --release --quick
echo.

echo === 2. Running Debug Examples with Verbose Output ===
call run.bat --debug --verbose
echo.

echo === 3. Running Performance Benchmarks ===
call run.bat --release --benchmark --threads 4
echo.

echo === 4. Running with Different Thread Counts ===
echo --- 2 Threads ---
call run.bat --release --threads 2
echo.
echo --- 8 Threads (if your CPU supports it) ---
call run.bat --release --threads 8
echo.

echo === All demonstrations completed ===
echo. 