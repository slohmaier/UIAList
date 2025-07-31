#include "uialist.h"
#include "uialisticon.h"
#include <QDebug>
#include <QListWidgetItem>
#include <QVariant>

#ifdef _WIN32
#include <comdef.h>
#include <atlbase.h>
#endif

UIAList::UIAList(QWidget *parent)
    : QMainWindow(parent), m_trayIcon(nullptr), m_centralWidget(nullptr), 
      m_layout(nullptr), m_filterEdit(nullptr), m_listWidget(nullptr),
      m_hideEmptyTitlesCheckBox(nullptr), m_uiAutomation(nullptr), m_controlViewWalker(nullptr)
{
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
    
    // Filter edit box
    m_filterEdit = new QLineEdit(this);
    m_filterEdit->setPlaceholderText("Filter controls...");
    connect(m_filterEdit, &QLineEdit::textChanged, this, &UIAList::onFilterChanged);
    
    // List widget
    m_listWidget = new QListWidget(this);
    connect(m_listWidget, &QListWidget::itemSelectionChanged, this, &UIAList::onItemSelectionChanged);
    
    // Hide empty titles checkbox
    m_hideEmptyTitlesCheckBox = new QCheckBox("Hide controls with no or empty title", this);
    m_hideEmptyTitlesCheckBox->setChecked(true); // Enabled by default
    connect(m_hideEmptyTitlesCheckBox, &QCheckBox::toggled, this, &UIAList::onHideEmptyTitlesChanged);
    
    m_layout->addWidget(m_filterEdit);
    m_layout->addWidget(m_listWidget);
    m_layout->addWidget(m_hideEmptyTitlesCheckBox);
    
    setWindowTitle("UI Automation List");
    resize(600, 400);
}

void UIAList::initializeUIAutomation()
{
#ifdef _WIN32
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
#endif
}

void UIAList::showWindow(void* foregroundWindow)
{
    show();
    raise();
    activateWindow();
    
    // Enumerate controls of the passed foreground window
    enumerateControls(foregroundWindow);
}

void UIAList::enumerateControls(void* windowHandle)
{
#ifdef _WIN32
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
    
    // Get window title for debugging
    wchar_t windowTitle[256];
    GetWindowTextW(targetWindow, windowTitle, 256);
    QString title = QString::fromWCharArray(windowTitle);
    qDebug() << "Enumerating controls for window:" << title << "HWND:" << targetWindow;
    
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
#endif
}

void UIAList::walkControls(IUIAutomationElement* element, IUIAutomationTreeWalker* walker)
{
#ifdef _WIN32
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
    
    // Store control info with original name
    ControlInfo controlInfo(displayText, controlName, element);
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
#endif
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
    
    for (int i = 0; i < m_allControls.size(); ++i) {
        const ControlInfo& controlInfo = m_allControls[i];
        
        // Skip controls with empty or no titles if checkbox is checked
        if (hideEmptyTitles) {
            QString name = controlInfo.originalName.trimmed();
            if (name.isEmpty() || name == "(no name)") {
                continue; // Skip this control
            }
        }
        
        QListWidgetItem* item = new QListWidgetItem(controlInfo.displayText);
        item->setData(Qt::UserRole, i); // Store index to m_allControls
        m_listWidget->addItem(item);
    }
}

void UIAList::onFilterChanged(const QString& text)
{
    for (int i = 0; i < m_listWidget->count(); ++i) {
        QListWidgetItem* item = m_listWidget->item(i);
        bool visible = text.isEmpty() || item->text().contains(text, Qt::CaseInsensitive);
        item->setHidden(!visible);
    }
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

void UIAList::cleanupUIAutomation()
{
#ifdef _WIN32
    if (m_controlViewWalker) {
        m_controlViewWalker->Release();
        m_controlViewWalker = nullptr;
    }
    
    if (m_uiAutomation) {
        m_uiAutomation->Release();
        m_uiAutomation = nullptr;
    }
    
    CoUninitialize();
#endif
}
