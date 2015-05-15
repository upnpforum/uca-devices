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

#include <Stack/failable.h>
#include <Stack/iupnpstack.h>
#include <Stack/iupnpdevice.h>

#include "sensorservices.h"

struct DeviceSettings;

class LightSensor : public QObject, public IUPnPDevice
{
    Q_OBJECT
    Q_PROPERTY(int lightLevel READ getLightLevel NOTIFY lightLevelChanged)
    Q_PROPERTY(int lightEnabled READ getLightEnabled WRITE setLightEnabled NOTIFY lightEnabledChanged)
    Q_PROPERTY(int lightLoadLevel READ getLightLoadLevel WRITE setLightLoadLevel NOTIFY lightLoadLevelChanged)

    Q_PROPERTY(bool udaStackEnabled READ getUdaStackEnabled NOTIFY udaStackEnabledChanged)
    Q_PROPERTY(bool ucaStackEnabled READ getUcaStackEnabled NOTIFY ucaStackEnabledChanged)

private:
    QCoreApplication* _application;

    int _enabled;
    int _loadLevel;

    SensorTransportGenericService *_transportService;
    ConfigurationManagementService *_configService;

    IUPnPStack *_udaStack;
    IUPnPStack *_ucaStack;

    DeviceSettings *_settings;

    QString _dimmingUrn;
    QString _switchUrn;
    QString _temperatureUrn;

    QString _udn;
    QString _friendlyName;
    QString _deviceType;

    QMap<QString, QString> _outArgs;
    QDomDocument *_deviceDescription;

    QDomDocument *_dataModel;

    void handleConfigManagement
        ( const QString &actionName
        , void *serviceToken
        , const QHash<QString, QString> &arguments
        );

    void handleSensorTransport
        ( const QString &actionName
        , void *serviceToken
        , const QHash<QString, QString> &arguments
        );

    void handleReadSensor
        ( const QHash<QString, QString> &arguments
        , QMap<QString, QString> &results
        );

    void handleWriteSensor
        ( const QHash<QString, QString> &arguments
        , QMap<QString, QString> &results
        );

    Failable<bool> setDeviceProperties(QDomDocument *desc);
    void readDeviceProperties(QDomDocument *description);

    void loadDeviceData(const QString &rootDevicePath);
    static void unloadDeviceDescriptions(QHash<QString, QDomDocument *> &descs);

public:
    LightSensor(DeviceSettings *settings, IUPnPStack *udaStack, IUPnPStack *ucaStack, QCoreApplication *app, const QString &appName);
    ~LightSensor();

    inline int getLightLevel() { return _loadLevel * _enabled; }
    inline int getLightEnabled() { return _enabled; }
    void setLightEnabled(int enabled);
    inline int getLightLoadLevel() { return _loadLevel; }
    void setLightLoadLevel(int lightLevel);

    inline bool getUdaStackEnabled() { return _udaStack == NULL ? false : _udaStack->isRunning(); }
    inline bool getUcaStackEnabled() { return _ucaStack == NULL ? false : _ucaStack->isRunning(); }

    Q_INVOKABLE void restart(){
        _application->quit();
        QProcess::startDetached(_application->arguments()[0],_application->arguments());
    }

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
