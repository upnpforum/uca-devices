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

#include "lightsensor.h"

#include "devicesettings.h"

#include <algorithm> /* max & min */

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QHash>
#include <cerrno>
#include <QException>

#include <QXmlQuery>
#include <QXmlItem>

static const char *DEVICE_PATH_SUFFIX = "description-xmls/device.xml";
static const char *DATA_MODEL_PATH_SUFFIX = "description-xmls/datamodel.xml";

static const char *CFGMGMT_SERV_TYPE
    = "urn:schemas-upnp-org:service:ConfigurationManagement:1";
static const char *SENSORTRANS_SERV_TYPE
    = "urn:schemas-upnp-org:service:SensorTransportGeneric:2";

static const char *CFGMGMT_SERV_ID
    = "urn:upnp-org:serviceId:ConfigurationManagement:1";
static const char *SENSORTRANS_SERV_ID
    = "urn:upnp-org:serviceId:SensorTransportGeneric:1";

static const char *GET_VALUES_RESPONSE_TEMPLATE
    = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
      "<cms:ParameterValueList xmlns:cms=\"urn:schemas-upnp-org:dm:cms\""
          "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
          "xsi:schemaLocation=\"urn:schemas-upnp-org:dm:cms http://www.upnp.org/schemas/dm/cms.xsd\">"
               "%1"
      "</cms:ParameterValueList>";

static const char *GET_VALUES_RESPONSE_TEMPLATE_ELEMENT
    = "<Parameter>"
          "<ParameterPath>%1</ParameterPath>"
          "<Value>%2</Value>"
      "</Parameter>";

/* %1 -- field name; %2 -- field type; %3 -- field value  */
static const char *READ_VALUES_RESPONSE_TEMPLATE
    = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
      "<DataRecords xmlns=\"urn:schemas-upnp-org:ds:drecs\""
          "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
          "xsi:schemaLocation=\"urn:schemas-upnp-org:ds:drecs http://www.upnp.org/schemas/ds/drecs-v1.xsd\">"
          "<datarecord>"
              "<field "
                  "name=\"%1\" "
                  "type=\"%2\" "
                  "encoding=\"ascii\">"
              "%3"
              "</field>"
          "</datarecord>"
      "</DataRecords>";

static const char *DEFAULT_DIMMING_URN
    = "urn:upnp-org:smgt-surn:light:upnp-org:brightness:Comarch:CloudLightDimmingDimming:setting/status";
static const char *DEFAULT_SWITCH_URN
    = "urn:upnp-org:smgt-surn:actuator:upnp-org:switch:Comarch:CloudLightDimmingSwitch:setting/status";
static const char *DEFAULT_TEMPERATURE_URN
    = "urn:upnp-org:smgt-surn:acuator:upnp-org:Temperature_cur:Comarch:CloudLightTemperature:status";

static bool checkIfAllArgumentsPresent
    ( const QStringList &expectedArguments
    , const QHash<QString, QString> &arguments
    , QStringList &missingArguments
    )
{
    missingArguments.clear();
    foreach (QString argument, expectedArguments)
        if (arguments.contains(argument) == false)
            missingArguments << argument;

    return missingArguments.empty() == false;
}

