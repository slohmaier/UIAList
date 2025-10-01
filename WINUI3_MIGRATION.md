# WinUI3 Migration Plan

## Overview

This document outlines the migration from Qt 6.9.x to WinUI3 (C++/WinRT) to eliminate LGPL dependencies for commercial Microsoft Store distribution.

## Migration Status: **IN PROGRESS**

Branch: `winui3-port`

## Architecture Changes

### Technology Stack

**Before (Qt)**:
- Framework: Qt 6.9.x (LGPL)
- Language: C++17
- UI: Qt Widgets
- Build: CMake + Qt build tools
- Dependencies: Qt Widgets, Qt Core (LGPL libraries)

**After (WinUI3)**:
- Framework: Windows App SDK 1.5+ (MIT License)
- Language: C++20 with C++/WinRT
- UI: WinUI 3 with XAML
- Build: CMake + MSBuild
- Dependencies: Windows App SDK (No LGPL, fully Microsoft Store compatible)

### Key Benefits

1. **No LGPL Dependencies**: Completely free for commercial distribution
2. **Native Windows**: Better performance and integration
3. **Microsoft Store Ready**: Direct MSIX packaging support
4. **Modern UI**: Fluent Design System built-in
5. **Better Accessibility**: Native Windows accessibility APIs
6. **Smaller Package Size**: No Qt runtime DLLs required

## File Mapping

### Source Files

| Qt File | WinUI3 File | Status |
|---------|-------------|--------|
| `src/main.cpp` | `src/Main.cpp` | ✅ Created |
| `src/uialist.cpp/h` | `src/MainWindow.xaml.cpp/h` | ⏳ In Progress |
| `src/uialisticon.cpp/h` | `src/SystemTrayManager.cpp/h` | 📝 Planned |
| `src/settingsdialog.cpp/h` | `src/SettingsDialog.xaml.cpp/h` | 📝 Planned |
| `src/aboutdialog.cpp/h` | `src/AboutDialog.xaml.cpp/h` | 📝 Planned |
| `src/welcomedialog.cpp/h` | `src/WelcomeDialog.xaml.cpp/h` | 📝 Planned |
| N/A | `src/ControlEnumerator.cpp/h` | 📝 Planned |
| N/A | `src/ControlInteraction.cpp/h` | 📝 Planned |
| N/A | `src/SettingsManager.cpp/h` | 📝 Planned |
| N/A | `src/App.xaml.cpp/h` | ✅ Created |
| N/A | `src/pch.h/cpp` | ✅ Created |

### UI Files

| Qt UI | WinUI3 XAML | Status |
|-------|-------------|--------|
| Widget-based UI | `src/MainWindow.xaml` | 📝 Planned |
| QDialog | `src/SettingsDialog.xaml` | 📝 Planned |
| QDialog | `src/AboutDialog.xaml` | 📝 Planned |
| QDialog | `src/WelcomeDialog.xaml` | 📝 Planned |

## Component Implementation Details

### 1. Main Window (`MainWindow.xaml`)

**Qt Implementation**:
- `QMainWindow` with `QVBoxLayout`
- `QLineEdit` for filter
- `QListWidget` for controls
- `QCheckBox` for filters
- `QPushButton` for actions

**WinUI3 Implementation**:
```xml
<Window>
  <Grid>
    <TextBlock/> <!-- Window title -->
    <TextBox/> <!-- Filter -->
    <ListView/> <!-- Controls list -->
    <CheckBox/> <!-- Hide empty titles -->
    <CheckBox/> <!-- Hide menus -->
    <StackPanel> <!-- Action buttons -->
      <Button/> <!-- Double Click -->
      <Button/> <!-- Click -->
      <Button/> <!-- Focus -->
    </StackPanel>
  </Grid>
</Window>
```

**Key Features to Port**:
- ✅ Arrow key navigation in filter box (lines 564-584 in Qt)
- ✅ Screen reader announcements (lines 633-648 in Qt)
- ✅ Auto-selection of first visible item
- ✅ Real-time filtering
- ✅ Enter key default action execution
- ✅ Escape key to close
- ✅ Auto-close on focus loss

