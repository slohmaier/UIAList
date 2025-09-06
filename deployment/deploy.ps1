# UIAList Unified Build & Deployment Script
# Handles building and packaging for all Windows architectures and formats

param(
    [string]$Configuration = "Release",
    [string]$OutputDir = "deployment",
    [string[]]$Architectures = @("x64", "arm64"),
    [switch]$SkipBuild = $false,
    [switch]$MSIOnly = $false,
    [switch]$ZipOnly = $false,
    [switch]$MSIXOnly = $false,
    [switch]$Clean = $false,
    [switch]$Help = $false
)

if ($Help) {
    Write-Host "UIAList Unified Build & Deployment Script" -ForegroundColor Green
    Write-Host "==========================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "Usage: ./deploy.ps1 [options]" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "Options:" -ForegroundColor Yellow
    Write-Host "  -Configuration    Build configuration (Release/Debug) [Default: Release]"
    Write-Host "  -OutputDir        Output directory [Default: deployment]"
    Write-Host "  -Architectures    Target architectures [Default: x64,arm64]"
    Write-Host "  -SkipBuild        Skip the build process, only package existing builds"
    Write-Host "  -MSIOnly          Create only MSI installer packages"
    Write-Host "  -ZipOnly          Create only portable ZIP packages"  
    Write-Host "  -MSIXOnly         Create only MSIX packages for Microsoft Store"
    Write-Host "  -Clean            Clean build directories before building"
    Write-Host "  -Help             Show this help message"
    Write-Host ""
    Write-Host "Examples:" -ForegroundColor Yellow
    Write-Host "  ./deploy.ps1                              # Build everything"
    Write-Host "  ./deploy.ps1 -MSIXOnly                    # Build only MSIX for Store"
    Write-Host "  ./deploy.ps1 -Architectures @('x64')      # Build only x64"
    Write-Host "  ./deploy.ps1 -SkipBuild -ZipOnly          # Package existing builds as ZIP"
    Write-Host ""
    Write-Host "Output Packages:" -ForegroundColor Yellow
    Write-Host "  - Portable ZIP:     UIAList-Portable-v0.1.0-{arch}.zip"
    Write-Host "  - MSI Installer:    UIAList-Installer-v0.1.0-{arch}.msi" 
    Write-Host "  - MSIX Package:     UIAList-Store-v0.1.0-{arch}.msix"
    Write-Host "  - MSIX Bundle:      UIAList-Store-Bundle-v0.1.0.msixbundle"
    Write-Host ""
    return
}

$ErrorActionPreference = "Stop"

Write-Host "UIAList Unified Build & Deployment Script" -ForegroundColor Green
Write-Host "==========================================" -ForegroundColor Green
Write-Host "Building for architectures: $($Architectures -join ', ')" -ForegroundColor Cyan
Write-Host "Configuration: $Configuration" -ForegroundColor Cyan
Write-Host "Output: $OutputDir" -ForegroundColor Cyan
Write-Host ""

# Navigate to project root (script is in deployment/ subdirectory)
Push-Location $PSScriptRoot\..

