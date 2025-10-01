# Claude Memory - UIAList Project

This document contains important information about the UIAList project for Claude Code to remember across conversations.

## Project Overview

**UIAList** is a Windows accessibility tool for blind and visually impaired screen reader users (JAWS, NVDA, Windows Narrator). It lists all UI controls in the foreground application with instant search and keyboard navigation, eliminating the need for sequential tabbing.

## Technology Stack

- **Language**: C++ (MSVC 2022)
- **Framework**: Qt 6.9.x
- **Build System**: CMake 3.16+
- **Windows API**: UI Automation (IUIAutomation), COM
- **Architectures**: x64, ARM64

## Core Architecture

### Main Components

1. **UIAList** (`src/uialist.cpp/.h`)
   - Main window and application logic
   - UI Automation integration
   - Control enumeration via background worker
   - Mouse simulation for control interaction

2. **UIAListIcon** (`src/uialisticon.cpp/.h`)
   - System tray integration
   - Global hotkey registration (default: Ctrl+Alt+U)
   - Context menu (Settings, About, Exit)

3. **ControlEnumerationWorker** (in `src/uialist.cpp`)
   - Background thread for control discovery
   - Walks UI Automation tree
   - Emits signals for found controls
   - Cancellable operation

4. **SettingsDialog** (`src/settingsdialog.cpp/.h`)
   - Auto-start configuration (Windows Registry)
   - Default action selection (Click/Double Click/Focus)
   - Global hotkey customization

5. **WelcomeDialog** (`src/welcomedialog.cpp/.h`)
   - First-run welcome screen
   - Basic usage instructions

6. **AboutDialog** (`src/aboutdialog.cpp/.h`)
   - Version information
   - License details (GPL v3)
   - Reset settings functionality

## Key Features & Implementation Details

### 1. Arrow Key Navigation in Search Box

**Critical Feature**: When focus is in the filter/search box, Up/Down arrows navigate the control list instead of moving cursor.

**Implementation** (`src/uialist.cpp:564-584`):
```cpp
bool UIAList::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_filterEdit && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

        if (keyEvent->key() == Qt::Key_Up) {
            selectVisibleListItem(-1);
            return true; // Suppress default behavior
        } else if (keyEvent->key() == Qt::Key_Down) {
            selectVisibleListItem(1);
            return true; // Suppress default behavior
        } else if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            ensureItemSelected();
            executeDefaultAction();
            hide();
            return true; // Suppress default behavior
        }
    }

    return QMainWindow::eventFilter(obj, event);
}
```

### 2. Screen Reader Announcements

**Purpose**: Ensure screen readers announce newly selected items when using arrow keys.

**Implementation** (`src/uialist.cpp:633-648`):
```cpp
void UIAList::announceSelectedItem(const QString& text)
{
    // Use QAccessible to announce the selected item to screen readers
    if (m_listWidget && m_listWidget->currentItem()) {
        QAccessibleEvent event(m_listWidget, QAccessible::Selection);
        QAccessible::updateAccessibility(&event);

        // Also send a focus event to ensure screen readers announce the text
        QAccessibleEvent focusEvent(m_listWidget, QAccessible::Focus);
        QAccessible::updateAccessibility(&focusEvent);
    }
}
```

**When Called**:
- After arrow key navigation changes selection
- After filter updates auto-select first item
- When list is first populated

### 3. Control Interaction Methods

**Three Action Types**:

1. **Click** (`src/uialist.cpp:715-803`)
   - Primary: Mouse simulation at center of bounding rectangle
   - Fallback: IUIAutomationInvokePattern
   - Fallback: IUIAutomationLegacyIAccessiblePattern (DoDefaultAction)
   - Fallback: IUIAutomationSelectionItemPattern (Select)

2. **Double Click** (`src/uialist.cpp:879-979`)
   - Primary: Double mouse click simulation
   - Fallback: IUIAutomationTogglePattern
   - Fallback: Legacy DoDefaultAction twice
   - Fallback: InvokePattern twice

