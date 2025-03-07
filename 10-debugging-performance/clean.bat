@echo off
echo === OpenMP Debugging and Performance Analysis Clean Script ===

rem Check if admin rights are needed for certain operations
>nul 2>&1 "%SYSTEMROOT%\system32\cacls.exe" "%SYSTEMROOT%\system32\config\system"
if %errorlevel% neq 0 (
    echo Warning: Some operations might require administrator rights.
    echo If you encounter permission errors, try running this script as administrator.
    echo.
)

echo.
echo Terminating any processes that might be locking files...
taskkill /F /IM race_conditions.exe /T >nul 2>&1
taskkill /F /IM false_sharing.exe /T >nul 2>&1
taskkill /F /IM memory_access_visualizer.exe /T >nul 2>&1
taskkill /F /IM custom_profiler.exe /T >nul 2>&1
taskkill /F /IM excessive_synchronization.exe /T >nul 2>&1
taskkill /F /IM load_imbalance.exe /T >nul 2>&1
taskkill /F /IM memory_issues.exe /T >nul 2>&1
taskkill /F /IM thread_timeline_visualizer.exe /T >nul 2>&1
taskkill /F /IM vs_diagnostics.exe /T >nul 2>&1
taskkill /F /IM analysis_tools.exe /T >nul 2>&1
taskkill /F /IM intel_vtune.exe /T >nul 2>&1
rem Allow some time for processes to terminate
timeout /t 1 /nobreak >nul

echo Cleaning build directories...

rem Clean the main build directory
if exist build (
    echo Removing build directory...
    
    rem Try standard removal first
    rmdir /s /q build
    
    rem If the directory still exists, try alternative method
    if exist build (
        echo First attempt failed. Trying alternative cleanup method...
        rd /s /q build 2>nul
        
        rem If still fails, use robocopy to empty the directory
        if exist build (
            echo Using robocopy to empty directory...
            if not exist empty mkdir empty
            robocopy empty build /mir /NFL /NDL /NJH /NJS /nc /ns /np
            rmdir /s /q build
            rmdir /s /q empty
            
            if exist build (
                echo Warning: Could not completely remove build directory.
                echo Please close any applications using files in this directory and try again.
            ) else (
                echo Build directory successfully removed.
            )
        ) else (
            echo Build directory successfully removed.
        )
    ) else (
        echo Build directory successfully removed.
    )
) else (
    echo Build directory not found, nothing to clean.
)

rem Clean the bin directory
if exist bin (
    echo Removing bin directory...
    
    rem Try standard removal first
    rmdir /s /q bin
    
    rem If the directory still exists, try alternative method
    if exist bin (
        echo First attempt failed. Trying alternative cleanup method...
        rd /s /q bin 2>nul
        
        rem If still fails, use robocopy to empty the directory
        if exist bin (
            echo Using robocopy to empty directory...
            if not exist empty mkdir empty
            robocopy empty bin /mir /NFL /NDL /NJH /NJS /nc /ns /np
            rmdir /s /q bin
            rmdir /s /q empty
            
            if exist bin (
                echo Warning: Could not completely remove bin directory.
                echo Please close any applications using files in this directory and try again.
            ) else (
                echo Bin directory successfully removed.
            )
        ) else (
            echo Bin directory successfully removed.
        )
    ) else (
        echo Bin directory successfully removed.
    )
) else (
    echo Bin directory not found, nothing to clean.
)

rem Clean the reports directory
if exist reports (
    echo Removing reports directory...
    rmdir /s /q reports
    if %errorlevel% neq 0 (
        echo First attempt failed. Trying alternative cleanup method...
        rd /s /q reports 2>nul
        
        if exist reports (
            echo Using robocopy to empty directory...
            if not exist empty mkdir empty
            robocopy empty reports /mir /NFL /NDL /NJH /NJS /nc /ns /np
            rmdir /s /q reports
            rmdir /s /q empty
            
            if exist reports (
                echo Warning: Could not completely remove reports directory.
            ) else (
                echo Reports directory successfully removed.
            )
        ) else (
            echo Reports directory successfully removed.
        )
    ) else (
        echo Reports directory successfully removed.
    )
) else (
    echo Reports directory not found, nothing to clean.
)

rem Clean any temporary files in the project
echo Removing temporary files...
del /s /f /q *.pdb > nul 2>&1
del /s /f /q *.obj > nul 2>&1
del /s /f /q *.ilk > nul 2>&1
del /s /f /q *.idb > nul 2>&1
del /s /f /q *.tlog > nul 2>&1
del /s /f /q *.log > nul 2>&1
del /s /f /q *.ipch > nul 2>&1
del /s /f /q *.suo > nul 2>&1
del /s /f /q *.user > nul 2>&1
del /s /f /q *.db > nul 2>&1
del /s /f /q *.opendb > nul 2>&1
del /s /f /q *.vs > nul 2>&1

rem Clean any diagnostic files created by the performance tools
echo Removing diagnostic files...
del /s /f /q *.etl > nul 2>&1
del /s /f /q *.vspx > nul 2>&1
del /s /f /q *.diagsession > nul 2>&1

echo.
echo Clean operation completed.
echo.
echo To rebuild the project, run the following commands:
echo 1. configure.bat
echo 2. build_all.bat
echo. 