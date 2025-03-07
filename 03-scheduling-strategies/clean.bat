@echo off
echo === OpenMP Scheduling Strategies Clean Script ===

:: Check for open processes
echo Checking for processes that might lock files...
taskkill /f /im OpenMP_SchedulingStrategies.exe 2>nul
timeout /t 1 /nobreak >nul

:: Check if build directory exists
if exist build (
    echo Cleaning build directory...
    
    :: First try to remove with rd (safer)
    rd /s /q build 2>nul
    
    :: If that fails, try with rmdir
    if exist build (
        echo Retrying with alternate method...
        rmdir /s /q build 2>nul
    )
    
    :: Final check
    if exist build (
        echo Warning: Could not completely remove the build directory.
        echo Some files may be locked by another process.
        echo Please close any applications that might be using these files and try again.
    ) else (
        echo Build directory successfully removed.
    )
) else (
    echo Build directory doesn't exist. Nothing to clean.
)

:: Check if bin directory exists
if exist bin (
    echo Cleaning bin directory...
    
    :: First try to remove with rd (safer)
    rd /s /q bin 2>nul
    
    :: If that fails, try with rmdir
    if exist bin (
        echo Retrying with alternate method...
        rmdir /s /q bin 2>nul
    )
    
    :: Final check
    if exist bin (
        echo Warning: Could not completely remove the bin directory.
        echo Some files may be locked by another process.
    ) else (
        echo Bin directory successfully removed.
    )
) else (
    echo Bin directory doesn't exist. Nothing to clean.
)

:: Check if reports directory exists
if exist reports (
    echo Cleaning reports directory...
    
    :: First try to remove with rd (safer)
    rd /s /q reports 2>nul
    
    :: If that fails, try with rmdir
    if exist reports (
        echo Retrying with alternate method...
        rmdir /s /q reports 2>nul
    )
    
    :: Final check
    if exist reports (
        echo Warning: Could not completely remove the reports directory.
        echo Some files may be locked by another process.
    ) else (
        echo Reports directory successfully removed.
    )
) else (
    echo Reports directory doesn't exist. Nothing to clean.
)

:: Also clean any additional temporary files that might be in the project root
echo Removing any additional temporary files...
if exist *.obj del *.obj
if exist *.pdb del *.pdb
if exist *.ilk del *.ilk
if exist *.exe del *.exe
if exist *.log del *.log

echo.
echo Clean operation completed.
echo.
echo To rebuild the project:
echo 1. Run configure.bat
echo 2. Run build_all.bat
echo 3. Run run.bat or run_all.bat

exit /b 0 