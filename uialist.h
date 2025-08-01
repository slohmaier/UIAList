#ifndef UIALIST_H
#define UIALIST_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QCheckBox>
#include <QMap>
#include <QString>
#include <QKeyEvent>

#ifdef _WIN32
#include <windows.h>
#include <uiautomation.h>
#include <comdef.h>
#endif

class UIAListIcon;

struct ControlInfo {
    QString displayText;
    QString originalName; // Store the original control name for filtering
    IUIAutomationElement* element;
    
    ControlInfo() : element(nullptr) {}
    ControlInfo(const QString& text, const QString& name, IUIAutomationElement* elem) 
        : displayText(text), originalName(name), element(elem) 
    {
        if (element) element->AddRef();
    }
    
    ~ControlInfo() {
        if (element) element->Release();
    }
    
    ControlInfo(const ControlInfo& other) : displayText(other.displayText), originalName(other.originalName), element(other.element) {
        if (element) element->AddRef();
    }
    
    ControlInfo& operator=(const ControlInfo& other) {
        if (this != &other) {
            if (element) element->Release();
            displayText = other.displayText;
            originalName = other.originalName;
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
    void onHideEmptyTitlesChanged(bool checked);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void setupUI();
    void initializeUIAutomation();
    void enumerateControls(void* windowHandle);
    void walkControls(IUIAutomationElement* element, IUIAutomationTreeWalker* walker);
    QString getControlTypeString(CONTROLTYPEID controlType);
    void populateListWidget();
    void cleanupUIAutomation();
    void selectVisibleListItem(int direction);
    void announceSelectedItem(const QString& text);

    UIAListIcon *m_trayIcon;
    
    // UI Components
    QWidget *m_centralWidget;
    QVBoxLayout *m_layout;
    QLineEdit *m_filterEdit;
    QListWidget *m_listWidget;
    QCheckBox *m_hideEmptyTitlesCheckBox;
    
    // UI Automation
    IUIAutomation *m_uiAutomation;
    IUIAutomationTreeWalker *m_controlViewWalker;
    
    // Data storage
    QMap<QString, ControlInfo> m_controlMap;
    QList<ControlInfo> m_allControls;
    ControlInfo m_selectedControl;
};
#endif // UIALIST_H
