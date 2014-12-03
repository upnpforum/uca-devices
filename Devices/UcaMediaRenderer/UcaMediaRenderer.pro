! include( ../common.pri ) {
    error( "Couldn't find the common.pri file!" )
}

QT += multimedia xml quick

# Additional import path used to resolve QML modules in Creator's code model
QML_IMPORT_PATH =

# If your application uses the Qt Mobility libraries, uncomment the following
# lines and add the respective components to the MOBILITY variable.
# CONFIG += mobility
# MOBILITY +=

# The .cpp file which was generated for your project. Feel free to hack it.
SOURCES += main.cpp \
    extendedmediaplayer.cpp \
    mediarendererdevice.cpp \
    avtransportservice.cpp \
    renderingcontrolservice.cpp \
    connectionmanagerservice.cpp \
    renderingcontrolutilities.cpp

# Installation path
target.path = /usr/bin
INSTALLS += target

! win32 {
    LIBS += -L../UcaStack -lUcaStack
}
win32 {
    debug {
        LIBS += -L../UcaStack/debug -lUcaStack
    }
    release {
        LIBS += -L../UcaStack/release -lUcaStack
    }
}

HEADERS += \
    extendedmediaplayer.h \
    mediarendererdevice.h \
    avtransportservice.h \
    avtscdptemplate.h \
    renderingcontrolservice.h \
    connectionmanagerservice.h \
    rcscdptemplate.h \
    cmscdptemplate.h \
    renderingcontrolutilities.h

OTHER_FILES += \
    settings.ini

RESOURCES += \
    resources.qrc