static QDomDocument *loadXml(const QString &path, const QString &docName)
{
    QFile file(path);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text) == false)
    {
        printf("Error opening file %s: %s \n", path.toUtf8().constData(), strerror(errno));
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

static void flattenValuesTree
    ( const QDomNodeList &nodes
    , const QString &pathPrefix
    , QHash<QString, QString> &output
    )
{
    qDebug() << nodes.count();
    for (int i = 0; i < nodes.count(); i++) {
        QDomNode node = nodes.at(i);
        if (node.isText()) {
            QDomText text = node.toText();
            QString value = text.data().toHtmlEscaped();
            output.insert(pathPrefix, value);
        } else if (node.isElement()) {
            QString newPath(pathPrefix);
            QDomElement element = node.toElement();
            QString tagName = element.tagName();
            if (tagName == "Instance") {
                tagName = element.attribute("id", "1");
            }

            newPath.append("/" + tagName);
            flattenValuesTree(node.childNodes(), newPath, output);
        }
    }
}

static QString pathToXPath(const QString &path)
{
    QString copy = path.trimmed(); /* replace is done in place */
    copy.replace(QRegExp("/([0-9]+)"), "/Instance[@id='\\1']");
    if (copy.endsWith('/')) {
        copy[copy.length() - 1] = '\0';
    }
    return copy;
}

static void getValueFromDataModel
    ( const QString &path
    , const QDomDocument &model
    , QHash<QString, QString> &values
    )
{
    const QString xPath(pathToXPath(path));

    QXmlQuery query;
    query.setFocus(model.toString(-1));
    query.setQuery(xPath);

    if (query.isValid() == false) {
        QString result = QString("Invalid path: '%1'.").arg(path);
        values.insert(path, result);
    } else {
        QString result;
        query.evaluateTo(&result);
        if (result.isEmpty() == false) {
            QDomDocument subTree;
            subTree.setContent(result);

            QDomNodeList children = subTree.firstChild().childNodes();
            flattenValuesTree(children, path, values);
        }
    }
}

static QStringList parseGetValuesInput(const QString &parameters)
{
    QString errorMessage; int col, line;
    QDomDocument document;
    document.setContent(parameters, &errorMessage, &line, &col);
    qDebug() << errorMessage << line << col;

    QDomNodeList paths = document.elementsByTagName("ContentPath");

    QStringList result;
    for (int i = 0; i < paths.count(); i++) {
        QDomElement contentPath = paths.at(i).toElement();
        result << contentPath.text();
    }

    return result;
}

static QString packGetValuesResult(const QHash<QString, QString> &values)
{
    QString parameters;
    QHash<QString, QString>::const_iterator it = values.constBegin();
    for (; it != values.constEnd(); it++) {
        QString path = it.key();
        QString value = it.value();
        QString parameter
            = QString(GET_VALUES_RESPONSE_TEMPLATE_ELEMENT).arg(path, value);
        parameters.append(parameter);
    }

    QString result = QString(GET_VALUES_RESPONSE_TEMPLATE).arg(parameters);

    return result;
}

static Failable<int> parseFirstDataRecordFieldValue(const QString &dataRecords)
{
    QXmlQuery query;
    query.setFocus(dataRecords);
    /* Oh the joys of XML namespaces! */
    query.setQuery("//*[local-name()='DataRecords']/*[local-name()='datarecord']/*[local-name()='field']/text()");

    if (query.isValid() == false) {
        return Failable<int>::Failure("Couln't find the value.");
    }

    QString rawValue;
    query.evaluateTo(&rawValue);

    bool conversionResult;
    int value = rawValue.toInt(&conversionResult, 10);
    if (conversionResult == false) {
        return Failable<int>::Failure("Couln't convert value to a number.");
    }

    return value;
}

static void logSOAPCall(QString serviceId, QString actionName, const QHash<QString, QString> &arguments)
{
    qDebug() << "* action:" << actionName << "of service:" << serviceId;
    qDebug() << "+---- called with: ----";

    QHash<QString, QString>::const_iterator it = arguments.constBegin();
    while (it != arguments.constEnd()) {
        qDebug() << "|" << it.key() << "=" << it.value();
        it++;
    }
    qDebug() << "+----------------------\n";
}

LightSensor::LightSensor
    ( DeviceSettings *settings
    , IUPnPStack *udaStack
    , IUPnPStack *ucaStack
    , QCoreApplication* app
    , const QString &appName
    ) : _application(app)
      , _enabled(1)
      , _loadLevel(100)
      , _transportService(NULL)
      , _configService(NULL)
      , _udaStack(udaStack)
      , _ucaStack(ucaStack)
      , _settings(settings)
      , _dimmingUrn(DEFAULT_DIMMING_URN)
      , _switchUrn(DEFAULT_SWITCH_URN)
      , _temperatureUrn(DEFAULT_TEMPERATURE_URN)
      , _udn()
      , _friendlyName()
      , _deviceType()

{
    const QString basePath = QDir::homePath() + "/." + appName + "/";

    loadDeviceData(basePath + DEVICE_PATH_SUFFIX);
    //loadDeviceDescriptions(basePath + DEVICE_PATH_SUFFIX, _descriptions, _serviceInfos);
    _dataModel = loadXml(basePath + DATA_MODEL_PATH_SUFFIX, "Data Model");

    /* TODO: it might be worthwhile to try to read sensor URNs form model */

    if (_udaStack != NULL)
        _udaStack->registerDevice(this);
    if (_ucaStack != NULL)
        _ucaStack->registerDevice(this);
}

LightSensor::~LightSensor()
{
    delete _transportService;
    delete _configService;

    delete _dataModel;
}

QMap<QString, QString> LightSensor::handleSOAP
    ( const QString &serviceId
    , void *serviceToken
    , const QString &actionName
    , const QHash<QString, QString> &arguments
    )
{
    _outArgs.clear();

    logSOAPCall(serviceId, actionName, arguments);

    if (serviceId == CFGMGMT_SERV_TYPE
        || serviceId == CFGMGMT_SERV_ID)
    {
        handleConfigManagement(actionName, serviceToken, arguments);
    }
    else if (serviceId == SENSORTRANS_SERV_TYPE
             || serviceId == SENSORTRANS_SERV_ID)
    {
        handleSensorTransport(actionName, serviceToken, arguments);
    }
    else
    {
        _outArgs["__errorCode"] = QString::number(401);
        _outArgs["__errorMessage"] = "Invalid action";
    }

    return _outArgs;
}

void LightSensor::handleConfigManagement
    ( const QString &actionName
    , void *serviceToken
    , const QHash<QString, QString> &arguments
    )
{
    Q_UNUSED(serviceToken)

    if (actionName == "GetValues") {
        if (arguments.contains("Parameters")) {
            QString parameters = arguments["Parameters"];
            QStringList paths = parseGetValuesInput(parameters);
            QHash<QString, QString> values;
            foreach (QString path, paths) {
                getValueFromDataModel(path, *_dataModel, values);
            }
            QString result = packGetValuesResult(values);
            _outArgs["ParameterValueList"] = result;
        } else {
            _outArgs["__errorCode"]    = QString::number(402);
            _outArgs["__errorMessage"] = "Missing argument 'Parameters'";
        }
    } else {
        QStringList actionNames;
        actionNames << "GetSupportedDataModels"
                    << "GetSupportedParameters"
                    << "GetInstances"
                    << "GetValues"
                    << "GetAttributes"
                    << "GetConfigurationUpdate"
                    << "GetCurrentConfigurationVersion"
                    << "GetSupportedDataModelsUpdate"
                    << "GetSupportedParametersUpdate"
                    ;

        if (actionNames.contains(actionName)) {
            _outArgs["__errorCode"] = QString::number(402);
            _outArgs["__errorMessage"] = "Invalid argument";
        } else {
            _outArgs["__errorCode"] = QString::number(401);
            _outArgs["__errorMessage"] = "Invalid action";
        }
    }
}

void LightSensor::handleSensorTransport
    ( const QString &actionName
    , void *serviceToken
    , const QHash<QString, QString> &arguments
    )
{
    Q_UNUSED(serviceToken)

    if (actionName == "ReadSensor") {
        handleReadSensor(arguments, _outArgs);
    } else if (actionName == "WriteSensor") {
        handleWriteSensor(arguments, _outArgs);
    } else {
        QStringList actionNames;
        actionNames << "ConnectSensor"
                    << "DisconnectSensor"
                    << "GetSensorTransportConnections"
                    ;

        if (actionNames.contains(actionName)) {
            _outArgs["__errorCode"] = QString::number(402);
            _outArgs["__errorMessage"] = "Invalid argument";
        } else {
            _outArgs["__errorCode"] = QString::number(401);
            _outArgs["__errorMessage"] = "Invalid action";
        }
    }
}

void LightSensor::handleWriteSensor
    ( const QHash<QString, QString> &arguments
    , QMap<QString, QString> &results
    )
{
    QStringList missingArgs;
    QStringList expectedArgs;
    expectedArgs << "SensorID" <<  "SensorURN" << "DataRecords";

    if (checkIfAllArgumentsPresent(expectedArgs, arguments, missingArgs)) {
        results["__errorCode"] = QString::number(402);
        results["__errorMessage"]
            = "Missing arguments: " + missingArgs.join(", ");
        return;
    }

    QString urn = arguments["SensorURN"].trimmed();

    Failable<int> maybeValue
            = parseFirstDataRecordFieldValue(arguments["DataRecords"]);
    if (maybeValue.hasValue() == false) {
        results["__errorCode"] = QString::number(706);
        results["__errorMessage"] = "Failed to parse 'DataRecords'.";
        return;
    }

    if (urn == _dimmingUrn) {
        setLightLoadLevel(maybeValue.value());
    } else if (urn == _switchUrn) {
        setLightEnabled(maybeValue.value());
    } else if (urn == _temperatureUrn) {
        results["__errorCode"] = QString::number(706);
        results["__errorMessage"] = "Sensor is read-only.";
        return;
    } else {
        results["__errorCode"] = QString::number(703);
        results["__errorMessage"] = "Unknown sensor";
        return;
    }
}

void LightSensor::handleReadSensor
    ( const QHash<QString, QString> &arguments
    , QMap<QString, QString> &results
    )
{
    QStringList missingArgs;
    QStringList expectedArgs;
    expectedArgs << "SensorID" <<  "SensorClientID" << "SensorURN"
                 << "SensorRecordInfo" << "SensorDataTypeEnable"
                 << "DataRecordCount";

    if (checkIfAllArgumentsPresent(expectedArgs, arguments, missingArgs)) {
        results["__errorCode"] = QString::number(402);
        results["__errorMessage"]
            = "Missing arguments: " + missingArgs.join(", ");
        return;
    }

    QString urn = arguments["SensorURN"].trimmed();

    QString fieldName("unknown");
    QString fieldType("unknown");
    QString value("unknown");

    if (urn == _dimmingUrn) {
        fieldName = "Dimming";
        fieldType = "uda:ui4";
        value = QString::number(_loadLevel);
    } else if (urn == _switchUrn) {
        fieldName = "Switch";
        fieldType = "uda:boolean";
        value = QString::number(_enabled);
    } else if (urn == _temperatureUrn) {
        fieldName = "Temperature Sensor";
        fieldType = "uda:ui4";
        value = QString::number((qrand() % 20) + 283);
    } else {
        results["__errorCode"] = QString::number(703);
        results["__errorMessage"] = "Unknown sensor";
        return;
    }

    QString records
            = QString(READ_VALUES_RESPONSE_TEMPLATE).arg(fieldName, fieldType, value);
    results["DataRecords"] = records;
}

void LightSensor::enableUca(bool status)
{
    if (status) {
        _ucaStack->start();
    } else {
        _ucaStack->stop();
    }
}

void LightSensor::enableUda(bool status)
{
    if (_udaStack == NULL) return;

    if (status) {
        _udaStack->start();
    } else {
        _udaStack->stop();
    }
}

QList<QString> fetchVariableNames(const QDomDocument *serviceDesc, bool eventableOnly)
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

        /* TODO: each of the chained calls in failable */
        QString name
            = variable.elementsByTagName("name").at(0).toElement().text();
        names.append(name);
    }

    return names;
}

