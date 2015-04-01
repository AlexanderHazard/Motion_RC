#ifndef IMU_TCP_CLIENT_H
#define IMU_TCP_CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QTime>
#include <QtEndian>

class IMU_Tcp_Client : public QObject
{
    Q_OBJECT

public:
    IMU_Tcp_Client(QString, int, QTcpSocket*);

private:
     QString IP = "192.168.88.36";
     int PORT = 5555;
    QTcpSocket *tcpSocket;

    //position in space parameters
    int iYaw = 0, iPitch = 0, iRoll = 0;
    bool isConnected();
    bool connectFlag = false;

public slots:
    void startConnection();//try to connect with server
    void tcpCloseConnection();//close connect with server
    void tcpReadyRead();//receive packet handler
    void tcpError(QAbstractSocket::SocketError);//connection error
    void tcpSendToServer(int *dataToSend);//send data to server
    void tcpConnected();//handler of connection success
    void tcpDisconnected();//handler of connection failed

signals:
    void tcpDataReady(int*);//when data was readed
    void connectSuccess();//when connection success
    void connectError();//when connection error

};

#endif // IMU_TCP_CLIENT_H
