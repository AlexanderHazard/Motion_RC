#include "form.h"
#include "ui_form.h"
#include <QDebug>
#include <qmath.h>
#include <QFile>
#include <QThreadPool>
#include <QCameraViewfinder>
//#include <QCameraViewfinderSettings>
#include <QGraphicsVideoItem>

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
    imuClient = new IMU_Tcp_Client("192.168.1.101", 5555, ptcpClient);

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
    //change motion platform control mode (hand, imu from car)
    connect(ui->controlBtn, SIGNAL(clicked()), this, SLOT(changePlatformControl()));

    //changes for settings
    connect(ui->motionSlider, SIGNAL(valueChanged(int)), this, SLOT(motionSensChange(int)));//change motiom mode

    //
    connect(ui->runButton, SIGNAL(clicked()), this, SLOT(changePlatformState()));
    connect(ui->motionConnectBtn, SIGNAL(clicked()), this, SLOT(connectToPlatform()));

    //hot keys
    hotKeyObject = new HotKeys();//exaplar of hot key class
    hotKeyThread = new QThread();//thread from hot key heaper
     //push keyobject to thread
    hotKeyObject->moveToThread(hotKeyThread);
    connect(hotKeyThread, SIGNAL(started()), hotKeyObject, SLOT(key_action_run()));
    connect(hotKeyThread, SIGNAL(finished()), hotKeyObject, SLOT(deleteLater()));

    hotKeyThread->start();

      //connect signals from key heaper to slots
    connect(hotKeyObject, SIGNAL(F4KeyPressed()), this, SLOT(F4Click()));
    connect(hotKeyObject, SIGNAL(F5KeyPressed()), this, SLOT(F5Click()));
    connect(hotKeyObject, SIGNAL(F6KeyPressed()), this, SLOT(F6Click()));
    connect(hotKeyObject, SIGNAL(F7KeyPressed()), this, SLOT(F7Click()));


    //audio effects
    engineAudio = new AudioEffects();
    audioThread = new QThread();

    engineAudio->moveToThread(audioThread);
    connect(audioThread, SIGNAL(started()), engineAudio, SLOT(BassAudioProcess()));
    connect(audioThread, SIGNAL(finished()), engineAudio, SLOT(deleteLater()));


    //video capture

    /*videoItem = new QGraphicsVideoItem();
    QGraphicsScene *scene = new QGraphicsScene();
    QVideoWidget *widget = new QVideoWidget();
    ui->graphicsView->setScene(scene);
    scene->addWidget(widget);*/
    ui->widget->setAspectRatioMode(Qt::IgnoreAspectRatio);
    //ui->graphicsView->scene()->addItem(videoItem);

    //ui->widget->setFullScreen(true);
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    foreach (const QCameraInfo &cameraInfo, cameras) {
       // if (QString::compare(cameraInfo.description(), "AVerMedia BDA Analog Capture", Qt::CaseInsensitive) == 0)
        {
            qDebug() << cameraInfo.description();
            camera = new QCamera(cameraInfo);
            camera->setViewfinder(ui->widget);
            camera->setCaptureMode(QCamera::CaptureVideo);
            camera->start();

        }

     }

//Regular expression
    QRegExp re("([0-9]{1,3}[\.]){3}[0-9]{1,3}[:][0-9]{1,4}");
    QRegExpValidator *validator = new QRegExpValidator(re, this);
    ui->lineEdit->setValidator(validator);


}

Form::~Form()
{
    delete ui;
    delete gamePad;
    hotKeyThread->deleteLater();
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
      if(speedR > speedLimit) speedR = speedLimit;

    /*converting speed value to PWM*/
      speed_pwm_value = (PWM_MAX_SHIFT/MAX_SPEED) * speedR;


   ui->speedMeter->setValue(speedR);

   //calculate the angle of rotation
   /*converting rotation */
     rotation_angle = js.lX;
     rotation_pwm_value = (PWM_MAX_SHIFT/(FULL_ROTATION_ANGLE/2)) * rotation_angle;

   /*audio effects*/
     audioChange();
    speedPrev = speedR;
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
  isCarConnect = true;

  int dataToSend[2];
    dataToSend[0] = rotation_pwm_value;
    dataToSend[1] = speed_pwm_value;

  imuClient->tcpSendToServer(dataToSend);
}

void Form::imuConnectErr()
{
  ui->carConnLabel->setText("<font color=\"red\">Not connected!</font>");
  qDebug() << "Connection Missed";
  isCarConnect = false;
  changePlatformControl();//switch to hand mode
}

//procedures and functions for working with imu data
void Form::convertImuToMotion()
{
   convertAngleIMU();
   convertRightLeftImu();
   convertBackImu();
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
         * for example prev_angle = 178 and new_angle = -172, for this condition
         * new_angle - prev_angle = -350, so
         * we make next new_angle = 180 + (180 - 172) = 188, now we calculate our shift
         * shift_angle = 188 - 178 = 10, so we have clockwise rotation on 10 grade
         */
          qDebug() << new_angle;

        if((new_angle - prev_angle) < -100) new_angle = 180 + (180 + new_angle);
        else if((new_angle - prev_angle) > 100) new_angle = -180 + (new_angle - 180);

        shift_angle = new_angle - prev_angle;//calculate shift and direction of ritation

        //calculate current angle
        //we add new shift to angle on each step
        //check if current angle not gone abroad
        if (((current_angle + shift_angle) < PLATFORM_MAX_ANGLE) && ((current_angle + shift_angle) > PLATFORM_MIN_ANGLE))
             current_angle += shift_angle;

        //converting data to motion viev
        angle  = (current_angle * ROTATION_STEP) + start_motion_angle;

        qDebug() << "ANGLE" << angle << "CURRENT ANGLE" << current_angle << "PREV ANGLE" << prev_angle << "NEW ANGLE" << new_angle;
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
    left = start_motion_left - current_rl;
    right = start_motion_right + current_rl;

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
    back = start_motion_back - current_back;

    //show on
    ui->backSlider->setValue(back);
}



