#include "form.h"
#include "ui_form.h"
#include <QDebug>
#include <qmath.h>
#include <QFile>

Form::Form(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Form)
{
    ui->setupUi(this);

    //settings
    configFile = QApplication::applicationDirPath()+ "/config.ini";
    setAppSettings();
    loadSettings();

    //make initialization of reading data from gamepad
    m_gameHWnd = (HWND)this->winId();//get Handle of window
    gamePad = new QT_Direct_Input(m_gameHWnd);//init direct input lib
    gamePad->DInitilization();

    //connect to new state of game pad reading
    connect(gamePad, SIGNAL(joyReadyRead(DIJOYSTATE2)), this, SLOT(gamePadNewState(DIJOYSTATE2)));

    //make initialization of tcp transaction between Car and PC
    ptcpClient = new QTcpSocket(this);
    imuClient = new IMU_Tcp_Client("192.168.88.44", 5555, ptcpClient);

    //make initialization of udp transaction between PC and Motion Platform
    pudpClient = new QUdpSocket(this);
    platformClient = new MotionUdp(pudpClient, "192.168.222.119", 3000);

    //default motion platform in hand mode
    /*connect sliders changes to motion values changes*/
    connect(ui->angleSlider, SIGNAL(valueChanged(int)), this, SLOT(angleChange(int)));
    connect(ui->rightSlider, SIGNAL(valueChanged(int)), this, SLOT(rightChange(int)));
    connect(ui->leftSlider, SIGNAL(valueChanged(int)), this, SLOT(leftChange(int)));
    connect(ui->backSlider, SIGNAL(valueChanged(int)), this, SLOT(backChange(int)));



    //gui
    //change motion platform control mode
    connect(ui->controlBtn, SIGNAL(clicked()), this, SLOT(changePlatformControl()));

    //speed limit changes
    ui->speedSpin->setEnabled(false);
    connect(ui->speedSlider, SIGNAL(valueChanged(int)), ui->speedSpin, SLOT(setValue(int)));
    connect(ui->speedSlider, SIGNAL(valueChanged(int)), this, SLOT(speedChange(int)));//change limit speed

    //changes for settings
    connect(ui->comboMotMode, SIGNAL(currentIndexChanged(int)), this, SLOT(motionModeChange(int)));//change motiom mode
    connect(ui->defContBox, SIGNAL(currentIndexChanged(int)), this, SLOT(motionControlChange(int)));//void motContChange(int);//change motion control
    connect(ui->autoCheck, SIGNAL(stateChanged(int)), this, SLOT(autoStartChange(int)));

    //
    connect(ui->runButton, SIGNAL(clicked()), this, SLOT(changePlatformState()));
    connect(ui->motionConnectBtn, SIGNAL(clicked()), this, SLOT(connectToPlatform()));
}

Form::~Form()
{
    delete ui;
    delete gamePad;
}

void Form::gamePadNewState(DIJOYSTATE2 gamePadState)
{
    js = gamePadState;
   //gamePad->getDataFromJoy(&js);

   //qDebug() << "AXIS Wheel =" << js.lX << "AXIS IGnition =" << js.lY << "AXIS Back =" << js.lRz;

   //calculate the speed of car
   /*formula which detect current speed of car*/
      speedF = fabs((MAX_SPEED/FULL_ROTATION_ANGLE) * (js.lY - FULL_ROTATION_ANGLE/2));
      speedB = fabs((MAX_SPEED/FULL_ROTATION_ANGLE) * (js.lRz - FULL_ROTATION_ANGLE/2));
      speedR = speedF - speedB;//detect the result speed
      //qDebug() << "speed = " << speedR;

    /*converting speed value to PWM*/
      speed_pwm_value = (PWM_MAX_SHIFT/MAX_SPEED) * speedR;


   ui->speedMeter->setValue(speedR);

   //calculate the angle of rotation
   /*converting rotation */
     rotation_angle = js.lX;
     rotation_pwm_value = (PWM_MAX_SHIFT/(FULL_ROTATION_ANGLE/2)) * rotation_angle;

   /*qDebug() << "AXIS AX =" << js.lAX << "AXIS AY =" << js.lAY << "AXIS AZ =" << js.lAZ;

   qDebug() << "AXIS RX =" << js.lRx << "AXIS RY =" << js.lRy << "AXIS RZ =" << js.lRz;

   qDebug() << "AXIS VX =" << js.lVX << "AXIS VY =" << js.lVY << "AXIS VZ =" << js.lVZ;*/
}


//Procedures for working with tcp socket

void Form::imuDataRead(int *dataRead)
{
   Yaw = dataRead[0];
   Roll = dataRead[1];
   Pitch = dataRead[2];

   int dataToSend[2];
     dataToSend[0] = rotation_pwm_value;
     dataToSend[1] = speed_pwm_value;

   convertImuToMotion();

   imuClient->tcpSendToServer(dataToSend);
}


