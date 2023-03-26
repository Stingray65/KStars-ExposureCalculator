QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    calculatedgainsubexposuretime.cpp \
    cameraexposureenvelope.cpp \
    fileutilitysensordata.cpp \
    genericfilterdata.cpp \
    imagingsensordata.cpp \
    main.cpp \
    optimalexposuredetail.cpp \
    optimalexposurestack.cpp \
    optimalsubexposurecalculator.cpp \
    qcustomplot.cpp \
    sensorgainreadnoise.cpp \
    subexposurecalculatorpoc.cpp

HEADERS += \
    calculatedgainsubexposuretime.h \
    cameraexposureenvelope.h \
    fileutilitysensordata.h \
    genericfilterdata.h \
    imagingsensordata.h \
    optimalexposuredetail.h \
    optimalexposurestack.h \
    optimalsubexposurecalculator.h \
    qcustomplot.h \
    sensorgainreadnoise.h \
    subexposurecalculatorpoc.h

FORMS += \
    subexposurecalculatorpoc.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
