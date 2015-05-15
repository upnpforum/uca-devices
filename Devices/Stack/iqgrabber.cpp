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

#include "iqgrabber.h"

#include "ucastack.h"

IqGrabber::IqGrabber(UcaStack *stack)
    : _stack(stack)
{
    bool check;
    check = QObject::connect( this, SIGNAL(iqReceived(QXmppIq))
                            , _stack, SLOT(iqReceived(QXmppIq))
                            );
    Q_ASSERT(check);
    Q_UNUSED(check);
}

bool IqGrabber::handleStanza(const QDomElement &stanza)
{
    if (stanza.tagName() != "iq")
        return false;

    QDomElement firstChild = stanza.firstChildElement();

    if (   firstChild.tagName() != "query"
        && firstChild.tagName() != "controlURL"
        && firstChild.tagName().contains(QString("envelope"), Qt::CaseInsensitive) == false
           )
        return false;

    qDebug() << "Stanza grabbed.";

    QXmppIq iqPacket;
    iqPacket.parse(stanza);
    emit iqReceived(iqPacket);

    return true;
}


