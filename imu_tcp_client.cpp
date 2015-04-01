#include "imu_tcp_client.h"
#include <QDebug>

IMU_Tcp_Client::IMU_Tcp_Client(QString ip, int port, QTcpSocket* pSocket)
{
    tcpSocket = pSocket;//socket point
    //set host parameters
    IP = ip;
    PORT = port;

    /*connection to handlers*/
      connect(tcpSocket, SIGNAL(connected()), this, SLOT(tcpConnected()));
      connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(tcpReadyRead()));
      connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(tcpError(QAbstractSocket::SocketError)));
}


void IMU_Tcp_Client::startConnection()
{   //Connect current socket
    tcpSocket->connectToHost(IP, PORT);
    qDebug() << "Connection to host...";
}

void IMU_Tcp_Client::tcpCloseConnection()
{
    tcpSocket->close();
}

void IMU_Tcp_Client::tcpReadyRead()
{
    //input stream then connected to socket
    QDataStream in(tcpSocket);

    //set byte LittleEndian order
    in.setByteOrder(QDataStream::LittleEndian);

    //set serialization for input stream
    int yaw=0, pitch=0, roll=0;
    in >> yaw >> pitch >> roll;

    iYaw = qFromLittleEndian(yaw);//convert to int
    iPitch = qFromLittleEndian(pitch);//convert to int
    iRoll = qFromLittleEndian(roll);//convert to int

    int telemetryBuffer[3];
      telemetryBuffer[0] = iYaw;
      telemetryBuffer[1] = iPitch;
      telemetryBuffer[2] = iRoll;
    //qDebug() << yaw << pitch << roll;
    //move data to main thread
    emit tcpDataReady(telemetryBuffer);

}

void IMU_Tcp_Client::tcpSendToServer(int *dataToSend)
{
   //make byte array for data which we want send
    QByteArray arrBlock;

   //make output stream and connect our array to it
    QDataStream out(&arrBlock, QIODevice::WriteOnly);

    //push data to stream
    out << dataToSend[0] << dataToSend[1];

    out.device()->seek(0);

    //write to stream
    tcpSocket->write(arrBlock);
}

void IMU_Tcp_Client::tcpError(QAbstractSocket::SocketError err)
{
    QString strError =
               "Error:" + (err == QAbstractSocket::HostNotFoundError ?
                           "The host was not found." :
                           err == QAbstractSocket::RemoteHostClosedError ?
                           "The reemote host is closed." :
                           err == QAbstractSocket::ConnectionRefusedError ?
                           "The connection was refused." :
                           QString(tcpSocket->errorString())
                           );
     qDebug() << strError;
     connectFlag = false;
    // emit errConn(strError);
     emit connectError();
}

//this function are using for send status data to main thread
void IMU_Tcp_Client::tcpConnected()
{
    connectFlag = true;
    emit connectSuccess();
}

void IMU_Tcp_Client::tcpDisconnected()
{
    connectFlag = false;
    emit connectError();
}

