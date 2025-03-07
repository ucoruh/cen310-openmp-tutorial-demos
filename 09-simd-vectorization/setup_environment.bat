@echo off
echo === OpenMP SIMD Vectorization Environment Setup ===

:: Check for Administrator privileges
net session >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo WARNING: This script is not running with Administrator privileges.
    echo Some settings might not be applied correctly.
    echo.
    echo It is recommended to run this script as Administrator.
    echo.
    echo Do you want to continue anyway? (Y/N)
    set /p CONTINUE_CHOICE="Your choice: "
    if /i NOT "%CONTINUE_CHOICE%"=="Y" (
        echo Setup cancelled.
        pause
        exit /b 1
    )
)

echo.
echo This script will set up the optimal environment for OpenMP SIMD development.
echo.

:: Find Visual Studio installation
set VS_PATH=

for /f "tokens=1,2*" %%i in ('reg query "HKLM\SOFTWARE\Microsoft\VisualStudio\SxS\VS7" /v "17.0" 2^>nul') do (
    if "%%i"=="17.0" (
        set VS_PATH=%%k
    )
)

if "%VS_PATH%"=="" (
    echo Error: Could not find Visual Studio 2022 installation.
    echo Please make sure Visual Studio 2022 is installed.
    pause
    exit /b 1
)

echo Found Visual Studio 2022 at: %VS_PATH%

:: Set up Visual Studio environment
echo.
echo Setting up Visual Studio environment...

set VS_ENV_SCRIPT="%VS_PATH%Common7\Tools\VsDevCmd.bat"
if exist %VS_ENV_SCRIPT% (
    call %VS_ENV_SCRIPT% -arch=amd64
    echo Visual Studio Developer Command Prompt environment loaded.
) else (
    echo Error: Could not find Visual Studio environment setup script.
    echo Manual setup may be required.
)

:: Set up OpenMP environment variables
echo.
echo Configuring OpenMP environment variables...

:: Determine optimal thread count based on physical cores
for /f "tokens=2 delims==" %%i in ('wmic cpu get NumberOfCores /value ^| findstr NumberOfCores') do (
    set NUM_CORES=%%i
)

echo Detected %NUM_CORES% physical CPU cores.

:: Ask user for OpenMP settings
echo.
echo Please configure OpenMP settings:
echo.

:: Thread Count
echo Enter the number of threads for OpenMP (default: %NUM_CORES%)
echo [Enter a number or press Enter for default]
set /p OMP_THREAD_COUNT="> "

if "%OMP_THREAD_COUNT%"=="" (
    set OMP_THREAD_COUNT=%NUM_CORES%
)

:: Thread Binding
echo.
echo Select thread binding policy:
echo 1. close (bind threads close to each other - better for cache sharing)
echo 2. spread (distribute threads evenly - better for bandwidth)
echo 3. none (let system decide)
echo [Enter a number or press Enter for default (close)]
set /p BINDING_CHOICE="> "

if "%BINDING_CHOICE%"=="1" (
    set OMP_PROC_BIND=close
) else if "%BINDING_CHOICE%"=="2" (
    set OMP_PROC_BIND=spread
) else if "%BINDING_CHOICE%"=="3" (
    set OMP_PROC_BIND=false
) else (
    set OMP_PROC_BIND=close
)

:: Apply temporary settings for current session
echo.
echo Applying environment settings for current session...

set OMP_NUM_THREADS=%OMP_THREAD_COUNT%
echo OMP_NUM_THREADS=%OMP_NUM_THREADS%

set OMP_PROC_BIND=%OMP_PROC_BIND%
echo OMP_PROC_BIND=%OMP_PROC_BIND%

:: Ask about permanent settings
echo.
echo Would you like to set these OpenMP variables permanently? (Y/N)
echo This will modify system environment variables.
set /p PERMANENT_CHOICE="Your choice: "

if /i "%PERMANENT_CHOICE%"=="Y" (
    echo.
    echo Setting permanent environment variables...
    
    setx OMP_NUM_THREADS %OMP_THREAD_COUNT% > nul
    setx OMP_PROC_BIND %OMP_PROC_BIND% > nul
    
    echo Permanent environment variables set!
    echo You may need to restart your command prompt or IDE for these changes to take effect.
)

:: Create directories if they don't exist
echo.
echo Creating project directories...

if not exist build mkdir build
if not exist benchmarks mkdir benchmarks
if not exist asm_output mkdir asm_output
if not exist include mkdir include
if not exist src mkdir src

echo.
echo Environment setup completed!
echo.
echo To build the project:
echo 1. Run configure.bat
echo 2. Run build.bat
echo.
echo To run the program:
echo - Run run.bat

pause