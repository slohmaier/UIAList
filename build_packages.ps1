# PowerShell script to build UIAList and create both MSI installer and portable ZIP
# This script automates building multiple distribution formats

param(
    [string]$Configuration = "Release",
    [string]$OutputDir = "dist",
    [string[]]$Architectures = @("x64", "arm64"),
    [switch]$SkipBuild = $false,
    [switch]$MSIOnly = $false,
    [switch]$ZipOnly = $false,
    [switch]$Clean = $false
)

# Colors for output
$Red = "Red"
$Green = "Green"
$Yellow = "Yellow"
$Cyan = "Cyan"
$Blue = "Blue"

Write-Host "UIAList Multi-Architecture Package Builder" -ForegroundColor $Green
Write-Host "=========================================" -ForegroundColor $Green
Write-Host "Building for architectures: $($Architectures -join ', ')" -ForegroundColor $Cyan
Write-Host ""

# Clean output directory if requested
if ($Clean -and (Test-Path $OutputDir)) {
    Write-Host "Cleaning output directory..." -ForegroundColor $Yellow
    Remove-Item $OutputDir -Recurse -Force
}

# Create output directories
New-Item -ItemType Directory -Path $OutputDir -Force | Out-Null
New-Item -ItemType Directory -Path "$OutputDir\msi" -Force | Out-Null
New-Item -ItemType Directory -Path "$OutputDir\portable" -Force | Out-Null

# Check prerequisites
Write-Host "Checking prerequisites..." -ForegroundColor $Yellow

# Check for Qt for each architecture
$qtDirs = @{}
foreach ($arch in $Architectures) {
    $qtDir = $null
    
    # Map architecture names
    $qtArchName = switch ($arch) {
        "x64" { "msvc2022_64" }
        "arm64" { "msvc2022_arm64" }
        default { $arch }
    }
    
    # Try environment variables first
    if ($arch -eq "x64") {
        $qtDir = $env:Qt6_DIR_x64
        if (-not $qtDir) { $qtDir = $env:Qt6_DIR }
    } elseif ($arch -eq "arm64") {
        $qtDir = $env:Qt6_DIR_ARM64
    }
    
    # Try common Qt installation paths
    if (-not $qtDir) {
        $commonPaths = @(
            "C:\Qt\6.9.2\$qtArchName",
            "C:\Qt\6.9\$qtArchName",
            "C:\Qt\6.8\$qtArchName",
            "C:\Qt\6.7\$qtArchName",
            "C:\Qt\6.6\$qtArchName"
        )
        foreach ($path in $commonPaths) {
            # For ARM64, check for Qt installation without requiring windeployqt
            if ($arch -eq "arm64") {
                if (Test-Path "$path\bin\Qt6Core.dll") {
                    $qtDir = $path
                    break
                }
            } else {
                # For x64, require windeployqt
                if (Test-Path "$path\bin\windeployqt.exe") {
                    $qtDir = $path
                    break
                }
            }
        }
    }
    
    if (-not $qtDir) {
        Write-Host "Qt installation for $arch not found. Skipping $arch build." -ForegroundColor $Yellow
        $Architectures = $Architectures | Where-Object { $_ -ne $arch }
    } else {
        $qtDirs[$arch] = $qtDir
        Write-Host "Qt directory for ${arch}: $qtDir" -ForegroundColor $Cyan
    }
}

if ($qtDirs.Count -eq 0) {
    Write-Host "No Qt installations found for any architecture." -ForegroundColor $Red
    Write-Host "Please install Qt for x64 and/or ARM64 to C:\Qt" -ForegroundColor $Red
    exit 1
}

# Check for CMake
try {
    $cmakeVersion = & "C:\Qt\Tools\CMake_64\bin\cmake.exe" --version 2>$null
    if ($LASTEXITCODE -ne 0) {
        throw "CMake check failed"
    }
    Write-Host "CMake found: $($cmakeVersion[0])" -ForegroundColor $Cyan
} catch {
    Write-Host "CMake not found. Please install CMake or check Qt Tools installation." -ForegroundColor $Red
    exit 1
}

# Check for MSVC
if (-not $env:VCINSTALLDIR) {
    Write-Host "Visual Studio not detected. Please run from VS Developer Command Prompt." -ForegroundColor $Red
    exit 1
}

Write-Host "Visual Studio: $env:VCINSTALLDIR" -ForegroundColor $Cyan

