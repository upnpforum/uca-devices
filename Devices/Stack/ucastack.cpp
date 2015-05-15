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

#include "ucastack.h"
#include "iqgrabber.h"

#include <QUuid>
#include <QString>
#include <QDebug>

#include <QSettings>

#include <QtConcurrent/QtConcurrent>

#include <qxmpp/QXmppPresence.h>
#include <qxmpp/QXmppLogger.h>

#include <qxmpp/QXmppIq.h>
#include <qxmpp/QXmppMessage.h>
#include <qxmpp/QXmppPresence.h>
#include <qxmpp/QXmppDiscoveryManager.h>
#include <qxmpp/QXmppClientExtension.h>

const char *ARGKEY_SENDER_JID = "__senderJid";
const char *ARGKEY_SENDER_NAME = "__senderName";

static QString idPrefix;
static ulong idNumber = 1L;
static int generateRandomInteger(int max) {
    Q_ASSERT(max > 0 && max <= RAND_MAX);
    int val;
    while (max <= (val = qrand() / (RAND_MAX/max))) {};
    return val;
}

static QString generateIdPrefix(size_t length)
{
    QByteArray bytes(length, 'm');
    for (size_t i = 0; i < length; i++) {
        bytes[(uint)i] = (char) (65 + generateRandomInteger(15));
    }
    return QString(bytes) + "-";
}

UcaStack::UcaStack(QSettings *settings)
    : _recivedAnyIq(false)
    , _eventing(this)
    , _device(NULL)
    , _isRunning(false)
    , _settings(settings)
{
    QXmppLogger::getLogger()->setLoggingType(QXmppLogger::StdoutLogging);

    const int prefixLength = 4;
    idPrefix = generateIdPrefix(prefixLength);
    qDebug() << "IQ id prefix: " << idPrefix;

}

UcaStack::~UcaStack()
{
    if (_isRunning) {
        stop();
    }
}

static void buildUpnpError
    (QXmppElement &body, const QMap<QString,QString> &results)
{
    QXmppElement upnpError;
    upnpError.setTagName("UPnPError");
    upnpError.setAttribute("xmlns", "urn:schemas-upnp-org:control-1-0");
    body.appendChild(upnpError);

    QXmppElement errorCode;
    errorCode.setTagName("errorCode");
    errorCode.setValue(results["__errorCode"]);
    upnpError.appendChild(errorCode);

    QXmppElement errorDescription;
    errorDescription.setTagName("errorDescription");
    errorDescription.setValue(results["__errorMessage"]);
    upnpError.appendChild(errorDescription);
}

static void buildErrorResponse
    (QXmppElement &body, const QMap<QString, QString> &results)
{
    QXmppElement fault;
    fault.setTagName("s:Fault");
    body.appendChild(fault);

    QXmppElement faultcode;
    faultcode.setTagName("s:faultcode");
    faultcode.setValue("s:Client");
    fault.appendChild(faultcode);

    QXmppElement faultstring;
    faultstring.setTagName("s:faultstring");
    faultstring.setValue("UPnPError");
    fault.appendChild(faultstring);

    QXmppElement detail;
    detail.setTagName("s:detail");
    fault.appendChild(detail);

    buildUpnpError(detail,results);
}

Failable<bool> UcaStack::setupConfig(QXmppConfiguration &config)
{
    if (_device == NULL) {
        return Failable<bool>::Failure("Cannot setup confing when 'device' in NULL.");
    }
    if (_settings == NULL) {
        return Failable<bool>::Failure("Cannot setup confing when 'settings' in NULL.");
    }

    _pubSubServiceName = _settings->value("xmpp/pubsubService", "pubsub.upnpcloud.comarch.com").toString();
    _resourceName = _device->getDeviceType() + ":" + _device->getUdn();

    config.setHost(_settings->value("xmpp/ipAddress","upnpcloud.comarch.com").toString());
    config.setPort(_settings->value("xmpp/port",5222).toInt());

    QString userName = _settings->value("xmpp/userName", "@upnpcloud.comarch.com").toString();
    QString password = _settings->value("xmpp/password", "").toString();

    config.setJid(userName + "/" + _resourceName);
    config.setPassword(password);
    config.setSaslAuthMechanism("SCRAM-SHA-1");
    config.setUseNonSASLAuthentication(false);
    return true;
}

