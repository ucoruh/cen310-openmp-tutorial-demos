@echo off
echo === OpenMP SIMD Vectorization Assembly Report Generator ===

:: Create asm_output directory if it doesn't exist
if not exist asm_output (
    echo Creating assembly output directory...
    mkdir asm_output
)

:: Check if the compiler (cl.exe) is in the path
where cl >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo Error: Microsoft C++ compiler (cl.exe) not found in the PATH.
    echo Please run this script from a Visual Studio Developer Command Prompt or
    echo run vcvarsall.bat to set up the environment.
    pause
    exit /b 1
)

echo.
echo Generating assembly output for SIMD examples...
echo This will create annotated assembly files that show which instructions are vectorized.
echo.

:: Define the source files to analyze
set SRC_FILES=src\basic_simd.cpp src\array_operations.cpp src\complex_math.cpp src\simd_alignment.cpp src\simd_width.cpp src\mixed_precision.cpp src\simd_parallelism.cpp

:: For each source file, generate assembly with and without SIMD enabled
for %%f in (%SRC_FILES%) do (
    echo Processing %%~nf.cpp...
    
    :: Generate assembly with SIMD (vectorization) enabled
    echo   Generating vectorized assembly...
    cl.exe /c /FA /O2 /openmp:experimental /arch:AVX2 /Fa"asm_output\%%~nf_vectorized.asm" %%f
    
    :: Generate assembly with SIMD explicitly disabled
    echo   Generating scalar assembly...
    cl.exe /c /FA /O2 /openmp- /Fa"asm_output\%%~nf_scalar.asm" %%f
    
    :: Clean up object files
    if exist %%~nf.obj del %%~nf.obj
)

echo.
echo Assembly output generation completed!
echo.
echo Assembly files are located in the 'asm_output' directory:
echo - *_vectorized.asm: Assembly with SIMD enabled
echo - *_scalar.asm: Assembly with SIMD disabled
echo.
echo Next steps:
echo 1. Run the main program with the '--asm-analysis' or '-a' option to analyze the assembly output
echo 2. Or select option 9 (Assembly Output Analysis) from the main menu

:: Check if object files remain and clean them up
echo.
echo Cleaning up temporary files...
if exist *.obj del *.obj

:: Include directory for header files
set INCLUDE_DIR=include

:: Generate assembly for main executable
echo.
echo Generating assembly for main program...
cl.exe /c /FA /O2 /openmp:experimental /arch:AVX2 /I"%INCLUDE_DIR%" /Fa"asm_output\main_vectorized.asm" src\main.cpp
if exist main.obj del main.obj

echo.
echo All assembly files have been generated. You can now run the analyzer to interpret the results.
echo.
echo If you want to compare vectorized vs non-vectorized code:
echo - For each source file, there are two ASM files:
echo   * _vectorized.asm (with SIMD enabled)
echo   * _scalar.asm (with SIMD disabled)
echo.
echo These files can help you understand how the compiler vectorizes your code
echo and identify opportunities for optimization.

pause