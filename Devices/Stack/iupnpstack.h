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

#ifndef IUPNPSTACK_H
#define IUPNPSTACK_H

#include "iupnpdevice.h"
#include "inotifiableservice.h"

struct EventMessage
{
    QString serviceId;
    void *serviceToken;
    QString variableName;
    QString value;
    INotifiableService* serviceToNotify;

    EventMessage(QString serviceId, void *serviceToken, QString variableName, QString value, INotifiableService* serviceToNotify)
        : serviceId(serviceId), serviceToken(serviceToken), variableName(variableName), value(value), serviceToNotify(serviceToNotify) { }
    EventMessage() {}
};

class IUPnPStack
{
public:
    virtual ~IUPnPStack() {}

    virtual void sendEvent(EventMessage eventMessage) = 0;
    void sendEvent(QString serviceId, void *serviceToken, QString variableName, QString value);
    virtual void registerDevice(IUPnPDevice *observer) = 0;

    virtual void start() = 0;
    virtual void stop() = 0;

    virtual bool isRunning() const = 0;

};

#endif // IUPNPSTACK_H
