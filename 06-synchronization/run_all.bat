@echo off
echo === OpenMP Synchronization - Running All Examples ===
echo.

echo This script will run all synchronization examples in sequence with different configurations.
echo Press Ctrl+C to abort or any key to continue...
pause > nul

echo === 1. Running Quick Demo (Release) ===
call run.bat --release --quick
if %ERRORLEVEL% EQU 999 (
    echo.
    echo User requested to exit. Stopping all examples.
    exit /b 0
)
if %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%
echo.

echo === 2. Running Individual Synchronization Types ===
echo --- Critical Section Example ---
call run.bat --release --sync critical --threads 4
if %ERRORLEVEL% EQU 999 (
    echo.
    echo User requested to exit. Stopping all examples.
    exit /b 0
)
if %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%
echo.
echo --- Atomic Operations Example ---
call run.bat --release --sync atomic --threads 4
if %ERRORLEVEL% EQU 999 (
    echo.
    echo User requested to exit. Stopping all examples.
    exit /b 0
)
if %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%
echo.
echo --- Ordered Execution Example ---
call run.bat --release --sync ordered --threads 4
if %ERRORLEVEL% EQU 999 (
    echo.
    echo User requested to exit. Stopping all examples.
    exit /b 0
)
if %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%
echo.
echo --- Barrier Example ---
call run.bat --release --sync barrier --threads 4
if %ERRORLEVEL% EQU 999 (
    echo.
    echo User requested to exit. Stopping all examples.
    exit /b 0
)
if %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%
echo.
echo --- Nowait Example ---
call run.bat --release --sync nowait --threads 4
if %ERRORLEVEL% EQU 999 (
    echo.
    echo User requested to exit. Stopping all examples.
    exit /b 0
)
if %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%
echo.

echo === 3. Running Debug Examples with Verbose Output ===
call run.bat --debug --verbose
if %ERRORLEVEL% EQU 999 (
    echo.
    echo User requested to exit. Stopping all examples.
    exit /b 0
)
if %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%
echo.

echo === 4. Running Performance Benchmarks ===
call run.bat --release --benchmark --threads 4
if %ERRORLEVEL% EQU 999 (
    echo.
    echo User requested to exit. Stopping all examples.
    exit /b 0
)
if %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%
echo.

echo === 5. Running with Different Thread Counts ===
echo --- 2 Threads ---
call run.bat --release --threads 2
if %ERRORLEVEL% EQU 999 (
    echo.
    echo User requested to exit. Stopping all examples.
    exit /b 0
)
if %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%
echo.
echo --- 8 Threads (if your CPU supports it) ---
call run.bat --release --threads 8
if %ERRORLEVEL% EQU 999 (
    echo.
    echo User requested to exit. Stopping all examples.
    exit /b 0
)
if %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%
echo.

echo === All demonstrations completed ===
echo. 