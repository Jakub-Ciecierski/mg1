#-------------------------------------------------
#
# Project created by QtCreator 2016-02-26T09:43:53
#
#-------------------------------------------------

PROJECT_ROOT=../../..
DEPENDENCIES=${PROJECT_ROOT}/depedencies

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = editor
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    glwidget.cpp

HEADERS  += mainwindow.h \
    glwidget.h

FORMS    += mainwindow.ui

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../dependencies/lib/gm/release/ -lgm
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../dependencies/lib/gm/debug/ -lgm
else:unix: LIBS += -L$$PWD/../../../dependencies/lib/gm/ -lgm

INCLUDEPATH += $$PWD/../../../dependencies/include/gm
DEPENDPATH += $$PWD/../../../dependencies/include/gm
