QT -= gui
QT += core network

CONFIG += c++1y console
CONFIG -= app_bundle


# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += main.cpp \
    cprocessing.cpp \
    spi_client.cpp \
    i2c_client.cpp \
    bcm2835.c \
    telaire.cpp \
    spi_aux_client.cpp \
    ad7608.cpp \
    ads1118.cpp \
    tcp_client.cpp \
    proximity_ind.cpp

HEADERS += \
    cprocessing.h \
    spi_client.h \
    i2c_client.h \
    spi_client.h \
    bcm2835.h \
    portablesleep.h \
    telaire.h \
    spi_aux_client.h \
    ad7608.h \
    ads1118.h \
    abstractsensorinterface.h \
    abstractsensor.h \
    tcp_client.h \
    timing.h \
    proximity_ind.h

LIBS += -L$$PWD/../../../../opt/qt5pi/sysroot/usr/lib -lwiringPi

INCLUDEPATH += $$PWD/../../../../opt/qt5pi/sysroot/usr/include

DEPENDPATH += $$PWD/../../../../opt/qt5pi/sysroot/usr/include

target.files = SimoldesTest
target.path = /home
INSTALLS = target