# Check for WiX Toolset (for MSI creation)
if (-not $ZipOnly) {
    try {
        $candle = Get-Command "candle.exe" -ErrorAction Stop
        $light = Get-Command "light.exe" -ErrorAction Stop
        Write-Host "WiX Toolset found: $($candle.Source)" -ForegroundColor $Cyan
    } catch {
        # Try looking in common WiX installation paths
        $wixPaths = @(
            "C:\WiX",
            "C:\Program Files (x86)\WiX Toolset v3.11\bin",
            "C:\Program Files (x86)\WiX Toolset v3.14\bin",
            "C:\Program Files\WiX Toolset v3.11\bin",
            "C:\Program Files\WiX Toolset v3.14\bin",
            "C:\Program Files (x86)\WiX Toolset\bin",
            "C:\Program Files\WiX Toolset\bin"
        )
        $wixFound = $false
        foreach ($path in $wixPaths) {
            if ((Test-Path "$path\candle.exe") -and (Test-Path "$path\light.exe")) {
                $env:PATH = $env:PATH + ";$path"
                Write-Host "WiX Toolset found: $path" -ForegroundColor $Cyan
                $wixFound = $true
                break
            }
        }
        
        if (-not $wixFound) {
            Write-Host "WiX Toolset not found. MSI creation will be skipped." -ForegroundColor $Yellow
            Write-Host "Install WiX Toolset from https://wixtoolset.org/ to enable MSI creation." -ForegroundColor $Yellow
            $MSIOnly = $false
        }
    }
}

Write-Host ""

if (-not $SkipBuild) {
    foreach ($arch in $Architectures) {
        Write-Host "" 
        Write-Host "Building UIAList for $arch..." -ForegroundColor $Yellow
        
        # Create architecture-specific build directory
        $buildDir = "build_packages_$arch"
        if (Test-Path $buildDir) {
            Remove-Item $buildDir -Recurse -Force
        }
        New-Item -ItemType Directory -Path $buildDir -Force | Out-Null
        
        # Map architecture to CMake generator platform
        $cmakeArch = switch ($arch) {
            "x64" { "x64" }
            "arm64" { "ARM64" }
            default { $arch }
        }
        
        # Configure with CMake
        Push-Location $buildDir
        try {
            Write-Host "Configuring build with CMake for $arch..." -ForegroundColor $Cyan
            
            # Set Qt prefix path for this architecture
            $qtPrefix = $qtDirs[$arch]
            
            & "C:\Qt\Tools\CMake_64\bin\cmake.exe" .. -G "Visual Studio 17 2022" -A $cmakeArch -DCMAKE_BUILD_TYPE=$Configuration -DCMAKE_PREFIX_PATH="$qtPrefix"
            if ($LASTEXITCODE -ne 0) {
                throw "CMake configuration failed for $arch"
            }
            
            # Build the project
            Write-Host "Building project for $arch..." -ForegroundColor $Cyan
            & "C:\Qt\Tools\CMake_64\bin\cmake.exe" --build . --config $Configuration --target UIAList
            if ($LASTEXITCODE -ne 0) {
                throw "Build failed for $arch"
            }
            
            Write-Host "Build completed successfully for $arch!" -ForegroundColor $Green
            
        } catch {
            Write-Host "Build failed for ${arch}: $_" -ForegroundColor $Red
            Pop-Location
            exit 1
        } finally {
            Pop-Location
        }
    }
} else {
    Write-Host "Skipping build as requested." -ForegroundColor $Yellow
}

# Determine what to build
$wixAvailable = (Get-Command "candle.exe" -ErrorAction SilentlyContinue) -ne $null
$buildMSI = (-not $ZipOnly) -and $wixAvailable
$buildZip = (-not $MSIOnly)

Write-Host ""
Write-Host "Creating packages..." -ForegroundColor $Yellow
Write-Host "MSI Installer: $(if ($buildMSI) { 'Yes' } else { 'No' })" -ForegroundColor $Cyan
Write-Host "Portable ZIP: $(if ($buildZip) { 'Yes' } else { 'No' })" -ForegroundColor $Cyan
Write-Host ""

