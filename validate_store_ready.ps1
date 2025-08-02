# Validation script to check if UIAList is ready for Microsoft Store submission
# This script verifies all required files and configurations are in place

Write-Host "UIAList Microsoft Store Readiness Validation" -ForegroundColor Green
Write-Host "=============================================" -ForegroundColor Green
Write-Host ""

$errors = @()
$warnings = @()
$passed = 0
$total = 0

function Test-Requirement {
    param(
        [string]$Name,
        [bool]$Condition,
        [string]$ErrorMessage,
        [bool]$IsWarning = $false
    )
    
    $script:total++
    
    if ($Condition) {
        Write-Host "✓ $Name" -ForegroundColor Green
        $script:passed++
    } else {
        if ($IsWarning) {
            Write-Host "⚠ $Name" -ForegroundColor Yellow
            $script:warnings += $ErrorMessage
        } else {
            Write-Host "✗ $Name" -ForegroundColor Red
            $script:errors += $ErrorMessage
        }
    }
}

# Check core files
Write-Host "Checking core files..." -ForegroundColor Yellow
Test-Requirement "Package.appxmanifest exists" (Test-Path "Package.appxmanifest") "Package.appxmanifest file is missing"
Test-Requirement "CMakeLists.txt exists" (Test-Path "CMakeLists.txt") "CMakeLists.txt file is missing"
Test-Requirement "PrivacyPolicy.html exists" (Test-Path "PrivacyPolicy.html") "PrivacyPolicy.html file is missing"
Test-Requirement "Assets directory exists" (Test-Path "Assets") "Assets directory is missing"

# Check source files
Write-Host ""
Write-Host "Checking source files..." -ForegroundColor Yellow
Test-Requirement "main.cpp exists" (Test-Path "main.cpp") "main.cpp file is missing"
Test-Requirement "uialist.h exists" (Test-Path "uialist.h") "uialist.h file is missing"
Test-Requirement "uialist.cpp exists" (Test-Path "uialist.cpp") "uialist.cpp file is missing"
Test-Requirement "aboutdialog.h exists" (Test-Path "aboutdialog.h") "aboutdialog.h file is missing"
Test-Requirement "aboutdialog.cpp exists" (Test-Path "aboutdialog.cpp") "aboutdialog.cpp file is missing"

# Check build scripts
Write-Host ""
Write-Host "Checking build scripts..." -ForegroundColor Yellow
Test-Requirement "build_msix.ps1 exists" (Test-Path "build_msix.ps1") "build_msix.ps1 script is missing"
Test-Requirement "generate_store_assets.ps1 exists" (Test-Path "generate_store_assets.ps1") "generate_store_assets.ps1 script is missing"

# Check required assets
Write-Host ""
Write-Host "Checking required store assets..." -ForegroundColor Yellow
$requiredAssets = @(
    "Square44x44Logo.png",
    "Square150x150Logo.png", 
    "Square310x310Logo.png",
    "Wide310x150Logo.png",
    "StoreLogo.png",
    "SplashScreen.png"
)

foreach ($asset in $requiredAssets) {
    $assetPath = "Assets\$asset"
    Test-Requirement "$asset exists" (Test-Path $assetPath) "Required asset $asset is missing from Assets directory" $true
}

# Check Package.appxmanifest content
Write-Host ""
Write-Host "Validating Package.appxmanifest..." -ForegroundColor Yellow
if (Test-Path "Package.appxmanifest") {
    $manifest = Get-Content "Package.appxmanifest" -Raw
    Test-Requirement "Contains app identity" ($manifest -match 'Name="StefanLohmaier.UIAList"') "Package.appxmanifest missing correct app identity"
    Test-Requirement "Contains publisher" ($manifest -match 'Publisher="CN=Stefan Lohmaier"') "Package.appxmanifest missing correct publisher"
    Test-Requirement "Contains version" ($manifest -match 'Version="1\.0\.0\.0"') "Package.appxmanifest missing version 1.0.0.0"
    Test-Requirement "Contains executable" ($manifest -match 'Executable="UIAList\.exe"') "Package.appxmanifest missing UIAList.exe executable"
}

