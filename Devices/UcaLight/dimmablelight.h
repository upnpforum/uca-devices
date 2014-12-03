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

#ifndef DIMMABLELIGHT_H
#define DIMMABLELIGHT_H

#include <QObject>
#include <QProcess>
#include <QCoreApplication>

#include <UcaStack/failable.h>
#include <UcaStack/iupnpstack.h>
#include <UcaStack/iupnpdevice.h>

#include "lightservices.h"

struct DeviceSettings;

class DimmableLight : public QObject, public IUPnPDevice
{
    Q_OBJECT
    Q_PROPERTY(int lightLevel READ getLightLevel NOTIFY lightLevelChanged)
    Q_PROPERTY(int lightEnabled READ getLightEnabled WRITE setLightEnabled NOTIFY lightEnabledChanged)
    Q_PROPERTY(int lightLoadLevel READ getLightLoadLevel WRITE setLightLoadLevel NOTIFY lightLoadLevelChanged)

    Q_PROPERTY(bool udaStackEnabled READ getUdaStackEnabled NOTIFY udaStackEnabledChanged)
    Q_PROPERTY(bool ucaStackEnabled READ getUcaStackEnabled NOTIFY ucaStackEnabledChanged)

private:
    QCoreApplication* _application;

    DimmingService *_dimming;
    SwitchPowerService *_switch;

    IUPnPStack *_udaStack;
    IUPnPStack *_ucaStack;

    DeviceSettings *_settings;

    QString _udn;
    QString _friendlyName;
    QString _deviceType;

    QMap<QString, QString> _outArgs;
    QDomDocument * _deviceDescription;

    void handleDimming(QString actionName, void *serviceToken, const QHash<QString, QString> &arguments);
    void handleSwitchPower(QString actionName, void *serviceToken, const QHash<QString, QString> &arguments);

    Failable<bool> setDeviceProperties(QDomDocument *desc);
    void readDeviceProperties(QDomDocument *description);

    void loadDeviceData(const QString &rootDevicePath);
    static void unloadDeviceDescriptions(QHash<QString, QDomDocument *> *descs);

public:
    DimmableLight
        ( DeviceSettings *settings
        , IUPnPStack *udaStack
        , IUPnPStack *ucaStack
        , QCoreApplication *app
        , const QString &appName
        );
    ~DimmableLight();

    inline int getLightLevel() const { return getLightLoadLevel() * getLightEnabled(); }
    inline int getLightEnabled() const { return _switch->getEnabled(); }
    inline int getLightLoadLevel() const { return _dimming->getDimmingLevel(); }

    void setLightEnabled(int enabled);
    void setLightLoadLevel(int lightLevel);

    inline bool getUdaStackEnabled() { return _udaStack == NULL ? false : _udaStack->isRunning(); }
    inline bool getUcaStackEnabled() { return _ucaStack == NULL ? false : _ucaStack->isRunning(); }

    Q_INVOKABLE void restart();

    /* IUPnPDevice implementation: */
    QMap<QString, QString> handleSOAP( const QString &serviceId
                                     , void *serviceToken
                                     , const QString &actionName
                                     , const QHash<QString, QString> &arguments
                                     );

    const IUPnPService *getService(const QString &id) const;
    const QList<QString> getServiceIds() const;
    const QDomDocument &getDescription(const QString &id) const;

    inline const QString getUdn() const { return _udn; }
    inline const QString getDeviceType() const { return _deviceType; }
    inline const QString getFriendlyName() const { return _friendlyName; }

    const QString serviceIdForType(const QString &url) const;

    void stackStatusChanged(const QString &name, bool connected);

signals:
    void lightLevelChanged(int level);
    void lightEnabledChanged(int enabled);
    void lightLoadLevelChanged(int level);

    void udaStackEnabledChanged(bool enabled);
    void ucaStackEnabledChanged(bool enabled);

public slots:
    void enableUda(bool status);
    void enableUca(bool status);

};

#endif // DIMMABLELIGHT_H
