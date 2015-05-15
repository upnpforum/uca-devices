#ifndef QXMPPSASLCLIENTSCRAM_H
#define QXMPPSASLCLIENTSCRAM_H

#include "QXmppSasl_p.h"
#include <Qca-qt5/QtCrypto/QtCrypto>

class QXmppSaslClientScram : public QXmppSaslClient
{

public:
    QXmppSaslClientScram(QObject *parent = 0);
    QString mechanism() const;
    bool respond(const QByteArray &challenge, QByteArray &response);
    QString generateRandomString(int letterCount);

private:
    bool secondResponse;
    QByteArray clientFirstMessage;
};

#endif // QXMPPSASLCLIENTSCRAM_H
