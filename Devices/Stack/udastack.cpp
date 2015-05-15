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

#include "udastack.h"

#include <QString>
#include <QHash>
#include <QDir>

#include <QDomDocument>
#include <QTextStream>

#include <Stack/iupnpservice.h>

const char *DEVICE_XML = "device.xml";

const char *TEMP_DESCRIPTION_DIR_PATH = "tmp-descs";

static const char *copy_qstring(QString string)
{
    int count = string.count() + 1;
    char *buffer = (char *)malloc(sizeof(char) * count);
    if (buffer == NULL) {
        return NULL;
    }

    const QChar *data = string.constData();
    char *c = buffer;
    while (*data != '\0') {
        *c = data->toLatin1();
        c++;
        data++;
    }
    *c = '\0';

    return buffer;
}

static bool is_error_message(const QMap<QString, QString> &rawOutput) {
    return rawOutput.contains("__errorCode");
}

static action_output_t pack_error_output(const QMap<QString, QString> &raw_output)
{
    //error_code; error_message; action_out_names; action_out_values;
    action_output_t packed = {0, NULL, NULL, NULL};

    QString errorCode = raw_output.value("__errorCode");
    QString errorMessage = raw_output.value("__errorMessage");

    packed.error_code = errorCode.toUInt();
    packed.error_message = copy_qstring(errorMessage);

    return packed;
}

static action_output_t pack_output(const QMap<QString, QString> &raw_output)
{
    //error_code; error_message; action_out_names; action_out_values;
    action_output_t packed = {0, NULL, NULL, NULL};

    int count = raw_output.count() + 1;
    const char **names  = (const char **)malloc(sizeof(char *) * count);
    const char **values = (const char **)malloc(sizeof(char *) * count);

    if (names == NULL || values == NULL) {
        return packed;
    }

    int i = 0;
    QMap<QString, QString>::const_iterator it = raw_output.constBegin();
    while (it != raw_output.constEnd()) {
        QString rawName  = it.key();
        QString rawValue = it.value();

        *(names  + i) = copy_qstring(rawName);
        *(values + i) = copy_qstring(rawValue);

        ++it;
        ++i;
    }

    *(names + count - 1) = NULL;
    *(values + count - 1) = NULL;

    packed.action_out_names  = names;
    packed.action_out_values = values;

    return packed;
}

static action_output_t global_action_handler(const action_args_t args)
{
    action_output_t packed_output;
    packed_output.error_code = 0;
    packed_output.error_message = NULL;
    packed_output.action_out_names = NULL;
    packed_output.action_out_values = NULL;

    UdaStack *udaStack = UdaStack::getInstance();

    IUPnPDevice *device =  udaStack->getDevice();
    if (device == NULL)
        return packed_output;

    QString serv_id(args.service_id);
    QString act_name(args.action_name);

    QHash<QString, QString> arguments;
    int i = 0;
    for (const char **name = args.action_arg_names; *name != NULL; name++) {
        QString arg_name(*name);
        QString arg_value(*(args.action_arg_values + i));
        arguments[arg_name] = arg_value;
        i++;
    }

    const QMap<QString, QString> &output = device->handleSOAP(serv_id, args.service_token, act_name, arguments);
    if (is_error_message(output)) {
        packed_output = pack_error_output(output);
    } else {
        packed_output = pack_output(output);
    }

    return packed_output;
}

UdaStack::UdaStack()
    : _isInitialized(false)
    , _isRunning(false)
{
}

static Failable<bool> saveXmlToFile( const QString &path
                                   , const QDomDocument &content
                                   )
{
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text) == false) {
        return Failable<bool>::Failure(QString("Couldn't open %1 for writing.").arg(path));
    }

    QTextStream stream(&file);
    stream << content.toString();

    file.close();

    return true;
}

static Failable<bool> setupTempDirectory( const IUPnPDevice *device
                                        , const QString &path
                                        , const QString &deviceFileName
                                        )
{
    bool directoryExists = QDir(path).exists();
    if (directoryExists == false) {
        bool success = QDir().mkpath(path);
        if (success == false) {
            return Failable<bool>::Failure("Couldn't create directory: " + path);
        }
    }

    const QDomDocument description = device->getDescription(QString());
    QString filePath(path + "/" + deviceFileName);
    Failable<bool> result = saveXmlToFile(filePath, description);
    if (result.hasValue() == false) {
        return result;
    }

    QStringList serviceIds = device->getServiceIds();
    foreach (QString id, serviceIds) {
        const IUPnPService *service = device->getService(id);

        filePath = QString(path + "/" + service->getScdpPath().toString());
        const QDomDocument scpd = service->getServiceDescription();
        result = saveXmlToFile(filePath, scpd);
        if (result.hasValue() == false) {
            return result;
        }
    }

    return true;
}

Failable<bool> UdaStack::initialize(IUPnPDevice *device)
{
    _device = device;

    Failable<bool> result
        = setupTempDirectory( _device
                            , QString(TEMP_DESCRIPTION_DIR_PATH)
                            , QString(DEVICE_XML)
                            );

    if (result.hasValue() == false) {
        return Failable<bool>::Propagate("Failed to create files for GUPnP", result);
    }

    upnp_params_t params;
    params.description_dir  = TEMP_DESCRIPTION_DIR_PATH;
    params.root_description = DEVICE_XML;

    _stack = create_uda_stack(NULL, &params, global_action_handler, NULL);
    if (_stack == NULL) {
        return Failable<bool>::Failure("Failed to create UDA stack.");
    }

    _isInitialized = true;
    return true;
}

UdaStack *UdaStack::getInstance()
{
    static UdaStack stack;
    return &stack;
}

void UdaStack::start()
{
    if (_isRunning == false) {
        start_uda_stack(_stack);
        _isRunning = true;
        _device->stackStatusChanged("uda", true);
    }
}

void UdaStack::stop()
{
    if (_isRunning) {
        stop_uda_stack(_stack);
        _isRunning = false;
        _device->stackStatusChanged("uda", false);
    }
}

void UdaStack::sendEvent(EventMessage eventMessage)
{
    const char *service_id = copy_qstring(eventMessage.serviceId);
    const char *var_name   = copy_qstring(eventMessage.variableName);
    const char *var_value  = copy_qstring(eventMessage.value);

    notification_params_t params;
    params.service_id = service_id;
    params.var_name   = var_name;
    params.value      = var_value;
    params.service_token = eventMessage.serviceToken;

    send_notification(_stack, params);
}

UdaStack::~UdaStack()
{
    if(this->isInitialized()){
        destroy_uda_stack(_stack);
    }
}

void UdaStack::registerDevice(IUPnPDevice *observer) {
    if (this->isInitialized()) {
        fprintf(stderr, "Cannot registerDevice after the stack has been initialized.");
        return;
    }
    _device = observer;
}
