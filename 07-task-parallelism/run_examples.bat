@echo off
echo === OpenMP Task Parallelism Example Runner ===

:: Check if build directory exists
if not exist build (
    echo Build directory does not exist. Please run configure.bat and build_all.bat first.
    pause
    exit /b 1
)

:: Define paths - check both build and bin directories
set DEBUG_DIR=build\Debug
set RELEASE_DIR=build\Release

:: Check if bin directory exists as an alternative
if exist bin\Debug (
    set DEBUG_DIR=bin\Debug
)
if exist bin\Release (
    set RELEASE_DIR=bin\Release
)

:: Prompt user to select configuration
echo.
echo Select configuration to run:
echo 1. Debug
echo 2. Release
echo.
set /p CONFIG_CHOICE="Enter your choice (1 or 2): "

if "%CONFIG_CHOICE%"=="1" (
    set "EXEC_DIR=%DEBUG_DIR%"
    echo.
    echo Running examples in Debug configuration...
) else if "%CONFIG_CHOICE%"=="2" (
    set "EXEC_DIR=%RELEASE_DIR%"
    echo.
    echo Running examples in Release configuration...
) else (
    echo.
    echo Invalid choice. Please enter 1 or 2.
    pause
    exit /b 1
)

:: Menu for selecting which example to run
:menu
cls
echo.
echo === OpenMP Task Parallelism Examples ===
echo.
echo Select an example to run:
echo 1. Basic Tasks Example
echo 2. Fibonacci Example
echo 3. Quicksort Example
echo 4. Tree Traversal Example
echo 5. Graph Processing Example
echo 6. Task Dependencies Example
echo 7. Task Priority Example
echo 8. Taskloop Example
echo 9. Taskgroup Example
echo 10. Task Visualizer
echo 11. Advanced: Task Stealing
echo 12. Advanced: Nested Tasks
echo 13. Advanced: Task Throttling
echo 14. Benchmark Suite
echo 15. Main Program (all examples)
echo 0. Exit
echo.
echo Note: Heterogeneous Tasks example is currently disabled due to compiler issues.
echo.
set /p EXAMPLE_CHOICE="Enter your choice (0-15): "

:: Execute the selected example
if "%EXAMPLE_CHOICE%"=="0" goto end
if "%EXAMPLE_CHOICE%"=="1" set "EXEC_NAME=basic_tasks.exe"
if "%EXAMPLE_CHOICE%"=="2" set "EXEC_NAME=fibonacci.exe"
if "%EXAMPLE_CHOICE%"=="3" set "EXEC_NAME=quicksort.exe"
if "%EXAMPLE_CHOICE%"=="4" set "EXEC_NAME=tree_traversal.exe"
if "%EXAMPLE_CHOICE%"=="5" set "EXEC_NAME=graph_processing.exe"
if "%EXAMPLE_CHOICE%"=="6" set "EXEC_NAME=task_dependencies.exe"
if "%EXAMPLE_CHOICE%"=="7" set "EXEC_NAME=task_priority.exe"
if "%EXAMPLE_CHOICE%"=="8" set "EXEC_NAME=taskloop.exe"
if "%EXAMPLE_CHOICE%"=="9" set "EXEC_NAME=taskgroup.exe"
if "%EXAMPLE_CHOICE%"=="10" set "EXEC_NAME=task_visualizer.exe"
if "%EXAMPLE_CHOICE%"=="11" set "EXEC_NAME=task_stealing.exe"
if "%EXAMPLE_CHOICE%"=="12" set "EXEC_NAME=nested_tasks.exe"
if "%EXAMPLE_CHOICE%"=="13" set "EXEC_NAME=task_throttling.exe"
if "%EXAMPLE_CHOICE%"=="14" set "EXEC_NAME=benchmark_suite.exe"
if "%EXAMPLE_CHOICE%"=="15" set "EXEC_NAME=OpenMP_TaskParallelism.exe"

:: Check if executable exists
if not exist "%EXEC_DIR%\%EXEC_NAME%" (
    echo.
    echo Executable not found: %EXEC_DIR%\%EXEC_NAME%
    echo.
    echo Available executables in %EXEC_DIR%:
    dir /b "%EXEC_DIR%\*.exe" 2>nul
    echo.
    echo Please build the project first by running build_all.bat.
    pause
    goto menu
)

:: Copy all necessary executables to the working directory for main program
if "%EXAMPLE_CHOICE%"=="15" (
    echo.
    echo Copying all executables to the working directory...
    copy "%EXEC_DIR%\*.exe" . > nul 2>&1
    if errorlevel 1 (
        echo Warning: Could not copy all executables. Some examples may not run correctly.
    ) else (
        echo Executables copied successfully.
    )
)

:: Run the executable
echo.
echo === Running %EXEC_NAME% ===
echo.

:: Record start time
set start_time=%time%

:: Run the executable
"%EXEC_DIR%\%EXEC_NAME%"

:: Record end time and show total runtime
set end_time=%time%
echo.
echo Program finished at %end_time%
echo Started at %start_time%

:: Check if program ran successfully
if %ERRORLEVEL% EQU 0 (
    echo.
    echo Example completed successfully!
) else (
    echo.
    echo Example exited with error code: %ERRORLEVEL%
)

:: Clean up copied executables if main program was run
if "%EXAMPLE_CHOICE%"=="15" (
    echo.
    echo Cleaning up copied executables...
    for %%F in (basic_tasks.exe fibonacci.exe quicksort.exe tree_traversal.exe graph_processing.exe task_dependencies.exe task_priority.exe taskloop.exe taskgroup.exe task_visualizer.exe task_stealing.exe nested_tasks.exe task_throttling.exe heterogeneous_tasks.exe) do (
        if exist "%%F" del "%%F" > nul 2>&1
    )
)

:: Ask if user wants to run another example
echo.
echo Do you want to run another example?
echo 1. Yes
echo 2. No
echo.
set /p ANOTHER_CHOICE="Enter your choice (1 or 2): "

if "%ANOTHER_CHOICE%"=="1" goto menu

:end
echo.
echo Thank you for using the OpenMP Task Parallelism example runner!
pause
exit /b 0