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

#ifndef IUPNPSTACKOBSERVER_H
#define IUPNPSTACKOBSERVER_H

#include <QHash>
#include <QMap>
#include <QList>
#include <QtXml/QDomDocument>
#include <QString>

class IUPnPService;

class IUPnPDevice
{
public:
    virtual ~IUPnPDevice() {}
    virtual QMap<QString, QString> handleSOAP( const QString &serviceId
                                              , void *serviceToken
                                              , const QString &actionName
                                              , const QHash<QString, QString> &arguments
                                              ) = 0;

    virtual const IUPnPService *getService(const QString &id) const = 0;
    virtual const QList<QString> getServiceIds() const = 0;
    virtual const QDomDocument &getDescription(const QString &id) const = 0;

    virtual const QString getUdn() const = 0;
    virtual const QString getDeviceType() const = 0;
    virtual const QString getFriendlyName() const = 0;

    virtual const QString serviceIdForType(const QString &url) const = 0;

    virtual void stackStatusChanged(const QString &name, bool connected) = 0;
};

#endif // IUPNPSTACKOBSERVER_H
