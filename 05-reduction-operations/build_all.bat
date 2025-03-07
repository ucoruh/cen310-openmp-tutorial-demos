@echo off
echo === OpenMP Reduction Operations Build Script ===

:: Check if build directory exists
if not exist build (
    echo Build directory does not exist. Please run configure.bat first.
    exit /b 1
)

:: Set the number of parallel jobs for faster building
set BUILD_THREADS=%NUMBER_OF_PROCESSORS%
echo Using %BUILD_THREADS% parallel jobs for building

:: Build Debug configuration
echo.
echo Building Debug configuration...
cd build
cmake --build . --config Debug --parallel %BUILD_THREADS%

:: Check if Debug build was successful
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo Debug build failed. Please check the error messages above.
    cd ..
    exit /b 1
)

:: Build Release configuration
echo.
echo Building Release configuration...
cmake --build . --config Release --parallel %BUILD_THREADS%

:: Check if Release build was successful
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo Release build failed. Please check the error messages above.
    cd ..
    exit /b 1
)

:: Check if Profile configuration exists in the project
cmake --help-property-list | findstr "PROFILE" >nul
if %ERRORLEVEL% EQU 0 (
    :: Build Profile configuration
    echo.
    echo Building Profile configuration...
    cmake --build . --config Profile --parallel %BUILD_THREADS%

    :: Check if Profile build was successful
    if %ERRORLEVEL% NEQ 0 (
        echo.
        echo Profile build failed. Please check the error messages above.
        cd ..
        exit /b 1
    )
) else (
    echo.
    echo Profile configuration not supported in this project. Skipping.
)

cd ..

:: Ensure output directories exist
echo.
echo Creating output directories...
if not exist bin mkdir bin
if not exist bin\Debug mkdir bin\Debug
if not exist bin\Release mkdir bin\Release

:: Simple copy of build outputs to standard locations
echo.
echo Copying output files to standard locations...

:: Copy Debug binaries
if exist build\Debug\*.exe (
    echo Copying Debug binaries...
    copy build\Debug\*.exe bin\Debug\ >nul 2>nul
)

:: Copy Release binaries
if exist build\Release\*.exe (
    echo Copying Release binaries...
    copy build\Release\*.exe bin\Release\ >nul 2>nul
)

:: Copy Profile binaries (if they exist)
if exist build\Profile\*.exe (
    if not exist bin\Profile mkdir bin\Profile
    echo Copying Profile binaries...
    copy build\Profile\*.exe bin\Profile\ >nul 2>nul
)

echo.
echo Build completed successfully!
echo.
echo Executables can be found in:
echo - Debug: bin\Debug\*.exe 
echo - Release: bin\Release\*.exe
if exist bin\Profile echo - Profile: bin\Profile\*.exe
echo.
echo Next steps:
echo - Run run.bat to execute examples with specific options
echo - Run run_all.bat to execute all examples in sequence
echo - Use --help option with run.bat for more information

exit /b 0 