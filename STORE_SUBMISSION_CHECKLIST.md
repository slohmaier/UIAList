# UIAList Microsoft Store Submission Checklist

## ✅ Completed Setup

### Core Files
- [x] **Package.appxmanifest** - MSIX package configuration
- [x] **PrivacyPolicy.html** - Store-required privacy policy  
- [x] **build_msix.ps1** - Automated build and packaging script
- [x] **generate_store_assets.ps1** - Icon generation script
- [x] **validate_store_ready.ps1** - Pre-submission validation

### Application Code
- [x] **Copyright headers** - Added to all source files
- [x] **GPL v3 License** - Updated with Stefan Lohmaier copyright
- [x] **About Dialog** - Complete with GitHub link and license info
- [x] **Version 1.0.0** - Ready for initial Store release

### Documentation
- [x] **STORE_LISTING.md** - Complete Store listing content
- [x] **PACKAGING_GUIDE.md** - Detailed packaging instructions
- [x] **Assets/README.md** - Icon requirements and generation guide

## 📋 Pre-Submission Tasks

### 1. Generate Store Assets
```powershell
# Run from project root
.\generate_store_assets.ps1
```

**Required Icons:**
- Square44x44Logo.png (44x44)
- Square150x150Logo.png (150x150)
- Square310x310Logo.png (310x310)
- Wide310x150Logo.png (310x150)
- StoreLogo.png (50x50)
- SplashScreen.png (620x300)

### 2. Setup Build Environment
- Install Visual Studio 2022 with C++ tools
- Install Qt 6.9+ with MSVC compiler
- Install Windows 10/11 SDK
- Set environment variables (Qt6_DIR)

### 3. Build and Test
```powershell
# Build MSIX package
.\build_msix.ps1 -Configuration Release

# Test installation
Add-AppxPackage .\package_output\UIAList.msix
```

### 4. Create Screenshots
Take screenshots showing:
- Main interface with control list
- Filter functionality in action
- System tray context menu
- About dialog
- Application in use with real windows

**Required Sizes:** 1366x768, 1920x1080, or 2560x1440

## 🏪 Microsoft Store Submission

### Partner Center Setup
1. **Create Developer Account**
   - Go to [Microsoft Partner Center](https://partner.microsoft.com/)
   - Pay $19 developer fee (one-time)
   - Complete identity verification

2. **Reserve App Name**
   - Create new app: "UIAList - UI Automation Control Browser"
   - Alternative: "UIAList" (if available)

### Package Upload
1. **Upload MSIX**
   - Upload `package_output/UIAList.msix`
   - Wait for automatic validation
   - Address any validation errors

2. **Store Listing** (use content from STORE_LISTING.md)
   - **Short Description:** Navigate and interact with UI controls quickly using screen readers. Perfect for blind and visually impaired users with JAWS, NVDA, Narrator.
   - **Full Description:** [Copy from STORE_LISTING.md]
   - **Screenshots:** Upload 3-5 high-quality screenshots
   - **Keywords:** accessibility, screen reader, JAWS, NVDA, Windows Narrator, UI Automation

3. **App Properties**
   - **Category:** Productivity → Accessibility
   - **Age Rating:** Everyone
   - **Pricing:** Free
   - **Markets:** Worldwide

4. **Privacy Policy**
   - **URL:** [Upload PrivacyPolicy.html to your website]
   - **Contact Email:** stefan@slohmaier.de

5. **Support Information**
   - **Website:** https://github.com/slohmaier/UIAList
   - **Support Email:** stefan@slohmaier.de

### Final Submission
1. Review all sections
2. Submit for certification
3. Wait 24-72 hours for Microsoft review
4. Address any certification feedback
5. App goes live after approval!

## 📊 App Details Summary

| Field | Value |
|-------|-------|
| **App Name** | UIAList - UI Automation Control Browser |
| **Publisher** | Stefan Lohmaier |
| **Email** | stefan@slohmaier.de |
| **Version** | 1.0.0.0 |
| **Category** | Productivity → Accessibility |
| **Price** | Free |
| **Age Rating** | Everyone |
| **License** | GPL v3 |
| **Privacy** | No data collection |
| **Platform** | Windows 10/11 Desktop |

## 🎯 Success Metrics

After Store approval, monitor:
- Download counts and user acquisition
- User ratings and reviews
- Crash reports and feedback
- Feature requests from accessibility community

## 🔄 Update Process

For future updates:
1. Increment version in Package.appxmanifest
2. Update source code and rebuild
3. Create new MSIX package
4. Submit update through Partner Center
5. Maintain backward compatibility

## 📞 Support Contacts

- **Developer:** Stefan Lohmaier
- **Email:** stefan@slohmaier.de
- **GitHub:** https://github.com/slohmaier/UIAList
- **License:** GPL v3

---

**Status:** ✅ Ready for Microsoft Store submission!

**Next Action:** Generate store assets and build MSIX package using the provided scripts.