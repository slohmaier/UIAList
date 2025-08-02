# PowerShell script to build UIAList and create MSIX package for Microsoft Store
# This script automates the build and packaging process

param(
    [string]$Configuration = "Release",
    [string]$OutputDir = "package_output",
    [switch]$SkipBuild = $false,
    [switch]$SignPackage = $false,
    [string]$CertificateThumbprint = ""
)

# Colors for output
$Red = "Red"
$Green = "Green"
$Yellow = "Yellow"
$Cyan = "Cyan"

Write-Host "UIAList MSIX Packaging Script" -ForegroundColor $Green
Write-Host "===============================" -ForegroundColor $Green
Write-Host ""

# Check prerequisites
Write-Host "Checking prerequisites..." -ForegroundColor $Yellow

# Check for Qt
$qtDir = $env:Qt6_DIR
if (-not $qtDir) {
    $qtDir = $env:QTDIR
}

if (-not $qtDir) {
    Write-Host "Qt installation not found. Please set Qt6_DIR or QTDIR environment variable." -ForegroundColor $Red
    exit 1
}

Write-Host "Qt directory: $qtDir" -ForegroundColor $Cyan

# Check for CMake
try {
    $cmakeVersion = & cmake --version 2>$null
    Write-Host "CMake found: $($cmakeVersion[0])" -ForegroundColor $Cyan
} catch {
    Write-Host "CMake not found. Please install CMake." -ForegroundColor $Red
    exit 1
}

# Check for MSVC
if (-not $env:VCINSTALLDIR) {
    Write-Host "Visual Studio not detected. Please run from VS Developer Command Prompt." -ForegroundColor $Red
    exit 1
}

Write-Host "Visual Studio: $env:VCINSTALLDIR" -ForegroundColor $Cyan

# Check for Windows SDK
try {
    $makeappx = Get-Command "makeappx.exe" -ErrorAction Stop
    Write-Host "Windows SDK found: $($makeappx.Source)" -ForegroundColor $Cyan
} catch {
    Write-Host "Windows SDK (makeappx.exe) not found. Please install Windows SDK." -ForegroundColor $Red
    exit 1
}

Write-Host ""

# Create output directory
if (Test-Path $OutputDir) {
    Write-Host "Cleaning existing output directory..." -ForegroundColor $Yellow
    Remove-Item $OutputDir -Recurse -Force
}
New-Item -ItemType Directory -Path $OutputDir -Force | Out-Null

if (-not $SkipBuild) {
    Write-Host "Building UIAList..." -ForegroundColor $Yellow
    
    # Create build directory
    $buildDir = "build_msix"
    if (Test-Path $buildDir) {
        Remove-Item $buildDir -Recurse -Force
    }
    New-Item -ItemType Directory -Path $buildDir -Force | Out-Null
    
    # Configure with CMake
    Push-Location $buildDir
    try {
        Write-Host "Configuring build with CMake..." -ForegroundColor $Cyan
        & cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=$Configuration
        if ($LASTEXITCODE -ne 0) {
            throw "CMake configuration failed"
        }
        
        # Build the project
        Write-Host "Building project..." -ForegroundColor $Cyan
        & cmake --build . --config $Configuration --target UIAList
        if ($LASTEXITCODE -ne 0) {
            throw "Build failed"
        }
        
        # Manual file copying to staging area (more reliable than CMake install)
        $stagingDir = Join-Path (Join-Path $PSScriptRoot $OutputDir) "staging"
        Write-Host "Copying files to staging area: $stagingDir" -ForegroundColor $Cyan
        
        # Create staging directory
        New-Item -ItemType Directory -Path $stagingDir -Force | Out-Null
        
        # Copy the main executable
        $exePath = Join-Path $Configuration "UIAList.exe"
        if (Test-Path $exePath) {
            Copy-Item $exePath $stagingDir -Force
            Write-Host "Copied UIAList.exe" -ForegroundColor $Cyan
        } else {
            throw "UIAList.exe not found at $exePath"
        }
        
        # Use windeployqt to copy Qt dependencies
        $qtBinDir = $null
        if ($qtDir) {
            $qtBinDir = Join-Path $qtDir "bin"
            if (-not (Test-Path $qtBinDir)) {
                # Try alternative Qt directory structures
                $qtBinDir = Join-Path $qtDir ".." ".." ".." "bin"
            }
        }
        
        if ($qtBinDir -and (Test-Path $qtBinDir)) {
            $windeployqt = Join-Path $qtBinDir "windeployqt.exe"
            if (Test-Path $windeployqt) {
                Write-Host "Running windeployqt..." -ForegroundColor $Cyan
                $targetExe = Join-Path $stagingDir "UIAList.exe"
                & "$windeployqt" "$targetExe" --release --no-translations --no-system-d3d-compiler --no-opengl-sw --dir "$stagingDir"
                if ($LASTEXITCODE -ne 0) {
                    Write-Host "windeployqt failed, but continuing..." -ForegroundColor $Yellow
                }
            } else {
                Write-Host "windeployqt not found, manual Qt deployment needed" -ForegroundColor $Yellow
            }
        } else {
            Write-Host "Qt bin directory not found, manual Qt deployment needed" -ForegroundColor $Yellow
        }
        
        # Copy Package.appxmanifest and Assets from project root
        $manifestSource = Join-Path $PSScriptRoot "Package.appxmanifest"
        $manifestTarget = Join-Path $stagingDir "AppxManifest.xml"
        Copy-Item $manifestSource $manifestTarget -Force
        Write-Host "Copied Package.appxmanifest as AppxManifest.xml" -ForegroundColor $Cyan
        
        $assetsSource = Join-Path $PSScriptRoot "Assets"
        $assetsTarget = Join-Path $stagingDir "Assets"
        if (Test-Path $assetsSource) {
            Copy-Item $assetsSource $assetsTarget -Recurse -Force
            Write-Host "Copied Assets directory" -ForegroundColor $Cyan
        } else {
            Write-Host "Warning: Assets directory not found" -ForegroundColor $Yellow
        }
        
        # Copy PrivacyPolicy.html if it exists
        $privacyPolicy = Join-Path $PSScriptRoot "PrivacyPolicy.html"
        if (Test-Path $privacyPolicy) {
            Copy-Item $privacyPolicy $stagingDir -Force
            Write-Host "Copied PrivacyPolicy.html" -ForegroundColor $Cyan
        }
        
    } catch {
        Write-Host "Build failed: $_" -ForegroundColor $Red
        Pop-Location
        exit 1
    } finally {
        Pop-Location
    }
    
    Write-Host "Build completed successfully!" -ForegroundColor $Green
} else {
    Write-Host "Skipping build as requested." -ForegroundColor $Yellow
}

