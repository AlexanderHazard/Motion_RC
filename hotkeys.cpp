#include "hotkeys.h"
#include <QDebug>


HotKeys::HotKeys()
{

}

HotKeys::~HotKeys()
{
    stopKeyHeaper();
}


void HotKeys::key_action_run()
{
    enum{F1_KEYID = 1, F4_KEYID = 2, F5_KEYID = 3, F6_KEYID = 4, F7_KEYID};

    RegisterHotKey(0, F1_KEYID, 0, VK_F1);
    RegisterHotKey(0, F4_KEYID, 0, VK_F4);
    RegisterHotKey(0, F5_KEYID, 0, VK_F5);
    RegisterHotKey(0, F6_KEYID, 0, VK_F6);
    RegisterHotKey(0, F7_KEYID, 0, VK_F7);


    MSG msg;
    while(!quit)
    {
        GetMessage(&msg,0,0,0);
        if(msg.message == WM_HOTKEY)
        {
           if(msg.wParam == F4_KEYID)
             {//qDebug() << "hot key";
               emit F4KeyPressed();
             }

           if(msg.wParam == F5_KEYID)
             {//qDebug() << "hot key";
               emit F5KeyPressed();
             }

           if(msg.wParam == F6_KEYID)
             {//qDebug() << "hot key";
               emit F6KeyPressed();
             }

           if(msg.wParam == F7_KEYID)
             {//qDebug() << "hot key";
               emit F7KeyPressed();
             }

           if(msg.wParam == F1_KEYID)
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
