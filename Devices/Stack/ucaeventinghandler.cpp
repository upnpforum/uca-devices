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

#include "ucaeventinghandler.h"

#include <QDebug>
#include <QEventLoop>
#include <QTimer>

#include <QtConcurrent/QtConcurrent>

#include <qxmpp/QXmppIq.h>

#include "ucastack.h"
#include "iupnpservice.h"

UcaEventingHandler::UcaEventingHandler(UcaStack *stack)
{
    _stack = stack;

    QObject::connect( _stack, SIGNAL(receivedResult(QString,QXmppIq))
                    , this, SLOT(receivedIq(QString,QXmppIq))
                    );
}

Failable<bool> UcaEventingHandler::startEventingCheck()
{
    if (isCheckStarted()) {
        return Failable<bool>::Failure("Check already started.");
    }

    _futureCheck = QtConcurrent::run(this, &UcaEventingHandler::check);

    return true;
}

Failable<bool> UcaEventingHandler::checkNodeExists(const QString &nodeSuffix)
{
    QXmppIq output;
    QString id = UcaStack::createId();

    output.setId(id);
    output.setFrom(_stack->fullJid());
    output.setTo(_stack->pubSubService());
    output.setType(QXmppIq::Get);

    QXmppElementList elements;

    QXmppElement query;
    query.setTagName("query");
    query.setAttribute("xmlns","http://jabber.org/protocol/disco#info");
    query.setAttribute("node", _stack->resourceName() + nodeSuffix);
    elements.append(query);

    output.setExtensions(elements);

    Failable<bool> maybeSent = _stack->sendStanza(output);
    if (maybeSent.hasValue() == false) {
        return Failable<bool>::Propagate("Failed to sent iq", maybeSent);
    }
    if (maybeSent.value() == false) {
        return Failable<bool>::Failure("Failed to sent iq.");
    }
    waitForResponse(id);

    return true;
}

Failable<bool> UcaEventingHandler::checkConfigIdCloudValue()
{
    QXmppIq stanza;
    QString id = UcaStack::createId();

    stanza.setId(id);
    stanza.setFrom(_stack->fullJid());
    stanza.setTo(_stack->pubSubService());
    stanza.setType(QXmppIq::Get);

    QXmppElementList elements;

    QXmppElement pubsub;
    pubsub.setTagName("pubsub");
    pubsub.setAttribute("xmlns","http://jabber.org/protocol/pubsub");
    elements.append(pubsub);

    QXmppElement items;
    items.setTagName("items");
    items.setAttribute("node", _stack->resourceName() + "/configIdCloud");
    pubsub.appendChild(items);

    stanza.setExtensions(elements);

    Failable<bool> maybeSent = _stack->sendStanza(stanza);
    if (maybeSent.hasValue() == false) {
        return Failable<bool>::Propagate("Failed to sent iq", maybeSent);
    }
    if (maybeSent.value() == false) {
        return Failable<bool>::Failure("Failed to sent iq.");
    }
    waitForResponse(id);

    return true;
}

void UcaEventingHandler::check()
{
    Failable<bool> maybeSupported = isPubSubSupported();
    qDebug() << "thread id:" << QObject::thread();
    if (maybeSupported.hasValue() == false) {
        qDebug() << maybeSupported.message();
        return;
    } else {
        qDebug() << "PubSub supported.";
    }

    Failable<bool> maybeCreated = areNodesCreated();
    if (maybeCreated.hasValue() == false) {
        qDebug() << maybeCreated.message();
    } else {
        qDebug() << "Nodes existence checked.";
    }

    //checkNodeExists("");
    Failable<bool> maybeConfigIdExists(false);
    if(maybeCreated.value() == true) {
        maybeConfigIdExists = checkNodeExists("/configIdCloud");
        if (maybeConfigIdExists.hasValue() == false) {
            qDebug() << maybeConfigIdExists.message();
        }else{
            qDebug() << "Node ConfigIdCloud checked";
        }
    }

    checkConfigIdCloudValue();

    const bool configIdMatch
        = maybeConfigIdExists.hasValue() == false
       || maybeConfigIdExists.value() == false;

    if (maybeCreated.value() == false && configIdMatch) {
        Failable<bool> maybeCreationResult = createNodes();
        if (maybeCreationResult.hasValue() == false) {
            qDebug() << maybeCreationResult.message();
        } else {
            qDebug() << "Nodes created.";
        }
    }
}

