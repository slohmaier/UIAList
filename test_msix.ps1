param(
    [string[]]$Architectures = @("x64", "arm64"),
    [string]$OutputDir = "dist"
)

Write-Host "Testing MSIX Creation" -ForegroundColor Green

# Check for makeappx
try {
    $makeappx = Get-Command "makeappx.exe" -ErrorAction Stop
    Write-Host "Found makeappx: $($makeappx.Source)" -ForegroundColor Cyan
} catch {
    Write-Host "makeappx not found" -ForegroundColor Red
    exit 1
}

# Create test staging area
New-Item -ItemType Directory -Path "$OutputDir\msix\test_x64" -Force | Out-Null
Copy-Item "build_packages_x64\Release\UIAList.exe" "$OutputDir\msix\test_x64\" -Force
Copy-Item "Package.appxmanifest" "$OutputDir\msix\test_x64\AppxManifest.xml" -Force
Copy-Item "Assets" "$OutputDir\msix\test_x64\" -Recurse -Force

# Test MSIX creation
try {
    & makeappx pack /d "$OutputDir\msix\test_x64" /p "$OutputDir\test-x64.msix" /o
    if ($LASTEXITCODE -eq 0) {
        Write-Host "MSIX test successful!" -ForegroundColor Green
    } else {
        Write-Host "MSIX test failed" -ForegroundColor Red
    }
} catch {
    Write-Host "Error: $_" -ForegroundColor Red
}