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

#include "avtransportservice.h"

#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>

#include <UcaStack/ucautilities.h>
#include <UcaStack/iupnpstack.h>
#include <UcaStack/ucastack.h>

#include <QStringBuilder>

#include "avtscdptemplate.h"
#include "renderingcontrolutilities.h"

static const char *SERVICE_ID = "urn:upnp-org:serviceId:AVTransport";
static const char *SERVICE_TYPE = "urn:schemas-upnp-org:service:AVTransport:1";

static const char *LAST_CHANGED_VARNAME = "LastChange";

static unsigned int INVALID_ID_CODE = 718;
static unsigned int INVALID_ARGS_CODE = 402;

static const char *TRANSPORT_STATE_STRINGS[]
    = { "STOPPED"
      , "PLAYING"
      , "PAUSED_PLAYBACK"
      , "TRANSITIONING"
      , "NO_MEDIA_PRESENT"
      };

static inline const char *transportStateToString(const AVTransportState value)
{
    return TRANSPORT_STATE_STRINGS[(int)value];
}

static const char *TRANSPORT_STATUS_STRINGS[]
    = { "OK"
      , "ERROR_OCCURRED"
      };

static inline const char *transportStatusToString(const AVTransportStatus value)
{
    return TRANSPORT_STATUS_STRINGS[(int)value];
}

static QDomDocument *buildDescription()
{
    QString scdpTempalte(SCDP_TEMPLATE);
    scdpTempalte = scdpTempalte.replace("$MAX_TRACK_NUMBER", QString::number(20));

    QDomDocument *description = new QDomDocument("scpd");
    description->setContent(scdpTempalte);

    return description;
}

AVTransportService::AVTransportService(IUPnPStack * const stack)
    : QObject(),
      _stack(stack)
    , _type(SERVICE_TYPE)
    , _id(SERVICE_ID)
    , _state(NO_MEDIA_PRESENT)
    , _status(OK)
{
    _player.setPlaylist(&_playlist);
    _playlist.setPlaybackMode(QMediaPlaylist::Sequential);
    connect(&_player,SIGNAL(stateChanged(QMediaPlayer::State)),this,SLOT(playerStateChanged(QMediaPlayer::State)));
    connect(&_player,SIGNAL(currentMediaChanged(const QMediaContent&)),this,SLOT(currentMediaChanged(const QMediaContent&)));
    _description = buildDescription();

    connect(&_eventTimer, SIGNAL(timeout()), this, SLOT(eventTimerTick()));
    _eventTimer.start(20);

    connect(&_player, SIGNAL(positionChanged(qint64)), this, SLOT(playerPositionChanged(qint64)));
}

AVTransportService::~AVTransportService()
{
    delete _description;
}

const QDomDocument &AVTransportService::getServiceDescription() const
{
    return *_description;
}

const QString AVTransportService::getServiceId() const
{
    return _id;
}

const QString AVTransportService::getServiceType() const
{
    return _type;
}

const QUrl AVTransportService::getScdpPath() const
{
    return QUrl("/avts");
}

const QUrl AVTransportService::getControlUrl() const
{
    return QUrl("/avtsControl");
}

const QUrl AVTransportService::getEventUrl() const
{
    return QUrl("/avtsEventing");
}

const QStringList AVTransportService::getEventedVariableNames() const
{
    QStringList variables;
    variables << LAST_CHANGED_VARNAME;
    return variables;
}

void AVTransportService::notifyChatListeners(const QString &message)
{
    UcaStack *stack = static_cast<UcaStack *>(_stack);
    if (stack != NULL) {
        stack->sendToChatListeneres(message);
    }
}

static bool checkInstanceID( const QHash<QString, QString> &arguments
                           , QMap<QString, QString> &results
                           )
{
    if (arguments.contains("InstanceID") == false) {
        QString message("Invalid SetAVTransportUri invocation: missing InstanceID");
        utilities::fillWithErrorMessage(results, INVALID_ARGS_CODE, message);
        return false;
    }

    QString id = arguments["InstanceID"];
    if (id != "0") {
        QString message
                = QString("Invalid InstanceID: '%1', expected '0'.").arg(id);
        utilities::fillWithErrorMessage(results, INVALID_ID_CODE, message);
        return false;
    }

    return true;
}