Write-Host ""
Write-Host "Preparing MSIX package..." -ForegroundColor $Yellow

$stagingDir = Join-Path $OutputDir "staging"
$packageDir = Join-Path $OutputDir "package"

# Ensure we have the staging directory
if (-not (Test-Path $stagingDir)) {
    Write-Host "Staging directory not found. Build may have failed." -ForegroundColor $Red
    exit 1
}

# Create package directory structure
New-Item -ItemType Directory -Path $packageDir -Force | Out-Null

# Copy all files to package directory
Write-Host "Copying files to package directory..." -ForegroundColor $Cyan
Copy-Item "$stagingDir\*" $packageDir -Recurse -Force

# Generate icons if they don't exist
$assetsDir = Join-Path $packageDir "Assets"
if (-not (Test-Path "$assetsDir\Square150x150Logo.png")) {
    Write-Host "Generating required store assets..." -ForegroundColor $Cyan
    $generateScript = Join-Path $PSScriptRoot "generate_store_assets.ps1"
    if (Test-Path $generateScript) {
        Push-Location $PSScriptRoot
        & powershell -ExecutionPolicy Bypass -File $generateScript
        Pop-Location
        
        # Copy generated assets to package
        if (Test-Path "Assets") {
            Copy-Item "Assets\*" $assetsDir -Force
        }
    } else {
        Write-Host "Warning: Store assets not found and generation script missing." -ForegroundColor $Yellow
        Write-Host "Please ensure all required icon files are in the Assets directory." -ForegroundColor $Yellow
    }
}

# Create the MSIX package
Write-Host ""
Write-Host "Creating MSIX package..." -ForegroundColor $Yellow

$packageFile = Join-Path $OutputDir "UIAList.msix"
try {
    & makeappx pack /d $packageDir /p $packageFile /overwrite
    if ($LASTEXITCODE -ne 0) {
        throw "makeappx failed"
    }
    
    Write-Host "MSIX package created: $packageFile" -ForegroundColor $Green
} catch {
    Write-Host "Failed to create MSIX package: $_" -ForegroundColor $Red
    exit 1
}

# Sign the package if requested
if ($SignPackage -and $CertificateThumbprint) {
    Write-Host ""
    Write-Host "Signing MSIX package..." -ForegroundColor $Yellow
    
    try {
        & signtool sign /fd SHA256 /sha1 $CertificateThumbprint /t http://timestamp.digicert.com $packageFile
        if ($LASTEXITCODE -ne 0) {
            throw "Package signing failed"
        }
        
        Write-Host "Package signed successfully!" -ForegroundColor $Green
    } catch {
        Write-Host "Failed to sign package: $_" -ForegroundColor $Red
        Write-Host "Package created but not signed." -ForegroundColor $Yellow
    }
}

Write-Host ""
Write-Host "============================================" -ForegroundColor $Green
Write-Host "MSIX Package Creation Complete!" -ForegroundColor $Green
Write-Host "============================================" -ForegroundColor $Green
Write-Host ""
Write-Host "Package location: $packageFile" -ForegroundColor $Cyan
Write-Host "Staging directory: $stagingDir" -ForegroundColor $Cyan
Write-Host ""
Write-Host "Next steps for Microsoft Store submission:" -ForegroundColor $Yellow
Write-Host "1. Test the MSIX package on clean Windows systems" -ForegroundColor $White
Write-Host "2. Prepare store listing with screenshots and descriptions" -ForegroundColor $White
Write-Host "3. Upload to Microsoft Partner Center" -ForegroundColor $White
Write-Host "4. Complete store certification process" -ForegroundColor $White
Write-Host ""

# Show package info
try {
    $packageSize = (Get-Item $packageFile).Length / 1MB
    Write-Host "Package size: $([math]::Round($packageSize, 2)) MB" -ForegroundColor $Cyan
} catch {
    # Size calculation failed, ignore
}

Write-Host "Build completed successfully!" -ForegroundColor $Green