### 2. UI Automation Control Enumeration

**Qt Implementation** (`uialist.cpp:1063-1225`):
- Worker thread with `QThread`
- `IUIAutomation` COM interface
- Tree walker pattern
- Signal/slot for progress

**WinUI3 Implementation** (`ControlEnumerator.cpp/h`):
- `std::thread` or `winrt::fire_and_forget` with co_await
- Same `IUIAutomation` COM interface (unchanged)
- Same tree walker algorithm
- `winrt::event<>` for callbacks or `std::function`

**Advantages**:
- No change to UI Automation logic
- Better async/await support with C++20 coroutines
- Direct integration with WinUI3 dispatcher

### 3. Control Interaction

**Qt Implementation** (`uialist.cpp:715-979`):
- Mouse simulation (lines 734-748)
- InvokePattern (lines 753-766)
- LegacyIAccessiblePattern (lines 769-782)
- SelectionItemPattern (lines 785-798)

**WinUI3 Implementation** (`ControlInteraction.cpp/h`):
- **Exact same algorithms** - this is pure Win32/COM code
- No changes needed to interaction logic
- Static helper methods

**Advantages**:
- No Qt dependencies in original code
- Direct port without modifications

### 4. System Tray Icon

**Qt Implementation** (`uialisticon.cpp`):
- `QSystemTrayIcon`
- `QMenu` for context menu
- Global hotkey via `nativeEventFilter`

**WinUI3 Implementation** (`SystemTrayManager.cpp/h`):
- Direct Win32 `Shell_NotifyIcon` API
- `NOTIFYICONDATA` structure
- Window message handling for tray events
- `RegisterHotKey` Win32 API
- `WM_HOTKEY` message handling

**Advantages**:
- More direct control
- No framework overhead
- Better integration with Windows

### 5. Settings Management

**Qt Implementation** (`settingsdialog.cpp`):
- `QSettings` for preferences
- Registry access for auto-start
- Key capture dialog

**WinUI3 Implementation** (`SettingsManager.cpp/h`):
- Windows Registry API directly
- JSON file for settings (or ApplicationData API)
- XAML dialog for key capture

**Advantages**:
- Direct Windows API access
- No Qt abstractions
- Faster performance

### 6. Translations

**Qt Implementation**:
- `.ts` files (Qt Linguist)
- `.qm` compiled translations
- `QTranslator`

**WinUI3 Implementation**:
- `.resw` resource files (standard Windows localization)
- `Windows.ApplicationModel.Resources.ResourceLoader`
- Built-in Windows localization system

**Advantages**:
- Standard Windows localization
- Better Microsoft Store integration
- Automatic language detection

## Build System

### CMake Configuration

**Changes**:
1. Remove Qt `find_package` commands
2. Add Windows App SDK NuGet packages
3. Configure C++/WinRT code generation
4. Add XAML compilation steps
5. Remove Qt deployment steps

### NuGet Packages Required

```xml
<packages>
  <package id="Microsoft.WindowsAppSDK" version="1.5.240311000" targetFramework="native" />
  <package id="Microsoft.Windows.CsWinRT" version="2.0.4" targetFramework="native" />
  <package id="Microsoft.Windows.SDK.BuildTools" version="10.0.22621.2428" targetFramework="native" />
</packages>
```

### Build Commands

```cmd
# Configure
cmake -G "Visual Studio 17 2022" -A x64 ..

# Build
cmake --build . --config Release

# Package MSIX
makeappx pack /d install /p UIAList.msix
```

## Testing Plan

### Functionality Tests

- [ ] Hotkey activation (Ctrl+Alt+U)
- [ ] Control enumeration for various apps
- [ ] Filter/search functionality
- [ ] Arrow key navigation in filter box
- [ ] Enter key default action
- [ ] Click action (all methods)
- [ ] Double-click action
- [ ] Focus action
- [ ] Settings persistence
- [ ] Auto-start registry
- [ ] Global hotkey customization
- [ ] Window auto-close behavior

### Screen Reader Tests