void UcaEventingHandler::receivedIq(const QString &id, const QXmppIq &iq)
{
    if (id == _currentlyAwaitedIqId) {
        _lastIqReceived = iq;
        emit awaitedIqReceived();
    }
}

Failable<QXmppIq> UcaEventingHandler::waitForResponse(const QString &id)
{
    this->_currentlyAwaitedIqId = id;
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    QObject::connect( &timer, SIGNAL(timeout())
                    , &loop, SLOT(quit())
                    );
    QObject::connect( this, SIGNAL(awaitedIqReceived())
                    , &loop, SLOT(quit())
                    );

    timer.start(_maxResponseAwaitingTime);
    loop.exec();

    this->_currentlyAwaitedIqId = QString();
    if (timer.isActive() == false) { /* loop closed by the timer */
        return Failable<QXmppIq>::Failure("Response timeout.");
    }

    return _lastIqReceived;
}

/* returns id of created stanza */
static Failable<QString> createPubSubQueryStanza(const QString &jid, const QString &pubsubNode, QXmppIq &output)
{
    if (jid.isEmpty()) { /* TODO: actually form is not required by XMPP, should this be omitted? */
        return Failable<QString>::Failure("Cannot create stanza with empty JID.");
    }
    /* TODO: it might be fairly good idea to check if jid is valid and full */
    if (pubsubNode.isEmpty()) {
        return Failable<QString>::Failure("Cannot create stanza with empty PubSub node.");
    }

    QString id = UcaStack::createId();

    output.setId(id);
    output.setFrom(jid);
    output.setTo(pubsubNode);
    output.setType(QXmppIq::Get);

    QXmppElementList elements;

    QXmppElement query;
    query.setTagName("query");
    query.setAttribute("xmlns", "http://jabber.org/protocol/disco#info");
    elements.append(query);

    output.setExtensions(elements);

    return output.id();
}

//<iq from='pubsub.upnpcloud' to='mateusz.belicki@upnpcloud/pidgin' id='purpledisco7a2c3201' type='result'>
//	<query xmlns='http://jabber.org/protocol/disco#info'>
//      ...
// -->	<identity category='pubsub' type='service' name='Publish-Subscribe'/>
//      ...
// -->	<feature var='http://jabber.org/protocol/pubsub'/>
//      ...
//	</query>
//</iq>

Failable<bool> UcaEventingHandler::checkIsPubSubSupportedResult(const QXmppIq &response) const
{
    bool indentityPresent = false;
    bool featurePresent = false;

    if (response.type() == QXmppIq::Error) {
        return Failable<bool>::Failure("Error response.");
    }
    if (response.type() != QXmppIq::Result) {
        return Failable<bool>::Failure("Unexpected response type (neither result nor error).");
    }

    QXmppElementList elements = response.extensions();

    if (elements.isEmpty()) {
        return Failable<bool>::Failure("Empty response.");
    }

    QXmppElement query = elements.first();
    if (query.tagName() != "query") {
        return false;
    }

    QXmppElement child = query.firstChildElement();
    while (child.isNull() == false) {
        if (   child.tagName() == "feature"
            && child.attribute("var") == "http://jabber.org/protocol/pubsub")
        {
            featurePresent = true;
        }

        if (   child.tagName() == "identity"
            && child.attribute("category") == "pubsub")
        {
            indentityPresent = true;
        }

        child = child.nextSiblingElement();
    }

    return indentityPresent && featurePresent;
}

