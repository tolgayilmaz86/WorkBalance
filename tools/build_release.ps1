# build_release.ps1
# Creates an optimized, ship-ready x64 Release build of WorkBalance
# Single executable with ALL dependencies statically linked (no DLLs needed)
# Usage: .\build_release.ps1 [-Clean] [-Verbose]

param(
    [switch]$Clean,
    [switch]$Verbose
)

$ErrorActionPreference = "Stop"
$ProjectRoot = Split-Path -Parent $PSScriptRoot
if (-not $ProjectRoot -or -not (Test-Path (Join-Path $ProjectRoot "CMakeLists.txt"))) {
    $ProjectRoot = Get-Location
}
$BuildDir = Join-Path $ProjectRoot "out\build\x64-release-static"
$OutputDir = Join-Path $ProjectRoot "out\release"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  WorkBalance Release Build Script" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Clean if requested
if ($Clean) {
    Write-Host "[1/4] Cleaning previous build..." -ForegroundColor Yellow
    if (Test-Path $BuildDir) {
        Remove-Item -Recurse -Force $BuildDir
        Write-Host "  Removed: $BuildDir" -ForegroundColor Gray
    }
    if (Test-Path $OutputDir) {
        Remove-Item -Recurse -Force $OutputDir
        Write-Host "  Removed: $OutputDir" -ForegroundColor Gray
    }
}
else {
    Write-Host "[1/4] Skipping clean (use -Clean to force)" -ForegroundColor Gray
}

# Configure with optimized release flags
Write-Host ""
Write-Host "[2/4] Configuring CMake with release optimizations..." -ForegroundColor Yellow

$ConfigureArgs = @(
    "--preset", "x64-release-static"
)

Push-Location $ProjectRoot
try {
    if ($Verbose) {
        Write-Host "  CMake args: cmake $($ConfigureArgs -join ' ')" -ForegroundColor Gray
    }
    
    $configOutput = & cmake @ConfigureArgs 2>&1
    if ($LASTEXITCODE -ne 0) {
        Write-Host $configOutput -ForegroundColor Red
        throw "CMake configuration failed"
    }
    if ($Verbose) {
        Write-Host $configOutput -ForegroundColor Gray
    }
    Write-Host "  Configuration complete" -ForegroundColor Green

    # Build
    Write-Host ""
    Write-Host "[3/4] Building optimized release..." -ForegroundColor Yellow
    
    $BuildArgs = @(
        "--build", "--preset", "x64-release-static",
        "--config", "Release",
        "--parallel"
    )
    
    $buildOutput = & cmake @BuildArgs 2>&1
    if ($LASTEXITCODE -ne 0) {
        Write-Host $buildOutput -ForegroundColor Red
        throw "Build failed"
    }
    if ($Verbose) {
        Write-Host $buildOutput -ForegroundColor Gray
    }
    Write-Host "  Build complete" -ForegroundColor Green

    # Copy to output directory
    Write-Host ""
    Write-Host "[4/4] Packaging release..." -ForegroundColor Yellow
    
    if (-not (Test-Path $OutputDir)) {
        New-Item -ItemType Directory -Path $OutputDir | Out-Null
    }
    
    $ExePath = Join-Path $BuildDir "Release\WorkBalance.exe"
    if (-not (Test-Path $ExePath)) {
        throw "Executable not found at: $ExePath"
    }
    
    $DestPath = Join-Path $OutputDir "WorkBalance.exe"
    Copy-Item $ExePath $DestPath -Force
    
    # Get file info
    $FileInfo = Get-Item $DestPath
    $FileSizeMB = [math]::Round($FileInfo.Length / 1MB, 2)
    
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Green
    Write-Host "  BUILD SUCCESSFUL" -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "  Output: $DestPath" -ForegroundColor White
    Write-Host "  Size:   $FileSizeMB MB" -ForegroundColor White
    Write-Host ""
    Write-Host "  Optimizations applied:" -ForegroundColor Gray
    Write-Host "    - Full optimization (/O2)" -ForegroundColor Gray
    Write-Host "    - Link-time code generation (/LTCG)" -ForegroundColor Gray
    Write-Host "    - Static CRT (/MT)" -ForegroundColor Gray
    Write-Host "    - Static GLFW, ImGui (no DLLs)" -ForegroundColor Gray
    Write-Host "    - Dead code elimination (/OPT:REF, /OPT:ICF)" -ForegroundColor Gray
    Write-Host ""
    
}
finally {
    Pop-Location
}
