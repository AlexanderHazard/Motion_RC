#-------------------------------------------------
#
# Project created by QtCreator 2015-03-26T14:06:21
#
#-------------------------------------------------

QT       += core gui network testlib

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = RC_CAR
TEMPLATE = app

INCLUDEPATH += QCircularBar

SOURCES += main.cpp\
        form.cpp \
    QCircularBar/qcircularbar.cpp \
    QCircularBar/qcircularbardrawfunctions.cpp \
    imu_tcp_client.cpp \
    motionudp.cpp

HEADERS  += form.h \
    QCircularBar/qcircularbar.h \
    qt_direct_input.h \
    qt_direct_input_global.h \
    imu_tcp_client.h \
    motionudp.h

CONFIG(debug, debug|release) {
   LIBS += "..\build-RC_CAR-Desktop_Qt_5_3_MinGW_32bit-Debug\debug\QT_Direct_Input.dll"
}
CONFIG(release, debug|release) {
LIBS += "..\build-RC_CAR-Desktop_Qt_5_3_MinGW_32bit-Release\release\QT_Direct_Input.dll"
}


FORMS    += form.ui

CONFIG += c++11