# Manual ARM64 deployment function
function Deploy-ARM64-Manually {
    param(
        [string]$StagingPath,
        [string]$QtDir
    )
    
    Write-Host "Manually deploying Qt ARM64 dependencies..." -ForegroundColor $Cyan
    
    # Core Qt libraries needed for UIAList
    $qtLibraries = @(
        "Qt6Core.dll",
        "Qt6Gui.dll", 
        "Qt6Widgets.dll",
        "Qt6Network.dll",
        "Qt6Svg.dll"
    )
    
    $qtBinDir = Join-Path $QtDir "bin"
    
    # Copy Qt DLLs
    foreach ($lib in $qtLibraries) {
        $libPath = Join-Path $qtBinDir $lib
        if (Test-Path $libPath) {
            Copy-Item $libPath $StagingPath -Force
            Write-Host "  Copied $lib" -ForegroundColor $Cyan
        } else {
            Write-Host "  Warning: $lib not found at $libPath" -ForegroundColor $Yellow
        }
    }
    
    # Copy Visual C++ Redistributable
    $vcRedist = Join-Path $qtBinDir "vc_redist.x64.exe"
    if (Test-Path $vcRedist) {
        Copy-Item $vcRedist $StagingPath -Force
        Write-Host "  Copied vc_redist.x64.exe" -ForegroundColor $Cyan
    }
    
    # Copy essential plugins
    $pluginDirs = @(
        @{Source = "platforms"; Files = @("qwindows.dll")},
        @{Source = "imageformats"; Files = @("qgif.dll", "qico.dll", "qjpeg.dll", "qsvg.dll")},
        @{Source = "iconengines"; Files = @("qsvgicon.dll")},
        @{Source = "styles"; Files = @("qmodernwindowsstyle.dll")}
    )
    
    $qtPluginDir = Join-Path $QtDir "plugins"
    
    foreach ($pluginDir in $pluginDirs) {
        $sourceDir = Join-Path $qtPluginDir $pluginDir.Source
        $targetDir = Join-Path $StagingPath $pluginDir.Source
        
        if (Test-Path $sourceDir) {
            New-Item -ItemType Directory -Path $targetDir -Force | Out-Null
            
            foreach ($file in $pluginDir.Files) {
                $sourceFile = Join-Path $sourceDir $file
                if (Test-Path $sourceFile) {
                    Copy-Item $sourceFile $targetDir -Force
                    Write-Host "  Copied plugin $($pluginDir.Source)\$file" -ForegroundColor $Cyan
                }
            }
        }
    }
    
    Write-Host "Manual ARM64 deployment completed." -ForegroundColor $Green
}

# Common staging function
function Create-StagingArea {
    param(
        [string]$StagingPath,
        [string]$Architecture
    )
    
    # Create staging directory
    New-Item -ItemType Directory -Path $StagingPath -Force | Out-Null
    
    # Copy the main executable
    $exePath = "build_packages_$Architecture\$Configuration\UIAList.exe"
    if (Test-Path $exePath) {
        Copy-Item $exePath $StagingPath -Force
        Write-Host "Copied UIAList.exe ($Architecture)" -ForegroundColor $Cyan
    } else {
        throw "UIAList.exe not found at $exePath for $Architecture"
    }
    
    # Use windeployqt to copy Qt dependencies
    $qtDir = $qtDirs[$Architecture]
    
    if ($Architecture -eq "arm64") {
        # For ARM64, use x64 windeployqt if available, or manual deployment
        $x64QtDir = $qtDirs["x64"]
        if ($x64QtDir) {
            $windeployqt = Join-Path $x64QtDir "bin\windeployqt.exe"
            if (Test-Path $windeployqt) {
                Write-Host "Running x64 windeployqt for ARM64 binary..." -ForegroundColor $Cyan
                $targetExe = Join-Path $StagingPath "UIAList.exe"
                & "$windeployqt" "$targetExe" --release --no-translations --no-system-d3d-compiler --no-opengl-sw --dir "$StagingPath" --force
                if ($LASTEXITCODE -ne 0) {
                    Write-Host "windeployqt failed for ARM64, falling back to manual deployment..." -ForegroundColor $Yellow
                    # Manual deployment for ARM64
                    Deploy-ARM64-Manually -StagingPath $StagingPath -QtDir $qtDir
                }
            } else {
                Write-Host "x64 windeployqt not found, using manual ARM64 deployment..." -ForegroundColor $Yellow
                Deploy-ARM64-Manually -StagingPath $StagingPath -QtDir $qtDir
            }
        } else {
            Write-Host "x64 Qt not available, using manual ARM64 deployment..." -ForegroundColor $Yellow
            Deploy-ARM64-Manually -StagingPath $StagingPath -QtDir $qtDir
        }
    } else {
        # For x64, use standard windeployqt
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
                Write-Host "Running windeployqt for $Architecture..." -ForegroundColor $Cyan
                $targetExe = Join-Path $StagingPath "UIAList.exe"
                & "$windeployqt" "$targetExe" --release --no-translations --no-system-d3d-compiler --no-opengl-sw --dir "$StagingPath"
                if ($LASTEXITCODE -ne 0) {
                    Write-Host "windeployqt failed for $Architecture, but continuing..." -ForegroundColor $Yellow
                }
            } else {
                Write-Host "windeployqt not found for $Architecture, manual Qt deployment needed" -ForegroundColor $Yellow
            }
        }
    }
    
    # Copy additional files
    $filesToCopy = @("LICENSE", "README.md", "PrivacyPolicy.html")
    foreach ($file in $filesToCopy) {
        if (Test-Path $file) {
            Copy-Item $file $StagingPath -Force
            Write-Host "Copied $file" -ForegroundColor $Cyan
        }
    }
    
    Write-Host "Staging area created: $StagingPath" -ForegroundColor $Green
}

