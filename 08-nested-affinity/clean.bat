@echo off
echo === OpenMP Nested Parallelism and Affinity Clean Script ===

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
echo 2. Run build.bat

:: Pause to see the output
pause