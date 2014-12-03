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

#include "ucautilities.h"

#include <QUuid>
#include <QStringList>
#include <QDomDocument>
#include <QStringBuilder>
#include <QTextStream>
#include <QSettings>
#include <QFile>
#include <QDir>

#include "iupnpdevice.h"
#include "iupnpservice.h"

static const char *ERROR_CODE_INDEX = "__errorCode";
static const char *ERROR_MESSAGE_INDEX = "__errorMessage";

QString utilities::generateNewUDN()
{
    QUuid uuid = QUuid::createUuid().toString();
    QString uuidString
        = uuid.toString().replace('{', ' ').replace('}', ' ').trimmed();
    return QString("uuid:%1").arg(uuidString);
}

Failable<bool>
    utilities::checkRequiredArguments( const QStringList &argNames
                                     , const QHash<QString, QString> &arguments
                                     ) {
    foreach (QString name, argNames) {
        if (arguments.contains(name) == false) {
            QString message = QString("Missing %1").arg(name);
            return Failable<bool>::Failure(message);
        }
    }
    return true;
}

void utilities::fillWithErrorMessage( QMap<QString, QString> &results
                                    , const unsigned int errorCode
                                    , const QString &message
                                    ) {
    results.clear();
    results[ERROR_CODE_INDEX] = QString::number(errorCode);
    results[ERROR_MESSAGE_INDEX] = message;
}

static const char *DDD_TEMPLATE
    = "<?xml version=\"1.0\"?>"
      "<root xmlns=\"urn:schemas-upnp-org:device-1-0\">"
          "<specVersion>"
              "<major>1</major>"
              "<minor>0</minor>"
          "</specVersion>"
          "<device>"
              "<deviceType>$TYPE</deviceType>"
              "<friendlyName>$FRIENDLY_NAME</friendlyName>"
              "<manufacturer>Comarch</manufacturer>"
              "<manufacturerURL>upnp.comarch.com</manufacturerURL>"
              "<modelDescription>UPnP Cloud Proof of Concept application.</modelDescription>"
              "<modelName>UPnP Cloud PoC</modelName>"
              "<UDN>$UDN</UDN>"
              "<serviceList>"
              "$SERVICES"
              "</serivceList>"
          "</device>"
      "</root>";

static const char *SERVICE_TAG
    =  "<service>"
           "<serviceType>$TYPE</serviceType>"
           "<serviceId>$ID</serviceId>"
           "<SCPDURL>$SCDPURL</SCPDURL>"
           "<controlURL>$CONTROLURL</controlURL>"
           "<eventSubURL>$EVENTINGURL</eventSubURL>"
       "</service>";

QDomDocument *utilities::createDeviceDescription(const IUPnPDevice &device)
{
    QDomDocument *description = new QDomDocument("ddd");

    QString ddd(DDD_TEMPLATE);
    ddd = ddd.replace("$TYPE", device.getDeviceType());
    ddd = ddd.replace("$FRIENDLY_NAME", device.getFriendlyName());
    ddd = ddd.replace("$UDN", device.getUdn());

    QString services;
    QStringList ids = device.getServiceIds();
    foreach(QString id, ids) {
        const IUPnPService *service = device.getService(id);
        if (service == NULL)
            continue;

        QString serviceTag(SERVICE_TAG);
        serviceTag = serviceTag.replace("$ID", service->getServiceId());
        serviceTag = serviceTag.replace("$TYPE", service->getServiceType());
        serviceTag = serviceTag.replace("$SCDPURL", service->getScdpPath().toString());
        serviceTag = serviceTag.replace("$EVENTINGURL", service->getEventUrl().toString());
        serviceTag = serviceTag.replace("$CONTROLURL", service->getControlUrl().toString());

        services = services % serviceTag;
    }

    ddd = ddd.replace("$SERVICES", services);

    description->setContent(ddd);
    return description;
}

bool utilities::copyIfNotPresent(const QString &sourcePath, const QString &destinationPath)
{
    QFile sourceFile(sourcePath);
    QFile destinationFile(destinationPath);

    if (destinationFile.exists()) return true;

    if (sourceFile.open(QIODevice::ReadOnly | QIODevice::Text) == false) {
        fprintf(stderr, "Failed to read template.\n");
        return false;
    }

    if (destinationFile.open(QIODevice::WriteOnly | QIODevice::Text) == false) {
        fprintf(stderr, "Failed to open user configuration file for writing.\n");
        return false;
    }

    QTextStream in(&sourceFile);
    QTextStream out(&destinationFile);
    out << in.readAll();

    return true;
}

QSettings *utilities::getApplicationSettings( const QString &settingsTemplatePath
                                            , const QString &appName
                                            ) {
    QString settingsPath = QDir::homePath() + "/." + appName + "/settings.ini";

    QDir::home().mkpath("." + appName);
    bool result = copyIfNotPresent(settingsTemplatePath, settingsPath);

    QSettings *settings = NULL;
    if (result) {
        settings = new QSettings(settingsPath, QSettings::IniFormat);
        if (settings->contains("upnp/udn") == false) {
            settings->setValue("upnp/udn", generateNewUDN());
            settings->sync();
        }
    }

    return settings;
}

QString utilities::parseArguments(const QStringList &args)
{
    QString settingsPath;

    QStringList::const_iterator it = args.constBegin();
    for (; it != args.constEnd(); it++) {
        QString arg = *it;
        if (arg == "--config") {
            it++;
            if (it != args.constEnd()) {
                settingsPath = *it;
            }
        }
    }

    return settingsPath;
}