# Check copyright headers
Write-Host ""
Write-Host "Checking copyright headers..." -ForegroundColor Yellow
$sourceFiles = @("main.cpp", "uialist.h", "uialist.cpp", "uialisticon.h", "uialisticon.cpp", "aboutdialog.h", "aboutdialog.cpp")
foreach ($file in $sourceFiles) {
    if (Test-Path $file) {
        $content = Get-Content $file -Raw
        Test-Requirement "$file has copyright header" ($content -match "Copyright \(C\) 2025 Stefan Lohmaier") "$file missing copyright header"
    }
}

# Check documentation
Write-Host ""
Write-Host "Checking documentation..." -ForegroundColor Yellow
Test-Requirement "README.md exists" (Test-Path "README.md") "README.md file is missing"
Test-Requirement "LICENSE exists" (Test-Path "LICENSE") "LICENSE file is missing"
Test-Requirement "STORE_LISTING.md exists" (Test-Path "STORE_LISTING.md") "STORE_LISTING.md file is missing"
Test-Requirement "PACKAGING_GUIDE.md exists" (Test-Path "PACKAGING_GUIDE.md") "PACKAGING_GUIDE.md file is missing"

# Check LICENSE content
if (Test-Path "LICENSE") {
    $license = Get-Content "LICENSE" -Raw
    Test-Requirement "LICENSE has proper copyright" ($license -match "Stefan Lohmaier") "LICENSE file missing Stefan Lohmaier copyright"
    Test-Requirement "LICENSE is GPL v3" ($license -match "GNU GENERAL PUBLIC LICENSE") "LICENSE file is not GPL v3"
}

# Environment checks
Write-Host ""
Write-Host "Checking build environment..." -ForegroundColor Yellow
Test-Requirement "CMake available" ((Get-Command "cmake" -ErrorAction SilentlyContinue) -ne $null) "CMake not found in PATH" $true
Test-Requirement "makeappx available" ((Get-Command "makeappx" -ErrorAction SilentlyContinue) -ne $null) "makeappx (Windows SDK) not found in PATH" $true

# Qt environment
$qtFound = $false
if ($env:Qt6_DIR) {
    Test-Requirement "Qt6_DIR environment variable set" $true "Qt6_DIR environment variable is set"
    $qtFound = $true
} elseif ($env:QTDIR) {
    Test-Requirement "QTDIR environment variable set" $true "QTDIR environment variable is set"
    $qtFound = $true
} else {
    Test-Requirement "Qt environment configured" $false "Neither Qt6_DIR nor QTDIR environment variable is set" $true
}

# Visual Studio
if ($env:VCINSTALLDIR) {
    Test-Requirement "Visual Studio environment" $true "Visual Studio environment detected"
} else {
    Test-Requirement "Visual Studio environment" $false "Visual Studio environment not detected. Run from VS Developer Command Prompt." $true
}

# Summary
Write-Host ""
Write-Host "=============================================" -ForegroundColor Green
Write-Host "Validation Summary" -ForegroundColor Green
Write-Host "=============================================" -ForegroundColor Green
Write-Host ""
Write-Host "Passed: $passed/$total tests" -ForegroundColor $(if ($passed -eq $total) { "Green" } else { "Yellow" })

if ($errors.Count -gt 0) {
    Write-Host ""
    Write-Host "ERRORS (must fix before Store submission):" -ForegroundColor Red
    foreach ($error in $errors) {
        Write-Host "  • $error" -ForegroundColor Red
    }
}

if ($warnings.Count -gt 0) {
    Write-Host ""
    Write-Host "WARNINGS (recommended to fix):" -ForegroundColor Yellow
    foreach ($warning in $warnings) {
        Write-Host "  • $warning" -ForegroundColor Yellow
    }
}

Write-Host ""
if ($errors.Count -eq 0) {
    Write-Host "🎉 UIAList is ready for Microsoft Store packaging!" -ForegroundColor Green
    Write-Host ""
    Write-Host "Next steps:" -ForegroundColor Cyan
    Write-Host "1. Generate store assets: .\generate_store_assets.ps1" -ForegroundColor White
    Write-Host "2. Build MSIX package: .\build_msix.ps1" -ForegroundColor White
    Write-Host "3. Test package locally" -ForegroundColor White
    Write-Host "4. Submit to Microsoft Partner Center" -ForegroundColor White
} else {
    Write-Host "❌ Please fix the errors above before proceeding with Store submission." -ForegroundColor Red
    exit 1
}

Write-Host ""