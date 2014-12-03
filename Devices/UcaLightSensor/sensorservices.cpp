#include "sensorservices.h"

#include <QFile>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QHash>
#include <cerrno>
#include <QException>

static const char *TRANSPORT_SERV_TYPE  = "urn:schemas-upnp-org:service:SensorTransportGeneric:1";
static const char *CONFIGMGMT_SERV_TYPE = "urn:schemas-upnp-org:service:ConfigurationManagement:2";

struct ServiceInfo {
    QString id;
    QString type;
    QString scdpPath;
    QString controlUrl;
    QString eventUrl;
    QList<QString> variables;
    QDomDocument *description;
};

static QDomDocument *loadXml(const QString &path, const QString &docName)
{
    QFile file(path);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text) == false)
    {
        printf("Error opening file %s: %s \n",path.toUtf8().constData(),strerror(errno));
        throw QException();
        return NULL;
    }

    QDomDocument *doc = new QDomDocument(docName);
    bool setContentResult = doc->setContent(&file);
    file.close();

    if (setContentResult == false) {
        return NULL;
    }

    return doc;
}

static void findServiceInformation
    (QDomDocument *deviceDoc, QHash<QString, ServiceInfo> &buffer)
{
    if (deviceDoc == NULL)
        return;

    QDomNodeList services = deviceDoc->elementsByTagName("service");

    int count = services.count();
    for (int i = 0; i < count; i++) {
        QString name;
        ServiceInfo info;

        QDomElement serviceNode = services.at(i).toElement();
        QDomElement child = serviceNode.firstChildElement();
        while (child.isNull() == false) {
            if (child.tagName() == "serviceId") {
                name = child.text();
                info.id = name;
            } else if (child.tagName() == "serviceType") {
                info.type = child.text();
            } else if (child.tagName() == "SCPDURL") {
                info.scdpPath = child.text();
            } else if (child.tagName() == "controlURL") {
                info.controlUrl = child.text();
            } else if (child.tagName() == "eventSubURL") {
                info.eventUrl = child.text();
            }

            child = child.nextSiblingElement();
        }

        buffer.insert(name, info);
    }
}

static QList<QString> fetchVariableNames
    (const QDomDocument *serviceDesc, bool eventableOnly)
{
    QList<QString> names;

    QDomNodeList variables = serviceDesc->elementsByTagName("stateVariable");
    QDomElement var = variables.at(0).toElement();
    for (; var.isNull() == false ; var = var.nextSiblingElement()) {
        QDomElement variable = var.toElement();
        if (variable.isNull())
            continue;

        QString sendEvents = variable.attribute("sendEvents");
        bool envetable =    sendEvents.compare("yes", Qt::CaseInsensitive) == 0
                         || sendEvents.compare("true", Qt::CaseInsensitive) == 0;

        if (envetable == false && eventableOnly)
            continue;

        /* TODO: each of the chained calls is failable */
        QString name
            = variable.elementsByTagName("name").at(0).toElement().text();
        names.append(name);
    }

    return names;
}


void loadServices( QDomDocument *deviceDescription
                 , QHash<QString, ServiceInfo *> &externalInfos
                 , const QDir &rootDirectory
                 )
{
    QHash<QString, ServiceInfo> infos;
    findServiceInformation(deviceDescription, infos);

    QHash<QString, ServiceInfo>::const_iterator it = infos.constBegin();
    for (; it != infos.constEnd(); it++) {
        QString name = it.key();
        ServiceInfo info = it.value();

        QString path = rootDirectory.path() + info.scdpPath;

        QDomDocument *desc = loadXml(path, name);
        info.description = desc;
        info.variables = fetchVariableNames(desc, true);

        externalInfos[name] = new (std::nothrow) ServiceInfo(info);
    }
}

/* SensorTransportGeneric: */

SensorTransportGenericService::SensorTransportGenericService(const ServiceInfo *info)
    : _id(info->id)
    , _type(TRANSPORT_SERV_TYPE)
    , _scdpPath(info->scdpPath)
    , _controlUrl(info->controlUrl)
    , _eventUrl(info->eventUrl)
    , _description(info->description)
    , _variables(info->variables)
{
}

/* Actual implementation is in the device */
QMap<QString, QString> SensorTransportGenericService::handleSOAP
        ( const QString &actionName
        , const QHash<QString, QString> &arguments
        ) {
    Q_UNUSED(actionName);
    Q_UNUSED(arguments);

    return QMap<QString, QString>();
}

/* SWITCH POWER: */

ConfigurationManagementService::ConfigurationManagementService(const ServiceInfo *info)
    : _id(info->id)
    , _type(CONFIGMGMT_SERV_TYPE)
    , _scdpPath(info->scdpPath)
    , _controlUrl(info->controlUrl)
    , _eventUrl(info->eventUrl)
    , _description(info->description)
    , _variables(info->variables)
{
}

/* Actual implementation is in the device */
QMap<QString, QString> ConfigurationManagementService::handleSOAP
        ( const QString &actionName
        , const QHash<QString, QString> &arguments
        ) {
    Q_UNUSED(actionName);
    Q_UNUSED(arguments);

    return QMap<QString, QString>();
}

