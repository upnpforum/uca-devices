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

#include "contentdirectoryservice.h"

#include "cdscdptemplate.h"
#include <Stack/ucautilities.h>
#include <QStringList>
#include <QStringBuilder>
#include <QFile>
#include <QDir>
#include <QString>

#include <QDebug>

static const char *SERVICE_ID = "urn:upnp-org:serviceId:ContentDirectory";
static const char *SERVICE_TYPE = "urn:schemas-upnp-org:service:ContentDirectory:1";

static const char *TRANSFERIDS_VARNAME = "TransferIDs";
static const char *SYSTEMUPDATEID_VARNAME = "SystemUpdateID";
static const char *CONTAINERUPDATEIDS_VARNAME = "ContainerUpdateIDs";

static const QByteArray readXmlFile()
{
    qDebug() << "reading xml file";

    QString browseResponsePath = QDir::homePath() + "/.UcaMediaServer/browseResponse.xml";
    QFile file(browseResponsePath);
    QByteArray xml;
    if(!file.open(QIODevice::ReadOnly)){
        qDebug() << "error " << file.errorString();
    }
    xml = file.readAll();
    file.close();
    return xml;
}

static QDomDocument *buildDescription()
{
    QString scdpTempalte(CD_SCDP_TEMPLATE);
    QDomDocument *description = new QDomDocument("scpd");
    description->setContent(scdpTempalte);

    return description;

}

ContentDirectoryService::ContentDirectoryService()
    : _type(SERVICE_TYPE)
    , _id(SERVICE_ID)
    , _updateId(0)
{
    _description = buildDescription();
}

ContentDirectoryService::~ContentDirectoryService()
{
    delete _description;
}

const QDomDocument &ContentDirectoryService::getServiceDescription() const
{
    return *_description;
}

const QString ContentDirectoryService::getServiceId() const
{
    return _id;
}

const QString ContentDirectoryService::getServiceType() const
{
    return _type;
}

const QUrl ContentDirectoryService::getScdpPath() const
{
    return QUrl("/cds");
}

const QUrl ContentDirectoryService::getControlUrl() const
{
    return QUrl("/cdsControl");
}

const QUrl ContentDirectoryService::getEventUrl() const
{
    return QUrl("/cdsEventing");
}

const QStringList ContentDirectoryService::getEventedVariableNames() const
{
    QStringList variables;
    variables << TRANSFERIDS_VARNAME;
    variables << SYSTEMUPDATEID_VARNAME;
    variables << CONTAINERUPDATEIDS_VARNAME;
    return variables;
}

const QMap<QString,QString> ContentDirectoryService::getInitialEventVariables() const
{
    QMap<QString,QString> variables;
    variables.insert(SYSTEMUPDATEID_VARNAME,"0");
    variables.insert(TRANSFERIDS_VARNAME,"0");
    variables.insert(CONTAINERUPDATEIDS_VARNAME,"0");
    return variables;
}

QMap<QString, QString>
    ContentDirectoryService::handleSOAP( const QString &actionName
                                       , const QHash<QString, QString> &arguments
                                       ) {
    _resultBuffer.clear();

    Failable<bool> result(true);

    qDebug() << "Action name " <<actionName;
    if (actionName == "Browse") {
        result = handleBrowse(arguments, _resultBuffer);
    } else if (actionName == "GetSearchCapabilities") {
        handleGetSearchCapabilities(_resultBuffer);
    } else if (actionName == "GetSortCapabilities") {
        handleGetSortCapabilities(_resultBuffer);
    } else if (actionName == "GetSystemUpdateID") {
        handleGetSystemUpdate(_resultBuffer);
    }else{
        _resultBuffer["__errorCode"] = "401";
        _resultBuffer["__errorMessage"] = "Invalid action";
    }

    if (result.hasValue() == false) {
        if(!_resultBuffer.contains("__errorCode")){
            /* TODO: should be somehow replaced with error specific codes */
            _resultBuffer["__errorCode"] = "501";
            _resultBuffer["__errorMessage"] = result.message();
        }
    }

    return _resultBuffer;
}

void ContentDirectoryService::handleGetSearchCapabilities
                                  (QMap<QString, QString> &results)
{
    results["SearchCaps"] = "";
}

void ContentDirectoryService::handleGetSortCapabilities
                                  (QMap<QString, QString> &results)
{
    results["SortCaps"] = "";
}

void ContentDirectoryService::handleGetSystemUpdate
                                  (QMap<QString, QString> &results)
{
    results["Id"] = "0";
}