void LightSensor::loadDeviceData(const QString &rootDevicePath)
{
    _deviceDescription = loadXml(rootDevicePath, "Root Device");
    if (_deviceDescription == NULL) {
        fprintf(stderr, "Failed to load device description!");
        return;
    }

    setDeviceProperties(_deviceDescription);
    readDeviceProperties(_deviceDescription);

    QHash<QString, ServiceInfo *> infos;

    QFileInfo rootFileInfo(rootDevicePath);
    QDir rootDirectory = rootFileInfo.dir();

    loadServices(_deviceDescription, infos, rootDirectory);

    QHash<QString, ServiceInfo *>::const_iterator it = infos.constBegin();
    for (; it != infos.constEnd(); it++) {
        QString name = it.key();
        ServiceInfo *info = it.value();
        if (name == SENSORTRANS_SERV_ID) {
            _transportService = new (std::nothrow) SensorTransportGenericService(info);
        } else if (name == CFGMGMT_SERV_ID) {
            _configService = new (std::nothrow) ConfigurationManagementService(info);
        }
    }
}

Failable<bool> LightSensor::setDeviceProperties(QDomDocument *desc)
{
    if (_settings == NULL) {
        Failable<bool>::Failure("The settings are missing.");
    }

    QDomElement udnNode = desc->elementsByTagName("UDN").item(0).toElement();
    if (udnNode.isNull()) {
        Failable<bool>::Failure("Couldn't find UDN node.");
    } else {
        qDebug() << "Found" << udnNode.tagName() << "with text value:" << udnNode.text();
    }

    QDomElement nameNode = desc->elementsByTagName("friendlyName").item(0).toElement();
    if (nameNode.isNull()) {
        Failable<bool>::Failure("Couldn't find friendlyName node.");
    } else {
        qDebug() << "Found" << nameNode.tagName() << "with text value:" << nameNode.text();
    }

    _udn          = _settings->getSetting(QString("upnp/udn"));
    _friendlyName = _settings->getSetting(QString("upnp/friendlyName"));
    /* TODO: the assertion that the both elements really have text nodes inside
     *       can badly fail */
    udnNode.firstChild().setNodeValue(_udn);
    nameNode.firstChild().setNodeValue(_friendlyName);

    return true;
}