static QString getTrackNameFormMetadata(const QString &metaData)
{
    QDomDocument document;
    document.setContent(metaData);

    QString title("unknown");

    const QDomNodeList nodes = document.elementsByTagName("dc:title");
    if (nodes.size() > 0) {
        const QDomElement titleTag = nodes.at(0).toElement();
        title = titleTag.text();
    }

    return title;
}

static QString createAVTransportUriChangeMessage
    ( const QString &trackName
    , const QString &controllerName
    )
{
    QString message = QString("Track has been changed to '%2' by %1.")
                        .arg(controllerName, trackName);
    return message;
}

Failable<bool>
    AVTransportService::handleSetAVTransportUri( const QHash<QString, QString> &arguments
                                               , QMap<QString, QString> &results
                                               ) {
    QStringList requiredArgs;
    requiredArgs << "InstanceID" << "CurrentURI" << "CurrentURIMetaData";

    Failable<bool> argCheck = utilities::checkRequiredArguments(requiredArgs, arguments);
    if (argCheck.hasValue() == false) {
        QString message
            = QString("Invalid SetAVTransportUri invocation: %1")
                  .arg(argCheck.message());
        utilities::fillWithErrorMessage(results, INVALID_ARGS_CODE, message);

        return true;
    }

    QString metadata = arguments["CurrentURIMetaData"].trimmed();

    QUrl url = QUrl(arguments["CurrentURI"].trimmed());
    if (url.isValid() == false) {
        QString message("Invalid URI");
        utilities::fillWithErrorMessage(results, 716, message);

        return true;
    }

    QString id = arguments["InstanceID"].trimmed();
    if (id != "0") {
        QString message
                = QString("Invalid InstanceID: '%1', expected '0'.").arg(id);
        utilities::fillWithErrorMessage(results, INVALID_ID_CODE, message);

        return true;
    }

    bool wasPlaying = _state == PLAYING;

    stop();

    _metadataForPlayer.clear();
    _metadataForPlayer.insert(url.toString(), metadata);

    _playlist.clear();
    _playlist.addMedia(url);
    _playlist.setCurrentIndex(0);

    eventedValueChanged("NumberOfTracks", QString::number(1));

    if (wasPlaying) play();

    const QString trackName = getTrackNameFormMetadata(metadata);
    const QString controllerName = arguments[ARGKEY_SENDER_NAME];
    const QString message
            = createAVTransportUriChangeMessage(trackName, controllerName);
    notifyChatListeners(message);

    return true;
}

Failable<bool>
    AVTransportService::handleSetNextAVTransportUri( const QHash<QString, QString> &arguments
                                                   , QMap<QString, QString> &results
                                                   ) {
    QStringList requiredArgs;
    requiredArgs << "InstanceID" << "NextURI" << "NextURIMetaData";

    Failable<bool> argCheck = utilities::checkRequiredArguments(requiredArgs, arguments);
    if (argCheck.hasValue() == false) {
        QString message
            = QString("Invalid SetAVTransportUri invocation: %1")
                  .arg(argCheck.message());
        utilities::fillWithErrorMessage(results, INVALID_ARGS_CODE, message);

        return true;
    }

    QString metadata = arguments["NextURIMetaData"].trimmed();

    QUrl url = QUrl(arguments["NextURI"]);
    if (url.isValid() == false) {
        QString message("Invalid URI");
        utilities::fillWithErrorMessage(results, 716, message);

        return true;
    }

    QString id = arguments["InstanceID"].trimmed();
    if (id != "0") {
        QString message
                = QString("Invalid InstanceID: '%1', expected '0'.").arg(id);
        utilities::fillWithErrorMessage(results, INVALID_ID_CODE, message);

        return true;
    }

    _metadataForPlayer.insert(url.toString(), metadata);
    _playlist.addMedia(url);
    eventedValueChanged("NumberOfTracks", QString::number(_playlist.mediaCount()));

    return true;
}

