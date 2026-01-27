# Code Quality Tools Installation Script for Windows

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Code Quality Tools Setup for MechanicGuy" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Check if running as admin
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

if (-not $isAdmin) {
    Write-Host "⚠️  Not running as Administrator. Some operations may require elevation." -ForegroundColor Yellow
    Write-Host ""
}

# Function to check if command exists
function Test-Command {
    param($CommandName)
    $null -ne (Get-Command $CommandName -ErrorAction SilentlyContinue)
}

# Check for winget
Write-Host "Checking for winget (Windows Package Manager)..." -ForegroundColor White
if (Test-Command winget) {
    Write-Host "✓ winget found" -ForegroundColor Green
    Write-Host ""
    Write-Host "Installing LLVM (includes clang-format and clang-tidy)..." -ForegroundColor White
    Write-Host "This may take several minutes..." -ForegroundColor Yellow
    Write-Host ""
    
    try {
        winget install --id LLVM.LLVM --silent --accept-package-agreements --accept-source-agreements
        Write-Host ""
        Write-Host "✓ LLVM installation completed" -ForegroundColor Green
        Write-Host ""
        Write-Host "⚠️  IMPORTANT: You need to restart your terminal/VS Code for PATH changes to take effect!" -ForegroundColor Yellow
        Write-Host ""
    }
    catch {
        Write-Host "✗ Installation failed: $_" -ForegroundColor Red
        Write-Host ""
        Write-Host "Manual installation:" -ForegroundColor Yellow
        Write-Host "1. Download from: https://github.com/llvm/llvm-project/releases/latest" -ForegroundColor White
        Write-Host "2. Run the installer and ensure 'Add to PATH' is checked" -ForegroundColor White
        Write-Host "3. Restart your terminal/VS Code" -ForegroundColor White
        exit 1
    }
}
else {
    Write-Host "✗ winget not found" -ForegroundColor Red
    Write-Host ""
    Write-Host "Please install LLVM manually:" -ForegroundColor Yellow
    Write-Host "1. Download from: https://github.com/llvm/llvm-project/releases/latest" -ForegroundColor White
    Write-Host "   - Look for LLVM-XX.X.X-win64.exe" -ForegroundColor White
    Write-Host "2. Run the installer" -ForegroundColor White
    Write-Host "3. During installation, check 'Add LLVM to system PATH'" -ForegroundColor White
    Write-Host "4. Restart your terminal/VS Code" -ForegroundColor White
    Write-Host ""
    Write-Host "Alternative: Install winget first, then run this script again" -ForegroundColor White
    Write-Host "https://github.com/microsoft/winget-cli/releases" -ForegroundColor White
    exit 1
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Verification (after restart)" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "After restarting VS Code/terminal, verify installation:" -ForegroundColor White
Write-Host "  clang-format --version" -ForegroundColor Gray
Write-Host "  clang-tidy --version" -ForegroundColor Gray
Write-Host ""
Write-Host "Then reconfigure CMake to detect the tools:" -ForegroundColor White
Write-Host "  cmake -B build -DCMAKE_TOOLCHAIN_FILE=`$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" -ForegroundColor Gray
Write-Host ""
Write-Host "Available tasks (Ctrl+Shift+P -> Run Task):" -ForegroundColor White
Write-Host "  - Format Code" -ForegroundColor Gray
Write-Host "  - Run Clang-Tidy" -ForegroundColor Gray
Write-Host "  - Run All Quality Checks" -ForegroundColor Gray
Write-Host ""
