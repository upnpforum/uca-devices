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

#include <QQmlContext>
#include <QtQuick/QQuickView>

#include "devicesettings.h"
#include "dimmablelight.h"
#include <Stack/ucastack.h>

#ifndef WIN32
#include <Stack/udastack.h>
#endif

static const char *APP_NAME = "UcaLight";

static bool setupFiles
    ( const QString &appName
    , const QStringList &requiredFilesNames
    );

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QStringList files;
    files << "settings.ini"
          << "description-xmls/device.xml"
          << "description-xmls/dimming.xml"
          << "description-xmls/switchpower.xml";
    setupFiles(APP_NAME, files);

    const QString settingsPath
            = QDir::homePath() + "/." + APP_NAME + "/settings.ini";
    DeviceSettings settings(settingsPath);

    UcaStack *uca = new UcaStack(settings.getQSettings());
    IUPnPStack *uda = NULL;
#ifndef WIN32
    uda = UdaStack::getInstance();
#endif

    int result = 0;
    try {
        DimmableLight model(&settings, uda, uca, &app, APP_NAME);

        if (uda != NULL) {
#ifndef WIN32
            ((UdaStack *)uda)->initialize(&model);
#endif
            uda->start();
        }
        uca->start();

        printf("%ld\n%ld\n", (long)&uda, (long)&model);

        QQuickView view;
        QQmlContext *context = view.rootContext();
        context->setContextProperty("dimmableLightModel", &model);
        context->setContextProperty("deviceSettings", &settings);
        view.setSource(QUrl("qrc:/qml/CloudLight/CloudLightUI.qml"));
        view.setResizeMode(QQuickView::SizeRootObjectToView);
        view.show();

        result = app.exec();
    } catch(QException e) {
        result = -1;
        app.exit();
    }
    delete uca;
    return result;
}

static bool copyIfNotPresent
    ( const QString &sourcePath
    , const QString &destinationPath
    )
{
    QFile sourceFile(sourcePath);
    QFile destinationFile(destinationPath);

    if (destinationFile.exists()) return true;

    if (sourceFile.open(QIODevice::ReadOnly | QIODevice::Text) == false) {
        fprintf(stderr, "Failed to read template.\n");
        return false;
    }

    QFileInfo destinationInfo(destinationFile);
    QDir(destinationInfo.absolutePath()).mkpath(".");

    if (destinationFile.open(QIODevice::WriteOnly | QIODevice::Text) == false) {
        fprintf(stderr, "Failed to open user configuration file for writing.\n");
        return false;
    }

    QTextStream in(&sourceFile);
    QTextStream out(&destinationFile);
    out << in.readAll();

    return true;
}

static bool setupFiles
    ( const QString &appName
    , const QStringList &requiredFilesNames
    )
{
    QDir::home().mkpath("." + appName);
    const QString targetDirectory = QDir::homePath() + "/." + appName;

    foreach (QString file, requiredFilesNames) {
        const QString source = ":/" + file;
        const QString destination = targetDirectory + "/" + file;
        copyIfNotPresent(source, destination);
    }

    return true;
}

