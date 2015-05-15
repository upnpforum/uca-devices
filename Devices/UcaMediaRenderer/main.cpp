/**
 *
 * Copyright 2013-2014 UPnP Forum All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation 
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE FREEBSD PROJECT "AS IS" AND ANY EXPRESS OR 
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE OR WARRANTIES OF 
 * NON-INFRINGEMENT, ARE DISCLAIMED. IN NO EVENT SHALL THE FREEBSD PROJECT OR 
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are 
 * those of the authors and should not be interpreted as representing official 
 * policies, either expressed or implied, by the UPnP Forum.
 *
 **/

#include <QtGui/QGuiApplication>

#include <QtQml/QQmlContext>
#include <QtQuick/QQuickView>
#include <QThread>

#include <QtMultimedia/QMediaPlaylist>

#include <Stack/ucastack.h>
#include <Stack/ucautilities.h>

#include "mediarendererdevice.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QString configPath = utilities::parseArguments(app.arguments());

    QSettings *settings;
    if (configPath.isEmpty()) {
        settings = utilities::getApplicationSettings(":/settings.ini", "UcaMediaRenderer");
    } else {
        settings = new QSettings(configPath, QSettings::IniFormat);
        if (settings->contains("upnp/udn") == false) {
            settings->setValue("upnp/udn", utilities::generateNewUDN());
            settings->sync();
        }
    }

    QString udn = settings->value("upnp/udn", utilities::generateNewUDN()).toString();
    QString friendlyName = settings->value("upnp/friendlyName", "Unnamed Cloud Renderer").toString();

    UcaStack *uca = new UcaStack(settings);

    MediaRendererDevice device(uca, udn, friendlyName);
    ExtendedMediaPlayer *player
            = &device.getAVTransportService().getMediaPlayer();

    uca->registerDevice(&device);
    uca->start();

    QQuickView view;
    QQmlContext *context = view.rootContext();
    context->setContextProperty("player", player);
    context->setContextProperty("jid", uca->bareJid());
    view.setSource(QUrl("qrc:/qml/UcaMediaRenderer/main.qml"));
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.show();

    int status = app.exec();

    delete uca;
    delete settings;
    return status;
}

