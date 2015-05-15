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

#ifndef UCAEVENTINGHANDLER_H
#define UCAEVENTINGHANDLER_H

#include <QObject>
#include <QList>

#include <QtConcurrent/QtConcurrent>

#include <qxmpp/QXmppIq.h>

#include "failable.h"
#include <QPair>
#include <QQueue>
#include <Stack/iupnpstack.h>
struct UcaStack;

class UcaEventingHandler : QObject
{
Q_OBJECT

private:
    static const int _maxResponseAwaitingTime = 5 * 1000; /* miliseconds */

    UcaStack *_stack;

    QString _currentlyAwaitedIqId;
    QXmppIq _lastIqReceived;

    QFuture<void> _futureCheck;
    QFuture<void> _futureEventSent;

    Failable<QXmppIq> waitForResponse(const QString &id);

    Failable<bool> checkIsPubSubSupportedResult(const QXmppIq &reponse) const;

    Failable<bool> isPubSubSupported();
    Failable<bool> areNodesCreated();
    Failable<bool> createNodes();

    Failable<bool> updateConfigIdCloud();
    Failable<bool> checkNodeExists(const QString &nodeSuffix);
    Failable<bool> checkConfigIdCloudValue();
    bool nextEventInQueue();
    bool eventSendingRunning;
    void check();
    void sendEventAsyncWrapper();

    Failable<bool> sendEvent(const QString &serviceId, const QMap <QString,QString> &variables, EventMessage* args);
    QQueue<EventMessage> uppnEventsQueue;
    QMutex mutex;

public:
    UcaEventingHandler(UcaStack *stack);

    Failable<bool> startEventingCheck();

    Failable<bool> sendEventAsync(EventMessage eventMessage);

    inline bool isAwaitingResponse() { return _currentlyAwaitedIqId.isEmpty() == false; }
    inline bool isSendingEvent() { return _futureEventSent.isRunning(); }
    inline bool isCheckStarted() { return _futureCheck.isRunning(); }

private slots:
    void receivedIq(const QString &id, const QXmppIq &iq);

signals:
    void awaitedIqReceived();

    void eventingChecked(bool available);
};

#endif // UCAEVENTINGHANDLER_H