void UcaStack::presenceReceived(const QXmppPresence &incoming)
{
    const QString name = incoming.statusText();
    if (name.isEmpty()) {
        qDebug() << "empty status, ignoring call.";
        return;
    }

    const QString source = incoming.from();
    _clientsNames.insert(source, name);
}

void UcaStack::messageReceived(const QXmppMessage &incoming)
{
    QString response;

    const QString source = incoming.from();
    const QString body = incoming.body();
    if (body.contains("add", Qt::CaseInsensitive)
            && _chatListeners.contains(source) == false) {
        _chatListeners.insert(source);
        response.append("You were successfully added to the listerners list.");
    }

    if (body.contains("remove", Qt::CaseInsensitive)
            && _chatListeners.contains(source)) {
        _chatListeners.remove(source);
        response.append("You were successfully removed from the listerners list.");
    }

    if (response.isEmpty() == false) {
        QXmppMessage outgoing;
        outgoing.setFrom(incoming.to());
        outgoing.setTo(incoming.from());
        outgoing.setBody(response);

        _clientMutex.lock();
        _xmppClient.sendPacket(outgoing);
        _clientMutex.unlock();
    }
}

void UcaStack::sendToChatListeneres(const QString &body)
{
    if (body.isEmpty()) {
        qDebug() << "Empty body, skipping call.";
        return;
    }

    QXmppMessage outgoing;
    outgoing.setBody(body);

    foreach (QString listener, _chatListeners) {
        outgoing.setTo(listener);
        _clientMutex.lock();
        _xmppClient.sendPacket(outgoing);
        _clientMutex.unlock();
    }
}
void handleDiscoInfo
    ( const QXmppIq &incoming
    , QXmppIq &outgoing
    , const QXmppElement &queryElement
    )
{
    Q_UNUSED(incoming)
    Q_UNUSED(queryElement)

    QXmppElement query;
    query.setTagName("query");
    query.setAttribute("xmlns", "http://jabber.org/protocol/disco#info");

    QXmppElement identity;
    identity.setTagName("identity");
    identity.setAttribute("category","automation");
    identity.setAttribute("type","soap");
    query.appendChild(identity);

    QXmppElement feature;
    feature.setTagName("feature");
    feature.setAttribute("var","http://jabber.org/protocol/soap");
    query.appendChild(feature);

    QXmppElementList elements;
    elements.append(query);

    outgoing.setExtensions(elements);
}

void UcaStack::handleDesciptionGet
    ( const QXmppIq &incoming
    , QXmppIq &outgoing
    , const QXmppElement &queryElement
    )
{
    Q_UNUSED(incoming)

    QXmppElement query;
    query.setTagName("query");
    query.setAttribute("xmlns", "urn:schemas-upnp-org:cloud-1-0");
    query.setAttribute("type", "described");
    query.setAttribute("name", _device->getUdn());

    QDomDocument rootDoc = _device->getDescription("");
    QDomElement device = rootDoc.documentElement().firstChildElement("device");
    //QString deviceType = device.firstChildElement("deviceType").text();
    QString UDN = device.firstChildElement("UDN").text();

    QString name = queryElement.attribute("name");
    if(name.isNull() || name.isEmpty() || name != UDN){
        QMap<QString,QString> map;
        map["__errorCode"] = QString::number(900);
        map["__errorMessage"] = "deviceType mismatch";
        buildUpnpError(query,map);
        outgoing.setType(QXmppIq::Result);
    }else{
        QXmppElement root(rootDoc.documentElement());
        query.appendChild(root);

        QList<QString> serviceIds = _device->getServiceIds();
        foreach (QString id, serviceIds) {
            QDomDocument serviceDesc = _device->getDescription(id);
            QXmppElement scdp(serviceDesc.documentElement());
            query.appendChild(scdp);
        }
    }
    QXmppElementList elements;
    elements.append(query);

    outgoing.setExtensions(elements);
}

