#include "motionudp.h"
#include <math.h>
#include <QtEndian>

MotionUdp::MotionUdp(QUdpSocket *uSock, QString ip, int port)
{
    udpSocket = uSock;
    this->ip = ip;
    this->port = port;
}

MotionUdp::~MotionUdp()
{
}

void MotionUdp::startConnection()
{
   udpSocket->bind(port);

   connect (udpSocket, SIGNAL(readyRead()), this, SLOT(readDatagramm()));
}

void MotionUdp::closeConnection()
{
    udpSocket->close();
}

void MotionUdp::writeDatagramm(int mRun, int angle, int right, int left, int back)
{
   QByteArray datagram;
   QDataStream out(&datagram, QIODevice::WriteOnly);
   out.setByteOrder(QDataStream::BigEndian);

   mAngle = angle * 131/2;
   mRight = right * 131/2;
   mLeft = left *131/2;
   mBack = back * 131/2;


   out << qToBigEndian(mRun) << qToBigEndian((short int)mAngle) << qToBigEndian((short int)mRight) << qToBigEndian((short int)mLeft) << qToBigEndian((short int)mBack)
          << qToBigEndian(zero) << qToBigEndian(zero) << qToBigEndian(zero) << qToBigEndian(zero) << qToBigEndian(zero)
          << qToBigEndian(numb) << qToBigEndian(numb) << qToBigEndian(numb) << (unsigned char)43 << count++;

   udpSocket->writeDatagram(datagram, QHostAddress(ip), port);
}

void MotionUdp::readDatagramm()
{

    while(udpSocket->hasPendingDatagrams())
      {
       QByteArray datagramm;
        datagramm.resize(udpSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        udpSocket->readDatagram(datagramm.data(), datagramm.size(), &sender, &senderPort);

        //datagramm = qFromBigEndian(datagramm);
        qDebug() << datagramm.toHex();

        int platformpos[3] = {0, 0, 0};
        platformpos[0] = (quint8) datagramm[2];
        platformpos[1] = (quint8) datagramm[3];
        platformpos[2] = (quint8) datagramm[28];
        emit newDatagramm(platformpos);
      }


}