void LightSensor::readDeviceProperties(QDomDocument *desc)
{
    QDomElement type = desc->elementsByTagName("deviceType").item(0).toElement();
    _deviceType = type.text();
}

void LightSensor::unloadDeviceDescriptions(QHash<QString, QDomDocument *> &descs)
{
    QHash<QString, QDomDocument *>::const_iterator it = descs.constBegin();
    for (; it != descs.constEnd(); it++) {
        delete it.value();
    }
}

const QList<QString> LightSensor::getServiceIds() const
{
    QList<QString> list;

    if (_transportService != NULL) list.append(_transportService->getServiceId());
    if (_configService != NULL) list.append(_configService->getServiceId());

    return list;
}

const IUPnPService *LightSensor::getService(const QString &id) const {
    if (_transportService != NULL && _transportService->getServiceId() == id)
        return _transportService;
    if (_configService != NULL && _configService->getServiceId() == id)
        return _configService;

    return NULL;
}

const QString LightSensor::serviceIdForType(const QString &type) const
{
    if (_transportService != NULL && _transportService->getServiceType() == type)
        return _transportService->getServiceId();
    if (_configService != NULL && _configService->getServiceType() == type)
        return _configService->getServiceId();

    return QString();
}

const QDomDocument &LightSensor::getDescription(const QString &id) const
{
    if (_transportService != NULL && _transportService->getServiceId() == id)
        return _transportService->getServiceDescription();
    if (_configService != NULL && _configService->getServiceId() == id)
        return _configService->getServiceDescription();

    return *_deviceDescription;
}

void LightSensor::stackStatusChanged(const QString &name, bool connected)
{
    Q_UNUSED(connected)

    if (name.contains("uca", Qt::CaseInsensitive)) {
        if (_ucaStack != NULL)
            emit ucaStackEnabledChanged(_ucaStack->isRunning());
    } else if (name.contains("uda", Qt::CaseInsensitive)) {
        if (_udaStack != NULL)
            emit udaStackEnabledChanged(_udaStack->isRunning());
    }
}

void LightSensor::setLightLoadLevel(int level){
    level = std::max(0, std::min(100, level));
    this->_loadLevel = level;
    emit this->lightLoadLevelChanged(level);
}

void LightSensor::setLightEnabled(int enabled){
    enabled = enabled > 0;
    this->_enabled = enabled;
    emit this->lightEnabledChanged(enabled);
}
