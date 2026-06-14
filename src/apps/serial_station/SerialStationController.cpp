#include "apps/serial_station/SerialStationController.h"

#include <QtCore/QMetaType>
#include <QtCore/QStringList>

#include "apps/serial_station/SerialStationConstants.h"

namespace serial_station {

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
        emitSendFailure(trimmedCommand,
                        encodeResult.normalizedMode,
                        encodeResult.errorMessage,
                        QStringLiteral("encode"));
        return;
    }

    if (!m_serialManager.session().isOpen()) {
        emitSendFailure(trimmedCommand,
                        normalizedMode,
                        tr("串口未连接，无法发送"),
                        QStringLiteral("not_connected"));
        return;
    }

    emit serialCommandPrepared(trimmedCommand, encodeResult.normalizedMode, encodeResult.frame);

    const qint64 bytesWritten = m_serialManager.send(encodeResult.frame);
    if (bytesWritten < static_cast<qint64>(encodeResult.frame.size())) {
        const QString detail = bytesWritten <= 0
                                  ? tr("串口写入失败")
                                  : tr("串口写入不完整: %1/%2 字节")
                                        .arg(bytesWritten)
                                        .arg(encodeResult.frame.size());
        emitSendFailure(trimmedCommand,
                        encodeResult.normalizedMode,
                        detail,
                        QStringLiteral("write_failed"));
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
    m_measurementService.reset();
    emit serialMeasurementUpdated(QStringList());
    emit serialMeasurementTrendUpdated(QStringList());
    emit serialMeasurementFramesUpdated(QStringList());
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
        handleMeasurementEvent(event);
        return;
    }

    emit serialErrorCounted();
    logError(tr("未知接收事件: %1").arg(event.type),
             {{QStringLiteral("eventType"), event.type},
              {QStringLiteral("protocolName"), event.protocolName}});
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
                                              const QString& message,
                                              const QString& reason)
{
    QString logText = message;
    if (!command.isEmpty()) {
        logText = tr("%1: %2").arg(command, message);
    }

    emit serialErrorCounted();
    logError(logText,
             {{QStringLiteral("command"), command},
              {QStringLiteral("mode"), mode},
              {QStringLiteral("reason"), reason},
              {QStringLiteral("message"), message}});
    emit serialCommandFailed(command, mode, message);
    emit serialCommandFailedWithReason(command, mode, reason, message);
}

} // namespace serial_station
