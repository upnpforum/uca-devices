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

#ifndef UCASTACK_H
#define UCASTACK_H

#include "ucaeventinghandler.h"
#include "iupnpstack.h"
#include "failable.h"

#include <QMutex>
#include <QSet>

#include <qxmpp/QXmppClient.h>
#include <qxmpp/QXmppConfiguration.h>

struct QString;
struct QSettings;

struct QXmppPresence;

struct IUPnPDevice;

extern const char *ARGKEY_SENDER_JID;
extern const char *ARGKEY_SENDER_NAME;

class UcaStack : public QObject, public IUPnPStack
{
Q_OBJECT

private:
    bool _recivedAnyIq;

    UcaEventingHandler _eventing;

    QXmppConfiguration _xmppConfig;
    QXmppClient _xmppClient;

    IUPnPDevice *_device;
    bool _isRunning;

    QString _pubSubServiceName;
    QString _resourceName;

    QMutex _clientMutex;

    QSettings *_settings;

    QSet<QString> _chatListeners;
    QHash<QString, QString> _clientsNames;

    /* as there is no unit type in C++ bool is used instead,
     * i.e. the actual value of bool is discareded */
    Failable<bool> setupConfig(QXmppConfiguration &config);

    static void setupPresence( QXmppPresence &presence
                             , const QString &descHash
                             , const QXmppConfiguration &config
                             );
    void addSelfToRoster();
    void connect();

    void dispatchSet(const QXmppIq &incoming, QXmppIq &outgoing);
    void dispatchGet(const QXmppIq &incoming, QXmppIq &outgoing);

    void handleDesciptionGet(const QXmppIq &incoming, QXmppIq &outgoing, const QXmppElement &queryElement);
    void handleActionInvocation(const QXmppIq &incoming, QXmppIq &outgoing, const QXmppElement &envelope);

public:
    UcaStack(QSettings *settings);
    ~UcaStack();

    void handleIq(const QXmppIq &incoming);

    void sendEvent(EventMessage eventMessage);
    void registerDevice(IUPnPDevice *device);

    void start();
    void stop();

    void sendToChatListeneres(const QString &body);

    Failable<bool> sendStanza(const QXmppStanza& stanza);

    inline bool isRunning() const { return _isRunning; }
    inline IUPnPDevice *getDevice() const { return _device; }

    inline QString fullJid() const { return _xmppConfig.jid();}
    inline QString bareJid() const { return _xmppConfig.jidBare();}
    inline QString pubSubService() const { return _pubSubServiceName;}
    inline QString resourceName() const { return _resourceName; }
    QString getConfigIdCloud() const;

    static QString createId();

signals:
    void receivedResult(const QString &id, const QXmppIq &iq);


private slots:
    void presenceReceived(const QXmppPresence &incoming);
    void messageReceived(const QXmppMessage &incoming);
    void iqReceived(const QXmppIq &);

    void xmppClientConnected();
};

#endif // UCASTACK_H
