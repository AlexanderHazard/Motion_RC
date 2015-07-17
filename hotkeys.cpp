#include "hotkeys.h"
#include <QDebug>


HotKeys::HotKeys()
{

}

HotKeys::~HotKeys()
{
    stopKeyHeaper();
}


void HotKeys::run()
{
    enum{ONE_KEYID = 1, SECOND_KEY_ID = 2, THIRD_KEY_ID = 3};

    RegisterHotKey(0, ONE_KEYID, 0, VK_F3);
    RegisterHotKey(0, SECOND_KEY_ID, 0, VK_F1);
    //RegisterHotKey(0, THIRD_KEY_ID, 0, VK_F6);

    MSG msg;
    while(!quit)
    {
        GetMessage(&msg,0,0,0);
        if(msg.message == WM_HOTKEY)
        {
           if(msg.wParam == 1)
             {//qDebug() << "hot key";
               emit hotKeyPressed();
               //keybd_event(VK_F5, 0xBF, 0, 0);//push
               //keybd_event(VK_F5, 0xBF, KEYEVENTF_KEYUP, 0);//release
             }

           if(msg.wParam == 2)
             {
               //qDebug() << "break";
               break;
             }

         /*  if(msg.wParam == 3)
             {
              emit demoPressed();
             }*/
        }
        QThread::msleep(100);

    }
    UnregisterHotKey(0, 1);
}


void HotKeys::stopKeyHeaper()
{
   /* nmtx.lock();
      quit = true;
    nmtx.unlock();*/
    keybd_event(VK_F1, 0xBB, 0, 0);//push
    keybd_event(VK_F1, 0xBB, KEYEVENTF_KEYUP, 0);//release

}
