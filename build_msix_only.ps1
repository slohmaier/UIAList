param(
    [string[]]$Architectures = @("x64", "arm64"),
    [string]$OutputDir = "dist"
)

Write-Host "UIAList MSIX Package Builder" -ForegroundColor Green
Write-Host "=============================" -ForegroundColor Green

# Check for makeappx
try {
    $makeappx = Get-Command "makeappx.exe" -ErrorAction Stop
    Write-Host "Found makeappx: $($makeappx.Source)" -ForegroundColor Cyan
} catch {
    Write-Host "makeappx not found" -ForegroundColor Red
    exit 1
}

# Create MSIX packages for each architecture
New-Item -ItemType Directory -Path "$OutputDir\msix" -Force | Out-Null
$msixPackages = @()

foreach ($arch in $Architectures) {
    Write-Host ""
    Write-Host "Creating MSIX package for $arch..." -ForegroundColor Blue
    
    $buildDir = "build_packages_$arch"
    if (-not (Test-Path "$buildDir\Release\UIAList.exe")) {
        Write-Host "Executable not found for $arch. Skipping." -ForegroundColor Yellow
        continue
    }
    
    # Create staging directory
    $stagingDir = "$OutputDir\msix\staging_$arch"
    New-Item -ItemType Directory -Path $stagingDir -Force | Out-Null
    
    # Copy executable
    Copy-Item "$buildDir\Release\UIAList.exe" $stagingDir -Force
    
    # Copy Qt dependencies (simplified - copy all DLLs from build directory)
    Get-ChildItem "$buildDir\Release\*.dll" | Copy-Item -Destination $stagingDir -Force
    
    # Copy Qt plugins
    if (Test-Path "$buildDir\Release\platforms") {
        Copy-Item "$buildDir\Release\platforms" $stagingDir -Recurse -Force
    }
    if (Test-Path "$buildDir\Release\imageformats") {
        Copy-Item "$buildDir\Release\imageformats" $stagingDir -Recurse -Force
    }
    if (Test-Path "$buildDir\Release\styles") {
        Copy-Item "$buildDir\Release\styles" $stagingDir -Recurse -Force
    }
    
    # Copy manifest and assets
    Copy-Item "Package.appxmanifest" "$stagingDir\AppxManifest.xml" -Force
    Copy-Item "Assets" $stagingDir -Recurse -Force
    
    # Create MSIX package
    $msixFile = "$OutputDir\UIAList-Store-v0.1.0-$arch.msix"
    
    Write-Host "Packaging $arch MSIX..." -ForegroundColor Cyan
    & makeappx pack /d $stagingDir /p $msixFile /o
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "✓ MSIX created: UIAList-Store-v0.1.0-$arch.msix" -ForegroundColor Green
        $msixPackages += $msixFile
    } else {
        Write-Host "✗ Failed to create MSIX for $arch" -ForegroundColor Red
    }
}

# Create MSIX Bundle if multiple packages were created
if ($msixPackages.Count -gt 1) {
    Write-Host ""
    Write-Host "Creating MSIX Bundle..." -ForegroundColor Blue
    
    $bundleDir = "$OutputDir\msix\bundle_staging"
    New-Item -ItemType Directory -Path $bundleDir -Force | Out-Null
    
    # Copy MSIX files to bundle directory
    foreach ($msixFile in $msixPackages) {
        $fileName = Split-Path $msixFile -Leaf
        Copy-Item $msixFile "$bundleDir\$fileName" -Force
    }
    
    # Create bundle manifest
    $bundleManifest = "$bundleDir\AppxBundleManifest.xml"
    $xml = @"
<?xml version="1.0" encoding="UTF-8"?>
<Bundle xmlns="http://schemas.microsoft.com/appx/2013/bundle">
  <Identity Name="StefanLohmaier.UIAList"
            Publisher="CN=Stefan Lohmaier"
            Version="0.1.0.0" />
  <Packages>
"@
    
    foreach ($arch in $Architectures) {
        if (Test-Path "$OutputDir\UIAList-Store-v0.1.0-$arch.msix") {
            $xml += "`n    <Package Type=`"application`" Version=`"0.1.0.0`" Architecture=`"$arch`" FileName=`"UIAList-Store-v0.1.0-$arch.msix`" />"
        }
    }
    
    $xml += "`n  </Packages>`n</Bundle>"
    
    Set-Content -Path $bundleManifest -Value $xml -Encoding UTF8
    
    # Create bundle
    $bundleFile = "$OutputDir\UIAList-Store-Bundle-v0.1.0.msixbundle"
    & makeappx bundle /d $bundleDir /p $bundleFile /o
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "✓ MSIX Bundle created: UIAList-Store-Bundle-v0.1.0.msixbundle" -ForegroundColor Green
        $bundleSize = (Get-Item $bundleFile).Length / 1MB
        Write-Host "Bundle size: $([math]::Round($bundleSize, 2)) MB (contains both x64 and ARM64)" -ForegroundColor Cyan
    } else {
        Write-Host "✗ Failed to create MSIX Bundle" -ForegroundColor Red
    }
}

Write-Host ""
Write-Host "MSIX packaging complete!" -ForegroundColor Green