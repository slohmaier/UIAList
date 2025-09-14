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

#ifndef UIALIST_H
#define UIALIST_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QMap>
#include <QString>
#include <QKeyEvent>
#include <QFocusEvent>
#include <QSettings>
#include <QThread>
#include <QMutex>
#include <QMovie>
#include <QStackedWidget>
#include <QProgressBar>
#include <QTimer>

#include <windows.h>
#include <uiautomation.h>
#include <comdef.h>

class UIAListIcon;

// Worker thread for enumerating controls
class ControlEnumerationWorker : public QObject
{
    Q_OBJECT

public:
    ControlEnumerationWorker(IUIAutomation* uiAutomation, IUIAutomationTreeWalker* walker, void* windowHandle);

public slots:
    void enumerateControls();
    void cancelEnumeration();

signals:
    void controlFound(const QString& displayText, const QString& originalName, void* element, int controlType);
    void enumerationFinished(const QString& windowTitle);
    void enumerationCancelled();

private:
    void walkControls(IUIAutomationElement* element, IUIAutomationTreeWalker* walker);
    QString getControlTypeString(int controlType);

    IUIAutomation* m_uiAutomation;
    IUIAutomationTreeWalker* m_walker;
    void* m_windowHandle;
    bool m_cancelled;
    QMutex m_cancelMutex;
};

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
    
    void showWelcomeScreen();

private slots:
    void showWindow(void* foregroundWindow);
    void onFilterChanged(const QString& text);
    void onItemSelectionChanged();
    void onHideEmptyTitlesChanged(bool checked);
    void onHideMenusChanged(bool checked);
    void onClickButtonClicked();
    void onFocusButtonClicked();
    void onDoubleClickButtonClicked();
    void onControlFound(const QString& displayText, const QString& originalName, void* element, int controlType);
    void onEnumerationFinished(const QString& windowTitle);
    void onEnumerationCancelled();
    void onCancelButtonClicked();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    bool event(QEvent *event) override;

private:
    void setupUI();
    void setupLoadingOverlay();
    void initializeUIAutomation();
    void checkAndShowWelcome();
    void startEnumeration(void* windowHandle);
    void showLoadingOverlay();
    void hideLoadingOverlay();
    void walkControls(IUIAutomationElement* element, IUIAutomationTreeWalker* walker);
    QString getControlTypeString(CONTROLTYPEID controlType);
    void populateListWidget();
    void cleanupUIAutomation();
    void selectVisibleListItem(int direction);
    void announceSelectedItem(const QString& text);
    void announceText(const QString& text);
    void clickSelectedControl();
    void focusSelectedControl();
    void doubleClickSelectedControl();
    void ensureItemSelected();
    void updateButtonStates();
    void executeDefaultAction();

    UIAListIcon *m_trayIcon;
    
    // UI Components
    QWidget *m_centralWidget;
    QStackedWidget *m_stackedWidget;
    QWidget *m_mainWidget;
    QWidget *m_loadingWidget;
    QVBoxLayout *m_layout;
    QHBoxLayout *m_buttonLayout;
    QLabel *m_windowTitleLabel;
    QLineEdit *m_filterEdit;
    QListWidget *m_listWidget;
    QCheckBox *m_hideEmptyTitlesCheckBox;
    QCheckBox *m_hideMenusCheckBox;
    QPushButton *m_clickButton;
    QPushButton *m_focusButton;
    QPushButton *m_doubleClickButton;

    // Loading overlay components
    QVBoxLayout *m_loadingLayout;
    QLabel *m_loadingLabel;
    QProgressBar *m_progressBar;
    QPushButton *m_cancelButton;
    
    // UI Automation
    IUIAutomation *m_uiAutomation;
    IUIAutomationTreeWalker *m_controlViewWalker;
    
    // Threading components
    QThread *m_workerThread;
    ControlEnumerationWorker *m_worker;

    // Data storage
    QMap<QString, ControlInfo> m_controlMap;
    QList<ControlInfo> m_allControls;
    ControlInfo m_selectedControl;
    QString m_targetWindowTitle;
    QSettings *m_settings;
};
#endif // UIALIST_H