3. **Focus** (`src/uialist.cpp:805-877`)
   - Primary: element->SetFocus()
   - Fallback: Mouse click to focus
   - Fallback: Tab key simulation

**Default Action**: Configurable in settings (default: Click)
- Executed when pressing Enter in search box
- Setting stored in QSettings: "defaultAction" (0=Click, 1=DoubleClick, 2=Focus)

### 4. Control Filtering

**Always Filtered**:
- Text controls (`UIA_TextControlTypeId`) - line 425
- Window controls (`UIA_WindowControlTypeId`) - line 426

**Optional Filters** (checkboxes):
- Hide controls with empty/no title (default: checked) - lines 431-436
- Hide menus and menu items (default: checked) - lines 439-445

**Search Filter** (`src/uialist.cpp:464-516`):
- Multi-word AND logic (all words must match)
- Case-insensitive
- Real-time filtering as user types
- Auto-selects first visible item after filter

### 5. Background Control Enumeration

**Why**: Enumerating large UI trees can take seconds; must not freeze UI.

**Implementation**:
- Worker thread spawned when window shown (`startEnumeration()`)
- Progress overlay with indeterminate progress bar
- Cancel button to abort enumeration
- Signals: `controlFound`, `enumerationFinished`, `enumerationCancelled`
- Thread-safe cancellation via QMutex

### 6. Window Behavior

**Auto-Show**: Triggered by system tray icon when foreground window changes
**Auto-Hide**:
- Window loses focus/activation (line 686-692)
- After executing any action (Click/Focus/DoubleClick)
- User presses Escape

**Focus Management**:
- Filter edit box receives focus on window show (line 227)
- Loading overlay sets focus to Cancel button (line 220)

### 7. Settings Storage

**QSettings**: Organization="UIAList", Application="Settings"

**Stored Settings**:
- `defaultAction` (int): 0=Click, 1=DoubleClick, 2=Focus
- `shortcutKey` (string): Global hotkey (default: "Ctrl+Alt+U")
- `welcomeShown` (bool): First-run flag

**Registry Settings**:
- Auto-start: `HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run\UIAList`

### 8. Translations

**Supported Languages**:
- English (en_US) - `translations/UIAList_en_US.ts`
- German (de_DE) - `translations/UIAList_de_DE.ts`

**Translation Files**:
- Source: `translations/*.ts` (Qt Linguist format)
- Compiled: `*.qm` files (embedded in executable via resources)
- Resource file: `src/translations.qrc`

**Update Commands**:
```cmd
lupdate src/ -ts translations/UIAList_en_US.ts translations/UIAList_de_DE.ts
lrelease translations/UIAList_en_US.ts translations/UIAList_de_DE.ts
```

## Build & Deployment

### Build Command (Manual)

```cmd
# x64
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="C:\Qt\6.9.2\msvc2022_64"
cmake --build . --config Release

# ARM64
cmake .. -G "Visual Studio 17 2022" -A ARM64 -DCMAKE_PREFIX_PATH="C:\Qt\6.9.2\msvc2022_arm64"
cmake --build . --config Release
```

### Automated Deployment

**Script**: `deployment/deploy.ps1`

**What it does**:
1. Builds for x64 and ARM64
2. Runs windeployqt to bundle Qt dependencies
3. Creates three package types:
   - Portable ZIP
   - MSI installer (WiX Toolset v6.0)
   - MSIX package (for Microsoft Store)

**Output**: `deployment/UIAList-{type}-v{version}-{arch}.{ext}`

### Package Types

1. **Portable ZIP**:
   - No installation required
   - Extract and run
   - Settings stored in user registry

2. **MSI Installer**:
   - Traditional Windows installer
   - Installs to Program Files
   - Adds Start Menu shortcuts
   - Proper uninstall support