void Form::imuConnectSucc()
{
  ui->carConnLabel->setText("<font color=\"green\">Connected!</font>");
  qDebug() << "Connection OK";

  int dataToSend[2];
    dataToSend[0] = rotation_pwm_value;
    dataToSend[1] = speed_pwm_value;

  imuClient->tcpSendToServer(dataToSend);
}

void Form::imuConnectErr()
{
  ui->carConnLabel->setText("<font color=\"red\">Not connected!</font>");
  qDebug() << "Connection Missed";
}

//procedures and functions for working with imu data
void Form::convertImuToMotion()
{
   convertAngleIMU();
   convertRightLeftImu();
}

void Form::convertAngleIMU()
{
    //set start angle
    if(prev_angle == 0 && new_angle == 0 && shift_angle == 0) prev_angle = Yaw;
    else
      {

        new_angle = Yaw;//get new angle

        //if step of angle more then 100 grad
        /*
         * for example prev_angle = 1 and new_angle = -172, for this condition
         * we make next new_angle = -180 - (-172) = -8, now we calculate our shift
         * shift_angle = 1 - (-8) = 9, so we have clockwise rotation on 8 grade
         */
        if((prev_angle - new_angle) < -100) new_angle = -180 - new_angle;
        else if((prev_angle - new_angle) > 100) new_angle = 180 + new_angle;

        shift_angle = prev_angle - new_angle;//calculate shift and direction of ritation

        //calculate current angle
        //we add new shift to angle on each step
        current_angle += shift_angle;

        //check if angle not more or little then limiting values of angle in platform
        if(current_angle >= (int)PLATFORM_MAX_ANGLE)  current_angle = (int)PLATFORM_MAX_ANGLE;
        else if(current_angle <= (int)(-1 * PLATFORM_MAX_ANGLE))  current_angle = (int)(-1 * PLATFORM_MAX_ANGLE);

        //converting data to motion viev
        angle  = (current_angle * ROTATION_STEP) + start_motion_angle;

        //qDebug() << angle << current_angle << prev_angle << new_angle;
        ui->angleSlider->setValue(angle);

        prev_angle = new_angle;//save current angle
       }
}

void Form::convertRightLeftImu()
{
    if(Roll >= 0)
      {
        if(Roll > PLATFORM_MAX_RLB) Roll = PLATFORM_MAX_RLB;//check if Roll value is not more then limit grad
      }
    else if(Roll < 0)
      {
        if(Roll < (-1 * PLATFORM_MAX_RLB)) Roll = -1* PLATFORM_MAX_RLB;//check if Roll value is not little then limit grad
      }

    current_rl = PLATFORM_RLB_STEP * Roll;//calculate current right left value
    //convert to motion view
    left = start_motion_left + current_rl;
    right = start_motion_right - current_rl;

    //show on gui
    ui->leftSlider->setValue(left);
    ui->rightSlider->setValue(right);
}

void Form::convertBackImu()
{
    if(Pitch >= 0)
      {
        if(Pitch > PLATFORM_MAX_RLB) Pitch = PLATFORM_MAX_RLB;//check if Pitch value is not more then limit grad
      }
    else if(Pitch < 0)
      {
         if(Pitch < (-1 * PLATFORM_MAX_RLB)) Pitch = -1* PLATFORM_MAX_RLB;//check if Roll value is not little then limit grad
      }

    current_back = PLATFORM_RLB_STEP * Pitch;//calculate current back value

    //convert to motion view
    back = start_motion_back + current_back;

    //show on
    ui->backSlider->setValue(back);
}



//gui

//game settings
void Form::setAppSettings()
{
    switch(cMotionMode)
    {
       case Low: PLATFORM_MAX_RLB = 90;//set low step for 1 grad
                 break;

       case Medium: PLATFORM_MAX_RLB = 45;//set medium step for 1 grad
                break;

       case High: PLATFORM_MAX_RLB = 30;//set hard step for 1 grad
                 break;

    }
    PLATFORM_RLB_STEP = PLATFORM_MAX_RLB_VALUE/PLATFORM_MAX_RLB;//set step for right left and back value for motion platform

    //set speed limit
    speedLimit = ui->speedSpin->value();
}



void Form::loadSettings()
{
   QFile ftry(configFile);
 if(ftry.exists())//if config file exist, load data from him, else set default
  {

   QSettings settings(configFile, QSettings::IniFormat);

          settings.beginGroup("Car");
           speedLimit = settings.value("speed","").toInt();
          settings.endGroup();

          settings.beginGroup("Game");
           cMotionMode = (motionMode)settings.value("motion_mode","").toInt();
           motionContMode = (motionControlMode)settings.value("default_control", "").toInt();
           autostart = settings.value("autostart", "").toBool();
          settings.endGroup();
  }
     //show this on gui
          ui->speedSlider->setValue(speedLimit);
          ui->comboMotMode->setCurrentIndex(cMotionMode);
          ui->defContBox->setCurrentIndex(motionContMode);
          ui->autoCheck->setChecked(autostart);
}

