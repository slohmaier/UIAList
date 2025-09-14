# UIAList Microsoft Store Packaging Guide

This guide provides step-by-step instructions for creating an MSIX package and submitting UIAList to the Microsoft Store.

## Prerequisites

### Software Requirements
1. **Visual Studio 2022** with C++ development tools
2. **Qt 6.9+** (with MSVC compiler)
3. **Windows 10/11 SDK** (latest version)
4. **CMake 3.16+**
5. **PowerShell 5.1+**

### Developer Account
- **Microsoft Partner Center** account for Store submissions
- **Developer License** ($19 one-time fee for individual developers)

## Quick Start

### 1. Generate Store Assets
```powershell
# Generate all required icon sizes from SVG
.\generate_store_assets.ps1
```

### 2. Build and Package
```powershell
# Build all package types (ZIP, MSI, MSIX) for both architectures
cd deployment
.\deploy.ps1

# Build only MSIX packages for Store submission
.\deploy.ps1 -MSIXOnly
```

### 3. Test Package
```powershell
# Install locally for testing
Add-AppxPackage .\package_output\UIAList.msix
```

## Detailed Steps

### Step 1: Prepare Development Environment

1. **Install Visual Studio 2022**
   - Include C++ desktop development workload
   - Install Windows 10/11 SDK

2. **Install Qt 6.9+**
   - Use Qt Online Installer
   - Select MSVC 2022 64-bit compiler
   - Set `Qt6_DIR` environment variable

3. **Verify Tools**
   ```cmd
   cmake --version
   makeappx /?
   signtool /?
   ```

### Step 2: Configure Project

1. **Update Package.appxmanifest**
   - Verify publisher name matches your certificate
   - Update version number if needed
   - Review app capabilities

2. **Generate Icons**
   ```powershell
   .\scripts\generate_store_assets.ps1
   ```
   
   **Required Sizes:**
   - Square44x44Logo.png (44x44)
   - Square150x150Logo.png (150x150)  
   - Square310x310Logo.png (310x310)
   - Wide310x150Logo.png (310x150)
   - StoreLogo.png (50x50)
   - SplashScreen.png (620x300)

### Step 3: Build Application

#### Option A: Automated Build
```powershell
# Full build and package - all types for both architectures
cd deployment
.\deploy.ps1

# Or build specific types only
.\deploy.ps1 -MSIXOnly    # For Store submission
.\deploy.ps1 -ZipOnly     # For portable distribution
.\deploy.ps1 -MSIOnly     # For traditional installers
```

#### Option B: Manual Build
```cmd
# Create build directory
mkdir build_release
cd build_release

# Configure
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . --config Release

# Install to staging
cmake --install . --config Release --prefix ../staging
```

### Step 4: Create MSIX Package

```powershell
# Package the application
makeappx pack /d staging /p UIAList.msix /overwrite
```

### Step 5: Test Package

1. **Install Package**
   ```powershell
   Add-AppxPackage UIAList.msix
   ```

2. **Test Functionality**
   - Launch from Start Menu
   - Test global hotkey (Ctrl+Alt+U)
   - Verify system tray operation
   - Test About dialog

3. **Remove Test Package**
   ```powershell
   Remove-AppxPackage StefanLohmaier.UIAList_1.0.0.0_x64__[hash]
   ```

### Step 6: Microsoft Store Submission

#### A. Prepare Store Assets

1. **Screenshots** (required)
   - 1366x768, 1920x1080, or 2560x1440
   - Show main interface, filtering, system tray
   - At least 1 screenshot required

2. **Store Listing**
   - Use content from `STORE_LISTING.md`
   - Short description (150 chars max)
   - Full description (10,000 chars max)
   - Keywords for search

3. **Privacy Policy**
   - Host `PrivacyPolicy.html` on a public URL
   - Or use Microsoft Partner Center's privacy policy tool

#### B. Partner Center Submission

1. **Create App Reservation**
   - Go to [Microsoft Partner Center](https://partner.microsoft.com/)
   - Create new app reservation
   - Choose app name: "UIAList - UI Automation Control Browser"

2. **Upload Package**
   - Upload `UIAList.msix`
   - Wait for automatic validation
   - Fix any reported issues

3. **Complete Store Listing**
   - Add screenshots
   - Set category: Productivity > Accessibility
   - Set age rating: Everyone
   - Add support information

4. **Pricing and Availability**
   - Set as Free
   - Choose markets (worldwide recommended)
   - Set availability date

5. **Submit for Certification**
   - Review all sections
   - Submit for Microsoft certification
   - Wait 24-72 hours for review

## Code Signing (Optional)

For self-signed testing:

```powershell
# Create test certificate
New-SelfSignedCertificate -Type Custom -Subject "CN=Stefan Lohmaier" -KeyUsage DigitalSignature -FriendlyName "UIAList Test" -CertStoreLocation "Cert:\CurrentUser\My"

# Sign package
signtool sign /fd SHA256 /a /s My /n "Stefan Lohmaier" UIAList.msix
```

## Troubleshooting

### Common Issues

1. **"Package validation failed"**
   - Check Package.appxmanifest syntax
   - Verify all referenced files exist
   - Ensure icon files are correct sizes

2. **"App won't launch"**
   - Check dependencies with Dependency Walker
   - Verify Qt DLLs are included
   - Test on clean Windows VM

3. **"Store certification failed"**
   - Review Microsoft Store policies
   - Check accessibility compliance
   - Verify privacy policy is accessible

### Build Issues

1. **CMake configuration fails**
   ```cmd
   # Set Qt path explicitly
   set Qt6_DIR=C:\Qt\6.9\msvc2022_64\lib\cmake\Qt6
   ```

2. **Missing Qt DLLs**
   ```cmd
   # Run windeployqt manually
   windeployqt.exe --release --no-translations UIAList.exe
   ```

3. **Icon generation fails**
   - Install ImageMagick or Inkscape
   - Manually create PNG files from SVG
   - Use online SVG to PNG converters

## Post-Submission

### After Store Approval

1. **Monitor Analytics**
   - Track downloads and ratings
   - Respond to user reviews
   - Monitor crash reports

2. **Update Process**
   - Increment version in Package.appxmanifest
   - Build new package
   - Submit update through Partner Center

3. **Support**
   - Monitor support email: stefan@slohmaier.de
   - Update documentation as needed
   - Consider feature requests

## File Checklist

Before submission, ensure these files are ready:

- [ ] `Package.appxmanifest` - Configured correctly
- [ ] `Assets/` - All required icon sizes
- [ ] `PrivacyPolicy.html` - Hosted publicly  
- [ ] `UIAList.exe` - Built and tested
- [ ] Qt runtime libraries - Included in package
- [ ] Screenshots - High quality, multiple scenarios
- [ ] Store description - From STORE_LISTING.md
- [ ] Support URL - GitHub repository
- [ ] Age rating - Everyone
- [ ] Category - Accessibility/Productivity

## Contact

For questions about packaging or Store submission:

**Email:** stefan@slohmaier.de  
**GitHub:** https://github.com/slohmaier/UIAList

---

*This guide is maintained alongside the UIAList project and updated with new Microsoft Store requirements.*