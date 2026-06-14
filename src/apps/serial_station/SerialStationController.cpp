#include "apps/serial_station/SerialStationController.h"

#include <QtCore/QThread>

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

void SerialStationController::sendCommand(const QString& command,
                                         const QString& mode,
                                         int retryCount,
                                         int retryDelayMs)
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

    const int maxRetryCount = qMax(0, retryCount);
    const int intervalMs = qMax(0, retryDelayMs);
    qint64 bytesWritten = 0;
    int retriesUsed = 0;

    for (int attempt = 0; attempt <= maxRetryCount; ++attempt) {
        if (attempt > 0 && intervalMs > 0) {
            QThread::msleep(intervalMs);
        }

        if (attempt > 0) {
            logSystem(tr("命令发送重试 %1/%2（间隔 %3ms）").arg(attempt).arg(maxRetryCount).arg(intervalMs),
                      {{QStringLiteral("command"), trimmedCommand},
                       {QStringLiteral("mode"), encodeResult.normalizedMode},
                       {QStringLiteral("retryAttempt"), attempt},
                       {QStringLiteral("retryDelayMs"), intervalMs},
                       {QStringLiteral("frameSize"), encodeResult.frame.size()}});
        }

        const QByteArray pendingFrame =
            encodeResult.frame.mid(static_cast<int>(bytesWritten));
        bytesWritten += m_serialManager.send(pendingFrame);
        if (bytesWritten >= encodeResult.frame.size()) {
            break;
        }

        if (attempt >= maxRetryCount) {
            break;
        }

        ++retriesUsed;
    }

    if (bytesWritten < static_cast<qint64>(encodeResult.frame.size())) {
        const QString detail = bytesWritten <= 0
                                  ? tr("串口写入失败（已重试 %1 次）").arg(retriesUsed)
                                  : tr("串口写入不完整: %1/%2 字节（已重试 %3 次）")
                                        .arg(bytesWritten)
                                        .arg(encodeResult.frame.size())
                                        .arg(retriesUsed);
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
           {QStringLiteral("bytesWritten"), bytesWritten},
           {QStringLiteral("retryCountConfigured"), maxRetryCount},
           {QStringLiteral("retryDelayMs"), intervalMs},
           {QStringLiteral("retriesUsed"), retriesUsed}});
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
