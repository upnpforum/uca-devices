INCLUDEPATH += . .. ../qxmpp-0.7.6/include
QMAKE_CXXFLAGS += -Wall -Werror

UI_DIR = uics
MOC_DIR = mocs
OBJECTS_DIR = objs

# Define how to create version.h
qxmppheaders.target = ../qxmpp-0.7.6/include
! win32 {
    qxmppheaders.commands = $$PWD/extract-headers.sh $$PWD $$OUT_PWD
}

QMAKE_EXTRA_TARGETS += qxmppheaders

PRE_TARGETDEPS += ../qxmpp-0.7.6/include

LIBS += -L../qxmpp-0.7.6/src

CONFIG( debug, debug|release ) {
    win32 {
        LIBS += -lqxmpp_d0
    } else {
        LIBS += -lqxmpp_d
    }
} else {
    LIBS += -lqxmpp
}

HEADERS +=

SOURCES +=
