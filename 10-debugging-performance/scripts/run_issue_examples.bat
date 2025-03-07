@echo off
echo === OpenMP Debugging and Performance Analysis Issue Examples Runner ===

:: Define paths for executables
set DEBUG_PATH=..\build\bin\Debug
set RELEASE_PATH=..\build\bin\Release
set PROFILE_PATH=..\build\bin\Profile
set REPORTS_PATH=..\reports

:: Check if executables exist
if not exist "%DEBUG_PATH%\race_conditions.exe" (
    echo Executables not found. Please run build_all.bat first.
    exit /b 1
)

:: Create reports directory if it doesn't exist
if not exist "%REPORTS_PATH%" mkdir "%REPORTS_PATH%"

:: Display menu for issue categories
echo.
echo Select OpenMP issue category to demonstrate:
echo 1. Race Conditions
echo 2. False Sharing
echo 3. Load Imbalance
echo 4. Excessive Synchronization
echo 5. Memory Issues
echo 6. All Issues (Sequential Run)
echo.
set /p ISSUE_CHOICE="Enter your choice (1-6): "

:: Get configuration to run
echo.
echo Select configuration to run:
echo 1. Debug (with additional checks, slower)
echo 2. Release (optimized, may hide some issues)
echo 3. Profile (optimized with debug symbols)
echo.
set /p CONFIG_CHOICE="Enter your choice (1-3): "

:: Set path based on selected configuration
if "%CONFIG_CHOICE%"=="1" (
    set EXEC_PATH=%DEBUG_PATH%
    echo Using Debug configuration.
) else if "%CONFIG_CHOICE%"=="2" (
    set EXEC_PATH=%RELEASE_PATH%
    echo Using Release configuration.
) else if "%CONFIG_CHOICE%"=="3" (
    set EXEC_PATH=%PROFILE_PATH%
    echo Using Profile configuration.
) else (
    echo Invalid choice. Using Debug configuration.
    set EXEC_PATH=%DEBUG_PATH%
)

:: Run with selected issue category
if "%ISSUE_CHOICE%"=="1" (
    call :RunRaceConditions
) else if "%ISSUE_CHOICE%"=="2" (
    call :RunFalseSharing
) else if "%ISSUE_CHOICE%"=="3" (
    call :RunLoadImbalance
) else if "%ISSUE_CHOICE%"=="4" (
    call :RunExcessiveSynchronization
) else if "%ISSUE_CHOICE%"=="5" (
    call :RunMemoryIssues
) else if "%ISSUE_CHOICE%"=="6" (
    call :RunAllIssues
) else (
    echo Invalid choice. Please try again.
    exit /b 1
)

echo.
echo All examples completed. Results are available in the %REPORTS_PATH% directory.
echo.
echo Next step: Review the generated reports or run run_diagnostics.bat for detailed analysis.

exit /b 0

:: Subroutines for running individual issue categories
:RunRaceConditions
echo.
echo Running Race Conditions Examples...
echo.

echo [1/2] Running problematic version...
"%EXEC_PATH%\race_conditions.exe" --threads=4 --iterations=1000 --report="%REPORTS_PATH%\race_conditions_problem.csv"

echo.
echo [2/2] Running fixed version...
"%EXEC_PATH%\race_conditions_fixed.exe" --threads=4 --iterations=1000 --report="%REPORTS_PATH%\race_conditions_fixed.csv"

echo.
echo Race Conditions example completed. Report files saved to:
echo - %REPORTS_PATH%\race_conditions_problem.csv
echo - %REPORTS_PATH%\race_conditions_fixed.csv
exit /b 0

:RunFalseSharing
echo.
echo Running False Sharing Examples...
echo.

echo [1/2] Running problematic version...
"%EXEC_PATH%\false_sharing.exe" --threads=4 --iterations=10000000 --report="%REPORTS_PATH%\false_sharing_problem.csv"

