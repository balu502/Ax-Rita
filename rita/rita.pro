#-------------------------------------------------
#
# Project created by QtCreator 2016-12-08T14:28:11
#
#-------------------------------------------------

QT += core gui
QT += xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
#greaterThan(QT_MAJOR_VERSION, 4): QT += axserver : CONFIG+= qaxserver


contains(CONFIG, static):DEFINES += QT_NODLL

QT += axserver

CONFIG += warn_off
#CONFIG += console
#CONFIG += qaxserver_no_postlink

#CONFIG  += dll

CONFIG(dll): {
        TEMPLATE = lib
        DEF_FILE = rita.def
        message( "Template dll" )
}else{
        TARGET = rita
        TEMPLATE = app
        message( "Template application" )
}


INCLUDEPATH+="D:/mysql/include/"

SOURCES += main.cpp \
    rita.cpp \
    listchoosewindow.cpp \
    dialogrt7import.cpp \
    hunterdialog.cpp \
    newaccountdialog.cpp

HEADERS  += rita.h \
    listchoosewindow.h \
    dialogrt7import.h \
    hunterdialog.h \
    newaccountdialog.h

HEADERS  += "C:/mysql/include/my_config.h"\
            "C:/mysql/include/my_global.h"\
            "C:/mysql/include/mysql.h"

#win32:LIBS+="-L C:/mysql/libmysqld/Debug -lmysqld"
win32:LIBS += C:/mysql/libmysqld/Debug/libmysqld.lib


RC_FILE  = rita.rc



TRANSLATIONS += rita_ru.ts

OTHER_FILES += \
    rita2.ico \
    rita.rc \
    rita.def

RESOURCES += \
    rita.qrc

DISTFILES += \
    README.txt \
    rita_ru.ts \
    rita2.ico

