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

#include "uialisticon.h"
#include "aboutdialog.h"
#include "settingsdialog.h"
#include <QApplication>
#include <QIcon>
#include <QDebug>

#include <windows.h>

UIAListIcon::UIAListIcon(QObject *parent)
    : QObject(parent), m_trayIcon(nullptr), m_contextMenu(nullptr), m_activateAction(nullptr), m_settingsAction(nullptr), m_aboutAction(nullptr), m_quitAction(nullptr), m_currentShortcut(QKeySequence("Ctrl+Alt+U"))
{
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        qDebug() << "System tray is not available!";
        return;
    }
    
    qDebug() << "System tray is available, creating icon...";
    
    QIcon icon(":/icons/uialist_icon.png");
    qDebug() << "Icon null?" << icon.isNull();
    qDebug() << "Icon available sizes:" << icon.availableSizes();
    
    m_trayIcon = new QSystemTrayIcon(icon, this);
    m_trayIcon->setToolTip(tr("UIAList"));
    qDebug() << "Tray icon created successfully";
    
    createContextMenu();
    m_trayIcon->setContextMenu(m_contextMenu);
    qDebug() << "Context menu set";
    
    // Load shortcut from settings
    QSettings settings("UIAList", "Settings");
    QString shortcutString = settings.value("shortcutKey", "Ctrl+Alt+U").toString();
    m_currentShortcut = QKeySequence::fromString(shortcutString);
    
    registerGlobalShortcut();
}

UIAListIcon::~UIAListIcon()
{
    unregisterGlobalShortcut();
    if (m_trayIcon) {
        m_trayIcon->hide();
    }
}

void UIAListIcon::show()
{
    if (m_trayIcon) {
        qDebug() << "Showing tray icon...";
        m_trayIcon->show();
        qDebug() << "Tray icon visible?" << m_trayIcon->isVisible();
    } else {
        qDebug() << "Cannot show tray icon - m_trayIcon is null";
    }
}

bool UIAListIcon::isVisible() const
{
    return m_trayIcon && m_trayIcon->isVisible();
}

void UIAListIcon::createContextMenu()
{
    m_contextMenu = new QMenu();
    
    m_activateAction = new QAction(tr("Activate"), this);
    m_activateAction->setShortcut(m_currentShortcut);
    connect(m_activateAction, &QAction::triggered, this, &UIAListIcon::activate);

    m_settingsAction = new QAction(tr("Settings..."), this);
    connect(m_settingsAction, &QAction::triggered, this, &UIAListIcon::showSettings);
    
    m_aboutAction = new QAction(tr("About..."), this);
    connect(m_aboutAction, &QAction::triggered, this, &UIAListIcon::showAbout);
    
    m_quitAction = new QAction(tr("Quit"), this);
    connect(m_quitAction, &QAction::triggered, this, &UIAListIcon::quit);
    
    m_contextMenu->addAction(m_activateAction);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_settingsAction);
    m_contextMenu->addAction(m_aboutAction);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_quitAction);
}

void UIAListIcon::activate()
{
    qDebug() << "Activate action triggered";
    
    // Capture the foreground window before showing our window
    HWND foregroundWindow = GetForegroundWindow();
    qDebug() << "Captured foreground window:" << foregroundWindow;
    emit activateRequested((void*)foregroundWindow);
}

void UIAListIcon::showSettings()
{
    // Temporarily unregister hotkey to prevent activation during settings change
    unregisterGlobalShortcut();
    
    SettingsDialog dialog;
    QKeySequence oldShortcut = m_currentShortcut;
    
    if (dialog.exec() == QDialog::Accepted) {
        QKeySequence newShortcut = dialog.shortcutKey();
        if (newShortcut != oldShortcut) {
            updateShortcut(newShortcut);
            qDebug() << "Shortcut updated from" << oldShortcut.toString() << "to" << newShortcut.toString();
        } else {
            // Re-register the same shortcut
            registerGlobalShortcut(m_currentShortcut);
        }
    } else {
        // Dialog was cancelled, re-register the old shortcut
        registerGlobalShortcut(m_currentShortcut);
    }
}

void UIAListIcon::showAbout()
{
    // Pass the main window as parent to the AboutDialog
    QWidget *mainWindow = qobject_cast<QWidget*>(parent());
    AboutDialog dialog(mainWindow);
    dialog.exec();
}

void UIAListIcon::quit()
{
    QApplication::quit();
}

void UIAListIcon::registerGlobalShortcut()
{
    registerGlobalShortcut(m_currentShortcut);
}

void UIAListIcon::registerGlobalShortcut(const QKeySequence &keySequence)
{
    // Unregister old hotkey first
    unregisterGlobalShortcut();
    
    // Install native event filter to handle hotkey messages
    qApp->installNativeEventFilter(this);
    
    // Convert QKeySequence to Windows virtual key and modifiers
    if (keySequence.isEmpty()) {
        qDebug() << "Empty key sequence, not registering hotkey";
        return;
    }
    
    int key = keySequence[0];
    int modifiers = key & 0xFFFF0000;
    int vk = key & 0x0000FFFF;
    
    UINT winModifiers = 0;
    if (modifiers & Qt::ControlModifier) winModifiers |= MOD_CONTROL;
    if (modifiers & Qt::AltModifier) winModifiers |= MOD_ALT;
    if (modifiers & Qt::ShiftModifier) winModifiers |= MOD_SHIFT;
    if (modifiers & Qt::MetaModifier) winModifiers |= MOD_WIN;
    
    // Convert Qt key to Windows virtual key
    UINT winVk = vk;
    if (vk >= Qt::Key_A && vk <= Qt::Key_Z) {
        winVk = vk - Qt::Key_A + 'A';
    } else if (vk >= Qt::Key_0 && vk <= Qt::Key_9) {
        winVk = vk - Qt::Key_0 + '0';
    } else if (vk >= Qt::Key_F1 && vk <= Qt::Key_F12) {
        winVk = vk - Qt::Key_F1 + VK_F1;
    }
    // Add more key mappings as needed
    
    if (RegisterHotKey(nullptr, HOTKEY_ID, winModifiers, winVk)) {
        qDebug() << "Global shortcut" << keySequence.toString() << "registered successfully";
        m_currentShortcut = keySequence;
    } else {
        qDebug() << "Failed to register global shortcut" << keySequence.toString();
    }
}

void UIAListIcon::unregisterGlobalShortcut()
{
    UnregisterHotKey(nullptr, HOTKEY_ID);
    qApp->removeNativeEventFilter(this);
    qDebug() << "Global shortcut unregistered";
}

void UIAListIcon::updateShortcut(const QKeySequence &newShortcut)
{
    registerGlobalShortcut(newShortcut);
    m_activateAction->setShortcut(newShortcut);
}

bool UIAListIcon::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result)
{
    Q_UNUSED(result)
    
    if (eventType == "windows_generic_MSG") {
        MSG *msg = static_cast<MSG*>(message);
        if (msg->message == WM_HOTKEY && msg->wParam == HOTKEY_ID) {
            qDebug() << "Global hotkey Ctrl+Alt+U activated";
            activate();
            return true;
        }
    }
    
    return false;
}