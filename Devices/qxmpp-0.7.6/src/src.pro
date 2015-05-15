include(../qxmpp.pri)

QT -= gui

TEMPLATE = lib

CONFIG += $$QXMPP_LIBRARY_TYPE
DEFINES += QXMPP_BUILD
DEFINES += $$QXMPP_INTERNAL_DEFINES
INCLUDEPATH += $$QXMPP_INCLUDEPATH $$QXMPP_INTERNAL_INCLUDES
LIBS += $$QXMPP_INTERNAL_LIBS -L/usr/include/Qca-qt5/QtCrypto -lqca-qt5

!isEmpty(QXMPP_USE_SPEEX) {
    DEFINES += QXMPP_USE_SPEEX
    LIBS += -lspeex
}

!isEmpty(QXMPP_USE_THEORA) {
    DEFINES += QXMPP_USE_THEORA
    LIBS += -ltheoradec -ltheoraenc
}

!isEmpty(QXMPP_USE_VPX) {
    DEFINES += QXMPP_USE_VPX
    LIBS += -lvpx
}

# Target definition
TARGET = $$QXMPP_LIBRARY_NAME
VERSION = $$QXMPP_VERSION
win32 {
    DESTDIR = $$OUT_PWD
}

include(base/base.pri)
include(client/client.pri)
include(server/server.pri)

HEADERS += $$INSTALL_HEADERS

# Installation
headers.files = $$INSTALL_HEADERS
headers.path = $$PREFIX/include/qxmpp
target.path = $$PREFIX/$$LIBDIR
INSTALLS += headers target

# pkg-config support
CONFIG += create_pc create_prl no_install_prl
QMAKE_PKGCONFIG_DESTDIR = pkgconfig
QMAKE_PKGCONFIG_LIBDIR = $$target.path
QMAKE_PKGCONFIG_INCDIR = $$headers.path
equals(QXMPP_LIBRARY_TYPE,staticlib) {
    QMAKE_PKGCONFIG_CFLAGS = -DQXMPP_STATIC
} else {
    QMAKE_PKGCONFIG_CFLAGS = -DQXMPP_SHARED
}
unix:QMAKE_CLEAN += -r pkgconfig lib$${TARGET}.prl
