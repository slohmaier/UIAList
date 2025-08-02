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

#ifndef UIALISTICON_H
#define UIALISTICON_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>

#include <QAbstractNativeEventFilter>

class UIAListIcon : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    UIAListIcon(QObject *parent = nullptr);
    ~UIAListIcon();
    
    void show();
    bool isVisible() const;

signals:
    void activateRequested(void* foregroundWindow);

private slots:
    void activate();
    void showAbout();
    void quit();

protected:
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;

private:
    void createContextMenu();
    void registerGlobalShortcut();
    void unregisterGlobalShortcut();
    
    QSystemTrayIcon *m_trayIcon;
    QMenu *m_contextMenu;
    QAction *m_activateAction;
    QAction *m_aboutAction;
    QAction *m_quitAction;
    static const int HOTKEY_ID = 1;
};

#endif // UIALISTICON_H