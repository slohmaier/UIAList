#include "uialisticon.h"
#include <QApplication>
#include <QIcon>
#include <QDebug>
#include <QShortcut>

UIAListIcon::UIAListIcon(QObject *parent)
    : QObject(parent), m_trayIcon(nullptr), m_contextMenu(nullptr), m_activateAction(nullptr), m_quitAction(nullptr), m_globalShortcut(nullptr)
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
    
    // Create global shortcut - Note: QShortcut only works when app has focus
    // For true global shortcuts, you'd need platform-specific code or a library
    m_globalShortcut = new QShortcut(QKeySequence("Ctrl+Alt+U"), qApp);
    connect(m_globalShortcut, &QShortcut::activated, this, &UIAListIcon::activate);
    qDebug() << "Global shortcut Ctrl+Alt+U registered";
}

UIAListIcon::~UIAListIcon()
{
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
    
    m_quitAction = new QAction("Quit", this);
    connect(m_quitAction, &QAction::triggered, this, &UIAListIcon::quit);
    
    m_contextMenu->addAction(m_activateAction);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_quitAction);
}

void UIAListIcon::activate()
{
    qDebug() << "Activate action triggered";
    emit activateRequested();
}

void UIAListIcon::quit()
{
    QApplication::quit();
}