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

#ifndef AVTRANSPORTSERVICE_H
#define AVTRANSPORTSERVICE_H

#include <Stack/iupnpservice.h>
#include <Stack/failable.h>

#include <QtMultimedia/QMediaPlaylist>
#include <QMutex>
#include "extendedmediaplayer.h"
#include <Stack/iupnpstack.h>

class IUPnPStack;

enum AVTransportState
{
    STOPPED = 0, PLAYING = 1, PAUSED_PLAYBACK = 2, TRANSITIONING = 3, NO_MEDIA_PRESENT = 4
};

enum AVTransportStatus
{
    OK = 0, ERROR_OCCURRED = 1
};

class AVTransportService : public QObject,public IUPnPService, public INotifiableService
{
    Q_OBJECT
    Q_INTERFACES(INotifiableService)

private:
    IUPnPStack *_stack;

    const QString _type;
    const QString _id;

    AVTransportState _state;
    AVTransportStatus _status;
    QMediaPlayer::MediaStatus _lastStatus;

    ExtendedMediaPlayer _player;
    QMediaPlaylist _playlist;
    QDomDocument *_description;

    QStringList _metadataBuffer;

    QMap<QString, QString> _resultBuffer;
    QHash<QString, QString> _metadataForPlayer;

    QHash<QString, QString> _lastChangedNotifications;

    void eventedValueChanged(const QString &name, const QString &value);
    void sendLastChangedEvent();
    void sendLastChangedEventNow();

    Failable<bool>
        handleSetAVTransportUri(const QHash<QString, QString> &arguments, QMap<QString, QString> &results);

    Failable<bool>
        handleSetNextAVTransportUri(const QHash<QString, QString> &arguments, QMap<QString, QString> &results);

    Failable<bool>
        handleGetMediaInfo( QMap<QString, QString> &results
                          ) const;

    Failable<bool>
        handleGetTransportInfo( QMap<QString, QString> &results
                              ) const;

    Failable<bool>
        handleGetPositionInfo( const QHash<QString, QString> &arguments
                             , QMap<QString, QString> &results
                             ) const;

    Failable<bool>
        handleGetDeviceCapablities( const QHash<QString, QString> &arguments
                                  , QMap<QString, QString> &results
                                  ) const;

    Failable<bool>
        handleGetTransportSettings( const QHash<QString, QString> &arguments
                                  , QMap<QString, QString> &results
                                  ) const;

    Failable<bool> handleNext(const QHash<QString, QString> &arguments);
    Failable<bool> handlePrevious(const QHash<QString, QString> &arguments);

    Failable<bool> handleSeek(const QHash<QString, QString> &arguments);


    void changeState(AVTransportState state);
    void changeStatus(AVTransportStatus status);

    void stop();
    void play(QMap<QString, QString> &results);
    void pause(QMap<QString, QString> &results);

    void printDebugState();

    void notifyChatListeners(const QString &message);
    bool invalidMedia(QMediaPlayer::MediaStatus status);
    bool playedMedia(QMediaPlayer::MediaStatus status);
    bool maySendLastChangeEventSetSend();
    bool sentEvent;
    QMutex sentEventMutex;

public:
    AVTransportService(IUPnPStack * const stack);
    ~AVTransportService();

    virtual QMap<QString, QString> handleSOAP(const QString &actionName, const QHash<QString, QString> &arguments);

    virtual const QDomDocument &getServiceDescription() const;
    virtual const QString getServiceId() const;
    virtual const QString getServiceType() const;

    virtual const QUrl getScdpPath() const;
    virtual const QUrl getControlUrl() const;
    virtual const QUrl getEventUrl() const;

    virtual const QStringList getEventedVariableNames() const;
    virtual const QMap<QString,QString> getInitialEventVariables() const;

    inline ExtendedMediaPlayer &getMediaPlayer() { return _player; }

 signals:
    void playInvoked();
    void eventSent(EventMessage eventMessage);

 private slots:
    void playerStateChanged(QMediaPlayer::State state);
    void currentMediaChanged(const QMediaContent& media);
    void playerPositionChanged(qint64 position);
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void onEventSent(EventMessage eventMessage);
    void periodElapsedFromEventSent();
};

#endif // AVTRANSPORTSERVICE_H