static Failable<bool> parseSoapMessage
    ( const QXmppElement &envelope
    , QString &actionName
    , QString &serviceType
    , QHash<QString, QString> &arguments
    )
{
    QXmppElement body = envelope.firstChildElement("Body");
    if (   body.isNull()
        || body.tagName().contains("body", Qt::CaseInsensitive) == false) {
        return Failable<bool>::Failure("Missing SOAP invocation body.");
    }

    QXmppElement action = body.firstChildElement();
    if (action.isNull()) {
        return Failable<bool>::Failure("Missing SOAP action element.");
    }

    /* strip namespace prefix if present */
    if (action.tagName().contains(':')) {
        actionName = action.tagName().section(':', 1);
    } else {
        actionName = action.tagName();
    }

    serviceType = QString();
    QList<QString> attributes = action.attributeNames();
    foreach (QString attributeName, attributes) {
        if (attributeName.startsWith("xmlns", Qt::CaseSensitive)) {
            QString maybeType = action.attribute(attributeName);
            if (maybeType.startsWith("urn:schemas-upnp-org:service")) {
                serviceType = maybeType;
                break;
            }
        }
    }

    if (serviceType.isEmpty()) {
        return Failable<bool>::Failure("Failed to find serviceType (action element namespace).");
    }

    QXmppElement argument = action.firstChildElement();
    while (argument.isNull() == false) {
        qDebug() << argument.tagName();
        arguments.insert(argument.tagName(), argument.value());
        argument = argument.nextSiblingElement();
    }

    return true;
}


static void buildResponse(const QMap<QString, QString> &results, QXmppElement &body, const QString &actionName, const QString &serviceType)
{
    QXmppElement action;
    action.setTagName("u:" + actionName + "Response");
    action.setAttribute("xmlns:u", serviceType);
    body.appendChild(action);

    QMap<QString, QString>::const_iterator it = results.constBegin();
    for (; it != results.constEnd(); it++) {
        QXmppElement argument;

        QString key = it.key();
        key = key.replace(QString("_"),QString(""));


        argument.setTagName(key);
        argument.setValue(it.value());
        action.appendChild(argument);
    }
}

static void packSoapResult( const QString &serviceId
                          , const QString &serviceType
                          , const QString &actionName
                          , const QMap<QString, QString> &results
                          , QXmppElementList &outElements
                          )
{
    Q_UNUSED(serviceId) /* TODO: change interface if really not used */

    QXmppElement envelope;
    envelope.setTagName("s:Envelope");
    envelope.setAttribute("xmlns:s", "http://schemas.xmlsoap.org/soap/envelope/");
    envelope.setAttribute("s:encodingStyle", "http://schemas.xmlsoap.org/soap/encoding/");
    envelope.setAttribute("xmlns", "urn:schemas-upnp-org:cloud-1-0");
    outElements.append(envelope);

    QXmppElement body;
    body.setTagName("s:Body");
    envelope.appendChild(body);

    if (results.contains("__errorCode")) {
        buildErrorResponse(body, results);
    } else {
        buildResponse(results, body, actionName, serviceType);
    }
}

void UcaStack::handleActionInvocation( const QXmppIq &incoming
                                     , QXmppIq &outgoing
                                     , const QXmppElement &envelope
                                     )
{
    Q_UNUSED(incoming)

    QString type;
    QString actionName;
    QHash<QString, QString> arguments;

    Failable<bool> result
        = parseSoapMessage(envelope, actionName, type, arguments);
    if (result.hasValue() == false) {
        qDebug() << "Parse stanza failed:" << result.message();
        /* TODO: malformed request, what now? */
        return;
    }

    const QString source = incoming.from();
    QString senderName("unknown");
    if (_clientsNames.contains(source)) {
        senderName = _clientsNames[source];
    }

    arguments.insert(ARGKEY_SENDER_JID, source);
    arguments.insert(ARGKEY_SENDER_NAME, senderName);

    QMap<QString,QString> tmp;
    QMap<QString, QString> &results = tmp;
    const QString &id = _device->serviceIdForType(type);
    if (id.isEmpty()) {
        results["__errorCode"] = QString::number(401);
        results["__errorMessage"] = "InvalidAction";
    }
    else{
        results = _device->handleSOAP(id, NULL, actionName, arguments);
    }

    QXmppElementList response;
    packSoapResult(id, type, actionName, results, response);
    outgoing.setExtensions(response);
}

