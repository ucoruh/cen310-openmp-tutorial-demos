# PowerShell Thorough Cleaning Script for SIMD Vectorization
Write-Host "=== OpenMP SIMD Vectorization Advanced Cleaning Script ===" -ForegroundColor Cyan

$projectPath = Get-Location
$buildPath = Join-Path -Path $projectPath -ChildPath "build"
$exePath = Join-Path -Path $buildPath -ChildPath "Debug\OpenMP_SIMD_Vectorization.exe"

# Function to check for locked files
function Test-IsFileLocked {
    param(
        [parameter(Mandatory=$true)]
        [string]$Path
    )
    
    try {
        $fileInfo = New-Object System.IO.FileInfo $Path
        $fileStream = $fileInfo.Open([System.IO.FileMode]::Open, 
                                    [System.IO.FileAccess]::ReadWrite, 
                                    [System.IO.FileShare]::None)
        if ($fileStream) {
            $fileStream.Close()
            return $false
        }
        return $true
    } catch {
        return $true
    }
}

# Check for running processes that might lock files
Write-Host "Checking for processes that may lock files..." -ForegroundColor Yellow
$foundProcess = $false

# Look for any executable that matches our project name
Get-Process | Where-Object {$_.ProcessName -like "*OpenMP_SIMD*" -or $_.ProcessName -like "*Vectorization*"} | ForEach-Object {
    Write-Host "Found process: $($_.ProcessName) (PID: $($_.Id))" -ForegroundColor Red
    $foundProcess = $true
    try {
        Write-Host "Attempting to stop process $($_.Id)..." -ForegroundColor Yellow
        Stop-Process -Id $_.Id -Force -ErrorAction Stop
        Write-Host "Successfully stopped process." -ForegroundColor Green
    } catch {
        Write-Host "Failed to stop process. You may need admin rights: $_" -ForegroundColor Red
    }
}

if (-not $foundProcess) {
    Write-Host "No locking processes found." -ForegroundColor Green
}

# Wait a moment to ensure processes are fully terminated
Start-Sleep -Seconds 2

# Check if build directory exists
if (Test-Path $buildPath) {
    Write-Host "Cleaning build directory..." -ForegroundColor Yellow
    
    try {
        # First try normal removal
        Remove-Item -Path $buildPath -Recurse -Force -ErrorAction Stop
        Write-Host "Build directory successfully removed." -ForegroundColor Green
    } catch {
        Write-Host "Standard removal failed, trying advanced techniques..." -ForegroundColor Yellow
        
        # Walk through directory and remove files one by one
        Get-ChildItem -Path $buildPath -Recurse | ForEach-Object {
            if (-not $_.PSIsContainer) {
                try {
                    if (Test-IsFileLocked -Path $_.FullName) {
                        Write-Host "File is locked: $($_.FullName)" -ForegroundColor Red
                    } else {
                        Remove-Item -Path $_.FullName -Force -ErrorAction SilentlyContinue
                    }
                } catch {
                    Write-Host "Could not remove file: $($_.FullName)" -ForegroundColor Red
                }
            }
        }
        
        # Now try to remove empty directories
        Get-ChildItem -Path $buildPath -Recurse -Directory | 
        Sort-Object -Property FullName -Descending | 
        ForEach-Object {
            try {
                Remove-Item -Path $_.FullName -Force -ErrorAction SilentlyContinue
            } catch {
                # Silently continue
            }
        }
        
        # Final attempt to remove build directory
        try {
            Remove-Item -Path $buildPath -Recurse -Force -ErrorAction SilentlyContinue
        } catch {
            # Silently continue
        }
        
        # Check if directory is still there
        if (Test-Path $buildPath) {
            Write-Host "Warning: Could not completely remove the build directory." -ForegroundColor Red
            Write-Host "Some files may be locked by another process." -ForegroundColor Red
            Write-Host "Please close any applications that might be using these files and try again." -ForegroundColor Red
        } else {
            Write-Host "Build directory successfully removed." -ForegroundColor Green
        }
    }
    
    # Create new build directory
    if (-not (Test-Path $buildPath)) {
        New-Item -Path $buildPath -ItemType Directory | Out-Null
        Write-Host "Empty build directory created." -ForegroundColor Green
    }
} else {
    Write-Host "Build directory does not exist, nothing to clean." -ForegroundColor Yellow
    New-Item -Path $buildPath -ItemType Directory | Out-Null
    Write-Host "Empty build directory created." -ForegroundColor Green
}

# Ask about cleaning benchmarks and asm_output
$cleanExtra = Read-Host "Would you like to clean benchmark and assembly output directories too? (Y/N)"
if ($cleanExtra -eq "Y" -or $cleanExtra -eq "y") {
    $benchmarksPath = Join-Path -Path $projectPath -ChildPath "benchmarks"
    $asmOutputPath = Join-Path -Path $projectPath -ChildPath "asm_output"
    
    # Clean benchmarks
    Write-Host "Cleaning benchmarks directory..." -ForegroundColor Yellow
    if (Test-Path $benchmarksPath) {
        Remove-Item -Path $benchmarksPath -Recurse -Force
        Write-Host "Benchmarks directory successfully removed." -ForegroundColor Green
    } else {
        Write-Host "Benchmarks directory does not exist." -ForegroundColor Yellow
    }
    
    New-Item -Path $benchmarksPath -ItemType Directory | Out-Null
    Write-Host "Empty benchmarks directory created." -ForegroundColor Green
    
    # Clean asm_output
    Write-Host "Cleaning asm_output directory..." -ForegroundColor Yellow
    if (Test-Path $asmOutputPath) {
        Remove-Item -Path $asmOutputPath -Recurse -Force
        Write-Host "ASM output directory successfully removed." -ForegroundColor Green
    } else {
        Write-Host "ASM output directory does not exist." -ForegroundColor Yellow
    }
    
    New-Item -Path $asmOutputPath -ItemType Directory | Out-Null
    Write-Host "Empty asm_output directory created." -ForegroundColor Green
}

# Clean temporary files in the root directory
Write-Host "Removing any additional temporary files..." -ForegroundColor Yellow
Remove-Item -Path (Join-Path -Path $projectPath -ChildPath "*.obj") -Force -ErrorAction SilentlyContinue
Remove-Item -Path (Join-Path -Path $projectPath -ChildPath "*.pdb") -Force -ErrorAction SilentlyContinue
Remove-Item -Path (Join-Path -Path $projectPath -ChildPath "*.ilk") -Force -ErrorAction SilentlyContinue
Remove-Item -Path (Join-Path -Path $projectPath -ChildPath "*.exp") -Force -ErrorAction SilentlyContinue
Remove-Item -Path (Join-Path -Path $projectPath -ChildPath "*.lib") -Force -ErrorAction SilentlyContinue

Write-Host "`nClean operation completed." -ForegroundColor Green
Write-Host "`nTo rebuild the project:" -ForegroundColor Cyan
Write-Host "1. Run configure.bat" -ForegroundColor Cyan
Write-Host "2. Run build.bat" -ForegroundColor Cyan

# Wait for user to press a key
Write-Host "`nPress any key to continue..." -ForegroundColor Gray
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown") 