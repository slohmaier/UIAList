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
#include <QApplication>
#include <QIcon>
#include <QDebug>

#include <windows.h>

UIAListIcon::UIAListIcon(QObject *parent)
    : QObject(parent), m_trayIcon(nullptr), m_contextMenu(nullptr), m_activateAction(nullptr), m_aboutAction(nullptr), m_quitAction(nullptr)
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
    qDebug() << "Tray icon created successfully";
    
    createContextMenu();
    m_trayIcon->setContextMenu(m_contextMenu);
    qDebug() << "Context menu set";
    
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
    
    m_activateAction = new QAction("Activate", this);
    m_activateAction->setShortcut(QKeySequence("Ctrl+Alt+U"));
    connect(m_activateAction, &QAction::triggered, this, &UIAListIcon::activate);
    
    m_aboutAction = new QAction("About...", this);
    connect(m_aboutAction, &QAction::triggered, this, &UIAListIcon::showAbout);
    
    m_quitAction = new QAction("Quit", this);
    connect(m_quitAction, &QAction::triggered, this, &UIAListIcon::quit);
    
    m_contextMenu->addAction(m_activateAction);
    m_contextMenu->addSeparator();
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

void UIAListIcon::showAbout()
{
    AboutDialog dialog;
    dialog.exec();
}

void UIAListIcon::quit()
{
    QApplication::quit();
}

void UIAListIcon::registerGlobalShortcut()
{
    // Install native event filter to handle hotkey messages
    qApp->installNativeEventFilter(this);
    
    // Register Ctrl+Alt+U hotkey (MOD_CONTROL | MOD_ALT, VK_U)
    if (RegisterHotKey(nullptr, HOTKEY_ID, MOD_CONTROL | MOD_ALT, 'U')) {
        qDebug() << "Global shortcut Ctrl+Alt+U registered successfully";
    } else {
        qDebug() << "Failed to register global shortcut Ctrl+Alt+U";
    }
}

void UIAListIcon::unregisterGlobalShortcut()
{
    UnregisterHotKey(nullptr, HOTKEY_ID);
    qApp->removeNativeEventFilter(this);
    qDebug() << "Global shortcut unregistered";
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