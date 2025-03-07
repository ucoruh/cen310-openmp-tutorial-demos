@echo off
echo === OpenMP Data Sharing - Running All Examples ===
echo.

echo This script will run all data sharing demos in sequence with different configurations.
echo Press Ctrl+C to abort or any key to continue...
pause > nul

echo === 1. Running Quick Demo (Release) ===
call run.bat --release --quick
echo.

echo === 2. Running Individual Data Sharing Demos ===
echo --- Shared Variables Demo ---
call run.bat --release --demo shared --threads 4
echo.
echo --- Private Variables Demo ---
call run.bat --release --demo private --threads 4
echo.
echo --- Firstprivate Variables Demo ---
call run.bat --release --demo firstprivate --threads 4
echo.
echo --- Lastprivate Variables Demo ---
call run.bat --release --demo lastprivate --threads 4
echo.
echo --- Threadprivate Variables Demo ---
call run.bat --release --demo threadprivate --threads 4
echo.
echo --- Reduction Variables Demo ---
call run.bat --release --demo reduction --threads 4
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