- [ ] JAWS announcements
- [ ] NVDA announcements
- [ ] Windows Narrator announcements
- [ ] Selection change announcements
- [ ] Button state announcements
- [ ] Accessible names and descriptions

### Performance Tests

- [ ] Startup time comparison
- [ ] Enumeration speed comparison
- [ ] Memory usage comparison
- [ ] Package size comparison

## Implementation Priority

### Phase 1: Core Infrastructure (Current)
1. ✅ Project structure and CMake setup
2. ✅ Application entry point
3. ⏳ Main window XAML layout
4. ⏳ Control enumeration (same algorithm)

### Phase 2: UI Components
1. Main window implementation
2. Filter and list functionality
3. Arrow key event handling
4. Action buttons

### Phase 3: Integration
1. System tray manager
2. Global hotkey registration
3. Settings manager
4. Control interaction methods

### Phase 4: Dialogs
1. Settings dialog
2. About dialog
3. Welcome dialog

### Phase 5: Polish
1. Screen reader accessibility
2. Localization
3. Error handling
4. Loading overlay

### Phase 6: Packaging
1. MSIX manifest
2. Store assets
3. Build automation
4. Documentation updates

## Timeline Estimate

- **Phase 1-2**: 2-3 days
- **Phase 3-4**: 2-3 days
- **Phase 5-6**: 2-3 days

**Total**: 6-9 days for complete migration and testing

## Risks and Mitigation

### Risk: WinUI3 Learning Curve
**Mitigation**: XAML is similar to other declarative UI frameworks; extensive Microsoft documentation available

### Risk: C++/WinRT Complexity
**Mitigation**: Most core logic (UI Automation) remains unchanged; only UI layer changes

### Risk: Breaking Screen Reader Compatibility
**Mitigation**: WinUI3 has better native accessibility; extensive testing with all screen readers

### Risk: Build System Complexity
**Mitigation**: CMake integration with WinUI3 is well-documented; use Visual Studio generator

## License Compliance

### Qt (Previous)
- **License**: LGPL v3
- **Requirements**: Dynamic linking, provide source code, license notices
- **Restrictions**: Commercial distribution requires compliance or commercial license

### WinUI3 (Current)
- **License**: MIT (Windows App SDK)
- **Requirements**: Include license notice
- **Restrictions**: None - fully commercial-friendly

### No Open Source Dependencies
- UI Automation: Windows SDK (Microsoft)
- WinUI3: Windows App SDK (Microsoft, MIT License)
- Win32 APIs: Windows SDK (Microsoft)
- C++ Standard Library: Compiler-provided (Microsoft)

**Result**: ✅ Fully compliant for Microsoft Store commercial distribution

## Next Steps

1. Complete MainWindow.xaml implementation
2. Port ControlEnumerator with exact UI Automation algorithm
3. Implement SystemTrayManager with Win32 APIs
4. Create ControlInteraction helper class
5. Build and test core functionality
6. Implement dialogs
7. Package as MSIX
8. Test on clean Windows system
9. Submit to Microsoft Store

## Questions/Decisions

1. **Localization Strategy**: Use Windows .resw files (recommended)
2. **Settings Storage**: Windows Registry + ApplicationData API
3. **Threading Model**: C++20 coroutines with winrt::fire_and_forget
4. **Icon Format**: Use existing PNG/ICO resources
5. **Version**: Bump to 0.2.0 to indicate major architecture change

## References

- [Windows App SDK Documentation](https://learn.microsoft.com/en-us/windows/apps/windows-app-sdk/)
- [WinUI 3 Documentation](https://learn.microsoft.com/en-us/windows/apps/winui/winui3/)
- [C++/WinRT Documentation](https://learn.microsoft.com/en-us/windows/uwp/cpp-and-winrt-apis/)
- [UI Automation Documentation](https://learn.microsoft.com/en-us/windows/win32/winauto/entry-uiauto-win32)
- [MSIX Packaging](https://learn.microsoft.com/en-us/windows/msix/)

---

**Last Updated**: 2025-01-01
**Status**: In Progress - Phase 1
**Branch**: `winui3-port`
