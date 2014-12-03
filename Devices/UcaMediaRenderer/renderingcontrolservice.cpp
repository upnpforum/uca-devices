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

#include "renderingcontrolservice.h"

#include <QMediaPlayer>

#include <UcaStack/ucautilities.h>

#include "rcscdptemplate.h"
#include "renderingcontrolutilities.h"

static unsigned int INVALID_ARGS_CODE = 402;
static unsigned int INVALID_ID_CODE = 718;

static const char *SERVICE_ID = "urn:upnp-org:serviceId:RenderingControl";
static const char *SERVICE_TYPE = "urn:schemas-upnp-org:service:RenderingControl:1";

static const char *LAST_CHANGED_VARNAME = "LastChange";

static QDomDocument *buildDescription()
{
    QString scdpTempalte(SCDP_TEMPLATE);
    QDomDocument *description = new QDomDocument("scpd");
    description->setContent(scdpTempalte);

    return description;
}

RenderingControlService::RenderingControlService(QMediaPlayer *player, IUPnPStack * stack)
    : _stack(stack)
    , _type(SERVICE_TYPE)
    , _id(SERVICE_ID)
    , _player(player)
{
    _description = buildDescription();
}

RenderingControlService::~RenderingControlService()
{
    delete _description;
}

void RenderingControlService::handleListPresets( const QHash<QString, QString> &arguments
                                               , QMap<QString, QString> &results
                                               ) const {
    QStringList requiredArgs;
    requiredArgs << "InstanceID";
    Failable<bool> argCheck = utilities::checkRequiredArguments(requiredArgs,arguments);
    if(argCheck.hasValue() == false){
        QString message("Invalid ListPresets invocation: missing InstanceID");
        utilities::fillWithErrorMessage(results, INVALID_ARGS_CODE, message);
        return;
    }else if(arguments["InstanceID"]!="0"){
        QString message = QString("Invalid InstanceID: '%1', expected '0'.").arg(arguments["InstanceID"]);
        utilities::fillWithErrorMessage(results, INVALID_ID_CODE, message);
        return;
    }

    results["CurrentPresetNameList"] = "FactoryDefaults";

}

void RenderingControlService::handleSelectPreset( const QHash<QString, QString> &arguments
                                                , QMap<QString, QString> &results
                                                ) const {

    QStringList requiredArgs;
    requiredArgs << "InstanceID" << "PresetName";
    Failable<bool> argCheck = utilities::checkRequiredArguments(requiredArgs,arguments);
    if(argCheck.hasValue() == false){
        QString message("Invalid SelectPreset invocation: missing InstanceID or PresetName");
        utilities::fillWithErrorMessage(results, INVALID_ARGS_CODE, message);
        return;
    }else if(arguments["InstanceID"]!="0"){
        QString message = QString("Invalid InstanceID: '%1', expected '0'.").arg(arguments["InstanceID"]);
        utilities::fillWithErrorMessage(results, INVALID_ID_CODE, message);
        return;
    }
}

void RenderingControlService::handleGetVolume(const QHash<QString, QString> &arguments,QMap<QString, QString> &result)
{
    QStringList requiredArgs;
    requiredArgs << "InstanceID" <<"Channel";
    Failable<bool> argCheck = utilities::checkRequiredArguments(requiredArgs,arguments);
    if(argCheck.hasValue() == false){
         QString message("Invalid GetVolume invocation: missing InstanceID or Channel");
        utilities::fillWithErrorMessage(result, INVALID_ARGS_CODE, message);
        return;
    }else if(arguments["InstanceID"]!="0"){
        QString message = QString("Invalid InstanceID: '%1', expected '0'.").arg(arguments["InstanceID"]);
        utilities::fillWithErrorMessage(result, INVALID_ID_CODE, message);
    }

    QString volume("0");
    if (_player != NULL) {
        volume = QString::number(_player->volume());
    }
    result["CurrentVolume"] = volume;
}

void RenderingControlService::handleSetVolume( const QHash<QString, QString> &arguments
                                             , QMap<QString, QString> &result
                                             ) {
    Q_UNUSED(result);

    QStringList requiredArgs;
    requiredArgs << "Channel" << "InstanceID" << "DesiredVolume";
    Failable<bool> argCheck = utilities::checkRequiredArguments(requiredArgs,arguments);
    if(argCheck.hasValue() == false){
        QString message("Invalid SetVolume invocation: missing InstanceID or Channel or DesiredVolume");
        utilities::fillWithErrorMessage(result, INVALID_ARGS_CODE, message);
        return;
        //return Failable<bool>::Propagate("Invalid SetVolume invocation", argCheck);
    }else if(arguments["InstanceID"]!="0"){
        QString message = QString("Invalid InstanceID: '%1', expected '0'.").arg(arguments["InstanceID"]);
        utilities::fillWithErrorMessage(result, INVALID_ID_CODE, message);
    }

    QString desiredVolume = arguments["DesiredVolume"];
    bool isOk;
    int desiredVolumeInt = desiredVolume.toInt(&isOk);
    if(!isOk){
        QString message("Invalid SetVolume invocation: wrong type of DesiredVolume");
        utilities::fillWithErrorMessage(result, INVALID_ARGS_CODE, message);
        return;
        //return Failable<bool>::Failure("Invalid SetVolume DesiredVolume");
    }
    if (_player != NULL) {
	QHash<QString, QString> volumeChange;
	volumeChange["Volume"] = desiredVolume;
        QString message
            = createLastChangedEventValueRCS(volumeChange);
        _player->setVolume(desiredVolumeInt);
        _stack->sendEvent(_id, NULL, LAST_CHANGED_VARNAME, message);
    }
}

QMap<QString, QString>
    RenderingControlService::handleSOAP( const QString &actionName
                                       , const QHash<QString, QString> &arguments
                                       ) {
    _resultBuffer.clear();

    if (actionName == "ListPresets") {
        handleListPresets(arguments, _resultBuffer);
    } else if (actionName == "SelectPreset") {
        handleSelectPreset(arguments, _resultBuffer);
    } else if (actionName == "GetVolume") {
        handleGetVolume(arguments,_resultBuffer);
    } else if (actionName == "SetVolume") {
        handleSetVolume(arguments, _resultBuffer);
    }else{
        _resultBuffer["__errorCode"] = "401";
        _resultBuffer["__errorMessage"] = "Invalid action";
    }

    return _resultBuffer;
}

const QDomDocument &RenderingControlService::getServiceDescription() const
{
    return *_description;
}

const QString RenderingControlService::getServiceId() const
{
    return _id;
}

const QString RenderingControlService::getServiceType() const
{
    return _type;
}

const QUrl RenderingControlService::getScdpPath() const
{
    return QUrl("/rcs");
}

const QUrl RenderingControlService::getControlUrl() const
{
    return QUrl("/rcsControl");
}

const QUrl RenderingControlService::getEventUrl() const
{
    return QUrl("/rcsEventing");
}

const QStringList RenderingControlService::getEventedVariableNames() const
{
    QStringList variables;
    variables << "LastChange";
    return variables;
}
