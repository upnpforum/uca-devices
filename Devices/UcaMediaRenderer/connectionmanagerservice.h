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

#ifndef CONNECTIONMANAGERSERVICE_H
#define CONNECTIONMANAGERSERVICE_H

#include <Stack/iupnpservice.h>

class IUPnPStack;

class ConnectionManagerService : public IUPnPService
{
private:

    const QString _type;
    const QString _id;

    QDomDocument *_description;

    QMap<QString, QString> _resultBuffer;

    void handleGetProtocolInfo(QMap<QString, QString> &results);
    void handleGetCurrentConnectionIDs(QMap<QString, QString> &results);
    void handleGetCurrentConnectionInfo(const QHash<QString,QString>& arguments,QMap<QString, QString> &results);

public:
    ConnectionManagerService();
    ~ConnectionManagerService();

    virtual QMap<QString, QString> handleSOAP( const QString &actionName
                                              , const QHash<QString, QString> &arguments
                                              );

    virtual const QDomDocument &getServiceDescription() const;

    virtual const QString getServiceId() const;
    virtual const QString getServiceType() const;

    virtual const QUrl getScdpPath() const;
    virtual const QUrl getControlUrl() const;
    virtual const QUrl getEventUrl() const;

    virtual const QStringList getEventedVariableNames() const;
    virtual const QMap<QString,QString> getInitialEventVariables() const;
};

#endif // CONNECTIONMANAGERSERVICE_H
