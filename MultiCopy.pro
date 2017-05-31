#-------------------------------------------------
#
# Project created by QtCreator 2017-01-13T14:33:05
#
#-------------------------------------------------

QT       += core gui concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MultiCopy
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    copyingdialog.cpp \
    copyprogress.cpp \
    formatdrivesdialog.cpp \
    copyprocess.cpp

HEADERS  += mainwindow.h \
    copyingdialog.h \
    copyprogress.h \
    formatdrivesdialog.h \
    copyprocess.h

FORMS    += mainwindow.ui \
    copyingdialog.ui \
    formatdrivesdialog.ui \
    copyprogress.ui

RESOURCES += \
    resources.qrc

TRANSLATIONS = da_DK.ts \