static QString formatMiliseconds(const unsigned long totalMiliseconds)
{
    const int totalSeconds = totalMiliseconds / 1000;
    const int totalMinutes = totalSeconds / 60;
    const int totalHours = totalMinutes / 60;

    const int minutes = totalMinutes - totalHours * 60;
    const int seconds = totalSeconds - totalMinutes * 60;

    const QChar zero('0');

    return QString("%1:%2:%3").arg(totalHours, 0, 10, zero)
                .arg(minutes, 2, 10, zero).arg(seconds, 2, 10, zero);
}

static QString getCurrentMediaDuration(const QMediaPlayer &player)
{
    QString mediaDuration("0:00:00");
    if (player.state() == QMediaPlayer::PlayingState) {
        mediaDuration = formatMiliseconds(player.duration());
    }
    return mediaDuration;
}

static QString getCurrentMediaPosition(const QMediaPlayer &player)
{
    QString mediaDuration("0:00:00");
    if (player.state() == QMediaPlayer::PlayingState) {
        mediaDuration = formatMiliseconds(player.position());
    }
    return mediaDuration;
}

Failable<bool>
    AVTransportService::handleGetMediaInfo( QMap<QString, QString> &results
                                          ) const {
    QString nrTracks("0");
    QString mediaDuration("0:00:00");
    QString currentUri("");
    QString currentUriMetadata("");
    QString nextUri("");
    QString nextUriMetadata("NOT_IMPLEMENTED");
    QString playMedium("NONE");
    QString recordMedium("NOT_IMPLEMENTED");
    QString writeStatus("NOT_IMPLEMENTED");

    if (_playlist.isEmpty() == false) {
        nrTracks = QString::number(_playlist.mediaCount());
        mediaDuration = getCurrentMediaDuration(_player);
        currentUri = _playlist.currentMedia().canonicalUrl().toString();
        QString metadata = _metadataForPlayer.value(currentUri);
        if(metadata!=NULL){
            currentUriMetadata = metadata;
        }
        if (_playlist.currentIndex() + 1 < _playlist.mediaCount()) {
            nextUri = _playlist.media(_playlist.nextIndex()).canonicalUrl().toString();
        }
        playMedium = "NETWORK";
    }

    results["NrTracks"] = nrTracks;
    results["MediaDuration"] = mediaDuration;
    results["CurrentURI"] = currentUri.toHtmlEscaped();
    results["CurrentURIMetadata"] = currentUriMetadata.toHtmlEscaped();
    results["NextURI"] = nextUri.toHtmlEscaped();
    results["NextURIMetadata"] = nextUriMetadata.toHtmlEscaped();
    results["PlayMedium"] = playMedium;
    results["RecordMedium"] = recordMedium;
    results["WriteStatus"] = writeStatus;

    return true;
}

Failable<bool>
    AVTransportService::handleGetTransportInfo( QMap<QString, QString> &results
                                              ) const {
    int playspeed = 1;

    results["CurrentTransportState"] = transportStateToString(_state);
    results["CurrentTransportStatus"] = transportStatusToString(_status);
    results["CurrentSpeed"] = QString::number(playspeed);

    return true;
}

Failable<bool>
    AVTransportService::handleGetPositionInfo( const QHash<QString, QString> &arguments
                                             , QMap<QString, QString> &results
                                             ) const {
    Q_UNUSED(arguments)

    QString trackDuration("00:00:00");
    QString trackMetadata("");
    QString trackUri("");
    QString relTime("00:00:00");
    QString absTime("00:00:00");
    QString relCount("0");
    QString absCount("0");

    if (_playlist.isEmpty() == false) {
        trackDuration = getCurrentMediaDuration(_player);
        trackUri = _playlist.currentMedia().canonicalUrl().toString();
        relTime = getCurrentMediaPosition(_player);
        absTime = relTime;
        relCount = QString::number(_player.position());
        absCount = relCount;
    }

    results["TrackDuration"] = trackDuration;
    results["TrackMetaData"] = trackMetadata;
    results["TrackURI"] = trackUri;
    results["RelTime"] = relTime;
    results["AbsTime"] = absTime;
    results["RelCount"] = relCount;
    results["AbsCount"] = absCount;

    return true;
}

