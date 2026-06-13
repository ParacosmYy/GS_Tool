#include "apps/serial_station/SerialStationController.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QStringList>
#include <QtCore/QVariantList>

#include <memory>

#include "apps/serial_station/SerialStationConstants.h"

namespace serial_station {
namespace {

QString measurementPayloadText(const SerialProtocolEvent& event)
{
    const QVariantList values = event.payload.value(QStringLiteral("values")).toList();
    if (values.isEmpty()) {
        return QString();
    }

    QStringList parts;
    parts.reserve(values.size());
    for (int i = 0; i < values.size(); ++i) {
        parts.append(QCoreApplication::translate("SerialStationController", "ch%1=%2")
                         .arg(QString::number(i + 1),
                              QString::number(values.at(i).toDouble(), 'g', 6)));
    }

    const QString format = event.payload.value(QStringLiteral("format")).toString();
    const QString label = format.compare(QStringLiteral("just_float"), Qt::CaseInsensitive) == 0
                              ? QStringLiteral("JustFloat")
                              : format.trimmed();
    return QCoreApplication::translate("SerialStationController", "%1 measurement: %2")
        .arg(label.isEmpty() ? event.protocolName : label, parts.join(QStringLiteral(", ")));
}

} // namespace

SerialStationController::SerialStationController(QObject* parent)
    : QObject(parent)
    , m_serialManager(this)
{
    m_protocols.registerBuiltInProtocols();
    resetReceiveDispatcher();
    connect(&m_serialManager, &SerialManager::stateChanged,
            this, &SerialStationController::serialStateChanged);
    connect(&m_serialManager, &SerialManager::errorOccurred,
            this, &SerialStationController::handleSerialManagerError);
    connect(&m_serialManager, &SerialManager::bytesReceived,
            this, &SerialStationController::handleBytesReceived);
}

SerialProtocolRegistry& SerialStationController::protocols()
{
    return m_protocols;
}

QStringList SerialStationController::availableProtocolNames() const
{
    return m_protocols.protocolNames();
}

QString SerialStationController::activeProtocolName() const
{
    return m_protocols.defaultProtocol();
}

SerialManager& SerialStationController::serialManager()
{
    return m_serialManager;
}

void SerialStationController::connectSerialPort(const SerialPortConfig& config)
{
    resetReceiveDispatcher();
    const QString validationError = config.validationError();
    if (!validationError.isEmpty()) {
        m_serialManager.rejectConfiguration(config, validationError);
        emit serialErrorCounted();
        logSystem(validationError, {{QStringLiteral("reason"), validationError}});
        return;
    }

    logSystem(tr("正在打开串口: %1").arg(config.summary()),
              {{QStringLiteral("portName"), config.portName.trimmed()},
               {QStringLiteral("baudRate"), config.baudRate}});
    m_serialManager.configure(config);
    if (!m_serialManager.open()) {
        emit serialErrorOccurred(m_serialManager.session().errorString());
    }
}

void SerialStationController::disconnectSerialPort()
{
    m_serialManager.close();
    m_dispatcher.reset();
}

void SerialStationController::sendCommand(const QString& command, const QString& mode)
{
    const QString trimmedCommand = command.trimmed();
    const QString normalizedMode = normalizeSendMode(mode);

    const SerialCodec::EncodeResult encodeResult =
        buildCommandFrame(trimmedCommand, normalizedMode);
    if (!encodeResult.ok) {
        emitSendFailure(trimmedCommand, encodeResult.normalizedMode, encodeResult.errorMessage);
        return;
    }

    if (!m_serialManager.session().isOpen()) {
        emitSendFailure(trimmedCommand, normalizedMode, tr("串口未连接，无法发送"));
        return;
    }

    emit serialCommandPrepared(trimmedCommand, encodeResult.normalizedMode, encodeResult.frame);

    const qint64 bytesWritten = m_serialManager.send(encodeResult.frame);
    if (bytesWritten <= 0) {
        emitSendFailure(trimmedCommand, encodeResult.normalizedMode, tr("串口写入失败"));
        return;
    }

    const QString logText = tr("%1 [%2] %3 bytes")
                                .arg(trimmedCommand,
                                     encodeResult.normalizedMode,
                                     QString::number(bytesWritten));
    emit serialTxCounted();
    logTx(logText,
          encodeResult.frame,
          {{QStringLiteral("command"), trimmedCommand},
           {QStringLiteral("mode"), encodeResult.normalizedMode},
           {QStringLiteral("bytesWritten"), bytesWritten}});
    emit serialCommandSent(trimmedCommand, encodeResult.normalizedMode, bytesWritten);
}

void SerialStationController::handleBytesReceived(const QByteArray& bytes)
{
    const QVector<SerialProtocolEvent> events = m_dispatcher.feed(bytes);
    const SerialDispatcher::FeedSummary summary = m_dispatcher.lastFeedSummary();

    if (summary.status == SerialDispatcher::FeedStatus::EmptyInput) {
        return;
    }

    if (summary.status == SerialDispatcher::FeedStatus::MissingProtocol) {
        emit serialErrorCounted();
        logError(tr("接收协议不可用，已丢弃 %1 bytes").arg(summary.inputBytes),
                 {{QStringLiteral("inputBytes"), summary.inputBytes}});
        return;
    }

    if (summary.status == SerialDispatcher::FeedStatus::Buffered) {
        logSystem(bufferedReceiveText(summary),
                  {{QStringLiteral("inputBytes"), summary.inputBytes},
                   {QStringLiteral("protocolName"), summary.protocolName}});
        return;
    }

    for (const SerialProtocolEvent& event : events) {
        processProtocolEvent(event);
    }
}

QString SerialStationController::normalizeSendMode(const QString& mode) const
{
    return m_codec.normalizeMode(mode);
}

SerialCodec::EncodeResult SerialStationController::buildCommandFrame(const QString& command,
                                                                     const QString& mode) const
{
    const std::unique_ptr<ISerialProtocol> protocol = m_protocols.createDefault();
    return m_codec.encode(command, mode, protocol.get());
}

void SerialStationController::resetReceiveDispatcher()
{
    m_dispatcher.setProtocol(m_protocols.createDefault());
}

void SerialStationController::clearLogRecords()
{
    m_logService.clear();
}

void SerialStationController::setActiveProtocol(const QString& protocolName)
{
    const QString trimmed = protocolName.trimmed();
    const QString previous = m_protocols.defaultProtocol();

    if (trimmed.isEmpty() || !m_protocols.contains(trimmed)) {
        emit serialErrorCounted();
        logError(tr("串口协议不可用: %1").arg(trimmed.isEmpty() ? tr("<empty>") : trimmed),
                 {{QStringLiteral("protocolName"), trimmed},
                  {QStringLiteral("previousProtocol"), previous}});
        emit activeProtocolChanged(previous);
        return;
    }

    if (trimmed == previous) {
        emit activeProtocolChanged(previous);
        return;
    }

    m_protocols.setDefaultProtocol(trimmed);
    resetReceiveDispatcher();
    logSystem(tr("已切换串口协议: %1").arg(trimmed),
              {{QStringLiteral("protocolName"), trimmed},
               {QStringLiteral("previousProtocol"), previous}});
    emit activeProtocolChanged(trimmed);
}

void SerialStationController::handleSerialManagerError(const QString& message)
{
    m_logService.appendError(message,
                             QStringLiteral("serial_manager"),
                             {{QStringLiteral("source"), QStringLiteral("serial_manager")}});
    emit serialErrorOccurred(message);
}

void SerialStationController::processProtocolEvent(const SerialProtocolEvent& event)
{
    if (event.type == serialStationConstants::kAsciiFrameType) {
        const QString text = eventPayloadText(event);
        emit serialRxCounted();
        logRx(text,
              event.raw,
              {{QStringLiteral("eventType"), event.type},
               {QStringLiteral("protocolName"), event.protocolName}});
        return;
    }

    if (event.type == serialStationConstants::kLogType) {
        const QString text = eventPayloadText(event);
        logSystem(text,
                  {{QStringLiteral("eventType"), event.type},
                   {QStringLiteral("protocolName"), event.protocolName}});
        return;
    }

    if (event.type == QStringLiteral("measurement")) {
        const QString measurementText = measurementPayloadText(event);
        const QString text = measurementText.isEmpty() ? eventPayloadText(event) : measurementText;
        emit serialRxCounted();
        logRx(text,
              event.raw,
              {{QStringLiteral("eventType"), event.type},
               {QStringLiteral("protocolName"), event.protocolName},
               {QStringLiteral("channelCount"),
                event.payload.value(QStringLiteral("channelCount")).toInt()}});
        return;
    }

    emit serialErrorCounted();
    logError(tr("未知接收事件: %1").arg(event.type),
             {{QStringLiteral("eventType"), event.type},
              {QStringLiteral("protocolName"), event.protocolName}});
}

QString SerialStationController::eventPayloadText(const SerialProtocolEvent& event) const
{
    const QString text = event.payload.value(QStringLiteral("text")).toString().trimmed();
    if (!text.isEmpty()) {
        return text;
    }

    const QString message = event.payload.value(QStringLiteral("message")).toString().trimmed();
    if (!message.isEmpty()) {
        return message;
    }

    if (!event.raw.isEmpty()) {
        return rawBytesSummary(event.raw);
    }

    return tr("空接收事件");
}

QString SerialStationController::rawBytesSummary(const QByteArray& bytes) const
{
    if (bytes.isEmpty()) {
        return tr("<empty>");
    }

    return QString::fromLatin1(bytes.toHex(' ').toUpper());
}

QString SerialStationController::bufferedReceiveText(const SerialDispatcher::FeedSummary& summary) const
{
    if (summary.protocolName.isEmpty()) {
        return tr("接收缓存 %1 bytes，等待完整帧").arg(summary.inputBytes);
    }

    return tr("接收缓存 %1 bytes，等待 %2 完整帧")
        .arg(QString::number(summary.inputBytes), summary.protocolName);
}

void SerialStationController::logTx(const QString& text,
                                    const QByteArray& payload,
                                    const QVariantMap& fields)
{
    m_logService.appendTx(text, payload, QStringLiteral("controller"), fields);
    emit serialTxLogged(text);
}

void SerialStationController::logRx(const QString& text,
                                    const QByteArray& payload,
                                    const QVariantMap& fields)
{
    m_logService.appendRx(text, payload, QStringLiteral("protocol"), fields);
    emit serialRxLogged(text);
}

void SerialStationController::logSystem(const QString& text, const QVariantMap& fields)
{
    m_logService.appendSystem(text, QStringLiteral("controller"), fields);
    emit serialSystemLogged(text);
}

void SerialStationController::logError(const QString& text, const QVariantMap& fields)
{
    m_logService.appendError(text, QStringLiteral("controller"), fields);
    emit serialSystemLogged(text);
}

void SerialStationController::emitSendFailure(const QString& command,
                                              const QString& mode,
                                              const QString& message)
{
    QString logText = message;
    if (!command.isEmpty()) {
        logText = tr("%1: %2").arg(command, message);
    }

    emit serialErrorCounted();
    logError(logText,
             {{QStringLiteral("command"), command},
              {QStringLiteral("mode"), mode},
              {QStringLiteral("message"), message}});
    emit serialCommandFailed(command, mode, message);
}

} // namespace serial_station
