#ifndef UIALIST_H
#define UIALIST_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QMap>
#include <QString>

#ifdef _WIN32
#include <windows.h>
#include <uiautomation.h>
#include <comdef.h>
#endif

class UIAListIcon;

struct ControlInfo {
    QString displayText;
    IUIAutomationElement* element;
    
    ControlInfo() : element(nullptr) {}
    ControlInfo(const QString& text, IUIAutomationElement* elem) 
        : displayText(text), element(elem) 
    {
        if (element) element->AddRef();
    }
    
    ~ControlInfo() {
        if (element) element->Release();
    }
    
    ControlInfo(const ControlInfo& other) : displayText(other.displayText), element(other.element) {
        if (element) element->AddRef();
    }
    
    ControlInfo& operator=(const ControlInfo& other) {
        if (this != &other) {
            if (element) element->Release();
            displayText = other.displayText;
            element = other.element;
            if (element) element->AddRef();
        }
        return *this;
    }
};

class UIAList : public QMainWindow
{
    Q_OBJECT

public:
    UIAList(QWidget *parent = nullptr);
    ~UIAList();

private slots:
    void showWindow(void* foregroundWindow);
    void onFilterChanged(const QString& text);
    void onItemSelectionChanged();

private:
    void setupUI();
    void initializeUIAutomation();
    void enumerateControls(void* windowHandle);
    void walkControls(IUIAutomationElement* element, IUIAutomationTreeWalker* walker);
    QString getControlTypeString(CONTROLTYPEID controlType);
    void populateListWidget();
    void cleanupUIAutomation();

    UIAListIcon *m_trayIcon;
    
    // UI Components
    QWidget *m_centralWidget;
    QVBoxLayout *m_layout;
    QLineEdit *m_filterEdit;
    QListWidget *m_listWidget;
    
    // UI Automation
    IUIAutomation *m_uiAutomation;
    IUIAutomationTreeWalker *m_controlViewWalker;
    
    // Data storage
    QMap<QString, ControlInfo> m_controlMap;
    QList<ControlInfo> m_allControls;
    ControlInfo m_selectedControl;
};
#endif // UIALIST_H
