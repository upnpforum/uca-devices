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

#include "mediarendererdevice.h"

#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>
#include <QtXml/QDomText>

#include <Stack/ucautilities.h>

const char *DEVICE_TYPE = "urn:schemas-upnp-org:device:MediaRenderer:1";

MediaRendererDevice::MediaRendererDevice( IUPnPStack * const stack
                                        , const QString &udn
                                        , const QString &friendlyName
                                        )
    : _stack(stack)
    , _avTransport(stack)
    , _renderingControl(&_avTransport.getMediaPlayer(), stack)
    , _deviceType(DEVICE_TYPE)
    , _udn(udn)
    , _friendlyName(friendlyName)
{
    fillServiceTable();
    //loadSettings();
    _description = utilities::createDeviceDescription(*this);
}

MediaRendererDevice::~MediaRendererDevice()
{
    delete _description;
}

static void addServiceToTable( QHash<QString, IUPnPService *> &table
                             , QList<QString> &keyList
                             , IUPnPService * const service
                             )
{
    table[service->getServiceId()] = service;
    keyList.append(service->getServiceId());
}

void MediaRendererDevice::fillServiceTable()
{
    addServiceToTable(_services, _serviceKeysOrder, &_avTransport);
    addServiceToTable(_services, _serviceKeysOrder, &_renderingControl);
    addServiceToTable(_services, _serviceKeysOrder, &_connectionManager);
}

void MediaRendererDevice::loadSettings()
{
    //QSettings settings(QString("settings.ini"), QSettings::IniFormat);
    //_udn = settings.value("upnp/udn", utilities::generateNewUDN()).toString();
    //_friendlyName = settings.value("upnp/friendlyName", "Unnamed Cloud Renderer").toString();
}

QMap<QString, QString>
    MediaRendererDevice::handleSOAP( const QString &serviceId
                                   , void *serviceToken
                                   , const QString &actionName
                                   , const QHash<QString, QString> &arguments
                                   )
{
    Q_UNUSED(serviceToken);

    QMap<QString, QString> results;

    if (_services.contains(serviceId)) {
        results = _services[serviceId]->handleSOAP(actionName, arguments);
    }

    return results;
}

const IUPnPService *MediaRendererDevice::getService(const QString &id) const
{
    const IUPnPService *service = NULL;
    if (_services.contains(id)) {
        service = _services[id];
    }
    return service;
}

const QList<QString> MediaRendererDevice::getServiceIds() const
{
    return _serviceKeysOrder;
}

const QDomDocument &MediaRendererDevice::getDescription(const QString &id) const
{
    /* TODO: this should be failable! */
    if (_services.contains(id)) {
        return _services[id]->getServiceDescription();
    } else {
        return *_description;
    }
}

const QString MediaRendererDevice::getUdn() const
{
    return _udn;
}

const QString MediaRendererDevice::getDeviceType() const
{
    return _deviceType;
}

const QString MediaRendererDevice::getFriendlyName() const
{
    return _friendlyName;
}

const QString MediaRendererDevice::serviceIdForType(const QString &type) const
{
    QString id;

    QHash<QString, IUPnPService *>::const_iterator it = _services.constBegin();
    for (; it != _services.constEnd(); it++) {
        IUPnPService *service = it.value();
        if (service->getServiceType() == type) {
            id = service->getServiceId();
            break;
        }
    }

    return id;
}

void MediaRendererDevice::stackStatusChanged(const QString &name, bool connected)
{
    Q_UNUSED(name);
    Q_UNUSED(connected);
}
