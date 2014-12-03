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

#include "dimmablelight.h"

#include "devicesettings.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QHash>
#include <cerrno>
#include <QException>

static const char *DEVICE_PATH = "description-xmls/device.xml";

static const char *DIMMING_SERV_ID     = "urn:upnp-org:serviceId:Dimming:1";
static const char *SWITCHPOWER_SERV_ID = "urn:upnp-org:serviceId:SwitchPower:1";

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

DimmableLight::DimmableLight
    ( DeviceSettings *settings
    , IUPnPStack *udaStack
    , IUPnPStack *ucaStack
    , QCoreApplication* app
    , const QString &appName
    ) : _application(app)
      , _udaStack(udaStack)
      , _ucaStack(ucaStack)
      , _settings(settings)
      , _udn()
      , _friendlyName()
      , _deviceType()
{
    const QString basePath = QDir::homePath() + "/." + appName + "/";
    const QString path(basePath + DEVICE_PATH);
    loadDeviceData(path);

    if (_udaStack != NULL)
        _udaStack->registerDevice(this);
    if (_ucaStack != NULL)
        _ucaStack->registerDevice(this);
}

DimmableLight::~DimmableLight()
{
    delete _dimming;
    delete _switch;
}

static void logSOAPCall(QString serviceId, QString actionName, const QHash<QString, QString> &arguments)
{
    qDebug() << "* action:" << actionName << "of service:" << serviceId;
    qDebug() << "+---- called with: ----";

    QHash<QString, QString>::const_iterator it = arguments.constBegin();
    while (it != arguments.constEnd()) {
        qDebug() << "|" << it.key() << "=" << it.value();
        it++;
    }
    qDebug() << "+----------------------\n";
}

QMap<QString, QString> DimmableLight::handleSOAP( const QString &serviceId
                                                , void *serviceToken
                                                , const QString &actionName
                                                , const QHash<QString, QString> &arguments
                                                )
{
    _outArgs.clear();

    logSOAPCall(serviceId, actionName, arguments);

    if (serviceId == _dimming->getServiceId()) {
        handleDimming(actionName, serviceToken, arguments);
    } else if (serviceId == _switch->getServiceId()) {
        handleSwitchPower(actionName, serviceToken, arguments);
    } else{
        _outArgs["__errorCode"] = QString::number(401);
        _outArgs["__errorMessage"] = "Invalid action";
    }

    return _outArgs;
}

void DimmableLight::handleDimming(QString actionName, void *serviceToken, const QHash<QString, QString> &arguments)
{
    if (actionName == "SetLoadLevelTarget") {
        QString raw = QString();
        if (arguments.contains("newLoadlevelTarget"))
            raw = arguments.value("newLoadlevelTarget");

        if (raw.isEmpty() == false) {
            bool parseResult;
            int parsed = raw.toInt(&parseResult);
            if (parseResult) {
                if(parseResult < 0 || parseResult > 100){
                    _outArgs["__errorCode"]    = QString::number(601);
                    _outArgs["__errorMessage"] = "Argument Value Out of range";
                }else{
                    _dimming->setDimmingLevel(parsed);

                    lightLoadLevelChanged(getLightLoadLevel());
                    lightLevelChanged(getLightLevel());

                    QString serviceId(DIMMING_SERV_ID);
                    if (_udaStack != NULL)
                        _udaStack->sendEvent(serviceId, serviceToken, "LoadLevelStatus", raw);
                    if (_ucaStack != NULL)
                        _ucaStack->sendEvent(serviceId, NULL, "LoadLevelStatus", raw);
                    }
            } else {
                _outArgs["__errorCode"]    = QString::number(402);
                _outArgs["__errorMessage"] = "Wrong value, expected number.";
            }
        }else{
            _outArgs["__errorCode"]    = QString::number(402);
            _outArgs["__errorMessage"] = "Missing argument newLoadlevelTarget";
        }
    } else if (actionName == "GetLoadLevelTarget") {
        _outArgs["retLoadlevelTarget"] = QString::number(getLightLoadLevel());
    } else if (actionName == "GetLoadLevelStatus") {
        _outArgs["retLoadlevelStatus"] = QString::number(getLightLoadLevel());
    } else{
        _outArgs["__errorCode"] = QString::number(401);
        _outArgs["__errorMessage"] = "Invalid action";
    }
}

