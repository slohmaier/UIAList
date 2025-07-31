#ifndef UIALISTICON_H
#define UIALISTICON_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>

#ifdef _WIN32
#include <QAbstractNativeEventFilter>
#endif

#ifdef _WIN32
class UIAListIcon : public QObject, public QAbstractNativeEventFilter
#else
class UIAListIcon : public QObject
#endif
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
    void quit();

protected:
#ifdef _WIN32
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;
#endif

private:
    void createContextMenu();
    void registerGlobalShortcut();
    void unregisterGlobalShortcut();
    
    QSystemTrayIcon *m_trayIcon;
    QMenu *m_contextMenu;
    QAction *m_activateAction;
    QAction *m_quitAction;
#ifdef _WIN32
    static const int HOTKEY_ID = 1;
#else
    class QShortcut *m_globalShortcut;
#endif
};

#endif // UIALISTICON_H