#-------------------------------------------------
#
# Project created by QtCreator 2014-07-25T14:17:30
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DyeEditor
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    nitrishapewidget.cpp

HEADERS  += mainwindow.h \
    nitrishapewidget.h \
    NifHeaders.h

FORMS    += mainwindow.ui

win32: LIBS += -L$$PWD/../niflib/lib/ -lniflib_dll

INCLUDEPATH += $$PWD/../niflib/include
DEPENDPATH += $$PWD/../niflib/include

win32:RC_ICONS += potion.ico

RESOURCES += \
    style.qrc
