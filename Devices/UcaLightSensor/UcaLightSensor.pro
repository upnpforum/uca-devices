! include( ../common.pri ) {
    error( "Couldn't find the common.pri file!" )
}

# Add more folders to ship with the application, here
folder_01.source = qml/CloudLight
folder_01.target = qml
description_dir.source = description-xmls
description_dir.target =
DEPLOYMENTFOLDERS = folder_01 \
                    description_dir

# Additional import path used to resolve QML modules in Creator's code model
QML_IMPORT_PATH =

TARGET = UcaLightSensor

# Installation path
target.path = /usr/bin
INSTALLS += target

SOURCES += main.cpp \
    devicesettings.cpp \
    lightsensor.cpp \
    sensorservices.cpp

! win32 {
    QMAKE_CFLAGS += -std=c99 \
                    -pthread

    PKGCONFIG += gupnp-1.0
    CONFIG += link_pkgconfig
    LIBS += -lglib-2.0 -lgobject-2.0 # this was not needed on Ubuntu 12.04, a bug in .pc?
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

QT += xml testlib xmlpatterns quick

HEADERS += \
    iupnpstackobserver.h \
    iqgrabber.h \
    devicesettings.h \
    cmodules/uda.h \
    lightsensor.h \
    sensorservices.h

OTHER_FILES += \
    description-xmls/device.xml \
    description-xmls/transport.xml \
    description-xmls/configmgmt.xml \
    description-xmls/datamodel.xml \
    settings.ini

RESOURCES += \
    Resources.qrc
