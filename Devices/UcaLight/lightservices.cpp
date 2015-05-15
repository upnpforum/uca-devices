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

#include "lightservices.h"

#include <QFile>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QHash>
#include <cerrno>
#include <QException>

static const char *DIMMING_SERV_TYPE       = "urn:schemas-upnp-org:service:Dimming:1";
static const char *SWITCHPOWER_SERV_TYPE   = "urn:schemas-upnp-org:service:SwitchPower:1";
static const char *LOADLEVELSTATUS_VARNAME = "LoadLevelStatus";
static const char *STATUS_VARNAME          = "Status";
struct ServiceInfo {
    QString id;
    QString type;
    QString scdpPath;
    QString controlUrl;
    QString eventUrl;
    QList<QString> variables;
    QDomDocument *description;
};

static QDomDocument *loadXml(const QString &path, const QString &docName)
{
    QFile file(path);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text) == false)
    {
        printf("Error opening file %s: %s \n",path.toUtf8().constData(),strerror(errno));
        throw QException();
        return NULL;
    }

    QDomDocument *doc = new QDomDocument(docName);
    bool setContentResult = doc->setContent(&file);
    file.close();

    if (setContentResult == false) {
        return NULL;
    }

    return doc;
}

static void findServiceInformation
    (QDomDocument *deviceDoc, QHash<QString, ServiceInfo> &buffer)
{
    if (deviceDoc == NULL)
        return;

    QDomNodeList services = deviceDoc->elementsByTagName("service");

    int count = services.count();
    for (int i = 0; i < count; i++) {
        QString name;
        ServiceInfo info;

        QDomElement serviceNode = services.at(i).toElement();
        QDomElement child = serviceNode.firstChildElement();
        while (child.isNull() == false) {
            if (child.tagName() == "serviceId") {
                name = child.text();
                info.id = name;
            } else if (child.tagName() == "serviceType") {
                info.type = child.text();
            } else if (child.tagName() == "SCPDURL") {
                info.scdpPath = child.text();
            } else if (child.tagName() == "controlURL") {
                info.controlUrl = child.text();
            } else if (child.tagName() == "eventSubURL") {
                info.eventUrl = child.text();
            }

            child = child.nextSiblingElement();
        }

        buffer.insert(name, info);
    }
}

static QList<QString> fetchVariableNames
    (const QDomDocument *serviceDesc, bool eventableOnly)
{
    QList<QString> names;

    QDomNodeList variables = serviceDesc->elementsByTagName("stateVariable");
    QDomElement var = variables.at(0).toElement();
    for (; var.isNull() == false ; var = var.nextSiblingElement()) {
        QDomElement variable = var.toElement();
        if (variable.isNull())
            continue;

        QString sendEvents = variable.attribute("sendEvents");
        bool envetable =    sendEvents.compare("yes", Qt::CaseInsensitive) == 0
                         || sendEvents.compare("true", Qt::CaseInsensitive) == 0;

        if (envetable == false && eventableOnly)
            continue;

        /* TODO: each of the chained calls is failable */
        QString name
            = variable.elementsByTagName("name").at(0).toElement().text();
        names.append(name);
    }

    return names;
}


void loadServices( QDomDocument *deviceDescription
                 , QHash<QString, ServiceInfo *> &externalInfos
                 , const QDir &rootDirectory
                 )
{
    QHash<QString, ServiceInfo> infos;
    findServiceInformation(deviceDescription, infos);

    QHash<QString, ServiceInfo>::const_iterator it = infos.constBegin();
    for (; it != infos.constEnd(); it++) {
        QString name = it.key();
        ServiceInfo info = it.value();

        QString path = rootDirectory.path() + info.scdpPath;

        QDomDocument *desc = loadXml(path, name);
        info.description = desc;
        info.variables = fetchVariableNames(desc, true);

        externalInfos[name] = new (std::nothrow) ServiceInfo(info);
    }
}

/* DIMMING: */

DimmingService::DimmingService(const ServiceInfo *info)
    : _id(info->id)
    , _type(DIMMING_SERV_TYPE)
    , _scdpPath(info->scdpPath)
    , _controlUrl(info->controlUrl)
    , _eventUrl(info->eventUrl)
    , _description(info->description)
    , _variables(info->variables)
    , _dimmingLevel(100)
{
}

/* Actual implementation is in the device */
QMap<QString, QString> DimmingService::handleSOAP
        ( const QString &actionName
        , const QHash<QString, QString> &arguments
        ) {
    Q_UNUSED(actionName);
    Q_UNUSED(arguments);

    return QMap<QString, QString>();
}

/* SWITCH POWER: */

SwitchPowerService::SwitchPowerService(const ServiceInfo *info)
    : _id(info->id)
    , _type(SWITCHPOWER_SERV_TYPE)
    , _scdpPath(info->scdpPath)
    , _controlUrl(info->controlUrl)
    , _eventUrl(info->eventUrl)
    , _description(info->description)
    , _variables(info->variables)
    , _enabled(1)
{
}

/* Actual implementation is in the device */
QMap<QString, QString> SwitchPowerService::handleSOAP
        ( const QString &actionName
        , const QHash<QString, QString> &arguments
        ) {
    Q_UNUSED(actionName);
    Q_UNUSED(arguments);

    return QMap<QString, QString>();
}

const QStringList SwitchPowerService::getEventedVariableNames() const
{
    QStringList variables;
    variables << STATUS_VARNAME;
    return variables;
}

const QMap<QString,QString> SwitchPowerService::getInitialEventVariables() const
{
    QMap<QString,QString> variables;
    variables.insert(STATUS_VARNAME, QString::number(_enabled));
    return variables;
}

const QStringList DimmingService::getEventedVariableNames() const
{
    QStringList variables;
    variables << LOADLEVELSTATUS_VARNAME;
    return variables;
}

const QMap<QString,QString> DimmingService::getInitialEventVariables() const
{
    QMap<QString,QString> variables;
    variables.insert(LOADLEVELSTATUS_VARNAME, QString::number(_dimmingLevel));
    return variables;
}
