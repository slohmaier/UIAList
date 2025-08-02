# UIAList - Accessibility Tool for Screen Reader Users

![UIAList Icon](uialist_icon.svg)

**UIAList** is an accessibility tool designed for blind and visually impaired users who rely on screen readers like JAWS, NVDA, or Windows Narrator. This application lists all controls by name and type for the current application with search functionality to quickly find and interact with controls. It saves significant time by eliminating the need for traditional tabbing navigation that is typical with screen readers.

## Overview

UIAList leverages Microsoft's UI Automation (UIA) framework to instantly enumerate and display all accessible controls in the foreground application window. Instead of forcing users to tab through dozens of controls sequentially, users can see all available controls at once, search by name, and jump directly to the desired element.

### Key Features

- **Instant Control Listing**: Shows all controls in the current application window at once - no more sequential tabbing
- **Smart Search Filtering**: Type to instantly filter controls by name - find what you need in seconds
- **Direct Control Interaction**: Click, focus, or double-click any control directly from the list
- **Screen Reader Optimized**: Designed specifically for JAWS, NVDA, and Windows Narrator users
- **Time-Saving Navigation**: Eliminate the tedious tab-through-everything approach
- **Global Hotkey Access**: Activate instantly with Ctrl+Alt+U from any application

### Technical Architecture

**UI Automation Integration**:
- Utilizes `IUIAutomation` COM interface for control discovery
- Implements `IUIAutomationTreeWalker` for hierarchical control enumeration  
- Supports multiple UIA control patterns: Invoke, Toggle, Selection, Legacy IAccessible
- Fallback mouse simulation for maximum compatibility with screen reader environments

**Qt Framework Implementation**:
- Built with Qt 6.9.x for cross-platform UI consistency
- Custom event filtering for enhanced keyboard navigation
- System tray integration for unobtrusive background operation
- Dynamic UI state management and accessibility event generation

## Installation & Usage

1. **Launch Application**: UIAList runs as a system tray application
2. **Activate Interface**: The interface appears when a foreground window change is detected
3. **Filter Controls**: Use the search field to filter available controls by name or type
4. **Navigate & Select**: Use arrow keys for navigation, Enter for interaction
5. **Execute Actions**: Use dedicated buttons or keyboard shortcuts for control interaction

## Keyboard Shortcuts & Navigation

### Primary Navigation
| Key Combination | Action |
|-----------------|--------|
| `Up Arrow` | Navigate to previous visible control in list |
| `Down Arrow` | Navigate to next visible control in list |
| `Enter` | Execute click action on selected control and close window |
| `Escape` | Close UIAList window |

### Accelerator Keys (Alt + Key)
| Shortcut | Button | Action |
|----------|--------|---------|
| `Alt + D` | **Double Click** | Perform double-click action on selected control |
| `Alt + C` | **Click** | Perform single click action on selected control |
| `Alt + F` | **Focus** | Set focus to selected control |

### Control Interaction Methods

The application implements multiple interaction strategies for maximum compatibility:

1. **Mouse Simulation**: Direct coordinate-based mouse events (primary method for JAWS compatibility)
2. **UI Automation Patterns**: 
   - Invoke Pattern (buttons, menu items)
   - Toggle Pattern (checkboxes, radio buttons) 
   - Selection Pattern (list items, tree nodes)
3. **Legacy IAccessible**: Fallback for older applications
4. **Keyboard Navigation**: Tab key simulation for focus management

### Advanced Features

**Automatic Control Selection**:
- If no control is selected, the first visible control is automatically chosen
- Smart handling of filtered results and empty lists

**Dynamic Button States**:
- Control interaction buttons are automatically disabled when no valid targets exist
- Real-time updates based on filter results

**Screen Reader Integration**:
- Suppresses default edit box announcements during arrow key navigation
- Provides custom announcements for newly selected controls
- Maintains focus context for optimal screen reader experience

## Technical Requirements

- **Operating System**: Windows 10/11 (x64)
- **Framework**: Qt 6.9.x or later
- **Dependencies**: Windows UI Automation API, COM interfaces
- **Screen Readers**: Compatible with JAWS, NVDA, Windows Narrator
- **Compiler**: MSVC 2022 or later for Windows-specific UIA integration

## Architecture Details

**Control Information Structure**:
```cpp
struct ControlInfo {
    QString displayText;      // Formatted display string
    QString originalName;     // Raw control name for filtering
    IUIAutomationElement* element;  // UIA element reference
};
```

**Event Processing**:
- Custom `QObject::eventFilter` implementation for keyboard event interception
- Focus management with `focusOutEvent` for window auto-closure
- Real-time filter updates with `QLineEdit::textChanged` signal handling

**Memory Management**:
- RAII-compliant COM interface handling with proper `AddRef`/`Release` cycles
- Automatic cleanup of UI Automation resources on application termination

## Use Cases

- **Form Navigation**: Quickly locate specific input fields in complex forms
- **Menu System Access**: Direct access to menu items without hierarchical navigation
- **Dialog Interaction**: Efficient button and control location in modal dialogs
- **Application Exploration**: Discovery of available controls in unfamiliar applications
- **Accessibility Testing**: Control enumeration for accessibility validation

## Development

This application was entirely developed using **Claude Code** - Anthropic's AI-powered development assistant. From initial concept to final implementation, Claude Code handled:

- Complete C++/Qt codebase architecture and implementation
- Windows UI Automation API integration
- Screen reader compatibility optimization
- User interface design and accessibility features
- Documentation and technical specification

---

**UIAList** - Enhancing digital accessibility through intelligent UI automation and screen reader integration.

*Developed with Claude Code by Anthropic*