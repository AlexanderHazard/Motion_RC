#ifndef HOTKEYS_H
#define HOTKEYS_H
#include <QRunnable>
#include <QObject>
#include <qt_windows.h>
#include <QMutex>
#include <QThread>

class HotKeys : public QObject, public QRunnable
{
    Q_OBJECT
public:
    HotKeys();
    ~HotKeys();

    void stopKeyHeaper();

private:
    QMutex nmtx;
    virtual void run();
    bool quit = false;

signals:
    void hotKeyPressed();
    void demoPressed();
};

#endif // HOTKEYS_H