echo.
echo [2/2] Running fixed version...
"%EXEC_PATH%\false_sharing_fixed.exe" --threads=4 --iterations=10000000 --report="%REPORTS_PATH%\false_sharing_fixed.csv"

echo.
echo False Sharing example completed. Report files saved to:
echo - %REPORTS_PATH%\false_sharing_problem.csv
echo - %REPORTS_PATH%\false_sharing_fixed.csv
exit /b 0

:RunLoadImbalance
echo.
echo Running Load Imbalance Examples...
echo.

echo [1/2] Running problematic version...
"%EXEC_PATH%\load_imbalance.exe" --threads=4 --report="%REPORTS_PATH%\load_imbalance_problem.csv"

echo.
echo [2/2] Running fixed version...
"%EXEC_PATH%\load_imbalance_fixed.exe" --threads=4 --report="%REPORTS_PATH%\load_imbalance_fixed.csv"

echo.
echo Load Imbalance example completed. Report files saved to:
echo - %REPORTS_PATH%\load_imbalance_problem.csv
echo - %REPORTS_PATH%\load_imbalance_fixed.csv
exit /b 0

:RunExcessiveSynchronization
echo.
echo Running Excessive Synchronization Examples...
echo.

echo [1/2] Running problematic version...
"%EXEC_PATH%\excessive_synchronization.exe" --threads=4 --elements=1000000 --report="%REPORTS_PATH%\excessive_synchronization_problem.csv"

echo.
echo [2/2] Running fixed version...
"%EXEC_PATH%\excessive_synchronization_fixed.exe" --threads=4 --elements=1000000 --report="%REPORTS_PATH%\excessive_synchronization_fixed.csv"

echo.
echo Excessive Synchronization example completed. Report files saved to:
echo - %REPORTS_PATH%\excessive_synchronization_problem.csv
echo - %REPORTS_PATH%\excessive_synchronization_fixed.csv
exit /b 0

:RunMemoryIssues
echo.
echo Running Memory Issues Examples...
echo.

echo [1/2] Running problematic version...
"%EXEC_PATH%\memory_issues.exe" --threads=4 --size=4096 --report="%REPORTS_PATH%\memory_issues_problem.csv"

echo.
echo [2/2] Running fixed version...
"%EXEC_PATH%\memory_issues_fixed.exe" --threads=4 --size=4096 --report="%REPORTS_PATH%\memory_issues_fixed.csv"

echo.
echo Memory Issues example completed. Report files saved to:
echo - %REPORTS_PATH%\memory_issues_problem.csv
echo - %REPORTS_PATH%\memory_issues_fixed.csv
exit /b 0

:RunAllIssues
echo.
echo Running all issue examples sequentially...
echo This may take some time...

call :RunRaceConditions
call :RunFalseSharing
call :RunLoadImbalance
call :RunExcessiveSynchronization
call :RunMemoryIssues

:: Generate summary report
echo.
echo Generating summary report comparing all issues...

:: Create comparative CSV report
echo Issue,Problematic Time (ms),Fixed Time (ms),Speedup > "%REPORTS_PATH%\summary_report.csv"
for %%i in (race_conditions false_sharing load_imbalance excessive_synchronization memory_issues) do (
    for /f "tokens=2 delims=," %%j in ('type "%REPORTS_PATH%\%%i_problem.csv" ^| findstr Time') do (
        set PROBLEM_TIME=%%j
    )
    for /f "tokens=2 delims=," %%k in ('type "%REPORTS_PATH%\%%i_fixed.csv" ^| findstr Time') do (
        set FIXED_TIME=%%k
    )
    echo %%i,!PROBLEM_TIME!,!FIXED_TIME!,=B2/C2 >> "%REPORTS_PATH%\summary_report.csv"
)

echo.
echo All issues examples completed. Summary report saved to:
echo - %REPORTS_PATH%\summary_report.csv
exit /b 0