Failable<bool> UcaEventingHandler::isPubSubSupported()
{
    if (_stack == NULL) {
        return Failable<bool>::Failure("Inner UcaStack is NULL.");
    }
    if (_stack->isRunning() == false) {
        return Failable<bool>::Failure("Cannot check isPubSubSupported when UCA stack is not running.");
    }
    if (isAwaitingResponse()) {
        return Failable<bool>::Failure("Cannot check isPubSubSupported awaiting response of other request.");
    }

    QXmppIq stanza;
    Failable<QString> maybeId = createPubSubQueryStanza(_stack->fullJid(), _stack->pubSubService(), stanza);
    if (maybeId.hasValue() == false) {
        return Failable<bool>::Propagate("Failed to create stanza:", maybeId);
    }
    if (maybeId.value().isEmpty() == true) {
        return Failable<bool>::Failure("Failed to create stanza: generated illegal id.");
    }

    Failable<bool> maybeSent = _stack->sendStanza(stanza);
    if (maybeSent.hasValue() == false) {
        return Failable<bool>::Propagate("Failed to sent iq", maybeSent);
    }
    if (maybeSent.value() == false) {
        return Failable<bool>::Failure("Failed to sent iq.");
    }

    Failable<QXmppIq> maybeResponse = waitForResponse(maybeId.value());
    if (maybeResponse.hasValue() == false) {
        return Failable<bool>::Propagate("Failed to receive response:", maybeResponse);
    }

    Failable<bool> maybeIsSupported = checkIsPubSubSupportedResult(maybeResponse.value());

    if (maybeIsSupported.hasValue() == false) {
        return Failable<bool>::Propagate("Failed to parse response:", maybeIsSupported);
    }

    return maybeIsSupported;
}

static Failable<QString> createPubSubNodesQueryStanza( const QString &jid
                                                     , const QString &pubsubNode
                                                     , const QString &resource
                                                     , QXmppIq &output
                                                     )
{
    QString id = UcaStack::createId();

    output.setId(id);
    output.setFrom(jid);
    output.setTo(pubsubNode);
    output.setType(QXmppIq::Get);

    QXmppElementList elements;

    QXmppElement query;
    query.setTagName("query");
    query.setAttribute("xmlns", "http://jabber.org/protocol/disco#info");
    query.setAttribute("node", resource);
    elements.append(query);

    output.setExtensions(elements);

    return Failable<QString>::Success(id);
}

static Failable<bool> checkAreNodesCreated(const QXmppIq &response)
{
    if (response.type() == QXmppIq::Error) {
        /* TODO: more detailed error handling */
        return false;
    }
    if (response.type() == QXmppIq::Result) {
        /* TODO: more detailed response handling */
        return true;
    }
    /**/
    return Failable<bool>::Failure("Unexpected result.");
}

Failable<bool> UcaEventingHandler::areNodesCreated()
{
    if (_stack == NULL) {
        return Failable<bool>::Failure("Inner UcaStack is NULL.");
    }
    if (_stack->isRunning() == false) {
        return Failable<bool>::Failure("Cannot check areNodesCreated when UCA stack is not running.");
    }
    if (isAwaitingResponse()) {
        return Failable<bool>::Failure("Cannot check isPubSubSupported awaiting response of other request.");
    }

    QXmppIq stanza;
    Failable<QString> maybeId
        = createPubSubNodesQueryStanza( _stack->fullJid()
                                      , _stack->pubSubService()
                                      , _stack->resourceName()
                                      , stanza
                                      );
    if (maybeId.hasValue() == false) {
        return Failable<bool>::Propagate("Failed to create stanza:", maybeId);
    }
    if (maybeId.value().isEmpty() == true) {
        return Failable<bool>::Failure("Failed to create stanza: generated illegal id.");
    }

    Failable<bool> maybeSent = _stack->sendStanza(stanza);
    if (maybeSent.hasValue() == false) {
        return Failable<bool>::Propagate("Failed to sent iq", maybeSent);
    }
    if (maybeSent.value() == false) {
        return Failable<bool>::Failure("Failed to sent iq.");
    }

    Failable<QXmppIq> maybeResponse = waitForResponse(maybeId.value());
    if (maybeResponse.hasValue() == false) {
        return Failable<bool>::Propagate("Failed to receive response:", maybeResponse);
    }

    Failable<bool> maybeResult = checkAreNodesCreated(maybeResponse.value());
    if (maybeResult.hasValue() == false) {
        return Failable<bool>::Propagate("Failed to parse response:", maybeResult);
    }

    return maybeResult.value();
}

static QXmppElement createNodeConfigField( const QString &var
                                         , const QString &value
                                         )
{
   QXmppElement fieldElement;
   fieldElement.setTagName("field");
   fieldElement.setAttribute("var", var);

   QXmppElement valueElement;
   valueElement.setTagName("value");
   valueElement.setValue(value);
   fieldElement.appendChild(valueElement);

   return fieldElement;
}