void DimmableLight::handleSwitchPower(QString actionName, void *serviceToken, const QHash<QString, QString> &arguments)
{
    if (actionName == "SetTarget") {
        QString raw = QString();
        if (arguments.contains("NewTargetValue"))
            raw = arguments.value("NewTargetValue");

        if (raw.isEmpty() == false) {
            bool parseResult;
            int parsed = raw.toInt(&parseResult);
            if (parseResult) {
                _switch->setEnabled(parsed);

                lightEnabledChanged(_switch->getEnabled() > 0);
                lightLevelChanged(getLightLevel());

                QString serviceId(SWITCHPOWER_SERV_ID);
                if (_udaStack != NULL)
                    _udaStack->sendEvent(serviceId, serviceToken, "Status", raw);
                if (_ucaStack != NULL)
                    _ucaStack->sendEvent(serviceId, NULL, "Status", raw);
            } else {
                _outArgs["__errorCode"]    = QString::number(402);
                _outArgs["__errorMessage"] = "Wrong value, expected boolean.";
            }
        }else{
            _outArgs["__errorCode"]    = QString::number(402);
            _outArgs["__errorMessage"] = "Missing argument NewTargetValue";
        }
    } else if (actionName == "GetTarget") {
        _outArgs["RetTargetValue"] = QString::number(getLightEnabled());
    } else if (actionName == "GetStatus") {
        _outArgs["ResultStatus"] = QString::number(getLightEnabled());
    } else{
        _outArgs["__errorCode"] = QString::number(401);
        _outArgs["__errorMessage"] = "Invalid action";
    }
}

void DimmableLight::enableUca(bool status)
{
    if (status) {
        _ucaStack->start();
    } else {
        _ucaStack->stop();
    }
}

void DimmableLight::enableUda(bool status)
{
    if (_udaStack == NULL) return;

    if (status) {
        _udaStack->start();
    } else {
        _udaStack->stop();
    }
}

void DimmableLight::restart() {
    _application->quit();
    QProcess::startDetached(_application->arguments()[0],_application->arguments());
}

void DimmableLight::loadDeviceData(const QString &rootDevicePath)
{
    _deviceDescription = loadXml(rootDevicePath, "Root Device");
    if (_deviceDescription == NULL) {
        fprintf(stderr, "Failed to load device description!");
        return;
    }

    setDeviceProperties(_deviceDescription);
    readDeviceProperties(_deviceDescription);

    QHash<QString, ServiceInfo *> infos;

    QFileInfo rootFileInfo(rootDevicePath);
    QDir rootDirectory = rootFileInfo.dir();

    loadServices(_deviceDescription, infos, rootDirectory);

    QHash<QString, ServiceInfo *>::const_iterator it = infos.constBegin();
    for (; it != infos.constEnd(); it++) {
        QString name = it.key();
        ServiceInfo *info = it.value();
        if (name == DIMMING_SERV_ID) {
            _dimming = new (std::nothrow) DimmingService(info);
        } else if (name == SWITCHPOWER_SERV_ID) {
            _switch = new (std::nothrow) SwitchPowerService(info);
        }
    }
}

