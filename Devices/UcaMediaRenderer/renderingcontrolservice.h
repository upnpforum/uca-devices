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

#ifndef RENDERINGCONTROLSERVICE_H
#define RENDERINGCONTROLSERVICE_H

#include <UcaStack/iupnpservice.h>
#include <UcaStack/ucastack.h>

class QMediaPlayer;

class RenderingControlService : public IUPnPService
{
private:
    IUPnPStack *_stack;

    const QString _type;
    const QString _id;

    QMediaPlayer *_player;

    const QDomDocument *_description;

    QMap<QString, QString> _resultBuffer;

    void handleListPresets( const QHash<QString, QString> &arguments
                          , QMap<QString, QString> &results
                          ) const;

    void handleSelectPreset( const QHash<QString, QString> &arguments
                           , QMap<QString, QString> &results
                           ) const;

    void handleGetVolume(const QHash<QString, QString> &arguments, QMap<QString, QString> &result);
    void handleSetVolume( const QHash<QString, QString> &arguments
                        , QMap<QString, QString> &result
                        );

public:
    RenderingControlService(QMediaPlayer *player, IUPnPStack * stack);
    ~RenderingControlService();

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
};

#endif // RENDERINGCONTROLSERVICE_H
