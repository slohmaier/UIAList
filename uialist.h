/*
 * UIAList - UI Automation Control Browser
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

#ifndef UIALIST_H
#define UIALIST_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QCheckBox>
#include <QPushButton>
#include <QMap>
#include <QString>
#include <QKeyEvent>
#include <QFocusEvent>

#include <windows.h>
#include <uiautomation.h>
#include <comdef.h>

class UIAListIcon;

struct ControlInfo {
    QString displayText;
    QString originalName; // Store the original control name for filtering
    IUIAutomationElement* element;
    CONTROLTYPEID controlType; // Store the control type for filtering
    
    ControlInfo() : element(nullptr), controlType(0) {}
    ControlInfo(const QString& text, const QString& name, IUIAutomationElement* elem, CONTROLTYPEID type = 0) 
        : displayText(text), originalName(name), element(elem), controlType(type) 
    {
        if (element) element->AddRef();
    }
    
    ~ControlInfo() {
        if (element) element->Release();
    }
    
    ControlInfo(const ControlInfo& other) : displayText(other.displayText), originalName(other.originalName), element(other.element), controlType(other.controlType) {
        if (element) element->AddRef();
    }
    
    ControlInfo& operator=(const ControlInfo& other) {
        if (this != &other) {
            if (element) element->Release();
            displayText = other.displayText;
            originalName = other.originalName;
            element = other.element;
            controlType = other.controlType;
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
    void onHideMenusChanged(bool checked);
    void onClickButtonClicked();
    void onFocusButtonClicked();
    void onDoubleClickButtonClicked();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

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
    void clickSelectedControl();
    void focusSelectedControl();
    void doubleClickSelectedControl();
    void ensureItemSelected();
    void updateButtonStates();

    UIAListIcon *m_trayIcon;
    
    // UI Components
    QWidget *m_centralWidget;
    QVBoxLayout *m_layout;
    QHBoxLayout *m_buttonLayout;
    QLineEdit *m_filterEdit;
    QListWidget *m_listWidget;
    QCheckBox *m_hideEmptyTitlesCheckBox;
    QCheckBox *m_hideMenusCheckBox;
    QPushButton *m_clickButton;
    QPushButton *m_focusButton;
    QPushButton *m_doubleClickButton;
    
    // UI Automation
    IUIAutomation *m_uiAutomation;
    IUIAutomationTreeWalker *m_controlViewWalker;
    
    // Data storage
    QMap<QString, ControlInfo> m_controlMap;
    QList<ControlInfo> m_allControls;
    ControlInfo m_selectedControl;
};
#endif // UIALIST_H