Failable<bool>
    AVTransportService::handleGetDeviceCapablities( const QHash<QString, QString> &arguments
                                                  , QMap<QString, QString> &results
                                                  ) const {
    Q_UNUSED(arguments)

    results["PlayMedia"] = "NETWORK";
    results["RecMedia"] = "NOT_IMPLEMENTED";
    results["RecQuailityModes"] = "NOT_IMPLEMENTED";

    return true;
}

Failable<bool>
    AVTransportService::handleGetTransportSettings( const QHash<QString, QString> &arguments
                                                  , QMap<QString, QString> &results
                                                  ) const {
    Q_UNUSED(arguments)

    results["PlayMode"] = "NORMAL";
    results["RecQualityMode"] = "NOT_IMPLEMENTED";

    return true;
}

Failable<bool>
    AVTransportService::handleNext(const QHash<QString, QString> &arguments)
{
    Q_UNUSED(arguments)

    int currentIndex = _playlist.currentIndex();
    int totalCount = _playlist.mediaCount();
    if (currentIndex + 1 >= totalCount) {
        return true;
    }

    _playlist.setCurrentIndex(currentIndex + 1);

    return true;
}

Failable<bool>
    AVTransportService::handlePrevious(const QHash<QString, QString> &arguments)
{
    Q_UNUSED(arguments)

    int currentIndex = _playlist.currentIndex();
    if (currentIndex - 1 < 0) {
        return true;
    }

    _playlist.setCurrentIndex(currentIndex + 1);

    return true;
}

Failable<bool>
    AVTransportService::handleSeek(const QHash<QString, QString> &arguments)
{
    QStringList requiredArgs;
    requiredArgs << "InstanceID" << "Unit" << "Target";
    Failable<bool> argCheck = utilities::checkRequiredArguments(requiredArgs, arguments);
    if(argCheck.hasValue() == false){
        return Failable<bool>::Propagate("Invalid SetVolume invocation", argCheck);
    }

    QString unit = arguments["Unit"];
    if (unit != "TRACK_NR") {
        QString message
            = QString("Unsupported SeekMode: '%1' expected: 'TRACK_NR'").arg(unit);
        return Failable<bool>::Failure(message);
    }

    int maxTarget = _playlist.mediaCount() - 1;

    QString target = arguments["Target"];
    bool correntInt = false;
    int targetValue = target.toInt(&correntInt);
    if (correntInt == false || targetValue < 0 || targetValue > maxTarget) {
        QString message = QString("Invalid value of Target: '%1'.").arg(target);
        return Failable<bool>::Failure(message);
    }

    _playlist.setCurrentIndex(targetValue);

    return true;
}

void AVTransportService::changeState(AVTransportState state)
{
    _state = state;
    eventedValueChanged("TransportState", transportStateToString(_state));
}

void AVTransportService::changeStatus(AVTransportStatus status)
{
    _status = status;
}

void AVTransportService::play()
{
    if (_playlist.isEmpty())
        return;

    _player.play();
}

void AVTransportService::pause()
{
    _player.pause();
}

void AVTransportService::stop()
{
    _player.stop();

}

void AVTransportService::printDebugState()
{
    qDebug() << "Number of items on the playlist: " << _playlist.mediaCount();
    qDebug() << "Current playlist index: " << _playlist.currentIndex();
}

