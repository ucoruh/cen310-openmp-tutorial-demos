@echo off
echo === OpenMP SIMD Vectorization Clean Script ===

REM Kill any processes that might be locking the build directory
echo Terminating any processes that might be locking the executables...
taskkill /F /IM OpenMP_SIMD_Vectorization.exe 2>nul
timeout /t 2 /nobreak > nul

REM Clean build directory
echo Cleaning build directory...
if exist build (
    rem Try using rmdir first
    rmdir /s /q build 2>nul
    
    rem If that fails, try removing individual files
    if exist build (
        echo Removing individual files...
        for /r "build" %%F in (*) do (
            del /f /q "%%F" 2>nul
        )
        
        rem Try to remove directories after files are deleted
        for /d /r "build" %%D in (*) do (
            rmdir /s /q "%%D" 2>nul
        )
        
        rem Finally, remove the build directory itself
        rmdir /s /q build 2>nul
    )
    
    rem Check if directory is still there, if yes, report but continue
    if exist build (
        echo Warning: Could not completely remove the build directory.
        echo Some files may be locked by another process.
        echo Please close any applications that might be using these files and try again.
    ) else (
        echo Build directory successfully removed.
        
        REM Recreate empty build directory
        mkdir build
        echo Empty build directory created.
    )
) else (
    echo Build directory does not exist, nothing to clean.
    mkdir build
    echo Empty build directory created.
)

REM Ask if user wants to clean benchmark and assembly output directories
echo.
echo Would you like to clean benchmark and assembly output directories too? (Y/N)
choice /c YN /n /m "Your choice: "
if errorlevel 2 goto no_clean_extras
if errorlevel 1 goto clean_extras

:clean_extras
echo Cleaning benchmarks directory...
if exist benchmarks (
    rmdir /s /q benchmarks
    echo Benchmarks directory successfully removed.
    mkdir benchmarks
    echo Empty benchmarks directory created.
) else (
    echo Benchmarks directory does not exist.
    mkdir benchmarks
    echo Empty benchmarks directory created.
)

echo Cleaning asm_output directory...
if exist asm_output (
    rmdir /s /q asm_output
    echo ASM output directory successfully removed.
    mkdir asm_output
    echo Empty asm_output directory created.
) else (
    echo ASM output directory does not exist.
    mkdir asm_output
    echo Empty asm_output directory created.
)

:no_clean_extras
echo Removing any additional temporary files...
del /f /q *.obj *.pdb *.ilk *.exp *.lib 2>nul

echo.
echo Clean operation completed.
echo.
echo To rebuild the project:
echo 1. Run configure.bat
echo 2. Run build.bat
pause