param(
    [string[]]$Architectures = @("x64", "arm64"),
    [string]$OutputDir = "dist"
)

Write-Host "Creating MSIX packages for Microsoft Store" -ForegroundColor Green

# Check for makeappx
$makeappx = Get-Command "makeappx.exe" -ErrorAction SilentlyContinue
if (-not $makeappx) {
    Write-Host "makeappx.exe not found" -ForegroundColor Red
    exit 1
}

New-Item -ItemType Directory -Path "$OutputDir\msix" -Force | Out-Null
$createdPackages = @()

foreach ($arch in $Architectures) {
    Write-Host ""
    Write-Host "Creating MSIX for $arch..." -ForegroundColor Cyan
    
    $buildDir = "build_packages_$arch\Release"
    if (-not (Test-Path "$buildDir\UIAList.exe")) {
        Write-Host "No build found for $arch. Skipping." -ForegroundColor Yellow
        continue
    }
    
    # Create staging directory
    $stagingDir = "$OutputDir\msix\staging_$arch"
    Remove-Item $stagingDir -Recurse -Force -ErrorAction SilentlyContinue
    New-Item -ItemType Directory -Path $stagingDir -Force | Out-Null
    
    # Copy application files
    Copy-Item "$buildDir\UIAList.exe" $stagingDir -Force
    Get-ChildItem "$buildDir\*.dll" | Copy-Item -Destination $stagingDir -Force
    
    # Copy Qt plugins if they exist
    $pluginDirs = @("platforms", "imageformats", "styles", "iconengines")
    foreach ($pluginDir in $pluginDirs) {
        $sourcePath = "$buildDir\$pluginDir"
        if (Test-Path $sourcePath) {
            Copy-Item $sourcePath $stagingDir -Recurse -Force
        }
    }
    
    # Copy manifest and assets
    Copy-Item "Package.appxmanifest" "$stagingDir\AppxManifest.xml" -Force
    Copy-Item "Assets" $stagingDir -Recurse -Force
    
    # Package MSIX
    $msixFile = "$OutputDir\UIAList-Store-$arch-v0.1.0.msix"
    & makeappx pack /d $stagingDir /p $msixFile /o
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "✓ Created: UIAList-Store-$arch-v0.1.0.msix" -ForegroundColor Green
        $createdPackages += $msixFile
        $size = (Get-Item $msixFile).Length / 1MB
        Write-Host "  Size: $([math]::Round($size, 2)) MB" -ForegroundColor Gray
    } else {
        Write-Host "✗ Failed to create MSIX for $arch" -ForegroundColor Red
    }
}

Write-Host ""
if ($createdPackages.Count -gt 0) {
    Write-Host "Created $($createdPackages.Count) MSIX package(s):" -ForegroundColor Green
    foreach ($pkg in $createdPackages) {
        $name = Split-Path $pkg -Leaf
        Write-Host "  - $name" -ForegroundColor Gray
    }
    
    if ($createdPackages.Count -gt 1) {
        Write-Host ""
        Write-Host "Note: For Microsoft Store submission, you can either:" -ForegroundColor Yellow
        Write-Host "  1. Submit individual MSIX packages for each architecture" -ForegroundColor Gray
        Write-Host "  2. Create an MSIX bundle containing both packages" -ForegroundColor Gray
        Write-Host "The Store will automatically deliver the correct architecture to users." -ForegroundColor Gray
    }
} else {
    Write-Host "No MSIX packages were created." -ForegroundColor Red
}

Write-Host ""
Write-Host "MSIX packaging complete!" -ForegroundColor Green