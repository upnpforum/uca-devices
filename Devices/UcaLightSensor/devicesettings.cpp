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

#include "devicesettings.h"

#include <QUuid>
#include <QDebug>
static const QString DEFAULT_SETTINGS_PATH("~/.CloudLightSM/settings.ini");

void initializeDefaultSettings(QHash<QString, QString> &table)
{
    table.insert("xmpp/pubsubService", "pubsub.upnpcloud.comarch.com");
    table.insert("xmpp/ipAddress", "upnpcloud.comarch.com");
    table.insert("xmpp/port", "5222");
    table.insert("xmpp/userName", "@upnpcloud.comarch.com");
    table.insert("xmpp/password", "");

    table.insert("upnp/friendlyName", "The Cloud Light");
    table.insert("upnp/udn", DeviceSettings::generateNewUDN());
}

DeviceSettings::DeviceSettings(const QString &settingsPath, QObject *parent)
    : QObject(parent)
    , _settings(settingsPath, QSettings::IniFormat)
    //, _settings(DEFAULT_SETTINGS_PATH, QSettings::IniFormat)
{
    initializeDefaultSettings(_defaultSettings);
}

bool DeviceSettings::hasSetting(const QString &fullName) const
{
    return _settings.contains(fullName);
}

QString DeviceSettings::getSetting(const QString &fullName)
{
    QString defaultValue = _defaultSettings.value(fullName, QString());
    if (hasSetting(fullName) == false) {
        _settings.setValue(fullName, defaultValue);
        return defaultValue;
    }

    return _settings.value(fullName, defaultValue).toString();
}
void DeviceSettings::setSetting(const QString &fullName,const QString &value){
    qDebug() << "Set settings " << fullName << ":" << value;
    _settings.setValue(fullName,value);
    _settings.sync();
}

QString DeviceSettings::generateNewUDN()
{
    QUuid uuid = QUuid::createUuid().toString();
    QString uuidString
        = uuid.toString().replace('{', ' ').replace('}', ' ').trimmed();
    return QString("uuid:%1").arg(uuidString);
}
