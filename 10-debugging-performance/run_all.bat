@echo off
echo === OpenMP Debugging and Performance Analysis - Running All Examples ===
echo.

echo This script will run all examples in sequence with different configurations.
echo Press Ctrl+C to abort or any key to continue...
pause > nul

echo === 1. Running Quick Demo (Release) ===
call run.bat --release --quick
echo.

echo === 2. Running Debug Examples ===
call run.bat --debug --verbose
echo.

echo === 3. Running Performance Benchmarks ===
call run.bat --release --benchmark
echo.

echo === All demonstrations completed ===
echo Check the 'reports' directory for generated visualizations and analysis results.
echo. 