static Failable<QString> createPubSubCreateNodeStanza( const QString &jid
                                                     , const QString &pubsubNode
                                                     , const QString &nodeName
                                                     , const QString &nodeType
                                                     , const QString &parentNode
                                                     , QXmppIq &output
                                                     )
{
    QString maxItems("#");
    //if (nodeName == "configId") {
        maxItems = QString("1");
    //} else if (nodeType == "leaf") {
    //    maxItems = QString("?");
    //}

    QString id = UcaStack::createId();

    output.setId(id);
    output.setFrom(jid);
    output.setTo(pubsubNode);
    output.setType(QXmppIq::Set);

    QXmppElementList elements;

    QXmppElement pubsub;
    pubsub.setTagName("pubsub");
    pubsub.setAttribute("xmlns", "http://jabber.org/protocol/pubsub");
    elements.append(pubsub);

    QXmppElement create;
    create.setTagName("create");
    create.setAttribute("node", nodeName);
    pubsub.appendChild(create);

    QXmppElement configure;
    configure.setTagName("configure");
    pubsub.appendChild(configure);

    QXmppElement x;
    x.setTagName("x");
    x.setAttribute("xmlns", "jabber:x:data");
    x.setAttribute("type", "submit");
    configure.appendChild(x);

    QXmppElement fieldFormType
        = createNodeConfigField("FORM_TYPE", "http://jabber.org/protocol/pubsub#node_config");
    fieldFormType.setAttribute("type", "hidden");
    x.appendChild(fieldFormType);

    QXmppElement fieldNodeType
        = createNodeConfigField("pubsub#node_type", nodeType);
    x.appendChild(fieldNodeType);

    if (parentNode.isEmpty() == false) {
        QXmppElement fieldCollection
            = createNodeConfigField("pubsub#collection", parentNode);
        x.appendChild(fieldCollection);
    }

    QXmppElement fieldAccessModel
        = createNodeConfigField("pubsub#access_model", "open");
    x.appendChild(fieldAccessModel);

    /* TODO: for some reason this is not supported: */
    QXmppElement fieldMaxItems
        = createNodeConfigField("pubsub#max_items", maxItems);
    x.appendChild(fieldMaxItems);

    QXmppElement fieldOther
        = createNodeConfigField("pubsub#other", "other");
    x.appendChild(fieldOther);

    output.setExtensions(elements);

    return id;
}

static Failable<bool> createPubSubNode( UcaStack *stack
                                      , const QString &name
                                      , const QString &type
                                      , const QString &parent
                                      )
{
    QXmppIq stanza;
    Failable<QString> maybeId
        = createPubSubCreateNodeStanza( stack->fullJid()
                                      , stack->pubSubService()
                                      , name
                                      , type
                                      , parent
                                      , stanza
                                      );
    if (maybeId.hasValue() == false) {
        return Failable<bool>::Propagate("Failed to create stanza:", maybeId);
    }
    if (maybeId.value().isEmpty() == true) {
        return Failable<bool>::Failure("Failed to create stanza: generated illegal id.");
    }

    Failable<bool> maybeSent = stack->sendStanza(stanza);
    if (maybeSent.hasValue() == false) {
        return Failable<bool>::Propagate("Failed to sent iq", maybeSent);
    }
    if (maybeSent.value() == false) {
        return Failable<bool>::Failure("Failed to sent iq.");
    }

    return true;
}

