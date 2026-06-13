#include "apps/serial_station/services/SerialProfileService.h"

#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QSaveFile>

namespace serial_station {

namespace {

constexpr int kCurrentSchemaVersion = 1;

QStringList supportedProtocols()
{
    return {
        QStringLiteral("ascii_text"),
        QStringLiteral("modbus_rtu"),
        QStringLiteral("custom_md"),
    };
}

QStringList supportedModes()
{
    return {
        QStringLiteral("ascii"),
        QStringLiteral("hex"),
        QStringLiteral("protocol"),
    };
}

} // namespace

QString SerialProfileService::toJson(const SerialStationProfile& profile,
                                     QString* errorMessage) const
{
    const QString error = validationError(profile);
    if (!error.isEmpty()) {
        setError(errorMessage, error);
        return {};
    }

    const QJsonDocument document(profileToJsonObject(profile));
    setError(errorMessage, {});
    return QString::fromUtf8(document.toJson(QJsonDocument::Indented));
}

SerialProfileResult SerialProfileService::fromJson(const QString& json) const
{
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(json.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        return {false, {}, makeError(QStringLiteral("JSON 解析失败: %1").arg(parseError.errorString()))};
    }
    if (!document.isObject()) {
        return {false, {}, makeError(QStringLiteral("JSON 顶层必须是对象"))};
    }

    SerialStationProfile profile;
    QString errorMessage;
    if (!profileFromJsonObject(document.object(), profile, errorMessage)) {
        return {false, {}, errorMessage};
    }

    const QString validation = validationError(profile);
    if (!validation.isEmpty()) {
        return {false, {}, validation};
    }

    return {true, profile, {}};
}

SerialProfileWriteResult SerialProfileService::saveToFile(
    const SerialStationProfile& profile,
    const QString& filePath) const
{
    const QString normalizedPath = filePath.trimmed();
    if (normalizedPath.isEmpty()) {
        return {false, filePath, 0, makeError(QStringLiteral("档案文件路径为空"))};
    }

    QString errorMessage;
    const QString json = toJson(profile, &errorMessage);
    if (json.isEmpty()) {
        return {false, normalizedPath, 0, errorMessage};
    }

    const QByteArray payload = json.toUtf8();
    QSaveFile file(normalizedPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return {false, normalizedPath, 0,
                makeError(QStringLiteral("写入档案失败: %1").arg(file.errorString()))};
    }

    const qint64 written = file.write(payload);
    if (written != payload.size()) {
        return {false, normalizedPath, written,
                makeError(QStringLiteral("写入档案不完整"))};
    }
    if (!file.commit()) {
        return {false, normalizedPath, written,
                makeError(QStringLiteral("保存档案失败: %1").arg(file.errorString()))};
    }

    return {true, normalizedPath, written, {}};
}

SerialProfileResult SerialProfileService::loadFromFile(const QString& filePath) const
{
    const QString normalizedPath = filePath.trimmed();
    QFile file(normalizedPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return {false, {}, makeError(QStringLiteral("读取档案失败: %1").arg(file.errorString()))};
    }

    const QByteArray payload = file.readAll();
    if (file.error() != QFile::NoError) {
        return {false, {}, makeError(QStringLiteral("读取档案失败: %1").arg(file.errorString()))};
    }

    return fromJson(QString::fromUtf8(payload));
}

QString SerialProfileService::validationError(const SerialStationProfile& profile) const
{
    if (profile.name.trimmed().isEmpty()) {
        return makeError(QStringLiteral("档案名称不能为空"));
    }
    if (!profile.port.isValid()) {
        return makeError(profile.port.validationError());
    }

    const QString protocolName = normalizedProtocolName(profile.protocolName);
    if (!isSupportedProtocol(protocolName)) {
        return makeError(QStringLiteral("协议名称不受支持: %1").arg(profile.protocolName.trimmed()));
    }

    const QString sendMode = normalizedSendMode(profile.sendMode);
    if (!isSupportedMode(sendMode)) {
        return makeError(QStringLiteral("发送模式不受支持: %1").arg(profile.sendMode.trimmed()));
    }

    for (int index = 0; index < profile.commands.size(); ++index) {
        const SerialProfileCommand& command = profile.commands.at(index);
        if (command.name.trimmed().isEmpty()) {
            return makeError(QStringLiteral("命令 %1 名称不能为空").arg(index + 1));
        }
        if (command.payload.trimmed().isEmpty()) {
            return makeError(QStringLiteral("命令 %1 内容不能为空").arg(index + 1));
        }
        const QString commandMode = normalizedCommandMode(command.mode);
        if (!isSupportedMode(commandMode)) {
            return makeError(QStringLiteral("命令 %1 发送模式不受支持").arg(index + 1));
        }
    }

    return {};
}

QString SerialProfileService::normalizedProtocolName(const QString& protocolName)
{
    return protocolName.trimmed().toLower();
}

QString SerialProfileService::normalizedSendMode(const QString& mode)
{
    return mode.trimmed().toLower();
}

QString SerialProfileService::normalizedCommandMode(const QString& mode)
{
    return mode.trimmed().toLower();
}

bool SerialProfileService::isSupportedProtocol(const QString& protocolName)
{
    return supportedProtocols().contains(normalizedProtocolName(protocolName));
}

bool SerialProfileService::isSupportedMode(const QString& mode)
{
    return supportedModes().contains(normalizedSendMode(mode));
}

QString SerialProfileService::portParityName(QSerialPort::Parity parity)
{
    switch (parity) {
    case QSerialPort::NoParity:
        return QStringLiteral("none");
    case QSerialPort::EvenParity:
        return QStringLiteral("even");
    case QSerialPort::OddParity:
        return QStringLiteral("odd");
    case QSerialPort::SpaceParity:
        return QStringLiteral("space");
    case QSerialPort::MarkParity:
        return QStringLiteral("mark");
    }
    return QStringLiteral("unknown");
}

QString SerialProfileService::portStopBitsName(QSerialPort::StopBits stopBits)
{
    switch (stopBits) {
    case QSerialPort::OneStop:
        return QStringLiteral("1");
    case QSerialPort::OneAndHalfStop:
        return QStringLiteral("1.5");
    case QSerialPort::TwoStop:
        return QStringLiteral("2");
    }
    return QStringLiteral("unknown");
}

QString SerialProfileService::portFlowControlName(QSerialPort::FlowControl flowControl)
{
    switch (flowControl) {
    case QSerialPort::NoFlowControl:
        return QStringLiteral("none");
    case QSerialPort::HardwareControl:
        return QStringLiteral("hardware");
    case QSerialPort::SoftwareControl:
        return QStringLiteral("software");
    }
    return QStringLiteral("unknown");
}

bool SerialProfileService::parseParity(const QString& text, QSerialPort::Parity& parity)
{
    const QString normalized = text.trimmed().toLower();
    if (normalized == QStringLiteral("none")) {
        parity = QSerialPort::NoParity;
    } else if (normalized == QStringLiteral("even")) {
        parity = QSerialPort::EvenParity;
    } else if (normalized == QStringLiteral("odd")) {
        parity = QSerialPort::OddParity;
    } else if (normalized == QStringLiteral("space")) {
        parity = QSerialPort::SpaceParity;
    } else if (normalized == QStringLiteral("mark")) {
        parity = QSerialPort::MarkParity;
    } else {
        return false;
    }
    return true;
}

bool SerialProfileService::parseStopBits(const QString& text, QSerialPort::StopBits& stopBits)
{
    const QString normalized = text.trimmed().toLower();
    if (normalized == QStringLiteral("1")) {
        stopBits = QSerialPort::OneStop;
    } else if (normalized == QStringLiteral("1.5")) {
        stopBits = QSerialPort::OneAndHalfStop;
    } else if (normalized == QStringLiteral("2")) {
        stopBits = QSerialPort::TwoStop;
    } else {
        return false;
    }
    return true;
}

bool SerialProfileService::parseFlowControl(const QString& text,
                                            QSerialPort::FlowControl& flowControl)
{
    const QString normalized = text.trimmed().toLower();
    if (normalized == QStringLiteral("none")) {
        flowControl = QSerialPort::NoFlowControl;
    } else if (normalized == QStringLiteral("hardware")) {
        flowControl = QSerialPort::HardwareControl;
    } else if (normalized == QStringLiteral("software")) {
        flowControl = QSerialPort::SoftwareControl;
    } else {
        return false;
    }
    return true;
}

QJsonObject SerialProfileService::portToJson(const SerialPortConfig& port)
{
    const SerialPortConfig normalized = port.normalized();
    return {
        {QStringLiteral("portName"), normalized.portName},
        {QStringLiteral("baudRate"), normalized.baudRate},
        {QStringLiteral("dataBits"), static_cast<int>(normalized.dataBits)},
        {QStringLiteral("parity"), portParityName(normalized.parity)},
        {QStringLiteral("stopBits"), portStopBitsName(normalized.stopBits)},
        {QStringLiteral("flowControl"), portFlowControlName(normalized.flowControl)},
        {QStringLiteral("dtrEnabled"), normalized.dtrEnabled},
        {QStringLiteral("rtsEnabled"), normalized.rtsEnabled},
    };
}

bool SerialProfileService::portFromJson(const QJsonObject& object,
                                        SerialPortConfig& port,
                                        QString& errorMessage)
{
    if (object.isEmpty()) {
        errorMessage = makeError(QStringLiteral("缺少串口配置"));
        return false;
    }

    QSerialPort::Parity parity = QSerialPort::NoParity;
    QSerialPort::StopBits stopBits = QSerialPort::OneStop;
    QSerialPort::FlowControl flowControl = QSerialPort::NoFlowControl;
    if (!parseParity(object.value(QStringLiteral("parity")).toString(QStringLiteral("none")), parity)) {
        errorMessage = makeError(QStringLiteral("串口校验位不受支持"));
        return false;
    }
    if (!parseStopBits(object.value(QStringLiteral("stopBits")).toString(QStringLiteral("1")), stopBits)) {
        errorMessage = makeError(QStringLiteral("串口停止位不受支持"));
        return false;
    }
    if (!parseFlowControl(object.value(QStringLiteral("flowControl")).toString(QStringLiteral("none")),
                          flowControl)) {
        errorMessage = makeError(QStringLiteral("串口流控不受支持"));
        return false;
    }

    port.portName = object.value(QStringLiteral("portName")).toString().trimmed();
    port.baudRate = object.value(QStringLiteral("baudRate")).toInt(115200);
    port.dataBits =
        static_cast<QSerialPort::DataBits>(object.value(QStringLiteral("dataBits")).toInt(8));
    port.parity = parity;
    port.stopBits = stopBits;
    port.flowControl = flowControl;
    port.dtrEnabled = object.value(QStringLiteral("dtrEnabled")).toBool(false);
    port.rtsEnabled = object.value(QStringLiteral("rtsEnabled")).toBool(false);
    return true;
}

QJsonObject SerialProfileService::commandToJson(const SerialProfileCommand& command)
{
    return {
        {QStringLiteral("name"), command.name.trimmed()},
        {QStringLiteral("payload"), command.payload.trimmed()},
        {QStringLiteral("mode"), normalizedCommandMode(command.mode)},
    };
}

bool SerialProfileService::commandFromJson(const QJsonObject& object,
                                           int index,
                                           SerialProfileCommand& command,
                                           QString& errorMessage)
{
    if (object.isEmpty()) {
        errorMessage = makeError(QStringLiteral("命令 %1 必须是对象").arg(index + 1));
        return false;
    }

    command.name = object.value(QStringLiteral("name")).toString().trimmed();
    command.payload = object.value(QStringLiteral("payload")).toString().trimmed();
    command.mode = normalizedCommandMode(object.value(QStringLiteral("mode")).toString(QStringLiteral("ascii")));
    if (command.name.isEmpty()) {
        errorMessage = makeError(QStringLiteral("命令 %1 名称不能为空").arg(index + 1));
        return false;
    }
    if (command.payload.isEmpty()) {
        errorMessage = makeError(QStringLiteral("命令 %1 内容不能为空").arg(index + 1));
        return false;
    }
    if (!isSupportedMode(command.mode)) {
        errorMessage = makeError(QStringLiteral("命令 %1 发送模式不受支持").arg(index + 1));
        return false;
    }
    return true;
}

QJsonObject SerialProfileService::profileToJsonObject(const SerialStationProfile& profile)
{
    QJsonArray tags;
    for (const QString& tag : profile.tags) {
        const QString normalizedTag = tag.trimmed();
        if (!normalizedTag.isEmpty()) {
            tags.append(normalizedTag);
        }
    }

    QJsonArray commands;
    for (const SerialProfileCommand& command : profile.commands) {
        commands.append(commandToJson(command));
    }

    return {
        {QStringLiteral("schemaVersion"), kCurrentSchemaVersion},
        {QStringLiteral("name"), profile.name.trimmed()},
        {QStringLiteral("description"), profile.description.trimmed()},
        {QStringLiteral("tags"), tags},
        {QStringLiteral("port"), portToJson(profile.port)},
        {QStringLiteral("protocolName"), normalizedProtocolName(profile.protocolName)},
        {QStringLiteral("sendMode"), normalizedSendMode(profile.sendMode)},
        {QStringLiteral("commands"), commands},
    };
}

bool SerialProfileService::profileFromJsonObject(const QJsonObject& object,
                                                 SerialStationProfile& profile,
                                                 QString& errorMessage)
{
    profile.schemaVersion = object.value(QStringLiteral("schemaVersion")).toInt(kCurrentSchemaVersion);
    profile.name = object.value(QStringLiteral("name")).toString().trimmed();
    profile.description = object.value(QStringLiteral("description")).toString().trimmed();

    profile.tags.clear();
    const QJsonArray tags = object.value(QStringLiteral("tags")).toArray();
    for (const QJsonValue& tag : tags) {
        const QString normalizedTag = tag.toString().trimmed();
        if (!normalizedTag.isEmpty()) {
            profile.tags.append(normalizedTag);
        }
    }

    if (!portFromJson(object.value(QStringLiteral("port")).toObject(), profile.port, errorMessage)) {
        return false;
    }
    profile.port = profile.port.normalized();
    profile.protocolName = normalizedProtocolName(
        object.value(QStringLiteral("protocolName")).toString(QStringLiteral("ascii_text")));
    profile.sendMode = normalizedSendMode(
        object.value(QStringLiteral("sendMode")).toString(QStringLiteral("ascii")));

    profile.commands.clear();
    const QJsonArray commands = object.value(QStringLiteral("commands")).toArray();
    for (int index = 0; index < commands.size(); ++index) {
        SerialProfileCommand command;
        if (!commandFromJson(commands.at(index).toObject(), index, command, errorMessage)) {
            return false;
        }
        profile.commands.append(command);
    }

    return true;
}

QString SerialProfileService::makeError(const QString& message)
{
    return message.trimmed();
}

void SerialProfileService::setError(QString* target, const QString& message)
{
    if (target) {
        *target = message;
    }
}

} // namespace serial_station