void Form::saveSettings()
{
    QSettings settings(configFile, QSettings::IniFormat);
               settings.beginGroup("Car");
               settings.setValue("speed", speedLimit);
              settings.endGroup();

             settings.beginGroup("Game");
               settings.setValue("motion_mode", cMotionMode);
               settings.setValue("default_control", motionContMode);
               settings.setValue("autostart", autostart);
             settings.endGroup();
}


//change limit speed
void Form::speedChange(int val)
{
    speedLimit = val;
}

//change motion mode
void Form::motionModeChange(int val)
{
    cMotionMode = (motionMode) val;
}

//change motion control
void Form::motionControlChange(int val)
{
    motionContMode = (motionControlMode) val;
}

//cahange auto start value
void Form::autoStartChange(int val)
{
    if(val) autostart = true;
    else autostart = false;
}

void Form::changePlatformControl()
{
   //here we change mode of motion platform control
    /*we can contron him by hand mode
     * and also we can send data from IMU to HIM
    */
    if(motionContMode == Imu)
      {
        ui->controlBtn->setText("Imu mode");
        ui->motionGroup->setEnabled(true);
        motionContMode = Hand;
        imuClient->tcpCloseConnection();//close connection

        //disconnect signals from socket handler
        disconnect(imuClient, SIGNAL(tcpDataReady(int*)), this, SLOT(imuDataRead(int*)));
        disconnect(imuClient, SIGNAL(connectSuccess()), this, SLOT(imuConnectSucc()));
        disconnect(imuClient, SIGNAL(connectError()), this, SLOT(imuConnectErr()));

        /*connect sliders changes to motion values changes*/
        connect(ui->angleSlider, SIGNAL(valueChanged(int)), this, SLOT(angleChange(int)));
        connect(ui->rightSlider, SIGNAL(valueChanged(int)), this, SLOT(rightChange(int)));
        connect(ui->leftSlider, SIGNAL(valueChanged(int)), this, SLOT(leftChange(int)));
        connect(ui->backSlider, SIGNAL(valueChanged(int)), this, SLOT(backChange(int)));
      }
    else if(motionContMode == Hand)
      {
        /*disconnect sliders changes to motion values changes*/
        disconnect(ui->angleSlider, SIGNAL(valueChanged(int)), this, SLOT(angleChange(int)));
        disconnect(ui->rightSlider, SIGNAL(valueChanged(int)), this, SLOT(rightChange(int)));
        disconnect(ui->leftSlider, SIGNAL(valueChanged(int)), this, SLOT(leftChange(int)));
        disconnect(ui->backSlider, SIGNAL(valueChanged(int)), this, SLOT(backChange(int)));

        ui->controlBtn->setText("Hand mode");
        ui->motionGroup->setEnabled(false);
        motionContMode = Imu;

        //connect signals to socket handler
        connect(imuClient, SIGNAL(tcpDataReady(int*)), this, SLOT(imuDataRead(int*)));
        connect(imuClient, SIGNAL(connectSuccess()), this, SLOT(imuConnectSucc()));
        connect(imuClient, SIGNAL(connectError()), this, SLOT(imuConnectErr()));
        imuClient->startConnection();
      }
}

//changing motion values(only in Hand Mode)
void Form::angleChange(int val)
{
    angle = val;
}

void Form::rightChange(int val)
{
    right = val;
}

void Form::leftChange(int val)
{
    left = val;
}

void Form::backChange(int val)
{
    back = val;
}

//connect to Motion Platform
void Form::connectToPlatform()
{
 if(QString::compare(ui->motionConnectBtn->text(), "Connect") == 0)
   {
     ui->motionConnectBtn->setText("Disconnect");
     connect(platformClient, SIGNAL(newDatagramm(int*)), this, SLOT(motionAnswer(int*)));
     platformClient->startConnection();
     platformClient->writeDatagramm(motionState, angle, right, left, back);
  }

 else if(QString::compare(ui->motionConnectBtn->text(), "Disconnect") == 0)
   {
     disconnect(platformClient, SIGNAL(newDatagramm(int*)), this, SLOT(motionAnswer(int*)));
     ui->motionConnectBtn->setText("Connect");
     platformClient->closeConnection();
   }
}

void Form::motionAnswer(int *data)
{
   ui->motConnLabel->setText("<font color=\"green\">Connected!</font>");
   ui->rightEdit->setText(QString::number(data[0]));
   ui->leftEdit->setText(QString::number(data[1]));
   ui->backEdit->setText(QString::number(data[2]));
   platformClient->writeDatagramm(motionState, angle, right, left, back);
}

//platform states
//change platform state
void Form::changePlatformState()
{
    if(motionState == Platform_Stop)
      {
        motionState = Platform_Run;
        ui->runButton->setText("Stop");
      }
    else if(motionState == Platform_Run)
      {
        motionState = Platform_Stop;
        ui->runButton->setText("Run");
      }
}
