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

#ifndef LIGHTSERVICES_H
#define LIGHTSERVICES_H

#include <Stack/iupnpservice.h>
#include <Stack/failable.h>

class QDir;

struct ServiceInfo;

void loadServices( QDomDocument *deviceDescription
                 , QHash<QString, ServiceInfo *> &infos
                 , const QDir &rootDirectory
                 );

class DimmingService : public IUPnPService
{
private:
    const QString _id;
    const QString _type;
    const QUrl _scdpPath;
    const QUrl _controlUrl;
    const QUrl _eventUrl;
    const QDomDocument *_description;
    const QStringList _variables;

    int _dimmingLevel;

public:
    DimmingService(const ServiceInfo *description);

    inline int getDimmingLevel() const { return _dimmingLevel; }
    inline void setDimmingLevel(int level) { _dimmingLevel = level; }

    /* IUPnPService implementation: */
    QMap<QString, QString> handleSOAP( const QString &actionName
                                     , const QHash<QString, QString> &arguments
                                     );

    const QDomDocument &getServiceDescription() const { return *_description; }

    const QString getServiceId() const { return _id; }
    const QString getServiceType() const { return _type; }

    const QUrl getScdpPath() const { return _scdpPath; }
    const QUrl getControlUrl() const { return _controlUrl; }
    const QUrl getEventUrl() const { return _eventUrl; }

    const QStringList getEventedVariableNames() const;
    const QMap<QString,QString> getInitialEventVariables() const;
};

class SwitchPowerService : public IUPnPService
{
private:
    const QString _id;
    const QString _type;
    const QUrl _scdpPath;
    const QUrl _controlUrl;
    const QUrl _eventUrl;
    const QDomDocument *_description;
    const QStringList _variables;

    int _enabled;

public:
    SwitchPowerService(const ServiceInfo *description);

    inline int getEnabled() const { return _enabled; }
    inline void setEnabled(int enabled) { _enabled = enabled; }

    /* IUPnPService implementation: */
    QMap<QString, QString> handleSOAP( const QString &actionName
                                     , const QHash<QString, QString> &arguments
                                     );

    const QDomDocument &getServiceDescription() const { return *_description; }

    const QString getServiceId() const { return _id; }
    const QString getServiceType() const { return _type; }

    const QUrl getScdpPath() const { return _scdpPath; }
    const QUrl getControlUrl() const { return _controlUrl; }
    const QUrl getEventUrl() const { return _eventUrl; }

    const QStringList getEventedVariableNames() const;
    const QMap<QString,QString> getInitialEventVariables() const;
};

#endif // LIGHTSERVICES_H
