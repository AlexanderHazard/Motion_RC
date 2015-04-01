#ifndef FORM_H
#define FORM_H

#include <QMainWindow>
#include <QTimer>
#include <QTcpSocket>
#include<QUdpSocket>
#include <QSettings>
#include <QtTest/QTest>

#include "qt_direct_input.h"
#include "imu_tcp_client.h"
#include "motionudp.h"


namespace Ui {
class Form;
}

class Form : public QMainWindow
{
    Q_OBJECT

public:
    explicit Form(QWidget *parent = 0);
    ~Form();
    QT_Direct_Input *gamePad;


public slots:
   void gamePadNewState(DIJOYSTATE2 gamePadState);

private:
    Ui::Form *ui;
    HWND m_gameHWnd;
    DIJOYSTATE2 js;//gamepad structure
    QTimer gamePadTimer;

    QTcpSocket *ptcpClient;//create socket
    QUdpSocket *pudpClient;
    IMU_Tcp_Client *imuClient;
    MotionUdp *platformClient;

    int Angle = 0, Yaw = 0, Pitch = 0, Roll = 0;


    int speedLimit = 25;//limit of car speed in km/h
    float speedF = 0;//speed forward
    float speedB = 0;//speed back
    float speedR = 0;//speed result

    int rotation_angle = 0;

    float PWM_MAX_SHIFT = 500;//MAX PWM value in us
    int speed_pwm_value = 0;//pwm value of speed
    int rotation_pwm_value = 0;//pwm value of rotation

    float MAX_SPEED = 60.0f;
    float FULL_ROTATION_ANGLE = 180.0f;

    //motion control mode
    enum motionControlMode{Hand, Imu};
    motionControlMode motionContMode = Hand;

    //motion modes enumeration
    enum motionMode{Low, Medium, High};
    motionMode cMotionMode = Low;//value for current Motion Mode

    //motion platform running
    enum platformState{Platform_Stop, Platform_Run};
    platformState motionState = Platform_Stop;//platform state

    //motion platform data
    int prev_angle = 0, new_angle = 0, shift_angle = 0, current_angle = 0, current_rl = 0, current_back = 0;// start angle from imu

    int start_motion_angle = 244, start_motion_left = 294, start_motion_right = 294, start_motion_back = 294;//motion platform start values
    int angle = 0, right = 0, left = 0, back = 0;//values of motion platform

    //rotation parameters
    float PLATFORM_MAX_ANGLE = 135.0f;//max angle of motion platform rotation
    float PLATFORM_MAX_ANGLE_VALUE = 244.0f;//max value of platform rotation
    #define ROTATION_STEP PLATFORM_MAX_ANGLE_VALUE/PLATFORM_MAX_ANGLE //step platform grad by imu

    //right left back parameters
    float PLATFORM_MAX_RLB = 90;//max right left back grad of motion platform
    float PLATFORM_MAX_RLB_VALUE = 294;//max right left back value of motion platform
    float PLATFORM_RLB_STEP = 0;




    void convertImuToMotion();//convertation imu data to motion view
    void convertAngleIMU();//convert yaw to Motion view
    void convertRightLeftImu();//convert roll and pitch to motion view
    void convertBackImu();



 //set settings for application
 QString configFile;
 void saveSettings();
 void loadSettings();
 void setAppSettings();
 bool autostart = false;//automation start all connections



private slots:
   //gui changes
   void speedChange(int);//change limit speed
   void motionModeChange(int);//change motion mode
   void motionControlChange(int);//change motion control
   void autoStartChange(int);//change auto start


   //-------------------
   void motionAnswer(int*);

   //platform
   void connectToPlatform();//connect to Motion Platform or disconnect from it
   void changePlatformState();//change motion platform state(Run, Stop)
   /*change motion platform control
    * Hand mode - motion control parameters change by user
    * Imu mode - motion control parameters change with data from sensors on car
    */
   void changePlatformControl();

   //changing motion values(only in Hand Mode)
   void angleChange(int);
   void rightChange(int);
   void leftChange(int);
   void backChange(int);

   //connect betveen
    void imuDataRead(int* dataRead);
    void imuConnectSucc();
    void imuConnectErr();

};

#endif // FORM_H
