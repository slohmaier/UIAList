#ifndef UIALIST_H
#define UIALIST_H

#include <QMainWindow>

class UIAListIcon;

class UIAList : public QMainWindow
{
    Q_OBJECT

public:
    UIAList(QWidget *parent = nullptr);
    ~UIAList();

private slots:
    void showWindow();

private:
    UIAListIcon *m_trayIcon;
};
#endif // UIALIST_H