3. **MSIX Package**:
   - Microsoft Store compatible
   - Sandboxed environment
   - Automatic updates via Store
   - Digital signatures required

## File Structure

```
UIAList/
├── src/                      # Source code
│   ├── uialist.cpp/.h       # Main window
│   ├── uialisticon.cpp/.h   # System tray
│   ├── settingsdialog.cpp/.h
│   ├── aboutdialog.cpp/.h
│   ├── welcomedialog.cpp/.h
│   ├── main.cpp
│   ├── UIAList.rc           # Windows resources
│   ├── resources.qrc.in     # Qt resources template
│   └── translations.qrc     # Translation resources
├── translations/            # Localization files
│   ├── UIAList_en_US.ts
│   └── UIAList_de_DE.ts
├── resources/
│   ├── icons/              # Application icons
│   ├── assets/             # MSIX assets
│   └── manifests/          # MSIX manifest
├── docs/                   # Documentation
│   ├── USER_GUIDE.md       # End-user documentation
│   ├── PACKAGING_GUIDE.md  # Build/packaging instructions
│   ├── STORE_LISTING.md    # Microsoft Store listing
│   └── STORE_SUBMISSION_CHECKLIST.md
├── deployment/             # Build and deployment scripts
│   └── deploy.ps1
├── screenshots/            # Application screenshots
├── CMakeLists.txt         # CMake build configuration
├── README.md              # Technical overview
└── CLAUDE.md              # This file
```

## Important Code Locations

### Keyboard Event Handling
- **File**: `src/uialist.cpp`
- **Method**: `UIAList::eventFilter()` (line 564)
- **Purpose**: Intercept Up/Down/Enter keys in search box

### Screen Reader Support
- **File**: `src/uialist.cpp`
- **Methods**:
  - `announceSelectedItem()` (line 633)
  - `announceText()` (line 650)
- **Purpose**: Generate QAccessible events for screen readers

### Control Actions
- **File**: `src/uialist.cpp`
- **Methods**:
  - `clickSelectedControl()` (line 715)
  - `doubleClickSelectedControl()` (line 879)
  - `focusSelectedControl()` (line 805)
  - `executeDefaultAction()` (line 1024)

### Background Enumeration
- **File**: `src/uialist.cpp`
- **Class**: `ControlEnumerationWorker` (line 1063)
- **Methods**:
  - `enumerateControls()` (line 1068)
  - `walkControls()` (line 1128)
  - `cancelEnumeration()` (line 1122)

### Global Hotkey
- **File**: `src/uialisticon.cpp`
- **Method**: `nativeEventFilter()` (handles WM_HOTKEY)

### Settings Management
- **File**: `src/settingsdialog.cpp`
- **Methods**:
  - `loadSettings()` (line 91)
  - `saveSettings()` (line 106)
  - `setAutoStartRegistry()` (line 159)
  - `getAutoStartRegistry()` (line 199)

## Common Development Tasks

### Adding a New Control Type

1. Add case to `getControlTypeString()` in `src/uialist.cpp` (line 369)
2. Consider adding to always-hidden list or optional filters
3. Update translations in both `.ts` files
4. Test with screen readers

### Adding a New Action

1. Add button to `setupUI()` in `src/uialist.cpp`
2. Implement action method (similar to `clickSelectedControl()`)
3. Add to `executeDefaultAction()` switch statement
4. Add accelerator key (Alt+X)
5. Update settings dialog if adding to default action dropdown
6. Update `USER_GUIDE.md`

### Changing Default Hotkey

1. Update default in `src/uialisticon.cpp` (hotkey registration)
2. Update default in `src/settingsdialog.cpp` (line 33 and 101)
3. Update all documentation

### Adding a New Language

1. Create `translations/UIAList_{lang}.ts`
2. Run `lupdate` to extract strings
3. Use Qt Linguist to translate
4. Run `lrelease` to compile
5. Add to `src/translations.qrc`
6. Update `CMakeLists.txt` if needed

