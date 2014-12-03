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

#include "mediaserverdevice.h"

#include <QSettings>
#include <QRegularExpression>

#include <UcaStack/ucautilities.h>

static const char *DEVICE_TYPE = "urn:schemas-upnp-org:device:MediaServer:1";

MediaServerDevice::MediaServerDevice(const QString &udn, const QString &friendlyName)
    : _udn(udn)
    , _friendlyName(friendlyName)
    , _deviceType(DEVICE_TYPE)
{
    _description = utilities::createDeviceDescription(*this);
}

MediaServerDevice::~MediaServerDevice()
{
    delete _description;
}

QMap<QString, QString>
    MediaServerDevice::handleSOAP( const QString &serviceId
                                 , void *serviceToken
                                 , const QString &actionName
                                 , const QHash<QString, QString> &arguments
                                 )
{
    Q_UNUSED(serviceToken);
    Q_UNUSED(serviceId);

    if (serviceId == _contentDirectory.getServiceId()) {
        return _contentDirectory.handleSOAP(actionName, arguments);
    } else if(serviceId == _connectionManager.getServiceId()){
        return _connectionManager.handleSOAP(actionName, arguments);
    }else{
        QMap<QString,QString> map;
        map["__errorCode"] = QString::number(401);
        map["__errorMessage"] = "Invalid Action";
        return map;
    }
}

const IUPnPService *MediaServerDevice::getService(const QString &id) const
{
    const IUPnPService *service = NULL;
    if (_contentDirectory.getServiceId() == id) {
        service = &_contentDirectory;
    } else if (_connectionManager.getServiceId() == id) {
        service = &_connectionManager;
    }
    return service;
}

const QList<QString> MediaServerDevice::getServiceIds() const
{
    QList<QString> result;
    result << _contentDirectory.getServiceId() << _connectionManager.getServiceId();
    return result;
}

const QDomDocument &MediaServerDevice::getDescription(const QString &id) const
{
    if (_contentDirectory.getServiceId() == id) {
        return _contentDirectory.getServiceDescription();
    } else if (_connectionManager.getServiceId() == id){
        return _connectionManager.getServiceDescription();
    } else {
        return *_description;
    }
}

const QString MediaServerDevice::getUdn() const
{
    return _udn;
}

const QString MediaServerDevice::getDeviceType() const
{
    return _deviceType;
}

const QString MediaServerDevice::getFriendlyName() const
{
    return _friendlyName;
}

const QString MediaServerDevice::serviceIdForType(const QString &type) const
{
    QString id;

    if (_contentDirectory.getServiceType() == type) {
        id = _contentDirectory.getServiceId();
    } else if (_connectionManager.getServiceType() == type){
        id = _connectionManager.getServiceId();
    }

    return id;
}

void MediaServerDevice::stackStatusChanged(const QString &name, bool connected)
{
    Q_UNUSED(name);
    Q_UNUSED(connected);
}