void UcaStack::dispatchGet(const QXmppIq &incoming, QXmppIq &outgoing)
{
    if (incoming.type() != QXmppIq::Get) return;

    foreach (QXmppElement element, incoming.extensions()) {
        if (element.tagName() == "query") {
            if (element.attribute("type") == "description") {
                handleDesciptionGet(incoming, outgoing, element);
                return;
            } else if (element.attribute("xmlns") == "http://jabber.org/protocol/disco#info"){
                handleDiscoInfo(incoming, outgoing, element);
                return;
            }
        }
    }
}

void UcaStack::dispatchSet(const QXmppIq &incoming, QXmppIq &outgoing)
{
    if (incoming.type() != QXmppIq::Set) return;

    //foreach (QXmppElement element, incoming.extensions()) {
    QXmppElementList elements = incoming.extensions();
    QXmppElementList::const_iterator it = elements.constBegin();
    for (; it != elements.constEnd(); it++) {
        QXmppElement element = *it;

        if (element.tagName().contains("envelope", Qt::CaseInsensitive)) {
            handleActionInvocation(incoming, outgoing, element);
        }
    }
}

void UcaStack::iqReceived(const QXmppIq &incoming)
{
    QXmppIq::Type type = incoming.type();

    //if (_recivedAnyIq == false) {
    //    _recivedAnyIq = true;
    //    QtConcurrent::run(this, &UcaStack::xmppClientConnected);
    //}

    qDebug() << "thread id:" << QObject::thread();
    qDebug() << "Processing iq with id:" << incoming.id();
    if (type == QXmppIq::Result || type == QXmppIq::Error) {
        qDebug() << "Processing result/error iq with id:" << incoming.id();
        receivedResult(incoming.id(), incoming);
    } else {
        handleIq(incoming);
    }
}

void UcaStack::handleIq(const QXmppIq &incoming)
{
    QXmppIq outgoing;

    /* generic handling */
    outgoing.setFrom(incoming.to());
    outgoing.setTo(incoming.from());
    outgoing.setId(incoming.id());
    outgoing.setType(QXmppIq::Result);

    if (incoming.type() == QXmppIq::Get) {
        dispatchGet(incoming, outgoing);
    } else if (incoming.type() == QXmppIq::Set) {
        dispatchSet(incoming, outgoing);
    }

    _clientMutex.lock();
    _xmppClient.sendPacket(outgoing);
    _clientMutex.unlock();
}

void UcaStack::sendEvent(EventMessage eventMessage)
{
    Failable<bool> result = _eventing.sendEventAsync(eventMessage);
    if (result.hasValue() == false) {
        qDebug() << result.message();
    }
}

void UcaStack::registerDevice(IUPnPDevice *device)
{
    _device = device;
}

static QString calcuateDescriptionHash(const IUPnPDevice *device)
{
    QCryptographicHash sha(QCryptographicHash::Sha256);

    QDomDocument ddd = device->getDescription("");
    sha.addData(ddd.toByteArray(-1));

    QStringList serviceIds = device->getServiceIds();
    foreach (QString id, serviceIds) {
        QDomDocument scpd = device->getDescription(id);
        sha.addData(scpd.toByteArray(-1));
    }

    QByteArray result = sha.result();
    return result.toBase64();
}

QString UcaStack::getConfigIdCloud() const {
    return calcuateDescriptionHash(_device);
}