Failable<bool> DimmableLight::setDeviceProperties(QDomDocument *desc)
{
    if (_settings == NULL) {
        Failable<bool>::Failure("The settings are missing.");
    }

    QDomElement udnNode = desc->elementsByTagName("UDN").item(0).toElement();
    if (udnNode.isNull()) {
        Failable<bool>::Failure("Couldn't find UDN node.");
    } else {
        qDebug() << "Found" << udnNode.tagName() << "with text value:" << udnNode.text();
    }

    QDomElement nameNode = desc->elementsByTagName("friendlyName").item(0).toElement();
    if (nameNode.isNull()) {
        Failable<bool>::Failure("Couldn't find friendlyName node.");
    } else {
        qDebug() << "Found" << nameNode.tagName() << "with text value:" << nameNode.text();
    }

    _udn          = _settings->getSetting(QString("upnp/udn"));
    _friendlyName = _settings->getSetting(QString("upnp/friendlyName"));
    /* TODO: the assertion that the both elements really have text nodes inside
     *       can badly fail */
    udnNode.firstChild().setNodeValue(_udn);
    nameNode.firstChild().setNodeValue(_friendlyName);

    return true;
}

void DimmableLight::readDeviceProperties(QDomDocument *desc)
{
    QDomElement type = desc->elementsByTagName("deviceType").item(0).toElement();
    _deviceType = type.text();
}

void DimmableLight::unloadDeviceDescriptions(QHash<QString, QDomDocument *> *descs)
{
    QHash<QString, QDomDocument *>::const_iterator it = descs->constBegin();
    for (; it != descs->constEnd(); it++) {
        delete it.value();
    }
}

const QList<QString> DimmableLight::getServiceIds() const
{
    QList<QString> list;

    if (_dimming != NULL) list.append(_dimming->getServiceId());
    if (_switch != NULL)  list.append(_switch->getServiceId());

    return list;
}

const IUPnPService * DimmableLight::getService(const QString &id) const {
    if (_dimming != NULL && _dimming->getServiceId() == id)
        return _dimming;
    if (_switch != NULL && _switch->getServiceId() == id)
        return _switch;

    return NULL;
}

const QString DimmableLight::serviceIdForType(const QString &type) const
{
    if (_dimming != NULL && _dimming->getServiceType() == type)
        return _dimming->getServiceId();
    if (_switch != NULL && _switch->getServiceType() == type)
        return _switch->getServiceId();

    return QString();
}

const QDomDocument &DimmableLight::getDescription(const QString &id) const
{
    if (_dimming != NULL && _dimming->getServiceId() == id)
        return _dimming->getServiceDescription();
    if (_switch != NULL && _switch->getServiceId() == id)
        return _switch->getServiceDescription();

    return *_deviceDescription;
}

void DimmableLight::stackStatusChanged(const QString &name, bool connected)
{
    Q_UNUSED(connected);
    if (name.contains("uca", Qt::CaseInsensitive)) {
        if (_ucaStack != NULL) {
            emit ucaStackEnabledChanged(_ucaStack->isRunning());
        }
    } else if (name.contains("uda", Qt::CaseInsensitive)) {
        if (_udaStack != NULL) {
            emit udaStackEnabledChanged(_udaStack->isRunning());
        }
    }
}

void DimmableLight::setLightLoadLevel(int level){
    _dimming->setDimmingLevel(level);
    QString serviceId = _dimming->getServiceId();

    if (_udaStack != NULL)
        _udaStack->sendEvent(serviceId, NULL, "LoadLevelStatus", QString::number(level));
    if (_ucaStack != NULL)
        _ucaStack->sendEvent(serviceId, NULL, "LoadLevelStatus", QString::number(level));
    emit this->lightLoadLevelChanged(level);
}

void DimmableLight::setLightEnabled(int enabled){
    _switch->setEnabled(enabled);
    QString serviceId = _switch->getServiceId();

    if (_udaStack != NULL)
        _udaStack->sendEvent(serviceId, NULL, "Status", QString::number(enabled));
    if (_ucaStack != NULL)
        _ucaStack->sendEvent(serviceId, NULL, "Status", QString::number(enabled));

    emit this->lightEnabledChanged(enabled);
}
