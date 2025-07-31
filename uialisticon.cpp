#include "uialisticon.h"
#include <QApplication>
#include <QIcon>
#include <QDebug>

#ifdef _WIN32
#include <windows.h>
#else
#include <QShortcut>
#endif

#ifdef _WIN32
UIAListIcon::UIAListIcon(QObject *parent)
    : QObject(parent), m_trayIcon(nullptr), m_contextMenu(nullptr), m_activateAction(nullptr), m_quitAction(nullptr)
#else
UIAListIcon::UIAListIcon(QObject *parent)
    : QObject(parent), m_trayIcon(nullptr), m_contextMenu(nullptr), m_activateAction(nullptr), m_quitAction(nullptr), m_globalShortcut(nullptr)
#endif
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
    
    m_quitAction = new QAction("Quit", this);
    connect(m_quitAction, &QAction::triggered, this, &UIAListIcon::quit);
    
    m_contextMenu->addAction(m_activateAction);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_quitAction);
}

void UIAListIcon::activate()
{
    qDebug() << "Activate action triggered";
    
#ifdef _WIN32
    // Capture the foreground window before showing our window
    HWND foregroundWindow = GetForegroundWindow();
    qDebug() << "Captured foreground window:" << foregroundWindow;
    emit activateRequested((void*)foregroundWindow);
#else
    emit activateRequested(nullptr);
#endif
}

void UIAListIcon::quit()
{
    QApplication::quit();
}

void UIAListIcon::registerGlobalShortcut()
{
#ifdef _WIN32
    // Install native event filter to handle hotkey messages
    qApp->installNativeEventFilter(this);
    
    // Register Ctrl+Alt+U hotkey (MOD_CONTROL | MOD_ALT, VK_U)
    if (RegisterHotKey(nullptr, HOTKEY_ID, MOD_CONTROL | MOD_ALT, 'U')) {
        qDebug() << "Global shortcut Ctrl+Alt+U registered successfully";
    } else {
        qDebug() << "Failed to register global shortcut Ctrl+Alt+U";
    }
#else
    // Fallback to QShortcut on non-Windows platforms
    m_globalShortcut = new QShortcut(QKeySequence("Ctrl+Alt+U"), qApp);
    connect(m_globalShortcut, &QShortcut::activated, this, &UIAListIcon::activate);
    qDebug() << "QShortcut Ctrl+Alt+U registered (application focus required)";
#endif
}

void UIAListIcon::unregisterGlobalShortcut()
{
#ifdef _WIN32
    UnregisterHotKey(nullptr, HOTKEY_ID);
    qApp->removeNativeEventFilter(this);
    qDebug() << "Global shortcut unregistered";
#else
    if (m_globalShortcut) {
        delete m_globalShortcut;
        m_globalShortcut = nullptr;
    }
#endif
}

#ifdef _WIN32
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
#endif