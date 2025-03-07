@echo off
echo === OpenMP Task Parallelism Environment Setup ===

:: Check if VS2022 is installed
call check_vs2022.bat
if %ERRORLEVEL% NEQ 0 (
    echo Visual Studio 2022 check failed. Aborting environment setup.
    pause
    exit /b 1
)

:: Find Visual Studio 2022 installation path
for /f "usebackq tokens=*" %%i in (`vswhere -version "[17.0,18.0)" -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    set VS_PATH=%%i
)

echo.
echo Setting up environment variables...

:: Check if VS Developer Command Prompt is available
if exist "%VS_PATH%\Common7\Tools\VsDevCmd.bat" (
    echo Found Visual Studio Developer Command Prompt.
    
    :: Create a batch file that sets up the VS environment
    echo @echo off > temp_vcvars.bat
    echo call "%VS_PATH%\Common7\Tools\VsDevCmd.bat" -arch=x64 >> temp_vcvars.bat
    echo echo Environment variables set successfully. >> temp_vcvars.bat
    echo exit /b 0 >> temp_vcvars.bat
    
    :: Execute the temporary batch file
    call temp_vcvars.bat
    
    :: Clean up
    del temp_vcvars.bat
) else (
    echo Warning: Visual Studio Developer Command Prompt not found.
    echo Environment setup might be incomplete.
)

:: Check if CMake is installed
where /q cmake
if %ERRORLEVEL% NEQ 0 (
    echo Warning: CMake not found in PATH.
    echo Please make sure CMake is installed and added to your PATH.
) else (
    echo Found CMake: 
    cmake --version | findstr version
)

echo.
echo Environment setup completed.
echo.
echo Next steps:
echo 1. Run configure.bat to generate the project files
echo 2. Run build_all.bat to compile the project
echo 3. Run run_examples.bat to execute examples

pause
exit /b 0