#-------------------------------------------------
#
# Project created by QtCreator 2016-03-29T11:30:34
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PanoGen
TEMPLATE = app

INCLUDEPATH += /usr/local/include/opencv
LIBS += `pkg-config opencv --libs`

SOURCES += main.cpp\
        gui.cpp \
    calibrationwindow.cpp \
    cameracalibration.cpp \
    cropimage.cpp \
    mousetrack_qlabel.cpp

HEADERS  += gui.h \
    calibrationwindow.h \
    cameracalibration.h \
    cropimage.h \
    mousetrack_qlabel.h

FORMS    += gui.ui \
    calibrationwindow.ui \
    cropimage.ui