# Create portable ZIP packages for each architecture
if ($buildZip) {
    foreach ($arch in $Architectures) {
        Write-Host ""
        Write-Host "Creating portable ZIP package for $arch..." -ForegroundColor $Blue
        
        $zipStagingDir = "$OutputDir\portable\UIAList-Portable-$arch"
        Create-StagingArea -StagingPath $zipStagingDir -Architecture $arch
    
    # Create a simple batch file to run the application
    $batchContent = @"
@echo off
echo Starting UIAList...
echo.
echo UIAList - UI Automation Control Browser
echo =====================================
echo.
echo This is the portable version. No installation required.
echo The application will start in system tray.
echo.
echo Press Ctrl+Alt+U to show the UIAList window.
echo.
start /b UIAList.exe
echo UIAList started. Check your system tray.
pause
"@
    Set-Content -Path "$zipStagingDir\Run-UIAList.bat" -Value $batchContent -Encoding ASCII
    
    # Create README for portable version
    $portableReadme = @"
# UIAList Portable

This is the portable version of UIAList - no installation required!

## Quick Start

1. Run `Run-UIAList.bat` or double-click `UIAList.exe`
2. The application starts in the system tray
3. Press `Ctrl+Alt+U` to open the UIAList window
4. Right-click the tray icon for options

## Files

- `UIAList.exe` - Main application
- `Run-UIAList.bat` - Convenient launcher with instructions
- Qt DLLs and dependencies - Required runtime files
- `LICENSE` - Software license
- `README.md` - Full documentation
- `PrivacyPolicy.html` - Privacy policy

## Requirements

- Windows 10 or later
- No additional software installation required

## Support

For questions or issues:
- GitHub: https://github.com/slohmaier/UIAList
- Email: stefan@slohmaier.de

---
UIAList Portable v0.1.0
"@
    Set-Content -Path "$zipStagingDir\README-Portable.txt" -Value $portableReadme -Encoding UTF8
    
        # Create the ZIP file
        $zipFile = "$OutputDir\UIAList-Portable-v0.1.0-$arch.zip"
        try {
            Compress-Archive -Path "$zipStagingDir\*" -DestinationPath $zipFile -Force
            Write-Host "Portable ZIP created: $zipFile" -ForegroundColor $Green
            
            $zipSize = (Get-Item $zipFile).Length / 1MB
            Write-Host "ZIP size: $([math]::Round($zipSize, 2)) MB" -ForegroundColor $Cyan
        } catch {
            Write-Host "Failed to create ZIP file for ${arch}: $_" -ForegroundColor $Red
        }
    }
}

