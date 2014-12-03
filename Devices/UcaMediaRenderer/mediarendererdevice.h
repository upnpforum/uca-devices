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

#ifndef MEDIARENDERERDEVICE_H
#define MEDIARENDERERDEVICE_H

#include <UcaStack/iupnpdevice.h>

#include "avtransportservice.h"
#include "renderingcontrolservice.h"
#include "connectionmanagerservice.h"

class QDomDocument;
class IUPnPStack;

class MediaRendererDevice : public IUPnPDevice
{
private:
    IUPnPStack *_stack;
    AVTransportService _avTransport;
    RenderingControlService _renderingControl;
    ConnectionManagerService _connectionManager;

    QDomDocument *_description;

    const QString _deviceType;
    const QString _udn;
    const QString _friendlyName;

    QHash<QString, IUPnPService *> _services;
    QList<QString> _serviceKeysOrder;

    void loadSettings();
    void fillServiceTable();
    QDomDocument *createDescription() const;

public:
    MediaRendererDevice( IUPnPStack * const stack
                       , const QString &udn
                       , const QString &friendlyName
                       );
    ~MediaRendererDevice();

    virtual QMap<QString, QString> handleSOAP( const QString &serviceId
                                              , void *serviceToken
                                              , const QString &actionName
                                              , const QHash<QString, QString> &arguments
                                              );

    virtual const IUPnPService *getService(const QString &id) const;
    virtual const QList<QString> getServiceIds() const;
    virtual const QDomDocument &getDescription(const QString &id) const;

    virtual const QString getUdn() const;
    virtual const QString getDeviceType() const;
    virtual const QString getFriendlyName() const;

    virtual const QString serviceIdForType(const QString &url) const;

    virtual void stackStatusChanged(const QString &name, bool connected);

    inline AVTransportService &getAVTransportService() { return _avTransport; }
};

#endif // MEDIARENDERERDEVICE_H
