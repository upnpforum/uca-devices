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

#include "connectionmanagerservice.h"

#include "cmscdptemplate.h"
#include <UcaStack/ucautilities.h>

static const char *SERVICE_ID = "urn:upnp-org:serviceId:ConnectionManager";
static const char *SERVICE_TYPE = "urn:schemas-upnp-org:service:ConnectionManager:1";

static QDomDocument *buildDescription()
{
    QString scdpTempalte(SCDP_TEMPLATE);
    QDomDocument *description = new QDomDocument("scpd");
    description->setContent(scdpTempalte);

    return description;
}

ConnectionManagerService::ConnectionManagerService()
    : _type(SERVICE_TYPE)
    , _id(SERVICE_ID)
{
    _description = buildDescription();
}

ConnectionManagerService::~ConnectionManagerService()
{
   delete _description;
}

void ConnectionManagerService::
        handleGetProtocolInfo(QMap<QString, QString> &results)
{
    results["Source"] = "";
    results["Sink"] = "http-get:*:video/webm:*";
}

void ConnectionManagerService::
        handleGetCurrentConnectionIDs(QMap<QString, QString> &results)
{
    results["ConnectionIDs"] = "0";
}

void ConnectionManagerService::
        handleGetCurrentConnectionInfo(const QHash<QString,QString> &arguments,QMap<QString, QString> &results)
{

    QStringList requiredArgs;
    requiredArgs << "ConnectionID";
    Failable<bool> argCheck = utilities::checkRequiredArguments(requiredArgs,arguments);
    if(argCheck.hasValue() == false){
        QString message("Invalid GetCurrentConnectionInfo invocation: missing ConnectionID");
        utilities::fillWithErrorMessage(results, 402, message);
        return;
    }else if(arguments["ConnectionID"]!="0"){
        QString message("Invalid GetCurrentConnectionInfo invocation: invalid ConnectionID reference");
        utilities::fillWithErrorMessage(results, 706, message);
    }

    results["RcsID"] = "0";
    results["AVTransportID"] = "0";
    results["ProtocolInfo"] = "";
    results["PeerConnectionManager"] = "";
    results["PeerConnectionID"] = "-1";
    results["Direction"] = "Input";
    results["Status"] = "OK";
}

QMap<QString, QString>
    ConnectionManagerService::handleSOAP( const QString &actionName
                                        , const QHash<QString, QString> &arguments
                                        ) {
    Q_UNUSED(actionName)
    Q_UNUSED(arguments)

    _resultBuffer.clear();

    if (actionName == "GetProtocolInfo") {
        handleGetProtocolInfo(_resultBuffer);
    } else if (actionName == "GetCurrentConnectionInfo") {
        handleGetCurrentConnectionInfo(arguments,_resultBuffer);
    } else if (actionName == "GetCurrentConnectionIDs") {
        handleGetCurrentConnectionIDs(_resultBuffer);
    }else{
        _resultBuffer["__errorCode"] = "401";
        _resultBuffer["__errorMessage"] = "Invalid action";
    }

    return _resultBuffer;
}

const QDomDocument &ConnectionManagerService::getServiceDescription() const
{
    return *_description;
}

const QString ConnectionManagerService::getServiceId() const
{
    return _id;
}

const QString ConnectionManagerService::getServiceType() const
{
    return _type;
}

const QUrl ConnectionManagerService::getScdpPath() const
{
    return QUrl("/cms");
}

const QUrl ConnectionManagerService::getControlUrl() const
{
    return QUrl("/cmsControl");
}

const QUrl ConnectionManagerService::getEventUrl() const
{
    return QUrl("/cmsEvent");
}

const QStringList ConnectionManagerService::getEventedVariableNames() const
{
    QStringList variables;
    return variables;
}