void UcaStack::setupPresence( QXmppPresence &presence
                            , const QString &descHash
                            , const QXmppConfiguration &config
                            )
{
    QXmppPresence::Type type
        = QXmppPresence::Available;
    QXmppPresence::AvailableStatusType status
        = QXmppPresence::Online;
    QString to = config.jidBare();
    QString from = config.jid();
    QString cNode = config.resource();

    presence.setType(type);
    presence.setAvailableStatusType(status);
    presence.setTo(to);
    presence.setFrom(from);

    /* TODO: line below apparently has no effect */
    presence.setCapabilityNode(cNode);
    /* TODO: look into XEP-0115 and make sure,
     * that autogenerated values are ok */
    //presence.setCapabilityHash("");
    //presence.setCapabilityVer("");

    QXmppElement uc;
    uc.setTagName("uc");
    uc.setAttribute("xmlns", "urn:schemas-upnp-org:cloud-1-0");

    QXmppElement configIdCloud;
    configIdCloud.setTagName("configIdCloud");
    configIdCloud.setAttribute("hash", "sha-256");
    configIdCloud.setValue(descHash);
    uc.appendChild(configIdCloud);

    QXmppElementList elements;
    elements.append(uc);

    presence.setExtensions(elements);
}


void UcaStack::connect()
{
    /* reinint config, _observer might have changed */
    Failable<bool> result = setupConfig(_xmppConfig);
    if (result.hasValue() == false) {
        qDebug() << result.message();
        /* TODO: connect should be failable */
        return;
    }

    QString descriptionHash = calcuateDescriptionHash(_device);
    QXmppPresence presence;
    setupPresence(presence, descriptionHash, _xmppConfig);
    printf("description hash: %s\n", descriptionHash.toUtf8().data());

    _clientMutex.lock();
    /* this extensions causes client to skip IQ with types RESULT and ERROR */
    QXmppDiscoveryManager *discoManager
        = _xmppClient.findExtension<QXmppDiscoveryManager>();
    _xmppClient.removeExtension(discoManager);

    IqGrabber *grabber = new IqGrabber(this);
    _xmppClient.insertExtension(0, grabber);

    bool check;
    (void) check;

    check = QObject::connect( &_xmppClient, SIGNAL(presenceReceived(QXmppPresence))
                            , this,         SLOT(presenceReceived(QXmppPresence))
                            );
    Q_ASSERT(check);

    check = QObject::connect( &_xmppClient, SIGNAL(messageReceived(QXmppMessage))
                            , this,         SLOT(messageReceived(QXmppMessage))
                            );
    Q_ASSERT(check);

    check = QObject::connect( &_xmppClient, SIGNAL(iqReceived(QXmppIq))
                            , this,         SLOT(iqReceived(QXmppIq))
                            );
    Q_ASSERT(check);

    check = QObject::connect( &_xmppClient, SIGNAL(connected())
                            , this,         SLOT(xmppClientConnected())
                            );
    Q_ASSERT(check);

    _xmppClient.connectToServer(_xmppConfig, presence);
    _clientMutex.unlock();
}

static QXmppIq createAddStanza(const QString &fullJid, const QString &name)
{
    QXmppIq result;
    result.setType(QXmppIq::Set);
    result.setFrom(fullJid);
    result.setId(UcaStack::createId());

    QXmppElement query;
    query.setTagName("query");
    query.setAttribute("xmlns", "jabber:iq:roster");

    QXmppElement item;
    item.setTagName("item");
    item.setAttribute("jid", fullJid);
    item.setAttribute("name", name);

    query.appendChild(item);

    QXmppElementList extensions;
    extensions.append(query);
    result.setExtensions(extensions);

    return result;
}

void UcaStack::addSelfToRoster()
{
    const QString jid = fullJid();
    const QString name = _device->getFriendlyName();
    QXmppIq addStanza = createAddStanza(jid, name);
    sendStanza(addStanza);
}

void UcaStack::xmppClientConnected()
{
    _isRunning = true;
    _device->stackStatusChanged("uca", true);
    addSelfToRoster();
    _eventing.startEventingCheck();
}

Failable<bool> UcaStack::sendStanza(const QXmppStanza& stanza)
{
    _clientMutex.lock();
    bool result = _xmppClient.sendPacket(stanza);
    _clientMutex.unlock();
    return result;
}

void UcaStack::start()
{
    connect();
}

void UcaStack::stop()
{
    _isRunning = false;

    _clientMutex.lock();
    _xmppClient.disconnectFromServer();
    _clientMutex.unlock();

    _device->stackStatusChanged("uca", false);
}

QString UcaStack::createId()
{
    return idPrefix + QString::number(idNumber++);
}
