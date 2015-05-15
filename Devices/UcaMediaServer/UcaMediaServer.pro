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
    mediaserverdevice.cpp \
    contentdirectoryservice.cpp \
    connectionmanagerservice.cpp

# Installation path
target.path = /usr/bin
INSTALLS += target

! win32 {
    LIBS += -L../Stack -lStack
}
win32 {
    debug {
        LIBS += -L../Stack/debug -lStack
    }
    release {
        LIBS += -L../Stack/release -lStack
    }
}

HEADERS += \
    mediaserverdevice.h \
    contentdirectoryservice.h \
    connectionmanagerservice.h \
    cmscdptemplate.h \
    cdscdptemplate.h

OTHER_FILES += \
    settings.ini \
    browseResponse.xml

RESOURCES += \
    resources.qrc
