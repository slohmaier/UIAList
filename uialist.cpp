#include "uialist.h"
#include "uialisticon.h"

UIAList::UIAList(QWidget *parent)
    : QMainWindow(parent), m_trayIcon(nullptr)
{
    m_trayIcon = new UIAListIcon(this);
    connect(m_trayIcon, &UIAListIcon::activateRequested, this, &UIAList::showWindow);
    m_trayIcon->show();
    
    // Hide the main window by default, only show tray icon
    hide();
}

UIAList::~UIAList() {}

void UIAList::showWindow()
{
    show();
    raise();
    activateWindow();
}