## Testing Checklist

### Basic Functionality
- [ ] Hotkey activates window from any app
- [ ] Controls listed for target application
- [ ] Search filter works (single and multi-word)
- [ ] Up/Down arrows navigate in search box
- [ ] Enter executes default action
- [ ] Click, Double Click, Focus buttons work
- [ ] Window closes after action
- [ ] Escape closes window

### Screen Reader Testing
- [ ] JAWS announces control selection changes
- [ ] NVDA reads full control descriptions
- [ ] Windows Narrator speaks selected items
- [ ] Filter checkboxes accessible and announced
- [ ] Buttons have proper accessible names
- [ ] Tab order is logical

### Settings
- [ ] Auto-start registry entry created/removed
- [ ] Default action setting persists
- [ ] Custom hotkey works after save
- [ ] Shortcut capture mode functions

### Edge Cases
- [ ] Empty window (no controls)
- [ ] Very large control list (hundreds of controls)
- [ ] Filter with no matches
- [ ] Cancel during enumeration
- [ ] Multiple rapid hotkey presses
- [ ] Application closes while UIAList open

## Known Issues & Limitations

1. **Some controls don't respond to actions**
   - Depends on application's UIA implementation
   - Custom controls may not support standard patterns
   - Workaround: Use Focus action, then keyboard

2. **Hotkey conflicts**
   - User must choose non-conflicting combination
   - No automatic detection of conflicts
   - Document common conflicts (screen reader keys)

3. **Performance with huge UI trees**
   - Enumeration can take 5-10 seconds
   - Background thread prevents UI freeze
   - Cancel button allows abort

4. **Window auto-closes on focus loss**
   - By design for fast workflow
   - Can surprise users initially
   - Documented in user guide

## Future Enhancement Ideas

- [ ] Save/recall custom filter presets
- [ ] History of recently activated controls
- [ ] Control property inspection (value, state, etc.)
- [ ] Export control list to text file
- [ ] Multiple hotkey profiles
- [ ] Control highlighting in target app
- [ ] Dark mode / high contrast themes
- [ ] Portable settings (INI file option)
- [ ] Scripting/automation API
- [ ] Cloud sync for settings

## Documentation Files

1. **README.md**: Technical overview, architecture, build instructions
2. **docs/USER_GUIDE.md**: Complete end-user documentation with keyboard navigation details
3. **docs/PACKAGING_GUIDE.md**: Build system and deployment process
4. **docs/STORE_LISTING.md**: Microsoft Store description and metadata
5. **CLAUDE.md** (this file): Developer memory and project context

## Important Notes for Claude

### When Modifying Keyboard Navigation:
- Always test with JAWS, NVDA, and Narrator
- Ensure QAccessible events are generated
- Document behavior in USER_GUIDE.md
- Consider impact on screen reader virtual cursor

### When Changing UI Layout:
- Maintain logical tab order
- Set accessible names and descriptions
- Test with keyboard only (no mouse)
- Verify focus indicators are visible

### When Adding Features:
- Update USER_GUIDE.md first (user-facing)
- Update CLAUDE.md with implementation details
- Add settings if user-configurable
- Consider screen reader implications
- Update translations

### When Fixing Bugs:
- Test with multiple applications (Notepad, Explorer, etc.)
- Test with multiple screen readers
- Consider edge cases (empty lists, no focus, etc.)
- Document fix in commit message

### Build System:
- Always test both x64 and ARM64 builds
- Verify all three package types (ZIP, MSI, MSIX)
- Test on clean Windows 10 and Windows 11 systems
- Ensure Qt dependencies are bundled correctly

---

**Project Start Date**: December 2024
**Current Version**: 0.1.0
**License**: GNU General Public License v3.0
**Developed with**: Claude Code by Anthropic