Failable<bool> ContentDirectoryService::handleBrowseDirectChildren( const QHash<QString, QString> &arguments
                                                                    , QMap<QString, QString> &results
                                                                    ){
    QString objectId = arguments["ObjectID"];
    QString filter = arguments["Filter"];

    if (objectId != "0") {
        QString message("Unknown ObjectID.");
        results["__errorCode"] = QString::number(701);
        results["__errorMessage"] = message;
        return Failable<bool>::Failure(message);
    }

    if (filter != "*") {
        QString message = QString("Unsupported value of 'Filter': %1").arg(filter);
        results["__errorCode"] = QString::number(709);
        results["__errorMessage"] = message;
        return Failable<bool>::Failure(message);
    }

    QByteArray xml = readXmlFile();
    QDomDocument doc;
    doc.setContent(xml);
    int size = doc.elementsByTagName("item").size();

    results["Result"] = doc.toString().toHtmlEscaped();
    results["NumberReturned"] = QString::number(size);
    results["TotalMatches"] = QString::number(size);
    results["UpdateID"] = QString::number(_updateId);
    return true;
}
static QDomElement elementNamedWithValue(QString name,QString value,QDomDocument doc){
    QDomElement el = doc.createElement(name);
    el.appendChild(doc.createTextNode(value));
    return el;
}
Failable<bool> ContentDirectoryService::handleBrowseMetadata(const QHash<QString, QString> &arguments
                                                             , QMap<QString, QString> &results
                                                             ) {
    QString objectId = arguments["ObjectID"];
    QString filter = arguments["Filter"];

    if (filter != "*") {
        QString message = QString("Unsupported value of 'Filter': %1").arg(filter);
        results["__errorCode"] = QString::number(709);
        results["__errorMessage"] = message;
        return Failable<bool>::Failure(message);
    }

    QByteArray xml = readXmlFile();
    QDomDocument doc;
    doc.setContent(xml);

    QDomNodeList items = doc.documentElement().childNodes();
    int size = items.size();
    bool found = false;
    for(int i=0;i<size;){
        QDomNode node = items.at(i);
        if(node.isElement() && node.toElement().attribute("id") != objectId){
            qDebug() << "found element to remove " << node.toElement().attribute("id");
            doc.documentElement().removeChild(node);
        }else{
            found = true;
            ++i;
        }
    }
    if(objectId == "0"){
        QDomElement container = doc.createElement("container");
        container.setAttribute("id","0");
        container.setAttribute("parentID","-1");
        container.setAttribute("childCount",QString::number(size));
        container.setAttribute("searchable","false");
        container.appendChild(elementNamedWithValue("dc:title","root dir",doc));
        container.appendChild(elementNamedWithValue("upnp:class","object.container.storageFolder",doc));
        container.appendChild(elementNamedWithValue("upnp:writeStatus","NOT_WRITABLE",doc));
        doc.documentElement().appendChild(container);
    }else if(!found){
        QString message("Unknown ObjectID");
        results["__errorCode"] = QString::number(701);
        results["__errorMessage"] = message;
        return Failable<bool>::Failure(message);
    }
    results["Result"] = doc.toString().toHtmlEscaped();
    results["NumberReturned"] = QString::number(1);
    results["TotalMatches"] = QString::number(1);
    results["UpdateID"] = QString::number(_updateId);
    return true;
}


Failable<bool>
    ContentDirectoryService::handleBrowse( const QHash<QString, QString> &arguments
                                         , QMap<QString, QString> &results
                                         ) {
    QStringList requiredArgs;
    requiredArgs << "ObjectID" << "BrowseFlag" << "Filter"; // << "StartingIndex" << "RequestedCount" << "SortCirteria";

    Failable<bool> argCheck = utilities::checkRequiredArguments(requiredArgs, arguments);
    if (argCheck.hasValue() == false) {
        results["__errorCode"] = QString::number(402);
        results["__errorMessage"] = "Invalid or missing arguments";
        return Failable<bool>::Propagate("Invalid Browse invocation", argCheck);
    }

    QString browseFlag = arguments["BrowseFlag"];
    if (browseFlag == "BrowseDirectChildren"){
        return handleBrowseDirectChildren(arguments,results);
    }else if(browseFlag == "BrowseMetadata"){
        return handleBrowseMetadata(arguments,results);
    }else{
        QString message = QString("Unsupported value of 'BrowseFlag': %1").arg(browseFlag);
        results["__errorCode"] = QString::number(402);
        results["__errorMessage"] = message;
        return Failable<bool>::Failure(message);
    }

    return true;
}
