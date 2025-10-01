# UIAList User Guide

## Table of Contents
- [Introduction](#introduction)
- [Getting Started](#getting-started)
- [Main Window Interface](#main-window-interface)
- [Keyboard Navigation](#keyboard-navigation)
- [Search and Filter](#search-and-filter)
- [Control Actions](#control-actions)
- [Settings](#settings)
- [Tips for Screen Reader Users](#tips-for-screen-reader-users)

## Introduction

UIAList is an accessibility tool specifically designed for blind and visually impaired users who rely on screen readers (JAWS, NVDA, Windows Narrator). Instead of tabbing through controls one by one, UIAList instantly lists all interactive controls in the current application window, allowing you to quickly search, navigate, and interact with them.

## Getting Started

### First Launch

1. **Installation**: After installing UIAList, the application will start automatically and appear in your system tray
2. **Welcome Screen**: On first launch, you'll see a welcome dialog explaining the basic features (this only appears once)
3. **System Tray**: UIAList runs in the background with an icon in the system tray

### Activating UIAList

**Default Hotkey**: Press `Ctrl+Alt+U` from any application window

When activated:
1. UIAList captures the foreground application
2. A loading screen appears with a progress indicator
3. All controls are enumerated in the background
4. The main window appears with the list of controls

**Note**: The hotkey can be customized in Settings (right-click system tray icon > Settings)

## Main Window Interface

### Window Layout

The UIAList window contains the following elements from top to bottom:

1. **Window Title Label** (read-only)
   - Displays which application's controls are being shown
   - Example: "Controls for: Notepad"

2. **Filter/Search Box** (edit field)
   - Type here to filter the control list
   - Supports multi-word search
   - Has focus when window opens

3. **Control List** (list view)
   - Shows all matching controls
   - Format: "ControlType: ControlName"
   - Example: "Button: Save", "Edit: Document Name"

4. **Filter Options**
   - Two checkboxes to customize what's shown:
     - "Hide controls with no or empty title" (checked by default)
     - "Hide menus and menu items" (checked by default)

5. **Action Buttons**
   - Three buttons for control interaction
   - Enabled only when valid controls exist

### Control Information Format

Each control in the list is displayed as:
```
ControlType: ControlName
```

**Examples**:
- `Button: OK`
- `Edit: Search`
- `CheckBox: Remember me`
- `ComboBox: Language Selection`

## Keyboard Navigation

UIAList is optimized for keyboard-only operation with special attention to screen reader users.

### Primary Navigation Keys

| Key | Action | Details |
|-----|--------|---------|
| `Up Arrow` | Previous control | Moves selection up in the list; wraps to bottom when at top |
| `Down Arrow` | Next control | Moves selection down in the list; wraps to top when at bottom |
| `Enter` | Execute default action | Performs configured action (Click/Double Click/Focus) and closes window |
| `Escape` | Close window | Closes UIAList without taking action |
| `Tab` | Navigate between UI elements | Cycles through filter box, checkboxes, and buttons |

### Special Arrow Key Behavior in Search Box

**Key Feature**: When focus is in the search/filter box, the Up and Down arrow keys work differently than in standard edit boxes:

- **Up Arrow**: Selects the previous visible control in the list (instead of moving cursor to start of text)
- **Down Arrow**: Selects the next visible control in the list (instead of moving cursor to end of text)
- **Enter**: Executes the default action on the currently selected control (instead of inserting newline)

**Benefits for Screen Readers**:
- No need to tab out of the search box to navigate the results
- Type to filter, then immediately use arrow keys to browse matches
- Screen reader automatically announces each newly selected item
- Enter key triggers action directly from the search box

**Example Workflow**:
1. Window opens with focus in search box
2. Type "save" to filter controls
3. Press Down Arrow - screen reader announces "Button: Save As"
4. Press Down Arrow again - screen reader announces "Button: Save All"
5. Press Enter - executes click on "Save All" and closes window

### Screen Reader Announcements

UIAList generates accessibility events to ensure screen readers properly announce:

1. **Window Opening**: "Showing controls for [Application Name]"
2. **Selection Changes**: Full text of newly selected control when using arrow keys
3. **Filter Results**: Updates as you type (screen reader announces count changes)
4. **Button States**: Announces when buttons become enabled/disabled

### Accelerator Keys (Alt Shortcuts)

While focused anywhere in the window:

| Shortcut | Action | Button |
|----------|--------|--------|
| `Alt+D` | Double-click selected control | **Double Click** button |
| `Alt+C` | Click selected control | **Click** button |
| `Alt+F` | Focus selected control | **Focus** button |

**Note**: These accelerators work even when the search box has focus.

## Search and Filter

### Basic Search

The search/filter box supports real-time filtering:

1. **Single Word**: Type any word to show only matching controls
   - Example: Type "save" to show all controls with "save" in the name

2. **Multiple Words**: Type multiple words separated by spaces (AND logic)
   - Example: Type "button save" to show only buttons with "save" in the name
   - Order doesn't matter: "save button" produces same results

3. **Case Insensitive**: Searches are not case-sensitive
   - "SAVE", "save", and "Save" all produce identical results

### Auto-Selection

**Smart First Item Selection**:
- When the window opens, the first visible control is automatically selected
- When you type in search box and filter results, the first matching item is auto-selected
- If currently selected item becomes hidden by filter, selection jumps to first visible item
- Your screen reader will announce these automatic selections

### Filter Options

**Hide controls with no or empty title** (Checkbox)
- Default: Checked
- Purpose: Many applications have numerous unlabeled technical controls
- Effect: Hides controls with empty names or "(no name)"
- Unchecking shows all controls including unlabeled ones

**Hide menus and menu items** (Checkbox)
- Default: Checked
- Purpose: Menu items often clutter the list and are usually accessed via Alt key
- Effect: Removes Menu, MenuBar, and MenuItem controls from list
- Unchecking shows all menu-related controls

**Always Filtered Out**:
- Text controls (static text labels) - always hidden
- Window controls - always hidden

### Visible Item Count

When filter is active:
- Only matching controls are visible in the list
- List view updates instantly as you type
- Screen reader announces the currently selected (visible) item
- Hidden items are not navigable with arrow keys

## Control Actions

UIAList provides three primary ways to interact with controls:

### 1. Click (Default Enter Action)

**What it does**: Simulates a left mouse click on the control

**Activation**:
- Press `Enter` while search box has focus (if Click is default action)
- Press `Alt+C` from anywhere in window
- Click the "Click" button with mouse/touch

**How it works** (in order of precedence):
1. Calculates center point of control's bounding rectangle
2. Moves mouse cursor to that position
3. Simulates left mouse button down and up events
4. Fallback: Uses UI Automation Invoke pattern
5. Fallback: Uses Legacy IAccessible pattern
6. Fallback: Uses SelectionItem pattern

**Best for**:
- Buttons
- Links/Hyperlinks
- Toolbar items
- List items
- Tree items

### 2. Double Click

**What it does**: Simulates a double-click (two rapid clicks) on the control

**Activation**:
- Press `Enter` while search box has focus (if Double Click is default action)
- Press `Alt+D` from anywhere in window
- Click the "Double Click" button

**How it works**:
1. Simulates mouse double-click at control's center
2. Fallback: Uses UI Automation Toggle pattern
3. Fallback: Calls legacy DoDefaultAction twice

**Best for**:
- Opening items (files, folders)
- Expanding tree nodes
- Toggle controls in some applications
- Custom controls requiring double-click

### 3. Focus

**What it does**: Sets keyboard focus to the control without activating it

**Activation**:
- Press `Enter` while search box has focus (if Focus is default action)
- Press `Alt+F` from anywhere in window
- Click the "Focus" button

**How it works**:
1. Calls UI Automation SetFocus method
2. Fallback: Clicks the control to set focus
3. Fallback: Simulates Tab key navigation

**Best for**:
- Edit boxes (text fields) - focus without clicking
- Combo boxes - focus without opening dropdown
- Controls where you want to position focus, then use keyboard commands
- Navigating to specific areas of an application

### Automatic Control Selection

**If no control is explicitly selected**:
- When you press Enter or use accelerator keys
- UIAList automatically selects the first visible control
- Then executes the requested action
- Ensures you can always take action even without explicit selection

### Window Auto-Close

**After executing any action** (Click, Double Click, or Focus):
- The UIAList window automatically closes
- Focus returns to the target application
- Your screen reader focuses on the control you just interacted with
- This keeps workflow fast and uninterrupted

### Button States

The action buttons are dynamically enabled/disabled:
- **Enabled**: When one or more controls are visible in the filtered list
- **Disabled**: When filter results in no visible controls
- Screen readers announce the enabled/disabled state when navigating to buttons

## Settings

### Accessing Settings

**From System Tray**:
1. Locate UIAList icon in system tray (notification area)
2. Right-click the icon
3. Select "Settings" from context menu

**Settings Dialog Layout**:
- Modal dialog box
- Three main settings
- OK and Cancel buttons

### Available Settings

#### 1. Start Automatically When Windows Starts

**Type**: Checkbox

**Default**: Unchecked

**What it does**:
- When checked, UIAList is added to Windows startup
- Application starts automatically when you log in
- Runs in system tray, ready for hotkey activation
- Uses Windows Registry: `HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run`

**When to enable**:
- If you use UIAList frequently throughout the day
- If you want immediate access after login
- If you prefer not to manually start applications

#### 2. Default Action When Pressing Enter

**Type**: Combo box (dropdown)

**Options**:
- Click (default)
- Double Click
- Focus

**What it does**:
- Determines which action is executed when you press Enter in the search box
- Also determines which action is considered "default" for the selected control
- Does not affect Alt+C, Alt+D, Alt+F shortcuts (they always do their specific action)

**Recommendation**:
- **Click**: Best for most users and general-purpose control interaction
- **Double Click**: If you frequently work with file managers or tree views
- **Focus**: If you primarily want to navigate to controls and use keyboard commands

**Example**:
- Set to "Click": Press Enter → Clicks the selected button
- Set to "Focus": Press Enter → Focuses the edit box (then you can type)

#### 3. Global Shortcut Key

**Type**: Button that opens key capture mode

**Default**: `Ctrl+Alt+U`

**How to change**:
1. Click the "Ctrl+Alt+U" button (or current shortcut display)
2. Button turns pink and shows "Press key combination..."
3. Press your desired key combination (must include at least one modifier)
4. Button displays the new shortcut
5. Click OK to save, Cancel to discard

**Valid modifiers**:
- Ctrl (Control)
- Alt
- Shift
- Any combination of above

**Valid keys**:
- Letters A-Z
- Numbers 0-9
- Function keys F1-F12
- Other special keys

**Examples**:
- `Ctrl+Shift+L`
- `Alt+Space`
- `Ctrl+Alt+F12`

**Restrictions**:
- Must include at least one modifier key (Ctrl, Alt, or Shift)
- Cannot use modifier keys alone
- Avoid combinations used by Windows or your screen reader

**Troubleshooting**:
- If shortcut doesn't work, it may conflict with another application
- Try a different combination
- Check your screen reader's keyboard shortcuts to avoid conflicts

### Saving Settings

**Automatic Save**:
- Click OK button to save all settings
- Click Cancel to discard changes
- Settings persist across application restarts

**Settings Storage**:
- Stored in Windows Registry: `HKEY_CURRENT_USER\Software\UIAList\Settings`
- Auto-start setting: Stored in Windows Run registry key
- Portable: Settings travel with your Windows user profile

## Tips for Screen Reader Users

### General Usage Tips

1. **Keep it Running**: Enable auto-start so UIAList is always ready
2. **Learn the Hotkey**: Make `Ctrl+Alt+U` (or your custom hotkey) muscle memory
3. **Filter First**: Type a few characters to reduce the list before browsing
4. **Multi-Word Search**: Combine control type and name (e.g., "button save")
5. **Stay in Search Box**: Use arrow keys without tabbing out

### JAWS-Specific Tips

- **Virtual Cursor**: UIAList works with virtual cursor on or off
- **Forms Mode**: Window automatically enters forms mode for editing
- **Announcement**: JAWS announces control selection changes via accessibility events
- **Mouse Simulation**: UIAList uses mouse simulation for maximum JAWS compatibility

### NVDA-Specific Tips

- **Browse Mode**: UIAList works in both browse and focus modes
- **Focus Follows**: NVDA follows focus to the UIAList window automatically
- **Control Announce**: NVDA reads the full control description on selection
- **Object Navigation**: You can use NVDA's object navigation within the list

### Windows Narrator Tips

- **Scan Mode**: Exit scan mode (Caps Lock + Space) for full interaction
- **Auto-Read**: Narrator automatically reads selected items as you navigate
- **Touch**: UIAList is fully touch-enabled for Narrator users on tablets
- **Verbosity**: Set Narrator to higher verbosity for more detailed announcements

### Workflow Optimizations

**Scenario 1: Quick Button Press**
1. `Ctrl+Alt+U` - Open UIAList
2. Type partial button name (e.g., "ok")
3. `Enter` - Click and close
- Total: 3 steps, ~2 seconds

**Scenario 2: Navigate Through Similar Controls**
1. `Ctrl+Alt+U` - Open UIAList
2. Type control type (e.g., "button")
3. Use arrow keys to browse all buttons
4. `Enter` on desired button
- Faster than tabbing through entire form

**Scenario 3: Find Hidden/Difficult Control**
1. `Ctrl+Alt+U` - Open UIAList
2. Uncheck "Hide controls with no or empty title"
3. Type control type (e.g., "checkbox")
4. Browse with arrow keys until screen reader announces the right one
5. `Enter` to activate

**Scenario 4: Focus Without Activating**
1. `Ctrl+Alt+U` - Open UIAList
2. Filter to desired control
3. `Alt+F` - Focus without clicking
4. Use keyboard commands in target application

### Common Issues and Solutions

**Problem**: Screen reader doesn't announce selected items
- **Solution**: Make sure accessibility features are enabled in Windows
- **Solution**: Restart UIAList and your screen reader

**Problem**: Hotkey doesn't activate UIAList
- **Solution**: Check Settings, ensure hotkey isn't conflicting
- **Solution**: Try closing other applications that might capture the hotkey
- **Solution**: Choose a different hotkey combination

**Problem**: Too many controls in list
- **Solution**: Enable "Hide controls with no or empty title"
- **Solution**: Enable "Hide menus and menu items"
- **Solution**: Use more specific search terms (multiple words)

**Problem**: Can't find a specific control
- **Solution**: Uncheck both filter checkboxes to show all controls
- **Solution**: Try searching by control type (Button, Edit, CheckBox, etc.)
- **Solution**: Try partial name matching (e.g., "save" finds "Save As")

**Problem**: Action doesn't work on control
- **Solution**: Try Focus action, then use keyboard to activate
- **Solution**: Try Double Click instead of Click
- **Solution**: Some controls may not support automation (report to developer)

**Problem**: Window closes unexpectedly
- **Solution**: This is by design when losing focus (helps workflow)
- **Solution**: Stay within the UIAList window to keep it open
- **Solution**: Press `Ctrl+Alt+U` again to reopen instantly

### Accessibility Features Summary

✓ Full keyboard navigation - no mouse required
✓ Screen reader optimized announcements
✓ Logical tab order through all controls
✓ Accelerator keys for all actions
✓ High contrast compatible
✓ Works with magnification software
✓ Touch-enabled for tablet users
✓ Clear focus indicators
✓ Descriptive button and control labels
✓ Accessible dialogs and messages

---

## About This Guide

**UIAList Version**: 0.1.0
**Last Updated**: 2025-01-01
**Guide Version**: 1.0

For technical documentation, see the main [README.md](../README.md).

For development and packaging information, see [PACKAGING_GUIDE.md](PACKAGING_GUIDE.md).

**Developed with Claude Code by Anthropic**
