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

TARGET = Stack
TEMPLATE = lib

DEFINES += STACK_LIBRARY

SOURCES += \
    ucastack.cpp \
    ucaeventinghandler.cpp \
    iqgrabber.cpp \
    ucautilities.cpp \
    iupnpstack.cpp

! win32 {
    SOURCES += cmodules/uda.c udastack.cpp
}

HEADERS +=\
        ucastack_global.h \
    ucastack.h \
    ucaeventinghandler.h \
    iupnpstack.h \
    iqgrabber.h \
    failable.h \
    iupnpdevice.h \
    iupnpservice.h \
    ucautilities.h \
    udastack.h \
    cmodules/uda.h \
    inotifiableservice.h

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
    Stack.pro.user

! win32 {
    QMAKE_CFLAGS += -std=c99 \
                    -pthread \
                    -I/usr/include/libxml2 \
                    -I/usr/include/libsoup-2.4 \
                    -I/usr/include/glib-2.0 \
                    -I/usr/lib/x86_64-linux-gnu/glib-2.0/include \
                    -I/usr/include/gupnp-1.0 \
                    -I/usr/include/gssdp-1.0 \

    LIBS += -lgupnp-1.0 -lgssdp-1.0 -lxml2 -lsoup-2.4 -lgio-2.0 -lgobject-2.0 -lglib-2.0
}
