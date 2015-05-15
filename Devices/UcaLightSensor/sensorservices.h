#ifndef SENSORSERVICES_H
#define SENSORSERVICES_H

#include <Stack/iupnpservice.h>
#include <Stack/failable.h>

class QDir;

struct ServiceInfo;

void loadServices( QDomDocument *deviceDescription
                 , QHash<QString, ServiceInfo *> &infos
                 , const QDir &rootDirectory
                 );

class SensorTransportGenericService : public IUPnPService
{
private:
    const QString _id;
    const QString _type;
    const QUrl _scdpPath;
    const QUrl _controlUrl;
    const QUrl _eventUrl;
    const QDomDocument *_description;
    const QStringList _variables;

public:
    SensorTransportGenericService(const ServiceInfo *description);

    /* IUPnPService implementation: */
    QMap<QString, QString> handleSOAP( const QString &actionName
                                     , const QHash<QString, QString> &arguments
                                     );

    const QDomDocument &getServiceDescription() const { return *_description; }

    const QString getServiceId() const { return _id; }
    const QString getServiceType() const { return _type; }

    const QUrl getScdpPath() const { return _scdpPath; }
    const QUrl getControlUrl() const { return _controlUrl; }
    const QUrl getEventUrl() const { return _eventUrl; }

    const QStringList getEventedVariableNames() const { return _variables; }
    const QMap<QString,QString> getInitialEventVariables() const {QMap<QString,QString> variables; return variables;}
};

class ConfigurationManagementService : public IUPnPService
{
private:
    const QString _id;
    const QString _type;
    const QUrl _scdpPath;
    const QUrl _controlUrl;
    const QUrl _eventUrl;
    const QDomDocument *_description;
    const QStringList _variables;

public:
    ConfigurationManagementService(const ServiceInfo *description);

    /* IUPnPService implementation: */
    QMap<QString, QString> handleSOAP( const QString &actionName
                                     , const QHash<QString, QString> &arguments
                                     );

    const QDomDocument &getServiceDescription() const { return *_description; }

    const QString getServiceId() const { return _id; }
    const QString getServiceType() const { return _type; }

    const QUrl getScdpPath() const { return _scdpPath; }
    const QUrl getControlUrl() const { return _controlUrl; }
    const QUrl getEventUrl() const { return _eventUrl; }

    const QStringList getEventedVariableNames() const { return _variables; }
    const QMap<QString,QString> getInitialEventVariables() const {QMap<QString,QString> variables; variables.insert("ConfigurationUpdate","");variables.insert("SupportedDataModelsUpdate","");variables.insert("SupportedParametersUpdate",""); return variables;}
};

#endif // SENSORSERVICES_H
