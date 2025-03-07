@echo off
echo === OpenMP Scheduling Strategies - Running All Examples ===
echo.

echo This script will run all scheduling strategies in sequence with different configurations.
echo Press Ctrl+C to abort or any key to continue...
pause > nul

echo === 1. Running Quick Demo (Release) ===
call run.bat --release --quick
echo.

echo === 2. Running Individual Scheduling Strategies ===
echo --- Static Schedule ---
call run.bat --release --schedule static --threads 4
echo.
echo --- Dynamic Schedule ---
call run.bat --release --schedule dynamic --threads 4
echo.
echo --- Guided Schedule ---
call run.bat --release --schedule guided --threads 4
echo.
echo --- Auto Schedule ---
call run.bat --release --schedule auto --threads 4
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
