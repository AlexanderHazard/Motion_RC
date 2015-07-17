#ifndef HOTKEYS_H
#define HOTKEYS_H
#include <QObject>
#include <qt_windows.h>
#include <QMutex>
#include <QThread>

class HotKeys : public QObject
{
    Q_OBJECT
public:
    HotKeys();
    ~HotKeys();

    void stopKeyHeaper();

private:
    QMutex nmtx;
public slots:
    void key_action_run();

public:
    bool quit = false;

signals:
    void F4KeyPressed();
    void F5KeyPressed();
    void F6KeyPressed();
    void F7KeyPressed();
    //void demoPressed();
};

#endif // HOTKEYS_H
