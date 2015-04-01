#ifndef MOTIONUDP_H
#define MOTIONUDP_H

#include <QUdpSocket>
#include <QObject>

class MotionUdp : public QObject
{
    Q_OBJECT
public:
    MotionUdp(QUdpSocket *uSock, QString ip, int port);
    ~MotionUdp();
    void startConnection();//start connection
    void closeConnection();//start connection
    void writeDatagramm(int mRun, int angle, int right, int left, int back);

    short int mAngle = 0, mRight = 0, mLeft = 0, mBack = 0, zero = 0, numb= 10000;
    unsigned char two = 1;

private slots:
void readDatagramm();

signals:
    void newDatagramm(int *data);

private:
unsigned char count = 1;
QString ip;
int port;
QUdpSocket *udpSocket;
};

#endif // MOTIONUDP_H
