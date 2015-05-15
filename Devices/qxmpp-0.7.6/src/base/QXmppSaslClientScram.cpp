#include "QXmppSaslClientScram.h"

#include <QByteArray>
#include <QString>
#include <QTextStream>
#include <QtDebug>
#include <QRegExp>
QXmppSaslClientScram::QXmppSaslClientScram(QObject *parent) :
    QXmppSaslClient(parent) { }

QString QXmppSaslClientScram::mechanism() const
{
    return "SCRAM-SHA-1";
}

QCA::SecureArray computeHmacSha1(const QCA::SecureArray &key, const QCA::SecureArray &str) {
        return QCA::MessageAuthenticationCode("hmac(sha1)", key).process(str);
    }

bool QXmppSaslClientScram::respond(const QByteArray &challenge, QByteArray &response)
{
    if(challenge.size() > 0)
    {
        QCA::Initializer initializer;
        QRegExp pattern("r=(.*),s=(.+),i=(\\d+)");
        pattern.exactMatch(QString(challenge));
        QString s = pattern.cap(2);
        QString i = pattern.cap(3);

        QCA::Hash shaHash("sha1");
        shaHash.update("", 0);
        QCA::PBKDF2 hi("sha1");

        QCA::SymmetricKey passSalted = hi.makeKey(QCA::SecureArray(password().toUtf8()),
                                                  QCA::InitializationVector(QCA::Base64().stringToArray(s)),
                                                  shaHash.final().size(), i.toULong());
        QCA::SecureArray rawKey(computeHmacSha1(passSalted.toByteArray(), QByteArray("Client Key")));
        QCA::SecureArray storedKey = QCA::Hash("sha1").process(rawKey);

        QString nonce = pattern.cap(1);
        QString clientMessageWithoutProof = "c=biws,r=" + nonce;
        QString firstClientBareMessage = clientFirstMessage.mid(3);

        QCA::SecureArray authenticationMessageBytes = QCA::SecureArray(
                    firstClientBareMessage.toUtf8() + "," + challenge +
                    "," + clientMessageWithoutProof.toUtf8());

        QCA::SecureArray signature = computeHmacSha1(storedKey, authenticationMessageBytes);

        QCA::SecureArray proof(rawKey.size());
        for(int i = 0; i < proof.size(); ++i) {
            proof[i] = rawKey[i] ^ signature[i];
        }
        QString clientFinalMessage = clientMessageWithoutProof + ",p=" + QCA::Base64().arrayToString(proof);
        response = clientFinalMessage.toUtf8();
        Q_UNUSED(initializer);
    }
    else
    {
        secondResponse = true;
        QString randomString = generateRandomString(24);
        response = QString("n,,n="+username()+",r=" + randomString).toUtf8();
        clientFirstMessage = response;
        return true;
    }
    return true;
}

QString QXmppSaslClientScram::generateRandomString(int letterCount)
{
    char* possibleCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqstuvwxyz0123456789+/";
    char* base64Letters = new char[letterCount+1];
    for(int n = 0; n < letterCount; ++n) {
        int randomNumber = qrand() % 64;
        base64Letters[n] = possibleCharacters[randomNumber];
    }

    base64Letters[letterCount] = 0;
    QString result = QString(base64Letters);
    delete base64Letters;
    return result;
}