# Create MSI installer packages for each architecture
if ($buildMSI) {
    foreach ($arch in $Architectures) {
        Write-Host ""
        Write-Host "Creating MSI installer package for $arch..." -ForegroundColor $Blue
        
        $msiStagingDir = "$OutputDir\msi\staging_$arch"
        Create-StagingArea -StagingPath $msiStagingDir -Architecture $arch
    
        # Create WiX source file
        $wixFile = "$OutputDir\msi\UIAList_$arch.wxs"
    $wixContent = @"
<?xml version="1.0" encoding="UTF-8"?>
<Wix xmlns="http://schemas.microsoft.com/wix/2006/wi">
  <Product Id="*" 
           Name="UIAList" 
           Language="1033" 
           Version="0.1.0.0" 
           Manufacturer="Stefan Lohmaier" 
           UpgradeCode="12345678-1234-5678-9ABC-123456789ABC">
    
    <Package InstallerVersion="200" Compressed="yes" InstallScope="perMachine" 
             Description="UIAList - UI Automation Control Browser" 
             Comments="A utility for browsing and inspecting UI Automation controls"
             Keywords="UIAList,UI Automation,Accessibility,Inspector" />
    
    <!-- Upgrade handling - removes older versions automatically -->
    <MajorUpgrade DowngradeErrorMessage="A newer version of [ProductName] is already installed."
                  Schedule="afterInstallInitialize" />
    
    <MediaTemplate EmbedCab="yes" />
    
    <!-- WiX UI extension for standard installer interface -->
    <UI>
      <UIRef Id="WixUI_InstallDir" />
    </UI>
    
    <!-- Set the installation directory property -->
    <Property Id="WIXUI_INSTALLDIR" Value="INSTALLFOLDER" />
    
    <!-- Custom dialogs properties -->
    <Property Id="WIXUI_EXITDIALOGOPTIONALTEXT" Value="Thank you for installing UIAList - UI Automation Control Browser. Press Ctrl+Alt+U to launch the application from anywhere." />

    <Feature Id="ProductFeature" Title="UIAList" Level="1">
      <ComponentGroupRef Id="ProductComponents" />
      <ComponentGroupRef Id="StagingFiles" />
      <ComponentRef Id="ApplicationShortcut" />
      <ComponentRef Id="DesktopShortcut" />
    </Feature>

    <!-- Directory structure -->
    <Directory Id="TARGETDIR" Name="SourceDir">
      <Directory Id="ProgramFilesFolder">
        <Directory Id="INSTALLFOLDER" Name="UIAList" />
      </Directory>
      <Directory Id="ProgramMenuFolder">
        <Directory Id="ApplicationProgramsFolder" Name="UIAList" />
      </Directory>
      <Directory Id="DesktopFolder" Name="Desktop" />
    </Directory>

    <!-- Registry and shortcut components only - files handled by heat.exe -->
    <ComponentGroup Id="ProductComponents" Directory="INSTALLFOLDER">
      <!-- Registry component -->
      <Component Id="UIAListRegistry" Guid="87654321-DCBA-4321-DCBA-987654321CBA">
        <RegistryValue Root="HKLM" 
                       Key="Software\Stefan Lohmaier\UIAList" 
                       Name="InstallPath" 
                       Type="string" 
                       Value="[INSTALLFOLDER]" 
                       KeyPath="yes" />
      </Component>
      
      <!-- Start menu shortcut as separate component -->
      <Component Id="StartMenuShortcut" Guid="11111111-2222-3333-4444-555555555555">
        <Shortcut Id="StartMenuShortcutFile" 
                  Directory="ApplicationProgramsFolder" 
                  Name="UIAList"
                  Description="UI Automation Control Browser"
                  Target="[INSTALLFOLDER]UIAList.exe"
                  WorkingDirectory="INSTALLFOLDER"
                  Icon="UIAList.exe"
                  IconIndex="0" />
        
        <RegistryValue Root="HKCU" 
                       Key="Software\Stefan Lohmaier\UIAList" 
                       Name="StartMenuShortcut" 
                       Type="integer" 
                       Value="1" 
                       KeyPath="yes" />
      </Component>
    </ComponentGroup>

    <!-- Desktop shortcut -->
    <Component Id="DesktopShortcut" Directory="DesktopFolder" Guid="AAAAAAAA-BBBB-CCCC-DDDD-EEEEEEEEEEEE">
      <Shortcut Id="DesktopShortcutFile"
                Name="UIAList"
                Description="UI Automation Control Browser"
                Target="[INSTALLFOLDER]UIAList.exe"
                WorkingDirectory="INSTALLFOLDER"
                Icon="UIAList.exe"
                IconIndex="0" />
      <RemoveFolder Id="DesktopFolder" On="uninstall" />
      <RegistryValue Root="HKCU" 
                     Key="Software\Stefan Lohmaier\UIAList" 
                     Name="DesktopShortcut" 
                     Type="integer" 
                     Value="1" 
                     KeyPath="yes" />
    </Component>

    <!-- Start menu folder cleanup -->
    <Component Id="ApplicationShortcut" Directory="ApplicationProgramsFolder" Guid="BBBBBBBB-CCCC-DDDD-EEEE-FFFFFFFFFFFF">
      <RemoveFolder Id="ApplicationProgramsFolder" On="uninstall" />
      <RegistryValue Root="HKCU" 
                     Key="Software\Stefan Lohmaier\UIAList" 
                     Name="ApplicationShortcut" 
                     Type="integer" 
                     Value="1" 
                     KeyPath="yes" />
    </Component>

    <!-- Icon -->
    <Icon Id="UIAList.exe" SourceFile="staging_$arch\UIAList.exe" />

    <!-- Properties for winget compatibility -->
    <Property Id="ARPPRODUCTICON" Value="UIAList.exe" />
    <Property Id="ARPHELPLINK" Value="https://github.com/slohmaier/UIAList" />
    <Property Id="ARPURLINFOABOUT" Value="https://github.com/slohmaier/UIAList" />
    <Property Id="ARPCONTACT" Value="stefan@slohmaier.de" />
    <Property Id="ARPCOMMENTS" Value="A utility for browsing and inspecting UI Automation controls" />
    <Property Id="ARPNOREPAIR" Value="1" />
    
    <!-- winget package identifier -->
    <Property Id="ARPNOREMOVE" Value="0" />
    
    <!-- Publisher information for winget -->
    <Property Id="ARPREADME" Value="https://github.com/slohmaier/UIAList/blob/main/README.md" />
    <Property Id="ARPURLUPDATEINFO" Value="https://github.com/slohmaier/UIAList/releases" />

  </Product>
</Wix>
"@

        Set-Content -Path $wixFile -Value $wixContent -Encoding UTF8
        Write-Host "WiX source file created: $wixFile" -ForegroundColor $Cyan

        # Compile WiX
        try {
            Push-Location "$OutputDir\msi"
            
            Write-Host "Generating file list with heat.exe for $arch..." -ForegroundColor $Cyan
            & heat dir "staging_$arch" -cg StagingFiles -gg -scom -sreg -sfrag -srd -dr INSTALLFOLDER -var var.SourceDir -out "StagingFiles_$arch.wxs"
            if ($LASTEXITCODE -ne 0) {
                throw "Heat file generation failed for $arch"
            }
            
            Write-Host "Compiling WiX source for $arch..." -ForegroundColor $Cyan
            & candle "UIAList_$arch.wxs" "StagingFiles_$arch.wxs" -dSourceDir="staging_$arch"
            if ($LASTEXITCODE -ne 0) {
                throw "WiX compilation failed for $arch"
            }
            
            Write-Host "Linking MSI package for $arch..." -ForegroundColor $Cyan
            & light "UIAList_$arch.wixobj" "StagingFiles_$arch.wixobj" -out "UIAList_$arch.msi" -ext WixUIExtension
            if ($LASTEXITCODE -ne 0) {
                throw "WiX linking failed for $arch"
            }
            
            # Move MSI to main output directory
            $finalMSI = "..\UIAList-Installer-v0.1.0-$arch.msi"
            Move-Item "UIAList_$arch.msi" $finalMSI -Force
            
            Pop-Location
            
            Write-Host "MSI installer created: $OutputDir\UIAList-Installer-v0.1.0-$arch.msi" -ForegroundColor $Green
            
            $msiSize = (Get-Item "$OutputDir\UIAList-Installer-v0.1.0-$arch.msi").Length / 1MB
            Write-Host "MSI size: $([math]::Round($msiSize, 2)) MB" -ForegroundColor $Cyan
            
        } catch {
            Write-Host "Failed to create MSI for ${arch}: $_" -ForegroundColor $Red
            Pop-Location
        }
    }
}

Write-Host ""
Write-Host "============================================" -ForegroundColor $Green
Write-Host "Package Creation Complete!" -ForegroundColor $Green
Write-Host "============================================" -ForegroundColor $Green
Write-Host ""

# Show summary
Write-Host "Output directory: $OutputDir" -ForegroundColor $Cyan
Write-Host ""
Write-Host "Created packages:" -ForegroundColor $Yellow

foreach ($arch in $Architectures) {
    if ($buildZip -and (Test-Path "$OutputDir\UIAList-Portable-v0.1.0-$arch.zip")) {
        Write-Host "✓ Portable ZIP ($arch): UIAList-Portable-v0.1.0-$arch.zip" -ForegroundColor $Green
    }
    
    if ($buildMSI -and (Test-Path "$OutputDir\UIAList-Installer-v0.1.0-$arch.msi")) {
        Write-Host "✓ MSI Installer ($arch): UIAList-Installer-v0.1.0-$arch.msi" -ForegroundColor $Green
    }
}

Write-Host ""
Write-Host "Distribution ready!" -ForegroundColor $Green