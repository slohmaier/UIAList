# UIAList WinUI3 Build Guide

## Prerequisites

### Required Software

1. **Visual Studio 2022** (17.8 or later)
   - Workloads:
     - Desktop development with C++
     - Universal Windows Platform development
   - Individual components:
     - C++/WinRT
     - Windows 11 SDK (10.0.22621.0 or later)
     - MSVC v143 - VS 2022 C++ x64/x86 build tools
     - CMake tools for Windows

2. **Windows App SDK** (included via NuGet)
   - Version 1.5.240311000 or later
   - Automatically restored via packages.config

3. **NuGet CLI** (optional, for command-line builds)
   - Download from https://www.nuget.org/downloads
   - Add to PATH

### System Requirements

- Windows 10 version 1809 (build 17763) or later
- Windows 11 recommended for development
- 8 GB RAM minimum, 16 GB recommended
- Visual Studio requires ~10 GB disk space

## Build Methods

### Method 1: Visual Studio (Recommended)

1. **Open Project**:
   ```
   File > Open > CMake...
   Select: C:\Users\...\UIAList\CMakeLists.txt
   ```

2. **Configure CMake**:
   - Visual Studio will automatically configure CMake
   - Select configuration: `x64-Release` or `x64-Debug`
   - Wait for CMake configuration to complete

3. **Restore NuGet Packages**:
   ```
   Tools > NuGet Package Manager > Package Manager Console
   Run: nuget restore packages.config -PackagesDirectory packages
   ```

4. **Build**:
   - Build > Build All (Ctrl+Shift+B)
   - Or right-click CMakeLists.txt > Build

5. **Run**:
   - Select `UIAList.exe` as startup item
   - Debug > Start Debugging (F5)

### Method 2: Command Line (CMake + MSBuild)

1. **Open Developer Command Prompt for VS 2022**

2. **Create Build Directory**:
   ```cmd
   cd C:\Users\...\UIAList
   mkdir build
   cd build
   ```

3. **Configure CMake**:
   ```cmd
   cmake .. -G "Visual Studio 17 2022" -A x64
   ```

4. **Restore NuGet Packages**:
   ```cmd
   nuget restore ..\packages.config -PackagesDirectory ..\packages
   ```

5. **Build**:
   ```cmd
   cmake --build . --config Release
   ```

6. **Output Location**:
   ```
   build\Release\UIAList.exe
   ```

### Method 3: Visual Studio Project (Alternative)

If CMake integration has issues, you can create a Visual Studio project:

1. **Create New Project**:
   - File > New > Project
   - Template: "Blank App, Packaged with WAP (WinUI 3 in Desktop)"
   - Name: UIAList

2. **Add Source Files**:
   - Copy all files from `src/` to project
   - Add to project via Solution Explorer

3. **Configure Project Settings**:
   - C/C++ > Language > C++ Language Standard: C++20
   - C/C++ > Precompiled Headers: Use pch.h
   - Linker > System > SubSystem: Windows

4. **Add Package References**:
   - Right-click project > Manage NuGet Packages
   - Install: Microsoft.WindowsAppSDK

5. **Build and Run**

## Common Build Issues

### Issue: CMake Cannot Find Windows SDK

**Solution**:
```cmd
# Set Windows SDK version explicitly
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_SYSTEM_VERSION=10.0.22621.0
```

### Issue: NuGet Packages Not Found

**Solution**:
```cmd
# Restore manually
cd C:\Users\...\UIAList
nuget restore packages.config -PackagesDirectory packages

# Or use Visual Studio NuGet manager
```

### Issue: C++/WinRT Code Generation Errors

**Solution**:
1. Verify C++/WinRT is installed in Visual Studio
2. Clean and rebuild:
   ```cmd
   cmake --build . --target clean
   cmake --build . --config Release
   ```

### Issue: XAML Compilation Errors

**Solution**:
1. Ensure Windows App SDK is properly installed
2. Check XAML files for syntax errors
3. Rebuild project (XAML is compiled on first build)

### Issue: Missing winrt/Microsoft.UI.Xaml.h

**Solution**:
1. Verify NuGet packages are restored
2. Check packages directory exists: `UIAList\packages\`
3. Re-run NuGet restore

### Issue: Linker Error LNK2001: unresolved external symbol

**Solution**:
1. Verify all source files are included in CMakeLists.txt
2. Check that windowsapp.lib is linked
3. Ensure Microsoft.WindowsAppRuntime is in link libraries

## Project Structure

```
UIAList/
├── CMakeLists.txt              # CMake build configuration
├── packages.config             # NuGet packages
├── BUILD_GUIDE.md             # This file
│
├── src/
│   ├── pch.h/cpp              # Precompiled headers
│   ├── Main.cpp               # Entry point
│   ├── App.xaml*              # Application class
│   ├── MainWindow.xaml*       # Main window
│   ├── SettingsDialog.xaml*   # Settings dialog
│   ├── AboutDialog.xaml*      # About dialog
│   ├── WelcomeDialog.xaml*    # Welcome dialog
│   ├── ControlEnumerator.*    # UI Automation enumeration
│   ├── ControlInteraction.*   # Control interaction logic
│   ├── SystemTrayManager.*    # Tray icon + hotkey
│   ├── SettingsManager.*      # Settings persistence
│   └── UIAList.rc             # Resources
│
├── resources/
│   ├── Package.appxmanifest   # MSIX manifest
│   ├── icons/                 # Application icons
│   └── assets/                # MSIX assets
│
└── build/                     # Build output (created)
    └── Release/
        └── UIAList.exe