try {
    # Clean output directory if requested
    if ($Clean -and (Test-Path $OutputDir)) {
        Write-Host "Cleaning output directory..." -ForegroundColor Yellow
        Remove-Item $OutputDir -Recurse -Force
    }

    # Create output directories
    New-Item -ItemType Directory -Path $OutputDir -Force | Out-Null
    New-Item -ItemType Directory -Path "$OutputDir\portable" -Force | Out-Null
    New-Item -ItemType Directory -Path "$OutputDir\msi" -Force | Out-Null
    New-Item -ItemType Directory -Path "$OutputDir\msix" -Force | Out-Null

    # Check prerequisites
    Write-Host "Checking prerequisites..." -ForegroundColor Yellow

    # Check for Qt installations
    $qtDirs = @{}
    foreach ($arch in $Architectures) {
        $qtArchName = switch ($arch) {
            "x64" { "msvc2022_64" }
            "arm64" { "msvc2022_arm64" }
            default { $arch }
        }
        
        $qtDir = "C:\Qt\6.9.2\$qtArchName"
        if (Test-Path "$qtDir\bin\Qt6Core.dll") {
            $qtDirs[$arch] = $qtDir
            Write-Host "Qt directory for ${arch}: $qtDir" -ForegroundColor Cyan
        } else {
            Write-Host "Qt installation for $arch not found. Skipping $arch build." -ForegroundColor Yellow
            $Architectures = $Architectures | Where-Object { $_ -ne $arch }
        }
    }

    if ($qtDirs.Count -eq 0) {
        Write-Host "No Qt installations found for any architecture." -ForegroundColor Red
        exit 1
    }

    # Check for CMake
    $cmake = "C:\Qt\Tools\CMake_64\bin\cmake.exe"
    if (-not (Test-Path $cmake)) {
        Write-Host "CMake not found at expected location: $cmake" -ForegroundColor Red
        exit 1
    }
    Write-Host "CMake found: $cmake" -ForegroundColor Cyan

    # Check for Visual Studio
    if (-not $env:VCINSTALLDIR) {
        Write-Host "Visual Studio not detected. Please run from VS Developer Command Prompt." -ForegroundColor Red
        exit 1
    }
    Write-Host "Visual Studio: $env:VCINSTALLDIR" -ForegroundColor Cyan

    # Check for WiX (optional)
    $wixAvailable = $false
    try {
        $wixExe = Get-Command "wix.exe" -ErrorAction SilentlyContinue
        if ($wixExe) {
            Write-Host "WiX Toolset found: $($wixExe.Source)" -ForegroundColor Cyan
            $wixAvailable = $true
        }
    } catch {
        Write-Host "WiX Toolset not found. MSI creation will be skipped." -ForegroundColor Yellow
    }

    # Check for MSIX tools (optional)
    $msixAvailable = $false
    try {
        $makeappx = Get-Command "makeappx.exe" -ErrorAction SilentlyContinue
        if ($makeappx) {
            Write-Host "MSIX Packaging Tool found: $($makeappx.Source)" -ForegroundColor Cyan
            $msixAvailable = $true
        }
    } catch {
        Write-Host "MSIX Packaging Tool not found. MSIX creation will be skipped." -ForegroundColor Yellow
    }

    # Determine what to build
    $buildMSI = (-not $ZipOnly) -and (-not $MSIXOnly) -and $wixAvailable
    $buildZip = (-not $MSIOnly) -and (-not $MSIXOnly)
    $buildMSIX = ((-not $ZipOnly) -and (-not $MSIOnly) -and $msixAvailable) -or $MSIXOnly

    Write-Host ""
    Write-Host "Package types to create:" -ForegroundColor Yellow
    Write-Host "  Portable ZIP: $(if ($buildZip) { 'Yes' } else { 'No' })" -ForegroundColor Cyan
    Write-Host "  MSI Installer: $(if ($buildMSI) { 'Yes' } else { 'No' })" -ForegroundColor Cyan
    Write-Host "  MSIX Package: $(if ($buildMSIX) { 'Yes' } else { 'No' })" -ForegroundColor Cyan
    Write-Host ""

    # Build phase
    if (-not $SkipBuild) {
        foreach ($arch in $Architectures) {
            Write-Host ""
            Write-Host "Building UIAList for $arch..." -ForegroundColor Yellow
            
            $buildDir = "build_$arch"
            if (Test-Path $buildDir) {
                Remove-Item $buildDir -Recurse -Force
            }
            New-Item -ItemType Directory -Path $buildDir -Force | Out-Null
            
            $cmakeArch = switch ($arch) {
                "x64" { "x64" }
                "arm64" { "ARM64" }
                default { $arch }
            }
            
            Push-Location $buildDir
            try {
                Write-Host "Configuring build with CMake for $arch..." -ForegroundColor Cyan
                $qtPrefix = $qtDirs[$arch]
                & $cmake .. -G "Visual Studio 17 2022" -A $cmakeArch -DCMAKE_PREFIX_PATH="$qtPrefix"
                if ($LASTEXITCODE -ne 0) { throw "CMake configuration failed for $arch" }
                
                Write-Host "Building project for $arch..." -ForegroundColor Cyan
                & $cmake --build . --config $Configuration --target UIAList
                if ($LASTEXITCODE -ne 0) { throw "Build failed for $arch" }
                
                Write-Host "Build completed successfully for $arch!" -ForegroundColor Green
            } catch {
                Write-Host "Build failed for ${arch}: $_" -ForegroundColor Red
                Pop-Location
                exit 1
            } finally {
                Pop-Location
            }
        }
    } else {
        Write-Host "Skipping build as requested." -ForegroundColor Yellow
    }

    # Function to create staging area with all dependencies
    function Create-StagingArea {
        param([string]$StagingPath, [string]$Architecture)
        
        New-Item -ItemType Directory -Path $StagingPath -Force | Out-Null
        
        # Copy the main executable
        $exePath = "build_$Architecture\$Configuration\UIAList.exe"
        if (Test-Path $exePath) {
            Copy-Item $exePath $StagingPath -Force
            Write-Host "  Copied UIAList.exe ($Architecture)" -ForegroundColor Cyan
        } else {
            throw "UIAList.exe not found at $exePath for $Architecture"
        }
        
        # Use windeployqt for Qt dependencies
        $qtDir = $qtDirs[$Architecture]
        if ($Architecture -eq "arm64") {
            # Use x64 windeployqt for ARM64 binary (works correctly)
            $x64QtDir = $qtDirs["x64"]
            if ($x64QtDir) {
                $windeployqt = "$x64QtDir\bin\windeployqt.exe"
                if (Test-Path $windeployqt) {
                    Write-Host "  Running x64 windeployqt for ARM64 binary..." -ForegroundColor Cyan
                    $targetExe = Join-Path $StagingPath "UIAList.exe"
                    & $windeployqt $targetExe --release --no-translations --no-system-d3d-compiler --no-opengl-sw --dir $StagingPath --force
                }
            }
        } else {
            # Standard x64 deployment
            $windeployqt = "$qtDir\bin\windeployqt.exe"
            if (Test-Path $windeployqt) {
                Write-Host "  Running windeployqt for $Architecture..." -ForegroundColor Cyan
                $targetExe = Join-Path $StagingPath "UIAList.exe"
                & $windeployqt $targetExe --release --no-translations --no-system-d3d-compiler --no-opengl-sw --dir $StagingPath
            }
        }
        
        # Copy additional files
        $filesToCopy = @("LICENSE", "README.md", "PrivacyPolicy.html")
        foreach ($file in $filesToCopy) {
            if (Test-Path $file) {
                Copy-Item $file $StagingPath -Force
                Write-Host "  Copied $file" -ForegroundColor Cyan
            }
        }
        
        Write-Host "  Staging area created: $StagingPath" -ForegroundColor Green
    }

    # Create portable ZIP packages
    if ($buildZip) {
        Write-Host ""
        Write-Host "Creating portable ZIP packages..." -ForegroundColor Blue
        
        foreach ($arch in $Architectures) {
            Write-Host ""
            Write-Host "Creating ZIP package for $arch..." -ForegroundColor Blue
            
            $zipStagingDir = "$OutputDir\portable\UIAList-Portable-$arch"
            Create-StagingArea -StagingPath $zipStagingDir -Architecture $arch
            
            # Create ZIP file
            $zipFile = "$OutputDir\UIAList-Portable-v0.1.0-$arch.zip"
            try {
                Compress-Archive -Path "$zipStagingDir\*" -DestinationPath $zipFile -Force
                Write-Host "√ Portable ZIP created: UIAList-Portable-v0.1.0-$arch.zip" -ForegroundColor Green
                $zipSize = (Get-Item $zipFile).Length / 1MB
                Write-Host "  Size: $([math]::Round($zipSize, 2)) MB" -ForegroundColor Gray
            } catch {
                Write-Host "× Failed to create ZIP file for ${arch}: $_" -ForegroundColor Red
            }
        }
    }

    # Create MSIX packages for Microsoft Store
    if ($buildMSIX) {
        Write-Host ""
        Write-Host "Creating MSIX packages for Microsoft Store..." -ForegroundColor Blue
        
        $msixPackages = @()
        
        foreach ($arch in $Architectures) {
            Write-Host ""
            Write-Host "Creating MSIX package for $arch..." -ForegroundColor Blue
            
            # Create staging area with all dependencies
            $msixStagingDir = "$OutputDir\msix\staging_$arch"
            Create-StagingArea -StagingPath $msixStagingDir -Architecture $arch
            
            # Copy MSIX manifest and assets (using new paths)
            Copy-Item "resources\manifests\Package.appxmanifest" "$msixStagingDir\AppxManifest.xml" -Force
            Copy-Item "resources\assets\*" "$msixStagingDir\Assets" -Recurse -Force
            
            # Create MSIX package
            $msixFile = "$OutputDir\UIAList-Store-v0.1.0-$arch.msix"
            
            try {
                Write-Host "  Packaging MSIX for $arch..." -ForegroundColor Cyan
                & makeappx pack /d $msixStagingDir /p $msixFile /o
                
                if ($LASTEXITCODE -eq 0) {
                    Write-Host "√ MSIX package created: UIAList-Store-v0.1.0-$arch.msix" -ForegroundColor Green
                    $msixSize = (Get-Item $msixFile).Length / 1MB
                    Write-Host "  Size: $([math]::Round($msixSize, 2)) MB" -ForegroundColor Gray
                    $msixPackages += $msixFile
                } else {
                    throw "makeappx failed for $arch"
                }
            } catch {
                Write-Host "× Failed to create MSIX for ${arch}: $_" -ForegroundColor Red
            }
        }
        
        # Create MSIX Bundle for Microsoft Store (contains both architectures)
        if ($msixPackages.Count -gt 1) {
            Write-Host ""
            Write-Host "Creating MSIX Bundle for Microsoft Store..." -ForegroundColor Blue
            
            # Create bundle staging directory
            $bundleStagingDir = "$OutputDir\msix\bundle_staging"
            New-Item -ItemType Directory -Path $bundleStagingDir -Force | Out-Null
            
            # Copy MSIX packages to bundle directory
            foreach ($msixFile in $msixPackages) {
                $fileName = Split-Path $msixFile -Leaf
                Copy-Item $msixFile "$bundleStagingDir\$fileName" -Force
            }
            
            # Create bundle manifest
            $bundleManifestPath = "$bundleStagingDir\AppxBundleManifest.xml"
            $xml = '<?xml version="1.0" encoding="UTF-8"?>' + "`r`n"
            $xml += '<Bundle xmlns="http://schemas.microsoft.com/appx/2013/bundle">' + "`r`n"
            $xml += '  <Identity Name="StefanLohmaier.UIAList"' + "`r`n"
            $xml += '            Publisher="CN=Stefan Lohmaier"' + "`r`n"
            $xml += '            Version="0.1.0.0" />' + "`r`n"
            $xml += '  <Packages>' + "`r`n"
            
            foreach ($arch in $Architectures) {
                if (Test-Path "$OutputDir\UIAList-Store-v0.1.0-$arch.msix") {
                    $xml += "    <Package Type=`"application`" Version=`"0.1.0.0`" Architecture=`"$arch`" FileName=`"UIAList-Store-v0.1.0-$arch.msix`" />" + "`r`n"
                }
            }
            
            $xml += '  </Packages>' + "`r`n"
            $xml += '</Bundle>' + "`r`n"
            
            Set-Content -Path $bundleManifestPath -Value $xml -Encoding UTF8
            
            # Create bundle
            $bundleFile = "$OutputDir\UIAList-Store-Bundle-v0.1.0.msixbundle"
            
            try {
                Write-Host "  Creating MSIX Bundle..." -ForegroundColor Cyan
                & makeappx bundle /d $bundleStagingDir /p $bundleFile /o
                
                if ($LASTEXITCODE -eq 0) {
                    Write-Host "√ MSIX Bundle created: UIAList-Store-Bundle-v0.1.0.msixbundle" -ForegroundColor Green
                    $bundleSize = (Get-Item $bundleFile).Length / 1MB
                    Write-Host "  Size: $([math]::Round($bundleSize, 2)) MB (both architectures)" -ForegroundColor Gray
                } else {
                    throw "Bundle creation failed"
                }
            } catch {
                Write-Host "× Failed to create MSIX Bundle: $_" -ForegroundColor Red
            }
        }
    }

    # Show final summary
    Write-Host ""
    Write-Host "============================================" -ForegroundColor Green
    Write-Host "Deployment Complete!" -ForegroundColor Green  
    Write-Host "============================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "Output directory: $OutputDir" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "Created packages:" -ForegroundColor Yellow

    $packageCount = 0
    foreach ($arch in $Architectures) {
        if ($buildZip -and (Test-Path "$OutputDir\UIAList-Portable-v0.1.0-$arch.zip")) {
            Write-Host "√ Portable ZIP ($arch): UIAList-Portable-v0.1.0-$arch.zip" -ForegroundColor Green
            $packageCount++
        }
        
        if ($buildMSIX -and (Test-Path "$OutputDir\UIAList-Store-v0.1.0-$arch.msix")) {
            Write-Host "√ MSIX Package ($arch): UIAList-Store-v0.1.0-$arch.msix" -ForegroundColor Green
            $packageCount++
        }
    }

    if ($buildMSIX -and (Test-Path "$OutputDir\UIAList-Store-Bundle-v0.1.0.msixbundle")) {
        Write-Host "√ MSIX Bundle (Multi-Arch): UIAList-Store-Bundle-v0.1.0.msixbundle" -ForegroundColor Green
        Write-Host "  Ready for Microsoft Store submission" -ForegroundColor Gray
        $packageCount++
    }

    if ($packageCount -eq 0) {
        Write-Host "No packages were created." -ForegroundColor Yellow
    } else {
        Write-Host ""
        Write-Host "Successfully created $packageCount package(s)!" -ForegroundColor Green
        Write-Host "Distribution ready for deployment!" -ForegroundColor Green
    }

} finally {
    Pop-Location
}