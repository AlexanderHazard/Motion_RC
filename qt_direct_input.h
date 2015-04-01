#ifndef QT_DIRECT_INPUT_H
#define QT_DIRECT_INPUT_H

#include <dinput.h>
#include <stdint.h>

#include <QObject>
#include <QTimer>
#include <QMutex>

#include "qt_direct_input_global.h"

class QT_DIRECT_INPUTSHARED_EXPORT QT_Direct_Input : public QObject
{

    Q_OBJECT
public:
    QT_Direct_Input(HWND win_hnd);

public:
    HRESULT DInitilization();//process of game pad initialization
    void getDataFromJoy(DIJOYSTATE2 *joyStateRead);

private:
    HRESULT DSelDevice();//select first game device
    HRESULT DSetProp();//set properties for this device
    HRESULT DEnumAxes();//enumerate all axes
    HRESULT DPolling();//polling device for getting all data

    /*callback functions*/
    static BOOL FAR PASCAL enumCallback(const DIDEVICEINSTANCE* instance, VOID* context);
    static BOOL FAR PASCAL enumAxesCallback(const DIDEVICEOBJECTINSTANCE* instance, VOID* context);
    /*callback functions*/

public slots:
    void getJoyData();//this funcion polling game pad all 100ms

signals:
    void joyReadyRead(DIJOYSTATE2 jState);//signals to ready new data from gamepad

private:
   static LPDIRECTINPUTDEVICE8 joystick;//joystick class
   static LPDIRECTINPUT8 di;//direct input class
   QTimer tm;//timer
   QMutex padMtx;


   DIJOYSTATE2 joyState;//gamepad structure
   static DIJOYSTATE2 joyStateBuffer;//gamepad structure
   HWND win_hnd;//window handle
};

#endif // QT_DIRECT_INPUT_H