QMap<QString, QString>
    AVTransportService::handleSOAP( const QString &actionName
                                  , const QHash<QString, QString> &arguments
                                  ) {
    _resultBuffer.clear();

    bool instanceIdCorrect = checkInstanceID(arguments, _resultBuffer);


    if (actionName == "Play") {
        if(instanceIdCorrect){ play(); }
    } else if (actionName == "Pause") {
        if(instanceIdCorrect){pause();}
    } else if (actionName == "Stop") {
        if(instanceIdCorrect){stop();}
    } else if (actionName == "SetAVTransportURI") {
        if(instanceIdCorrect){handleSetAVTransportUri(arguments, _resultBuffer);}
    } else if (actionName == "SetNextAVTransportURI") {
        if(instanceIdCorrect){handleSetNextAVTransportUri(arguments, _resultBuffer);}
    } else if (actionName == "GetMediaInfo") {
       if(instanceIdCorrect){ handleGetMediaInfo(_resultBuffer);}
    } else if (actionName == "GetTransportInfo") {
        if(instanceIdCorrect){handleGetTransportInfo(_resultBuffer);}
    } else if (actionName == "GetPositionInfo") {
        if(instanceIdCorrect){handleGetPositionInfo(arguments, _resultBuffer);}
    } else if (actionName == "GetDeviceCapabilities") {
       if(instanceIdCorrect){ handleGetDeviceCapablities(arguments, _resultBuffer);}
    } else if (actionName == "GetTransportSettings") {
        if(instanceIdCorrect){handleGetTransportSettings(arguments, _resultBuffer);}
    } else if (actionName == "Seek") {
        if(instanceIdCorrect){handleSeek(arguments);}
    } else if (actionName == "Previous") {
       if(instanceIdCorrect){ handlePrevious(arguments);}
    } else if (actionName == "Next") {
        if(instanceIdCorrect){handleNext(arguments);}
    }else{
        _resultBuffer["__errorCode"] = "401";
        _resultBuffer["__errorMessage"] = "Invalid action";
    }


    printDebugState();

    return _resultBuffer;
}
void AVTransportService::playerStateChanged(QMediaPlayer::State state){
    qDebug()<< "Player State Changed "<<state;

    if(state==QMediaPlayer::StoppedState){
        changeState(STOPPED);
    } else if(state==QMediaPlayer::PlayingState){
        changeState(PLAYING);
    } else if(state == QMediaPlayer::PausedState){
        changeState(PAUSED_PLAYBACK);
    }
}

void AVTransportService::sendLastChangedEvent()
{
    if (_lastChangedNotifications.isEmpty()) return;

    QString message
        = createLastChangedEventValueAVT(_lastChangedNotifications);

    if (_stack != NULL) {
        qDebug() << "Sending LAST_CHANGED event!";
        _stack->sendEvent(_id, NULL, LAST_CHANGED_VARNAME, message);
    }

    _lastChangedNotifications.clear();
}

void AVTransportService::playerPositionChanged(qint64 position)
{
    QString timePosition = formatMiliseconds(position);
    QString counterPosition = QString::number(position);

    eventedValueChanged("RelativeTimePosition", timePosition);
    eventedValueChanged("AbsoluteTimePosition", timePosition);
    eventedValueChanged("RelativeCounterPosition", counterPosition);
    eventedValueChanged("AbsoluteCounterPosition", counterPosition);
}

void AVTransportService::eventTimerTick()
{
    sendLastChangedEvent();
    _eventTimer.start(5000); /* 5 s */
}

void AVTransportService::eventedValueChanged( const QString &name
                                            , const QString &value
                                            ) {
    _lastChangedNotifications[name] = value;
}

void AVTransportService::currentMediaChanged(const QMediaContent &media){
    QString trackUri = media.canonicalUrl().toString();
    QString metadata = _metadataForPlayer.value(trackUri);
    QString trackDuration = getCurrentMediaPosition(_player);

    eventedValueChanged("AVTransportURI", trackUri);
    eventedValueChanged("AVTransportURIMetaData", metadata);
    eventedValueChanged("CurrentTrackURI", trackUri);
    eventedValueChanged("CurrentTrackMetaData", metadata);
    eventedValueChanged("CurrentTrackDuration", trackDuration);
    eventedValueChanged("CurrentMediaDuration", trackDuration);
    eventedValueChanged("CurrentTrack", QString::number(_playlist.currentIndex()));
}
