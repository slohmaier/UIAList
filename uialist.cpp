/*
 * UIAList - Accessibility Tool for Screen Reader Users
 * Copyright (C) 2025 Stefan Lohmaier
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "uialist.h"
#include "uialisticon.h"
#include <QDebug>
#include <QListWidgetItem>
#include <QVariant>
#include <QKeyEvent>
#include <QAccessible>
#include <QRegularExpression>
#include <QIcon>

#include <comdef.h>
#include <atlbase.h>

UIAList::UIAList(QWidget *parent)
    : QMainWindow(parent), m_trayIcon(nullptr), m_centralWidget(nullptr), 
      m_layout(nullptr), m_buttonLayout(nullptr), m_windowTitleLabel(nullptr), m_filterEdit(nullptr), m_listWidget(nullptr),
      m_hideEmptyTitlesCheckBox(nullptr), m_hideMenusCheckBox(nullptr), m_clickButton(nullptr), m_focusButton(nullptr), 
      m_doubleClickButton(nullptr), m_uiAutomation(nullptr), m_controlViewWalker(nullptr), m_settings(nullptr)
{
    m_settings = new QSettings("UIAList", "Settings", this);
    
    setupUI();
    initializeUIAutomation();
    
    m_trayIcon = new UIAListIcon(this);
    connect(m_trayIcon, &UIAListIcon::activateRequested, this, &UIAList::showWindow);
    m_trayIcon->show();
    
    // Hide the main window by default, only show tray icon
    hide();
}

UIAList::~UIAList() 
{
    cleanupUIAutomation();
}

void UIAList::setupUI()
{
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    m_layout = new QVBoxLayout(m_centralWidget);
    
    // Window title label
    m_windowTitleLabel = new QLabel(this);
    m_windowTitleLabel->setFocusPolicy(Qt::TabFocus);
    m_windowTitleLabel->setText("No window selected");
    
    // Filter edit box
    m_filterEdit = new QLineEdit(this);
    m_filterEdit->setPlaceholderText("Filter controls...");
    connect(m_filterEdit, &QLineEdit::textChanged, this, &UIAList::onFilterChanged);
    m_filterEdit->installEventFilter(this);
    
    // List widget
    m_listWidget = new QListWidget(this);
    connect(m_listWidget, &QListWidget::itemSelectionChanged, this, &UIAList::onItemSelectionChanged);
    
    // Hide empty titles checkbox
    m_hideEmptyTitlesCheckBox = new QCheckBox("Hide controls with no or empty title", this);
    m_hideEmptyTitlesCheckBox->setChecked(true); // Enabled by default
    connect(m_hideEmptyTitlesCheckBox, &QCheckBox::toggled, this, &UIAList::onHideEmptyTitlesChanged);
    
    // Hide menus checkbox
    m_hideMenusCheckBox = new QCheckBox("Hide menus and menu items", this);
    m_hideMenusCheckBox->setChecked(true); // Enabled by default
    connect(m_hideMenusCheckBox, &QCheckBox::toggled, this, &UIAList::onHideMenusChanged);
    
    // Buttons layout
    m_buttonLayout = new QHBoxLayout();
    
    // Double Click, Click, and Focus buttons with accelerator keys (reordered)
    m_doubleClickButton = new QPushButton("&Double Click", this);
    m_clickButton = new QPushButton("&Click", this);
    m_focusButton = new QPushButton("&Focus", this);
    
    connect(m_clickButton, &QPushButton::clicked, this, &UIAList::onClickButtonClicked);
    connect(m_focusButton, &QPushButton::clicked, this, &UIAList::onFocusButtonClicked);
    connect(m_doubleClickButton, &QPushButton::clicked, this, &UIAList::onDoubleClickButtonClicked);
    
    m_buttonLayout->addWidget(m_doubleClickButton);
    m_buttonLayout->addWidget(m_clickButton);
    m_buttonLayout->addWidget(m_focusButton);
    m_buttonLayout->addStretch(); // Add stretch to push buttons to the left
    
    m_layout->addWidget(m_windowTitleLabel);
    m_layout->addWidget(m_filterEdit);
    m_layout->addWidget(m_listWidget);
    m_layout->addWidget(m_hideEmptyTitlesCheckBox);
    m_layout->addWidget(m_hideMenusCheckBox);
    m_layout->addLayout(m_buttonLayout);
    
    setWindowTitle("UIAList - Screen Reader Control Navigator");
    setWindowIcon(QIcon(":/icons/uialist_icon.png"));
    resize(600, 400);
}

void UIAList::initializeUIAutomation()
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) {
        qDebug() << "Failed to initialize COM";
        return;
    }
    
    hr = CoCreateInstance(__uuidof(CUIAutomation), nullptr, CLSCTX_INPROC_SERVER,
                         __uuidof(IUIAutomation), (void**)&m_uiAutomation);
    if (FAILED(hr)) {
        qDebug() << "Failed to create UI Automation instance";
        return;
    }
    
    hr = m_uiAutomation->get_ControlViewWalker(&m_controlViewWalker);
    if (FAILED(hr)) {
        qDebug() << "Failed to get ControlViewWalker";
        return;
    }
    
    qDebug() << "UI Automation initialized successfully";
}

void UIAList::showWindow(void* foregroundWindow)
{
    show();
    raise();
    activateWindow();
    
    // Clear filter and set focus to filter edit box
    m_filterEdit->clear();
    m_filterEdit->setFocus();
    
    // Enumerate controls of the passed foreground window
    enumerateControls(foregroundWindow);
    
    // Announce to screen reader
    if (!m_targetWindowTitle.isEmpty()) {
        announceText(QString("Showing controls for %1").arg(m_targetWindowTitle));
    }
}

void UIAList::enumerateControls(void* windowHandle)
{
    if (!m_uiAutomation || !m_controlViewWalker) {
        qDebug() << "UI Automation not initialized";
        return;
    }
    
    HWND targetWindow = (HWND)windowHandle;
    if (!targetWindow) {
        qDebug() << "No target window provided";
        return;
    }
    
    // Clear previous data
    m_allControls.clear();
    m_controlMap.clear();
    m_listWidget->clear();
    
    // Get window title for debugging and display
    wchar_t windowTitle[256];
    GetWindowTextW(targetWindow, windowTitle, 256);
    m_targetWindowTitle = QString::fromWCharArray(windowTitle);
    if (m_targetWindowTitle.isEmpty()) {
        m_targetWindowTitle = "Untitled Window";
    }
    qDebug() << "Enumerating controls for window:" << m_targetWindowTitle << "HWND:" << targetWindow;
    
    // Update the window title label
    m_windowTitleLabel->setText(QString("Controls for: %1").arg(m_targetWindowTitle));
    
    // Get UI Automation element for the target window
    IUIAutomationElement* rootElement = nullptr;
    HRESULT hr = m_uiAutomation->ElementFromHandle(targetWindow, &rootElement);
    if (FAILED(hr) || !rootElement) {
        qDebug() << "Failed to get root element from target window";
        return;
    }
    
    qDebug() << "Starting control enumeration...";
    walkControls(rootElement, m_controlViewWalker);
    
    rootElement->Release();
    
    populateListWidget();
    qDebug() << "Enumeration complete. Found" << m_allControls.size() << "controls";
}

void UIAList::walkControls(IUIAutomationElement* element, IUIAutomationTreeWalker* walker)
{
    if (!element || !walker) return;
    
    // Get control type
    CONTROLTYPEID controlType;
    HRESULT hr = element->get_CurrentControlType(&controlType);
    if (FAILED(hr)) return;
    
    // Get control name
    BSTR name = nullptr;
    element->get_CurrentName(&name);
    QString controlName = name ? QString::fromWCharArray(name) : QString("(no name)");
    if (name) SysFreeString(name);
    
    // Get control type string
    QString controlTypeStr = getControlTypeString(controlType);
    
    // Create display text
    QString displayText = QString("%1: %2").arg(controlTypeStr, controlName);
    
    // Store control info with original name and control type
    ControlInfo controlInfo(displayText, controlName, element, controlType);
    m_allControls.append(controlInfo);
    
    // Walk child elements
    IUIAutomationElement* child = nullptr;
    hr = walker->GetFirstChildElement(element, &child);
    while (SUCCEEDED(hr) && child) {
        walkControls(child, walker);
        
        IUIAutomationElement* nextChild = nullptr;
        hr = walker->GetNextSiblingElement(child, &nextChild);
        child->Release();
        child = nextChild;
    }
}

QString UIAList::getControlTypeString(CONTROLTYPEID controlType)
{
    switch (controlType) {
        case UIA_ButtonControlTypeId: return "Button";
        case UIA_CheckBoxControlTypeId: return "CheckBox";
        case UIA_ComboBoxControlTypeId: return "ComboBox";
        case UIA_EditControlTypeId: return "Edit";
        case UIA_HyperlinkControlTypeId: return "Hyperlink";
        case UIA_ImageControlTypeId: return "Image";
        case UIA_ListItemControlTypeId: return "ListItem";
        case UIA_ListControlTypeId: return "List";
        case UIA_MenuControlTypeId: return "Menu";
        case UIA_MenuBarControlTypeId: return "MenuBar";
        case UIA_MenuItemControlTypeId: return "MenuItem";
        case UIA_ProgressBarControlTypeId: return "ProgressBar";
        case UIA_RadioButtonControlTypeId: return "RadioButton";
        case UIA_ScrollBarControlTypeId: return "ScrollBar";
        case UIA_SliderControlTypeId: return "Slider";
        case UIA_SpinnerControlTypeId: return "Spinner";
        case UIA_StatusBarControlTypeId: return "StatusBar";
        case UIA_TabControlTypeId: return "Tab";
        case UIA_TabItemControlTypeId: return "TabItem";
        case UIA_TextControlTypeId: return "Text";
        case UIA_ToolBarControlTypeId: return "ToolBar";
        case UIA_ToolTipControlTypeId: return "ToolTip";
        case UIA_TreeControlTypeId: return "Tree";
        case UIA_TreeItemControlTypeId: return "TreeItem";
        case UIA_CustomControlTypeId: return "Custom";
        case UIA_GroupControlTypeId: return "Group";
        case UIA_ThumbControlTypeId: return "Thumb";
        case UIA_DataGridControlTypeId: return "DataGrid";
        case UIA_DataItemControlTypeId: return "DataItem";
        case UIA_DocumentControlTypeId: return "Document";
        case UIA_SplitButtonControlTypeId: return "SplitButton";
        case UIA_WindowControlTypeId: return "Window";
        case UIA_PaneControlTypeId: return "Pane";
        case UIA_HeaderControlTypeId: return "Header";
        case UIA_HeaderItemControlTypeId: return "HeaderItem";
        case UIA_TableControlTypeId: return "Table";
        case UIA_TitleBarControlTypeId: return "TitleBar";
        case UIA_SeparatorControlTypeId: return "Separator";
        default: return QString("Unknown(%1)").arg(controlType);
    }
}

void UIAList::populateListWidget()
{
    m_listWidget->clear();
    
    bool hideEmptyTitles = m_hideEmptyTitlesCheckBox && m_hideEmptyTitlesCheckBox->isChecked();
    bool hideMenus = m_hideMenusCheckBox && m_hideMenusCheckBox->isChecked();
    
    for (int i = 0; i < m_allControls.size(); ++i) {
        const ControlInfo& controlInfo = m_allControls[i];
        
        // Always filter out Text and Window control types
        if (controlInfo.controlType == UIA_TextControlTypeId || 
            controlInfo.controlType == UIA_WindowControlTypeId) {
            continue;
        }
        
        // Skip controls with empty or no titles if checkbox is checked
        if (hideEmptyTitles) {
            QString name = controlInfo.originalName.trimmed();
            if (name.isEmpty() || name == "(no name)") {
                continue; // Skip this control
            }
        }
        
        // Skip menus and menu items if checkbox is checked
        if (hideMenus) {
            if (controlInfo.controlType == UIA_MenuControlTypeId ||
                controlInfo.controlType == UIA_MenuBarControlTypeId ||
                controlInfo.controlType == UIA_MenuItemControlTypeId) {
                continue; // Skip this control
            }
        }
        
        QListWidgetItem* item = new QListWidgetItem(controlInfo.displayText);
        item->setData(Qt::UserRole, i); // Store index to m_allControls
        m_listWidget->addItem(item);
    }
    
    // Auto-select the first item if none is selected
    if (m_listWidget->count() > 0 && m_listWidget->currentRow() == -1) {
        m_listWidget->setCurrentRow(0);
        QListWidgetItem* firstItem = m_listWidget->item(0);
        if (firstItem) {
            announceSelectedItem(firstItem->text());
        }
    }
    
    updateButtonStates();
}

void UIAList::onFilterChanged(const QString& text)
{
    // Split filter text into individual words
    QStringList filterWords = text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    
    for (int i = 0; i < m_listWidget->count(); ++i) {
        QListWidgetItem* item = m_listWidget->item(i);
        bool visible = true;
        
        if (!filterWords.isEmpty()) {
            QString itemText = item->text();
            
            // Check if all filter words are present in the item text (case insensitive)
            for (const QString& word : filterWords) {
                if (!itemText.contains(word, Qt::CaseInsensitive)) {
                    visible = false;
                    break;
                }
            }
        }
        
        item->setHidden(!visible);
    }
    
    // If no item is currently selected, select the first visible item
    if (m_listWidget->currentRow() == -1) {
        for (int i = 0; i < m_listWidget->count(); ++i) {
            QListWidgetItem* item = m_listWidget->item(i);
            if (item && !item->isHidden()) {
                m_listWidget->setCurrentRow(i);
                announceSelectedItem(item->text());
                break;
            }
        }
    } else {
        // Check if the currently selected item is still visible
        QListWidgetItem* currentItem = m_listWidget->currentItem();
        if (currentItem && currentItem->isHidden()) {
            // Current item is hidden, select the first visible item
            m_listWidget->setCurrentRow(-1); // Clear selection first
            for (int i = 0; i < m_listWidget->count(); ++i) {
                QListWidgetItem* item = m_listWidget->item(i);
                if (item && !item->isHidden()) {
                    m_listWidget->setCurrentRow(i);
                    announceSelectedItem(item->text());
                    break;
                }
            }
        }
    }
    
    updateButtonStates();
}

void UIAList::onItemSelectionChanged()
{
    QList<QListWidgetItem*> selectedItems = m_listWidget->selectedItems();
    if (selectedItems.isEmpty()) {
        m_selectedControl = ControlInfo();
        return;
    }
    
    QListWidgetItem* selectedItem = selectedItems.first();
    int index = selectedItem->data(Qt::UserRole).toInt();
    
    if (index >= 0 && index < m_allControls.size()) {
        m_selectedControl = m_allControls[index];
        qDebug() << "Selected control:" << m_selectedControl.displayText;
    }
}

void UIAList::onHideEmptyTitlesChanged(bool checked)
{
    Q_UNUSED(checked)
    // Repopulate the list with the new filter setting
    populateListWidget();
}

void UIAList::onHideMenusChanged(bool checked)
{
    Q_UNUSED(checked)
    // Repopulate the list with the new filter setting
    populateListWidget();
}

void UIAList::cleanupUIAutomation()
{
    if (m_controlViewWalker) {
        m_controlViewWalker->Release();
        m_controlViewWalker = nullptr;
    }
    
    if (m_uiAutomation) {
        m_uiAutomation->Release();
        m_uiAutomation = nullptr;
    }
    
    CoUninitialize();
}

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

void UIAList::selectVisibleListItem(int direction)
{
    if (!m_listWidget || m_listWidget->count() == 0) {
        return;
    }
    
    int currentRow = m_listWidget->currentRow();
    int newRow = currentRow;
    
    // Find the next visible item in the specified direction
    for (int i = 0; i < m_listWidget->count(); ++i) {
        int candidateRow = currentRow + (direction * (i + 1));
        
        // Wrap around if needed
        if (candidateRow < 0) {
            candidateRow = m_listWidget->count() - 1;
        } else if (candidateRow >= m_listWidget->count()) {
            candidateRow = 0;
        }
        
        QListWidgetItem *item = m_listWidget->item(candidateRow);
        if (item && !item->isHidden()) {
            newRow = candidateRow;
            break;
        }
    }
    
    // If no visible item found and we don't have a current selection, select the first visible item
    if (currentRow == -1) {
        for (int i = 0; i < m_listWidget->count(); ++i) {
            QListWidgetItem *item = m_listWidget->item(i);
            if (item && !item->isHidden()) {
                newRow = i;
                break;
            }
        }
    }
    
    if (newRow != currentRow && newRow >= 0) {
        m_listWidget->setCurrentRow(newRow);
        QListWidgetItem *selectedItem = m_listWidget->item(newRow);
        if (selectedItem) {
            announceSelectedItem(selectedItem->text());
        }
    }
}

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
    
    // Also announce using Windows SAPI if available
    // This would require additional Windows-specific screen reader announcement code
    // For now, we rely on Qt's accessibility framework
}

void UIAList::announceText(const QString& text)
{
    // Set the window title label as accessible name and description for screen readers
    if (m_windowTitleLabel) {
        m_windowTitleLabel->setAccessibleName(text);
        m_windowTitleLabel->setAccessibleDescription(text);
        
        // Send a focus event to the label to make screen readers announce it
        QAccessibleEvent labelEvent(m_windowTitleLabel, QAccessible::Focus);
        QAccessible::updateAccessibility(&labelEvent);
        
        // Also send a name changed event
        QAccessibleEvent nameEvent(m_windowTitleLabel, QAccessible::NameChanged);
        QAccessible::updateAccessibility(&nameEvent);
    }
}

void UIAList::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        hide();
        return;
    }
    
    QMainWindow::keyPressEvent(event);
}

void UIAList::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event)
    // Close window when it loses focus
    hide();
}

bool UIAList::event(QEvent *event)
{
    if (event->type() == QEvent::WindowDeactivate) {
        // Window lost focus/activation - hide it
        hide();
        return true;
    }
    return QMainWindow::event(event);
}

void UIAList::onClickButtonClicked()
{
    ensureItemSelected();
    clickSelectedControl();
    hide();
}

void UIAList::onFocusButtonClicked()
{
    ensureItemSelected();
    focusSelectedControl();
    hide();
}

void UIAList::onDoubleClickButtonClicked()
{
    ensureItemSelected();
    doubleClickSelectedControl();
    hide();
}

void UIAList::clickSelectedControl()
{
    QList<QListWidgetItem*> selectedItems = m_listWidget->selectedItems();
    if (selectedItems.isEmpty()) {
        qDebug() << "No control selected";
        return;
    }
    
    QListWidgetItem* selectedItem = selectedItems.first();
    int index = selectedItem->data(Qt::UserRole).toInt();
    
    if (index >= 0 && index < m_allControls.size()) {
        const ControlInfo& controlInfo = m_allControls[index];
        IUIAutomationElement* element = controlInfo.element;
        
        if (element) {
            qDebug() << "Attempting to click control:" << controlInfo.displayText;
            
            // Method 1: Try to get bounding rectangle and simulate mouse click
            RECT rect;
            HRESULT hr = element->get_CurrentBoundingRectangle(&rect);
            if (SUCCEEDED(hr)) {
                int x = rect.left + (rect.right - rect.left) / 2;
                int y = rect.top + (rect.bottom - rect.top) / 2;
                
                qDebug() << "Attempting mouse click at coordinates:" << x << "," << y;
                
                // Use Windows API to simulate mouse click
                SetCursorPos(x, y);
                Sleep(50); // Small delay for stability
                mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
                mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
                
                qDebug() << "Successfully clicked control via mouse simulation:" << controlInfo.displayText;
                return;
            }
            
            // Method 2: Try to invoke the element (for buttons, etc.)
            IUIAutomationInvokePattern* invokePattern = nullptr;
            hr = element->GetCurrentPatternAs(UIA_InvokePatternId, __uuidof(IUIAutomationInvokePattern), (void**)&invokePattern);
            
            if (SUCCEEDED(hr) && invokePattern) {
                qDebug() << "Trying invoke pattern";
                hr = invokePattern->Invoke();
                invokePattern->Release();
                if (SUCCEEDED(hr)) {
                    qDebug() << "Successfully invoked control:" << controlInfo.displayText;
                    return;
                } else {
                    qDebug() << "Invoke pattern failed with HRESULT:" << hr;
                }
            }
            
            // Method 3: Try legacy action (for older controls)
            IUIAutomationLegacyIAccessiblePattern* legacyPattern = nullptr;
            hr = element->GetCurrentPatternAs(UIA_LegacyIAccessiblePatternId, __uuidof(IUIAutomationLegacyIAccessiblePattern), (void**)&legacyPattern);
            
            if (SUCCEEDED(hr) && legacyPattern) {
                qDebug() << "Trying legacy accessible pattern";
                hr = legacyPattern->DoDefaultAction();
                legacyPattern->Release();
                if (SUCCEEDED(hr)) {
                    qDebug() << "Successfully clicked control via legacy pattern:" << controlInfo.displayText;
                    return;
                } else {
                    qDebug() << "Legacy pattern failed with HRESULT:" << hr;
                }
            }
            
            // Method 4: Try selection pattern for list items
            IUIAutomationSelectionItemPattern* selectionPattern = nullptr;
            hr = element->GetCurrentPatternAs(UIA_SelectionItemPatternId, __uuidof(IUIAutomationSelectionItemPattern), (void**)&selectionPattern);
            
            if (SUCCEEDED(hr) && selectionPattern) {
                qDebug() << "Trying selection item pattern";
                hr = selectionPattern->Select();
                selectionPattern->Release();
                if (SUCCEEDED(hr)) {
                    qDebug() << "Successfully selected control:" << controlInfo.displayText;
                    return;
                } else {
                    qDebug() << "Selection pattern failed with HRESULT:" << hr;
                }
            }
            
            qDebug() << "All click methods failed for control:" << controlInfo.displayText;
        }
    }
}

void UIAList::focusSelectedControl()
{
    QList<QListWidgetItem*> selectedItems = m_listWidget->selectedItems();
    if (selectedItems.isEmpty()) {
        qDebug() << "No control selected";
        return;
    }
    
    QListWidgetItem* selectedItem = selectedItems.first();
    int index = selectedItem->data(Qt::UserRole).toInt();
    
    if (index >= 0 && index < m_allControls.size()) {
        const ControlInfo& controlInfo = m_allControls[index];
        IUIAutomationElement* element = controlInfo.element;
        
        if (element) {
            qDebug() << "Attempting to focus control:" << controlInfo.displayText;
            
            // Method 1: Use UI Automation SetFocus
            HRESULT hr = element->SetFocus();
            if (SUCCEEDED(hr)) {
                qDebug() << "Successfully focused control via SetFocus:" << controlInfo.displayText;
                return;
            } else {
                qDebug() << "SetFocus failed with HRESULT:" << hr;
            }
            
            // Method 2: Try to get bounding rectangle and click to focus
            RECT rect;
            hr = element->get_CurrentBoundingRectangle(&rect);
            if (SUCCEEDED(hr)) {
                int x = rect.left + (rect.right - rect.left) / 2;
                int y = rect.top + (rect.bottom - rect.top) / 2;
                
                qDebug() << "Attempting focus via mouse click at coordinates:" << x << "," << y;
                
                // Click on the control to give it focus
                SetCursorPos(x, y);
                Sleep(50);
                mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
                mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
                
                qDebug() << "Successfully focused control via mouse click:" << controlInfo.displayText;
                return;
            }
            
            // Method 3: Try to use keyboard navigation to focus (Tab key simulation)
            qDebug() << "Trying keyboard Tab navigation to focus";
            
            // Get current focused element to compare
            IUIAutomationElement* currentFocus = nullptr;
            m_uiAutomation->GetFocusedElement(&currentFocus);
            
            // Send Tab key to try to navigate to the control
            // This is a simplified approach - in practice you'd need more sophisticated navigation
            keybd_event(VK_TAB, 0, 0, 0);
            keybd_event(VK_TAB, 0, KEYEVENTF_KEYUP, 0);
            
            Sleep(100); // Give time for focus to change
            
            IUIAutomationElement* newFocus = nullptr;
            m_uiAutomation->GetFocusedElement(&newFocus);
            
            if (currentFocus) currentFocus->Release();
            if (newFocus) {
                qDebug() << "Tab navigation attempted";
                newFocus->Release();
            }
            
            qDebug() << "All focus methods failed for control:" << controlInfo.displayText;
        }
    }
}

void UIAList::doubleClickSelectedControl()
{
    QList<QListWidgetItem*> selectedItems = m_listWidget->selectedItems();
    if (selectedItems.isEmpty()) {
        qDebug() << "No control selected";
        return;
    }
    
    QListWidgetItem* selectedItem = selectedItems.first();
    int index = selectedItem->data(Qt::UserRole).toInt();
    
    if (index >= 0 && index < m_allControls.size()) {
        const ControlInfo& controlInfo = m_allControls[index];
        IUIAutomationElement* element = controlInfo.element;
        
        if (element) {
            qDebug() << "Attempting to double-click control:" << controlInfo.displayText;
            
            // Method 1: Try to get bounding rectangle and simulate mouse double-click
            RECT rect;
            HRESULT hr = element->get_CurrentBoundingRectangle(&rect);
            if (SUCCEEDED(hr)) {
                int x = rect.left + (rect.right - rect.left) / 2;
                int y = rect.top + (rect.bottom - rect.top) / 2;
                
                qDebug() << "Attempting mouse double-click at coordinates:" << x << "," << y;
                
                // Use Windows API to simulate mouse double-click
                SetCursorPos(x, y);
                Sleep(50);
                mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
                mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
                Sleep(50); // Brief pause between clicks
                mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
                mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
                
                qDebug() << "Successfully double-clicked control via mouse simulation:" << controlInfo.displayText;
                return;
            }
            
            // Method 2: For double click, we can use the Toggle pattern for checkboxes/radio buttons
            IUIAutomationTogglePattern* togglePattern = nullptr;
            hr = element->GetCurrentPatternAs(UIA_TogglePatternId, __uuidof(IUIAutomationTogglePattern), (void**)&togglePattern);
            
            if (SUCCEEDED(hr) && togglePattern) {
                qDebug() << "Trying toggle pattern";
                hr = togglePattern->Toggle();
                togglePattern->Release();
                if (SUCCEEDED(hr)) {
                    qDebug() << "Successfully toggled control:" << controlInfo.displayText;
                    return;
                } else {
                    qDebug() << "Toggle pattern failed with HRESULT:" << hr;
                }
            }
            
            // Method 3: Try double click via legacy pattern
            IUIAutomationLegacyIAccessiblePattern* legacyPattern = nullptr;
            hr = element->GetCurrentPatternAs(UIA_LegacyIAccessiblePatternId, __uuidof(IUIAutomationLegacyIAccessiblePattern), (void**)&legacyPattern);
            
            if (SUCCEEDED(hr) && legacyPattern) {
                qDebug() << "Trying legacy accessible double-click";
                // Simulate double click by calling DoDefaultAction twice
                hr = legacyPattern->DoDefaultAction();
                if (SUCCEEDED(hr)) {
                    Sleep(50);
                    legacyPattern->DoDefaultAction();
                }
                legacyPattern->Release();
                if (SUCCEEDED(hr)) {
                    qDebug() << "Successfully double-clicked control via legacy pattern:" << controlInfo.displayText;
                    return;
                } else {
                    qDebug() << "Legacy double-click failed with HRESULT:" << hr;
                }
            }
            
            // Method 4: Fallback - try regular invoke pattern twice
            IUIAutomationInvokePattern* invokePattern = nullptr;
            hr = element->GetCurrentPatternAs(UIA_InvokePatternId, __uuidof(IUIAutomationInvokePattern), (void**)&invokePattern);
            
            if (SUCCEEDED(hr) && invokePattern) {
                qDebug() << "Trying double invoke pattern";
                hr = invokePattern->Invoke();
                if (SUCCEEDED(hr)) {
                    Sleep(50);
                    invokePattern->Invoke(); // Second invoke for double click
                }
                invokePattern->Release();
                if (SUCCEEDED(hr)) {
                    qDebug() << "Successfully double-invoked control:" << controlInfo.displayText;
                    return;
                } else {
                    qDebug() << "Double invoke failed with HRESULT:" << hr;
                }
            }
            
            qDebug() << "All double-click methods failed for control:" << controlInfo.displayText;
        }
    }
}

void UIAList::ensureItemSelected()
{
    if (!m_listWidget || m_listWidget->count() == 0) {
        return;
    }
    
    // If no item is selected, select the first visible item
    if (m_listWidget->currentRow() == -1) {
        for (int i = 0; i < m_listWidget->count(); ++i) {
            QListWidgetItem* item = m_listWidget->item(i);
            if (item && !item->isHidden()) {
                m_listWidget->setCurrentRow(i);
                QListWidgetItem *selectedItem = m_listWidget->item(i);
                if (selectedItem) {
                    announceSelectedItem(selectedItem->text());
                }
                break;
            }
        }
    }
}

void UIAList::updateButtonStates()
{
    // Check if there are any visible items in the list
    bool hasVisibleItems = false;
    
    if (m_listWidget) {
        for (int i = 0; i < m_listWidget->count(); ++i) {
            QListWidgetItem* item = m_listWidget->item(i);
            if (item && !item->isHidden()) {
                hasVisibleItems = true;
                break;
            }
        }
    }
    
    // Enable/disable buttons based on whether there are visible items
    if (m_clickButton) m_clickButton->setEnabled(hasVisibleItems);
    if (m_focusButton) m_focusButton->setEnabled(hasVisibleItems);
    if (m_doubleClickButton) m_doubleClickButton->setEnabled(hasVisibleItems);
}

void UIAList::executeDefaultAction()
{
    int defaultAction = m_settings->value("defaultAction", 0).toInt(); // 0 = Click by default
    
    switch (defaultAction) {
        case 0: // Click
            clickSelectedControl();
            break;
        case 1: // Double Click
            doubleClickSelectedControl();
            break;
        case 2: // Focus
            focusSelectedControl();
            break;
        default:
            clickSelectedControl(); // Fallback to click
            break;
    }
}
