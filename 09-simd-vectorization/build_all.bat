@echo off
echo === OpenMP SIMD Vectorization Build Script ===
echo.

:: First, make sure no processes are locking the exe file
taskkill /F /IM OpenMP_SIMD_Vectorization.exe 2>nul
if exist "build\Debug\OpenMP_SIMD_Vectorization.exe" (
    del /F /Q "build\Debug\OpenMP_SIMD_Vectorization.exe" 2>nul
)
if exist "build\Release\OpenMP_SIMD_Vectorization.exe" (
    del /F /Q "build\Release\OpenMP_SIMD_Vectorization.exe" 2>nul
)

:: Create bin directory as an alternative output
if not exist bin mkdir bin
if not exist bin\Debug mkdir bin\Debug
if not exist bin\Release mkdir bin\Release

echo Building Debug configuration...
cmake --build build --config Debug

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo Debug build failed. Please check the error messages above.
    echo Trying alternative build approach...
    
    :: Try to build with direct MSBuild command
    echo Building with MSBuild directly...
    msbuild build\OpenMP_SIMD_Vectorization.sln /p:Configuration=Debug /p:Platform=x64 /p:OutDir=..\..\bin\Debug\
    
    if %ERRORLEVEL% NEQ 0 (
        echo Alternative build approach also failed.
    ) else (
        echo Alternative build completed successfully.
        echo Executable can be found in bin\Debug\
    )
) else (
    echo Debug build completed successfully.
    :: Copy the executable to bin directory as a backup
    copy build\Debug\OpenMP_SIMD_Vectorization.exe bin\Debug\ > nul 2>&1
)
echo.

echo Building Release configuration...
cmake --build build --config Release

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo Release build failed. Please check the error messages above.
    echo Trying alternative build approach...
    
    :: Try to build with direct MSBuild command
    echo Building with MSBuild directly...
    msbuild build\OpenMP_SIMD_Vectorization.sln /p:Configuration=Release /p:Platform=x64 /p:OutDir=..\..\bin\Release\
    
    if %ERRORLEVEL% NEQ 0 (
        echo Alternative build approach also failed.
    ) else (
        echo Alternative build completed successfully.
        echo Executable can be found in bin\Release\
    )
) else (
    echo Release build completed successfully.
    :: Copy the executable to bin directory as a backup
    copy build\Release\OpenMP_SIMD_Vectorization.exe bin\Release\ > nul 2>&1
)
echo.

if exist bin\Debug\OpenMP_SIMD_Vectorization.exe (
    echo Debug executable can be found in: bin\Debug\
)
if exist bin\Release\OpenMP_SIMD_Vectorization.exe (
    echo Release executable can be found in: bin\Release\
)
if exist build\Debug\OpenMP_SIMD_Vectorization.exe (
    echo Debug executable (CMake) can be found in: build\Debug\
)
if exist build\Release\OpenMP_SIMD_Vectorization.exe (
    echo Release executable (CMake) can be found in: build\Release\
)

echo.
echo Next step: Run run.bat to execute the program
pause