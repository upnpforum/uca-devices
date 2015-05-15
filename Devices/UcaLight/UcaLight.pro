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

TARGET = UcaLight
# Installation path
target.path = /usr/bin
INSTALLS += target

# The .cpp file which was generated for your project. Feel free to hack it.
SOURCES += main.cpp \
    dimmablelight.cpp \
    devicesettings.cpp \
    lightservices.cpp

! win32 {
    SOURCES +=
}

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

QT += xml testlib quick

HEADERS += \
    dimmablelight.h \
    devicesettings.h \
    lightservices.h

OTHER_FILES += \
    description-xmls/switchpower.xml \
    description-xmls/dimming.xml \
    description-xmls/device.xml

RESOURCES += \
    Resources.qrc


