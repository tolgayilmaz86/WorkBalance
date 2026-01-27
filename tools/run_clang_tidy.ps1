param([switch]$Fix)
$files = Get-ChildItem -Path src -Recurse -Include *.cpp,*.h
$args = @("--quiet", "--header-filter=.*src/.*")
if ($Fix) { $args += "--fix", "--fix-errors" }
$errors = 0
foreach ($f in $files) {
    Write-Host "Checking $($f.Name)..."
    $result = & clang-tidy @args $f.FullName -- -std=c++20 -I"$PWD\src" -I"$PWD\include" -I"$PWD" -I"$PWD\vcpkg_installed\x64-windows\include" -I"$PWD\build\vcpkg_installed\x64-windows-static\include" -DUNICODE -D_UNICODE -D_CRT_SECURE_NO_WARNINGS --target=x86_64-pc-windows-msvc -fms-compatibility -fms-extensions 2>&1
    $srcWarnings = $result | Select-String "src[/\\].*\.(cpp|h):" | Select-String "warning:"
    if ($srcWarnings) { $srcWarnings | ForEach-Object { Write-Host "  $_" -ForegroundColor Yellow } }
    $srcErrors = $result | Select-String "src[/\\].*\.(cpp|h):" | Select-String "error:"
    if ($srcErrors) { $srcErrors | ForEach-Object { Write-Host "  $_" -ForegroundColor Red }; $errors++ }
}
if ($errors -gt 0) { exit 1 }