```

## Build Configurations

### Debug Build

- Includes debug symbols
- No optimizations
- Larger binary size
- Useful for debugging with Visual Studio

```cmd
cmake --build . --config Debug
```

### Release Build

- Optimizations enabled
- Smaller binary size
- No debug symbols (unless explicitly enabled)
- Ready for distribution

```cmd
cmake --build . --config Release
```

### Release with Debug Info

```cmd
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build . --config RelWithDebInfo
```

## Packaging for Distribution

### Create MSIX Package

1. **Build Release Version**:
   ```cmd
   cmake --build . --config Release
   ```

2. **Prepare Package Directory**:
   ```cmd
   mkdir package
   xcopy /E /I build\Release package\
   xcopy /E /I resources\assets package\Assets\
   copy resources\Package.appxmanifest package\
   ```

3. **Create MSIX**:
   ```cmd
   makeappx pack /d package /p UIAList.msix /o
   ```

4. **Sign Package** (for distribution):
   ```cmd
   signtool sign /fd SHA256 /a /f certificate.pfx /p password UIAList.msix
   ```

### Create Portable ZIP

```cmd
cd build\Release
7z a ..\..\UIAList-Portable-v0.2.0-x64.zip UIAList.exe *.dll
```

## Development Workflow

### 1. Make Code Changes

Edit source files in `src/` directory

### 2. Build

```cmd
cmake --build . --config Debug
```

### 3. Test

```cmd
build\Debug\UIAList.exe
```

### 4. Debug

- Open Visual Studio
- Debug > Attach to Process
- Select UIAList.exe
- Set breakpoints and debug

### 5. Commit

```cmd
git add .
git commit -m "Description of changes"
git push
```

## Testing

### Manual Testing Checklist

- [ ] Application starts without errors
- [ ] System tray icon appears
- [ ] Global hotkey (Ctrl+Alt+U) works
- [ ] Window shows for foreground application
- [ ] Controls are enumerated
- [ ] Filter/search works
- [ ] Arrow keys navigate in filter box
- [ ] Enter executes default action
- [ ] Click/DoubleClick/Focus buttons work
- [ ] Settings dialog opens and saves
- [ ] About dialog displays correctly
- [ ] Welcome dialog shows on first run
- [ ] Auto-start registry setting works

### Screen Reader Testing

Test with:
- JAWS (recommended)
- NVDA
- Windows Narrator

Verify:
- Control announcements work
- Selection changes are announced
- Buttons have accessible names
- Keyboard navigation is smooth

## Performance Optimization

### Build Optimizations

Add to CMakeLists.txt for smaller binaries:

```cmake
if(MSVC)
    target_compile_options(UIAList PRIVATE /O2 /GL)
    target_link_options(UIAList PRIVATE /LTCG /OPT:REF /OPT:ICF)
endif()
```

### Runtime Optimizations

- Use Release build for distribution
- Enable Link-Time Code Generation (LTCG)
- Strip debug symbols

## Troubleshooting

### Application Crashes on Startup

1. Check Event Viewer for crash details
2. Run in Debug mode and check Output window
3. Verify all DLLs are present in executable directory
4. Check that Windows App SDK runtime is installed

### Controls Not Showing

1. Verify UI Automation is working: `inspect.exe` tool
2. Check that target application supports UI Automation
3. Try with a known-good application (e.g., Notepad)

### Hotkey Not Working

1. Check for conflicts with other applications
2. Try different key combination in Settings
3. Verify RegisterHotKey succeeds (check debug output)

## Advanced Build Options

### Cross-Compilation for ARM64

```cmd
cmake .. -G "Visual Studio 17 2022" -A ARM64
cmake --build . --config Release
```

### Static Linking C++ Runtime

```cmake
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
```

### Enable Whole Program Optimization

```cmake
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
```

## Continuous Integration

### GitHub Actions Example

```yaml
name: Build

on: [push, pull_request]

jobs:
  build:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v3
      - name: Setup MSBuild
        uses: microsoft/setup-msbuild@v1.1
      - name: Restore NuGet packages
        run: nuget restore packages.config -PackagesDirectory packages
      - name: Configure CMake
        run: cmake -B build -G "Visual Studio 17 2022" -A x64
      - name: Build
        run: cmake --build build --config Release
      - name: Upload artifact
        uses: actions/upload-artifact@v3
        with:
          name: UIAList-Release
          path: build/Release/UIAList.exe
```

## Additional Resources

- [WinUI 3 Documentation](https://learn.microsoft.com/en-us/windows/apps/winui/winui3/)
- [C++/WinRT Documentation](https://learn.microsoft.com/en-us/windows/uwp/cpp-and-winrt-apis/)
- [Windows App SDK](https://learn.microsoft.com/en-us/windows/apps/windows-app-sdk/)
- [CMake Documentation](https://cmake.org/documentation/)
- [UI Automation](https://learn.microsoft.com/en-us/windows/win32/winauto/entry-uiauto-win32)

## Support

- GitHub Issues: https://github.com/slohmaier/UIAList/issues
- Documentation: See README.md and USER_GUIDE.md

---

**Last Updated**: 2025-01-01
**Build System Version**: CMake 3.20+ with Visual Studio 2022