Failable<bool> UcaEventingHandler::createNodes()
{
    if (_stack == NULL) {
        return Failable<bool>::Failure("Inner UcaStack is NULL.");
    }
    if (_stack->isRunning() == false) {
        return Failable<bool>::Failure("Cannot check createNodes when UCA stack is not running.");
    }
    if (isAwaitingResponse()) {
        return Failable<bool>::Failure("Cannot check isPubSubSupported awaiting response of other request.");
    }


    QString base = _stack->resourceName();
    Failable<bool> creationResult = createPubSubNode(_stack, base, "collection", "");
    if (creationResult.hasValue() == false) {
        return creationResult;
    } else if (creationResult.value() == false) {
        qDebug() << "Already created:" << base;
    }

    QString configIdCloudNodeName = base + "/configIdCloud";
    creationResult = createPubSubNode(_stack, configIdCloudNodeName, "leaf", base);
    if (creationResult.hasValue() == false) {
        return creationResult;
    } else if (creationResult.value() == false) {
        qDebug() << "Already created: service";
    }

    const IUPnPDevice *device = _stack->getDevice();
    QList<QString> ids = device->getServiceIds();

    foreach (QString id, ids) {
       const IUPnPService *service = device->getService(id);
       if (service == NULL) {
           qDebug() << "Missing service: " << id;
           continue;
       }

       QString serviceNode = QString("%1/%2").arg(base).arg(service->getServiceType());

       creationResult = createPubSubNode(_stack, serviceNode, "collection", base);
       if (creationResult.hasValue() == false) {
           return creationResult;
       } else if (creationResult.value() == false) {
           qDebug() << "Already created:" << serviceNode;
       }

       QList<QString> vars = service->getEventedVariableNames();
       foreach (QString var, vars) {
           QString varNode = QString("%1/%2").arg(serviceNode).arg(var);
           creationResult = createPubSubNode(_stack, varNode, "leaf", serviceNode);
           if (creationResult.hasValue() == false) {
               return creationResult;
           } else if (creationResult.value() == false) {
               qDebug() << "Already created:" << varNode;
           }
       }
       QMap<QString,QString> values = service->getInitialEventVariables();
       foreach (QString name, values.keys()) {
           QMap<QString,QString> tmp;
           tmp.insert(name,values[name]);
           this->sendEvent(service->getServiceType(),tmp, 0);
       }
    }
    this->updateConfigIdCloud();

    return true;
}

static QString createPublishConfigIdCloud
        ( const QString &jid
        , const QString &pubsubUrl
        , const QString &pubsubNode
        , const QString &configIdCloudValue
        , QXmppIq &output
        )
{
    QString id = UcaStack::createId();

    output.setId(id);
    output.setFrom(jid);
    output.setTo(pubsubUrl);
    output.setType(QXmppIq::Set);

    QXmppElementList elements;

    QXmppElement pubsub;
    pubsub.setTagName("pubsub");
    pubsub.setAttribute("xmlns", "http://jabber.org/protocol/pubsub");
    elements.append(pubsub);

    QXmppElement publish;
    publish.setTagName("publish");
    publish.setAttribute("node", pubsubNode);
    pubsub.appendChild(publish);

    QXmppElement item;
    item.setTagName("item");
    publish.appendChild(item);

    QXmppElement configIdCloud;
    configIdCloud.setTagName("e:configIdCloud");
    configIdCloud.setAttribute("xmlns:e", "urn:schemas-upnp-org:cloud-1-0");
    configIdCloud.setValue(configIdCloudValue);
    item.appendChild(configIdCloud);

    output.setExtensions(elements);

    return id;
}

Failable<bool> UcaEventingHandler::updateConfigIdCloud(){
    QXmppIq stanza;
    QString id
        = createPublishConfigIdCloud
            ( _stack->fullJid()
            , _stack->pubSubService()
            , _stack->resourceName() + "/configIdCloud"
            , _stack->getConfigIdCloud()
            , stanza
            );

    if (id.isEmpty()) {
        return Failable<bool>::Failure("Failed to create stanza: generated illegal id.");
    }

    Failable<bool> maybeSent = _stack->sendStanza(stanza);
    if (maybeSent.hasValue() == false) {
        return Failable<bool>::Propagate("Failed to sent iq", maybeSent);
    }
    if (maybeSent.value() == false) {
        return Failable<bool>::Failure("Failed to sent iq.");
    }

    return true;
}

