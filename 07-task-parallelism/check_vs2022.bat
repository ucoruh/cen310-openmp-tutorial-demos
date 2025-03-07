@echo off
echo === Visual Studio 2022 Components Check ===
echo.

:: Check for Visual Studio 2022 installation
where /q vswhere
if %ERRORLEVEL% NEQ 0 (
    echo Error: vswhere utility not found. Cannot detect Visual Studio installation.
    echo Please make sure Visual Studio 2022 is properly installed.
    pause
    exit /b 1
)

:: Use vswhere to check for VS2022 installation
echo Checking for Visual Studio 2022 installation...
set VS2022_FOUND=0
for /f "usebackq tokens=*" %%i in (`vswhere -version "[17.0,18.0)" -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    set VS_PATH=%%i
    set VS2022_FOUND=1
)

if %VS2022_FOUND% EQU 0 (
    echo.
    echo Error: Visual Studio 2022 with C++ Desktop Development workload not found.
    echo Please install Visual Studio 2022 with the required components.
    pause
    exit /b 1
) else (
    echo Visual Studio 2022 found at: %VS_PATH%
)

:: Check if MSBuild is available
echo.
echo Checking for MSBuild...
where /q MSBuild
if %ERRORLEVEL% NEQ 0 (
    echo Error: MSBuild not found in PATH.
    echo Please make sure Visual Studio 2022 is properly installed with Build Tools.
    pause
    exit /b 1
) else (
    echo MSBuild found.
)

:: Check for OpenMP support
echo.
echo Checking for OpenMP support...
set VC_TOOLS_PATH=%VS_PATH%\VC\Tools\MSVC
if exist "%VC_TOOLS_PATH%" (
    echo Visual C++ Tools directory found.
    :: Check if omp.h exists in the include directory
    for /d %%d in ("%VC_TOOLS_PATH%\*") do (
        if exist "%%d\include\omp.h" (
            echo OpenMP header (omp.h) found.
            set OPENMP_FOUND=1
        )
    )
    
    if not defined OPENMP_FOUND (
        echo Warning: OpenMP header (omp.h) not found in the expected location.
        echo OpenMP support might be missing.
    )
) else (
    echo Warning: Could not find Visual C++ Tools directory.
    echo Unable to verify OpenMP support.
)

echo.
echo All checks completed.
echo Next step: Run setup_environment.bat to configure the environment.
pause
exit /b 0