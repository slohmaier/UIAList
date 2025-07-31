#ifndef UIALISTICON_H
#define UIALISTICON_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>

class UIAListIcon : public QObject
{
    Q_OBJECT

public:
    UIAListIcon(QObject *parent = nullptr);
    ~UIAListIcon();
    
    void show();
    bool isVisible() const;

signals:
    void activateRequested();

private slots:
    void activate();
    void quit();

private:
    void createContextMenu();
    
    QSystemTrayIcon *m_trayIcon;
    QMenu *m_contextMenu;
    QAction *m_activateAction;
    QAction *m_quitAction;
    class QShortcut *m_globalShortcut;
};

#endif // UIALISTICON_H