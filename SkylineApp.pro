#-------------------------------------------------
#
# Project created by QtCreator 2013-03-12T01:10:25
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SkylineApp
TEMPLATE = app

SOURCES += main.cpp\
        skyline.cpp

HEADERS  += skyline.h

FORMS    += skyline.ui

QMAKE_CXXFLAGS += -fpermissive

LIBS += -lglut -lGLU -lGL
