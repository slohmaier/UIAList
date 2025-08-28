# PowerShell script to build UIAList and create both MSI installer and portable ZIP
# This script automates building multiple distribution formats

param(
    [string]$Configuration = "Release",
    [string]$OutputDir = "dist",
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

Write-Host "UIAList Package Builder" -ForegroundColor $Green
Write-Host "======================" -ForegroundColor $Green
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

# Check for Qt
$qtDir = $env:Qt6_DIR
if (-not $qtDir) {
    $qtDir = $env:QTDIR
}

# Try to detect Qt from common paths if environment variables aren't set
if (-not $qtDir) {
    # Check if windeployqt is in PATH (common in Qt command prompt)
    $windeployqtPath = (Get-Command "windeployqt.exe" -ErrorAction SilentlyContinue)
    if ($windeployqtPath) {
        $qtDir = (Split-Path (Split-Path $windeployqtPath.Source -Parent) -Parent)
        Write-Host "Qt detected from PATH: $qtDir" -ForegroundColor $Cyan
    } else {
        # Try common Qt installation paths
        $commonPaths = @("C:\Qt\6.9\msvc2022_64", "C:\Qt\6.8\msvc2022_64", "C:\Qt\6.7\msvc2022_64")
        foreach ($path in $commonPaths) {
            if (Test-Path "$path\bin\windeployqt.exe") {
                $qtDir = $path
                Write-Host "Qt detected at: $qtDir" -ForegroundColor $Cyan
                break
            }
        }
    }
}

if (-not $qtDir) {
    Write-Host "Qt installation not found. Please:" -ForegroundColor $Red
    Write-Host "1. Run from Qt command prompt, or" -ForegroundColor $Red
    Write-Host "2. Set Qt6_DIR or QTDIR environment variable, or" -ForegroundColor $Red
    Write-Host "3. Ensure windeployqt.exe is in your PATH" -ForegroundColor $Red
    exit 1
}

Write-Host "Qt directory: $qtDir" -ForegroundColor $Cyan

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
if (-not $MSIOnly -and -not $ZipOnly) {
    try {
        $candle = Get-Command "candle.exe" -ErrorAction Stop
        $light = Get-Command "light.exe" -ErrorAction Stop
        Write-Host "WiX Toolset found: $($candle.Source)" -ForegroundColor $Cyan
    } catch {
        # Try looking in common WiX installation paths
        $wixPaths = @("C:\WiX", "C:\Program Files (x86)\WiX Toolset v3.11\bin", "C:\Program Files (x86)\WiX Toolset v3.14\bin")
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
    Write-Host "Building UIAList..." -ForegroundColor $Yellow
    
    # Create build directory
    $buildDir = "build_packages"
    if (Test-Path $buildDir) {
        Remove-Item $buildDir -Recurse -Force
    }
    New-Item -ItemType Directory -Path $buildDir -Force | Out-Null
    
    # Configure with CMake
    Push-Location $buildDir
    try {
        Write-Host "Configuring build with CMake..." -ForegroundColor $Cyan
        & "C:\Qt\Tools\CMake_64\bin\cmake.exe" .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=$Configuration
        if ($LASTEXITCODE -ne 0) {
            throw "CMake configuration failed"
        }
        
        # Build the project
        Write-Host "Building project..." -ForegroundColor $Cyan
        & "C:\Qt\Tools\CMake_64\bin\cmake.exe" --build . --config $Configuration --target UIAList
        if ($LASTEXITCODE -ne 0) {
            throw "Build failed"
        }
        
        Write-Host "Build completed successfully!" -ForegroundColor $Green
        
    } catch {
        Write-Host "Build failed: $_" -ForegroundColor $Red
        Pop-Location
        exit 1
    } finally {
        Pop-Location
    }
} else {
    Write-Host "Skipping build as requested." -ForegroundColor $Yellow
}

# Determine what to build
$buildMSI = (-not $ZipOnly) -and (Get-Command "candle.exe" -ErrorAction SilentlyContinue)
$buildZip = (-not $MSIOnly)

Write-Host ""
Write-Host "Creating packages..." -ForegroundColor $Yellow
Write-Host "MSI Installer: $(if ($buildMSI) { 'Yes' } else { 'No' })" -ForegroundColor $Cyan
Write-Host "Portable ZIP: $(if ($buildZip) { 'Yes' } else { 'No' })" -ForegroundColor $Cyan
Write-Host ""

# Common staging function
function Create-StagingArea {
    param([string]$StagingPath)
    
    # Create staging directory
    New-Item -ItemType Directory -Path $StagingPath -Force | Out-Null
    
    # Copy the main executable
    $exePath = "build_packages\$Configuration\UIAList.exe"
    if (Test-Path $exePath) {
        Copy-Item $exePath $StagingPath -Force
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
            $targetExe = Join-Path $StagingPath "UIAList.exe"
            & "$windeployqt" "$targetExe" --release --no-translations --no-system-d3d-compiler --no-opengl-sw --dir "$StagingPath"
            if ($LASTEXITCODE -ne 0) {
                Write-Host "windeployqt failed, but continuing..." -ForegroundColor $Yellow
            }
        } else {
            Write-Host "windeployqt not found, manual Qt deployment needed" -ForegroundColor $Yellow
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

# Create portable ZIP package
if ($buildZip) {
    Write-Host "Creating portable ZIP package..." -ForegroundColor $Blue
    
    $zipStagingDir = "$OutputDir\portable\UIAList-Portable"
    Create-StagingArea $zipStagingDir
    
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
    $zipFile = "$OutputDir\UIAList-Portable-v0.1.0.zip"
    try {
        Compress-Archive -Path "$zipStagingDir\*" -DestinationPath $zipFile -Force
        Write-Host "Portable ZIP created: $zipFile" -ForegroundColor $Green
        
        $zipSize = (Get-Item $zipFile).Length / 1MB
        Write-Host "ZIP size: $([math]::Round($zipSize, 2)) MB" -ForegroundColor $Cyan
    } catch {
        Write-Host "Failed to create ZIP file: $_" -ForegroundColor $Red
    }
}

# Create MSI installer package
if ($buildMSI) {
    Write-Host "Creating MSI installer package..." -ForegroundColor $Blue
    
    $msiStagingDir = "$OutputDir\msi\staging"
    Create-StagingArea $msiStagingDir
    
    # Create WiX source file
    $wixFile = "$OutputDir\msi\UIAList.wxs"
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

    <Feature Id="ProductFeature" Title="UIAList" Level="1">
      <ComponentGroupRef Id="ProductComponents" />
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

    <!-- Components -->
    <ComponentGroup Id="ProductComponents" Directory="INSTALLFOLDER">
      <!-- Main executable -->
      <Component Id="UIAListExe" Guid="12345678-ABCD-1234-ABCD-123456789ABC">
        <File Id="UIAListExeFile" 
              Source="staging\UIAList.exe" 
              Checksum="yes" 
              KeyPath="yes" />
      </Component>
      
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

      <!-- Qt dependencies -->
"@

    # Add Qt DLL files dynamically
    $qtFiles = Get-ChildItem "$msiStagingDir\*.dll" -ErrorAction SilentlyContinue
    $componentIndex = 1
    foreach ($dll in $qtFiles) {
        $wixContent += @"

      <Component Id="QtDll$componentIndex" Guid="*">
        <File Id="QtDllFile$componentIndex" Source="staging\$($dll.Name)" KeyPath="yes" />
      </Component>
"@
        $componentIndex++
    }

    # Add other files
    $otherFiles = @("LICENSE", "README.md", "PrivacyPolicy.html")
    foreach ($file in $otherFiles) {
        if (Test-Path "$msiStagingDir\$file") {
            $safeId = $file -replace "[^a-zA-Z0-9]", ""
            $wixContent += @"

      <Component Id="File$safeId" Guid="*">
        <File Id="FileId$safeId" Source="staging\$file" KeyPath="yes" />
      </Component>
"@
        }
    }

    $wixContent += @"
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
    <Icon Id="UIAList.exe" SourceFile="staging\UIAList.exe" />

    <!-- Properties for winget compatibility -->
    <Property Id="ARPPRODUCTICON" Value="UIAList.exe" />
    <Property Id="ARPHELPLINK" Value="https://github.com/slohmaier/UIAList" />
    <Property Id="ARPURLINFOABOUT" Value="https://github.com/slohmaier/UIAList" />
    <Property Id="ARPCONTACT" Value="stefan@slohmaier.de" />
    <Property Id="ARPCOMMENTS" Value="A utility for browsing and inspecting UI Automation controls" />
    <Property Id="ARPNOREPAIR" Value="1" />
    <Property Id="ARPNOMODIFY" Value="1" />
    
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
        
        Write-Host "Compiling WiX source..." -ForegroundColor $Cyan
        & candle UIAList.wxs -out UIAList.wixobj
        if ($LASTEXITCODE -ne 0) {
            throw "WiX compilation failed"
        }
        
        Write-Host "Linking MSI package..." -ForegroundColor $Cyan
        & light UIAList.wixobj -out UIAList.msi -ext WixUIExtension
        if ($LASTEXITCODE -ne 0) {
            throw "WiX linking failed"
        }
        
        # Move MSI to main output directory
        $finalMSI = "..\UIAList-Installer-v0.1.0.msi"
        Move-Item "UIAList.msi" $finalMSI -Force
        
        Pop-Location
        
        Write-Host "MSI installer created: $OutputDir\UIAList-Installer-v0.1.0.msi" -ForegroundColor $Green
        
        $msiSize = (Get-Item "$OutputDir\UIAList-Installer-v0.1.0.msi").Length / 1MB
        Write-Host "MSI size: $([math]::Round($msiSize, 2)) MB" -ForegroundColor $Cyan
        
    } catch {
        Write-Host "Failed to create MSI: $_" -ForegroundColor $Red
        Pop-Location
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

if ($buildZip -and (Test-Path "$OutputDir\UIAList-Portable-v0.1.0.zip")) {
    Write-Host "✓ Portable ZIP: UIAList-Portable-v0.1.0.zip" -ForegroundColor $Green
}

if ($buildMSI -and (Test-Path "$OutputDir\UIAList-Installer-v0.1.0.msi")) {
    Write-Host "✓ MSI Installer: UIAList-Installer-v0.1.0.msi" -ForegroundColor $Green
}

Write-Host ""
Write-Host "Distribution ready!" -ForegroundColor $Green