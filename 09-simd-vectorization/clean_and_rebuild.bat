@echo off
echo === OpenMP SIMD Vectorization Clean and Rebuild Script ===
echo.
echo This script will perform a thorough clean and rebuild of the project.
echo.

REM First clean the project using PowerShell for thorough cleaning
echo Running advanced cleaning script...
powershell -ExecutionPolicy Bypass -File "clean_thorough.ps1"

REM Configure the project
echo.
echo Configuring project...
call configure.bat

REM Build the project
echo.
echo Building project...
call build_all.bat

echo.
echo Clean and rebuild process completed.
echo.
pause 