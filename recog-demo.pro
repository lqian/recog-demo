#-------------------------------------------------
#
# Project created by QtCreator 2018-06-16T14:15:58
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = recog-demo
TEMPLATE = app

CONFIG += c++11

SOURCES += main.cpp\
        recogdemo.cpp \


HEADERS  += recogdemo.h \
    build/ui_recogdemo.h

FORMS    += recogdemo.ui

INCLUDEPATH += /usr/local/cuda/include /usr/local/include /usr/local/tensorrt/include
DEPENDPATH +=  /usr/local/cuda/lib64 /usr/local/lib /usr/local/tensorrt/lib

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../build/lib -lcore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../build/lib
else:unix: LIBS += -L/usr/local/cuda/lib64 -L/usr/local/lib -L/usr/local/tensorrt/lib -lnvinfer -lnvinfer_plugin -lnvcaffe_parser -lnvparsers -lcore -lboost_system -lboost_filesystem  -lopencv_freetype -lcuda -lcudart -lcublas -lcurand -fopenmp -lcudnn -lstdc++fs -ldl


unix|win32: LIBS += `pkg-config --libs opencv`


RESOURCES +=
