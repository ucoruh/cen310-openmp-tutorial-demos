@echo off
echo === OpenMP Reduction Operations - Running All Examples ===
echo.

echo This script will run all reduction examples in sequence with different configurations.
echo Press Ctrl+C to abort or any key to continue...
pause > nul

echo === 1. Running Quick Demo (Release) ===
call run.bat --release --quick
echo.

echo === 2. Running Individual Reduction Types ===
echo --- Sum Reduction ---
call run.bat --release --reduction sum --threads 4
echo.
echo --- Product Reduction ---
call run.bat --release --reduction product --threads 4
echo.
echo --- Min/Max Reduction ---
call run.bat --release --reduction min --threads 4
echo.
call run.bat --release --reduction max --threads 4
echo.
echo --- Custom Reduction ---
call run.bat --release --reduction custom --threads 4
echo.

echo === 3. Running Debug Examples with Verbose Output ===
call run.bat --debug --verbose
echo.

echo === 4. Running Performance Benchmarks ===
call run.bat --release --benchmark --threads 4
echo.

echo === 5. Running with Different Thread Counts ===
echo --- 2 Threads ---
call run.bat --release --threads 2
echo.
echo --- 8 Threads (if your CPU supports it) ---
call run.bat --release --threads 8
echo.

echo === All demonstrations completed ===
echo. 