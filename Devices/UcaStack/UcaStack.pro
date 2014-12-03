#-------------------------------------------------
#
# Project created by QtCreator 2014-02-20T13:53:46
#
#-------------------------------------------------

! include( ../common.pri ) {
    error( "Couldn't find the common.pri file!" )
}

QT       += xml network
QT       -= gui

TARGET = UcaStack
TEMPLATE = lib

DEFINES += UCASTACK_LIBRARY

SOURCES += \
    ucastack.cpp \
    ucaeventinghandler.cpp \
    iqgrabber.cpp \
    ucautilities.cpp

HEADERS +=\
        ucastack_global.h \
    ucastack.h \
    ucaeventinghandler.h \
    iupnpstack.h \
    iqgrabber.h \
    failable.h \
    iupnpdevice.h \
    iupnpservice.h \
    ucautilities.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

#LIBS += -lqxmpp

OTHER_FILES += \
    UcaStack.pro.user