//gui

//game settings
void Form::setAppSettings()
{
    PLATFORM_MAX_RLB = cMotionSensitivity;
    ui->spinMotBox->setValue(PLATFORM_MAX_RLB);

    PLATFORM_RLB_STEP = PLATFORM_MAX_RLB_VALUE/PLATFORM_MAX_RLB;//set step for right left and back value for motion platform

}



void Form::loadSettings()
{
   QFile ftry(configFile);
 if(ftry.exists())//if config file exist, load data from him, else set default
  {

   QSettings settings(configFile, QSettings::IniFormat);

          settings.beginGroup("Game");
           cMotionSensitivity = settings.value("motion_sens","").toInt();
          settings.endGroup();
  }

          //ui->comboMotMode->setCurrentIndex(cMotionMode);
          ui->motionSlider->setValue(cMotionSensitivity);
}

void Form::saveSettings()
{
    QSettings settings(configFile, QSettings::IniFormat);
      settings.beginGroup("Game");
         settings.setValue("motion_mode", cMotionSensitivity);
       settings.endGroup();
}




//change motion mode if we change sensitivity
void Form::motionSensChange(int val)
{
    cMotionSensitivity = val;
    setAppSettings();
}


void Form::changePlatformControl()
{
   //here we change mode of motion platform control
    /*we can contron him by hand mode
     * and also we can send data from IMU to HIM
     * Dont work if
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

   if(motionContMode == Demo) demoPlatform();//demo shift calculation

   platformClient->writeDatagramm(motionState, angle, right, left, back);
}

void Form::demoPlatform()
{
  if(!isDemo)
  {
    if(motionState == Platform_Run)//stop platform if it run
     {
        changePlatformState();
     }
    leftDirect = DownDir;
    rightDirect = StopDir;
    backDirect = StopDir;
    ui->leftSlider->setValue(294);//left slider to the middle
    ui->rightSlider->setValue(294);//right slider to the middle
    ui->backSlider->setValue(294);//back slider to the middle
    changePlatformState();//enable platform
    isDemo = true;
  }

int left = ui->leftSlider->value();
int right = ui->rightSlider->value();
int back = ui->backSlider->value();

  if(left == 100) leftDirect = UpDir;
  if(left == 580) leftDirect = DownDir;

  if(right == 100) rightDirect = UpDir;
  if(right == 580) rightDirect = DownDir;

  if(back == 100) backDirect = UpDir;
  if(back == 580) backDirect = DownDir;



  if(leftDirect == UpDir)
    {
      ui->leftSlider->setValue(++left);//to up
    }
  else if(leftDirect == DownDir)
    {
      ui->leftSlider->setValue(--left);//to down
    }

  if(rightDirect == UpDir)
    {
      ui->rightSlider->setValue(++right);//to up
    }
  else if(rightDirect == DownDir)
    {
     ui->rightSlider->setValue(--right);//to down
    }
  else if((rightDirect == StopDir) && (left < 150))
    {
       rightDirect = DownDir;
    }

  if(backDirect == UpDir)
    {
      ui->backSlider->setValue(++back);//to up
    }
  else if(backDirect == DownDir)
    {
      ui->backSlider->setValue(--back);//to down
    }
  else if((backDirect == StopDir) && ( right < 150))
    {
       backDirect = DownDir;
    }
  qDebug() << left << right << back;

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


//hot keys slots
//connect to platform with hot key
void Form::F4Click()
{
    qDebug() <<"F4";
    connectToPlatform();
}
//change motion platform(Run, Stop) mode with hot key
void Form::F5Click()
{
    qDebug() <<"F5";
    changePlatformState();
}


void Form::F6Click()
{
    qDebug() << "F6";
    if(isDemo == false)
     {//enable and disable demo
      ui->motionGroup->setEnabled(false);//disable motion sliders
      motionContMode = Demo;
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
    else
    {
        isDemo = false;//clear demo flag
        changePlatformState();//disable platform
        motionContMode = Hand;//change mode flag to hand
    }
}

void Form::F7Click()
{
    qDebug() << "F7";
    //change platform control(from imu sensors or by hand )
    changePlatformControl();
}

//change audio track
void Form::audioChange()
{
 //set track for start at the beginning
 // if(isCarConnect)
  {
    if((currentEffect == None))
      {
        //engineAudio->FoneAudioEffect();
        audioThread->start();
        currentEffect = Fone;
      }

    if(speedR > 0)
      {

        currentEffect = Run;
        engineAudio->RunSpeed(speedR);
      }
    //else if(speedR == 0 && currentEffect != Down && currentEffect != Fone) {engineAudio->SpeedDnAudioEffect(); currentEffect = Down;}
  }
}