static Failable<QString> createPubSubEventStanza( const QString &jid
                                                , const QString &pubsubNode
                                                , const QString &nodeName
                                                , const QMap <QString,QString> &variables
                                                , QXmppIq &output
                                                )
{
    QString id = UcaStack::createId();

    output.setId(id);
    output.setFrom(jid);
    output.setTo(pubsubNode);
    output.setType(QXmppIq::Set);

    QXmppElementList elements;

    QXmppElement pubsub;
    pubsub.setTagName("pubsub");
    pubsub.setAttribute("xmlns", "http://jabber.org/protocol/pubsub");
    elements.append(pubsub);

    QXmppElement publish;
    publish.setTagName("publish");
    publish.setAttribute("node", nodeName);
    pubsub.appendChild(publish);

    QXmppElement item;
    item.setTagName("item");
    publish.appendChild(item);

    QXmppElement propertyset;
    propertyset.setTagName("e:propertyset");
    propertyset.setAttribute("xmlns:e", "urn:schemas-upnp-org:event-1-0");
    item.appendChild(propertyset);

    foreach (QString s, variables.keys()){
        QXmppElement property;
        property.setTagName("e:property");
        propertyset.appendChild(property);
        QXmppElement variable;
        variable.setTagName(s);
        variable.setValue(variables[s]);
        property.appendChild(variable);
    }




    output.setExtensions(elements);

    return id;
}

Failable<bool> UcaEventingHandler::sendEventAsync(EventMessage eventMessage)
{
    uppnEventsQueue.enqueue(eventMessage);
    mutex.lock();
    bool startSender = !eventSendingRunning;
    if (startSender) {
        _futureEventSent = QtConcurrent::run(this, &UcaEventingHandler::sendEventAsyncWrapper);
    }
    mutex.unlock();
    return true;
}

#include "UcaMediaRenderer/avtransportservice.h"

void UcaEventingHandler::sendEventAsyncWrapper()
{
    mutex.lock();
    eventSendingRunning = true;
    mutex.unlock();
    while(nextEventInQueue())
    {
        EventMessage args = uppnEventsQueue.dequeue();
        QString eventServiceId = args.serviceId;
        QMap<QString,QString> variables;
        variables.insert(args.variableName, args.value);
        Failable<bool> result =
            sendEvent(eventServiceId, variables, &args);
        if (result.hasValue() == false) {
            qDebug() << "Failed to send event:" << result.message();
        }
    }
}

bool UcaEventingHandler::nextEventInQueue()
{
    bool result;
    mutex.lock();
    if(!uppnEventsQueue.isEmpty())
        result = true;
    else
    {
        result = false;
        eventSendingRunning = false;
    }
    mutex.unlock();
    return result;
}

Failable<bool> UcaEventingHandler::sendEvent( const QString &serviceId
                                            , const QMap <QString,QString> &variables,
                                              EventMessage* args
                                            )
{
    if (_stack == NULL) {
        return Failable<bool>::Failure("Inner UcaStack is NULL.");
    }
    if (_stack->isRunning() == false) {
        return Failable<bool>::Failure("Cannot sendEvent when UCA stack is not running.");
    }
    if (isAwaitingResponse()) {
        return Failable<bool>::Failure("Cannot check isPubSubSupported awaiting response of other request.");
    }

    QString nodeName = "";

    if (variables.count()>1)
    {
    nodeName = QString("%1/%2").arg(_stack->resourceName(), serviceId);
    }
    else
    {
    QString varName = variables.keys().first();
    nodeName = QString("%1/%2/%3").arg(_stack->resourceName(), serviceId, varName);
}
    QXmppIq stanza;
    Failable<QString> maybeId
        = createPubSubEventStanza( _stack->fullJid()
                                 , _stack->pubSubService()
                                 , nodeName
                                 , variables
                                 , stanza
                                 );

    if (maybeId.hasValue() == false) {
        return Failable<bool>::Propagate("Failed to create stanza:", maybeId);
    }
    if (maybeId.value().isEmpty() == true) {
        return Failable<bool>::Failure("Failed to create stanza: generated illegal id.");
    }

    Failable<bool> maybeSent = _stack->sendStanza(stanza);
    if (maybeSent.hasValue() == false) {
        return Failable<bool>::Propagate("Failed to sent iq", maybeSent);
    }
    if (maybeSent.value() == false) {
        return Failable<bool>::Failure("Failed to sent iq.");
    }

    if (args != 0)
    {
        emit args->serviceToNotify->eventSent(*args);
    }

    Failable<QXmppIq> maybeResponse = waitForResponse(maybeId.value());
    if (maybeResponse.hasValue() == false) {
        return Failable<bool>::Propagate("Failed to receive response:", maybeResponse);
    }

    return true;
}
