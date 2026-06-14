#include "apps/serial_station/SerialStationController.h"

#include <QtCore/QMetaType>
#include <QtCore/QTimer>
#include <QtCore/QStringList>

#include "apps/serial_station/SerialStationConstants.h"

namespace serial_station {

SerialStationController::SerialStationController(QObject* parent)
    : QObject(parent)
    , m_serialManager(this)
    , m_reconnectTimer(new QTimer(this))
{
    m_protocols.registerBuiltInProtocols();
    resetReceiveDispatcher();
    m_reconnectTimer->setSingleShot(true);
    connect(m_reconnectTimer,
            &QTimer::timeout,
            this,
            &SerialStationController::attemptAutoReconnect);
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
    SerialStationConfig stationConfig;
    stationConfig.port = config;
    connectSerialPort(stationConfig);
}

void SerialStationController::connectSerialPort(const SerialStationConfig& config)
{
    applyReconnectConfig(config);
    clearReconnectState();
    m_sendQueueFrozen = false;
    resetReceiveDispatcher();
    const QString validationError = m_lastConnectConfig.port.validationError();
    if (!validationError.isEmpty()) {
        m_serialManager.rejectConfiguration(m_lastConnectConfig.port, validationError);
        emit serialErrorCounted();
        logSystem(validationError, {{QStringLiteral("reason"), validationError}});
        return;
    }

    logSystem(tr("正在打开串口: %1").arg(m_lastConnectConfig.port.summary()),
              {{QStringLiteral("portName"), m_lastConnectConfig.port.portName.trimmed()},
               {QStringLiteral("baudRate"), m_lastConnectConfig.port.baudRate}});
    m_serialManager.configure(m_lastConnectConfig.port);
    if (!m_serialManager.open()) {
        emit serialErrorCounted();
        emit serialErrorOccurred(m_serialManager.session().errorString());
        logSystem(m_serialManager.session().errorString(),
                  {{QStringLiteral("error"), m_serialManager.session().errorString()}});
        if (m_autoReconnectEnabled) {
            m_sendQueueFrozen = true;
            scheduleAutoReconnect();
            return;
        }

        m_sendQueueFrozen = true;
        return;
    }

    m_sendQueueFrozen = false;
    processNextQueuedSend();
}

void SerialStationController::applyReconnectConfig(const SerialStationConfig& config)
{
    m_lastConnectConfig = config;
    m_lastConnectConfig.port = config.port.normalized();
    m_autoReconnectEnabled = config.isReconnectEnabled();
    m_manualDisconnectRequested = false;

    if (m_lastConnectConfig.reconnectIntervalMs < 500) {
        m_lastConnectConfig.reconnectIntervalMs = 500;
    }
}

void SerialStationController::clearReconnectState()
{
    if (!m_reconnectTimer) {
        return;
    }

    m_reconnectTimer->stop();
    m_reconnectInFlight = false;
}

void SerialStationController::cancelAutoReconnect()
{
    clearReconnectState();
}

void SerialStationController::scheduleAutoReconnect()
{
    if (!m_autoReconnectEnabled || m_manualDisconnectRequested) {
        return;
    }

    if (!m_reconnectTimer || m_reconnectTimer->isActive()) {
        return;
    }

    m_sendQueueFrozen = true;
    m_reconnectInFlight = true;
    m_reconnectTimer->start(m_lastConnectConfig.reconnectIntervalMs);
    logSystem(
        tr("检测到连接中断，%1 ms 后将重试连接（重连策略已启用）").arg(m_lastConnectConfig.reconnectIntervalMs),
        {{QStringLiteral("reason"), QStringLiteral("auto_reconnect")}});
}

void SerialStationController::attemptAutoReconnect()
{
    if (!m_autoReconnectEnabled || m_manualDisconnectRequested) {
        m_sendQueueFrozen = true;
        m_reconnectInFlight = false;
        return;
    }

    if (m_lastConnectConfig.port.validationError().isEmpty()
        && m_serialManager.session().state() != SerialSessionState::Open) {
        m_reconnectInFlight = true;
        m_serialManager.configure(m_lastConnectConfig.port);
        if (m_serialManager.open()) {
            m_sendQueueFrozen = false;
            m_reconnectInFlight = false;
            logSystem(tr("自动重连成功"),
                      {{QStringLiteral("state"), QStringLiteral("open")}}); 
            processNextQueuedSend();
            return;
        }

        emit serialErrorCounted();
        emit serialErrorOccurred(m_serialManager.session().errorString());
        logSystem(tr("自动重连失败: %1").arg(m_serialManager.session().errorString()),
                  {{QStringLiteral("state"), QStringLiteral("error")}}
        );
        m_reconnectInFlight = false;
    }

    scheduleAutoReconnect();
}

void SerialStationController::disconnectSerialPort()
{
    m_manualDisconnectRequested = true;
    cancelAutoReconnect();

    m_sendQueueFrozen = false;
    if (m_sendInFlight && !m_sendContext.command.isEmpty()) {
        emitSendFailure(m_sendContext.command,
                        m_sendContext.mode,
                        tr("串口连接已断开，发送请求中断"),
                        QStringLiteral("not_connected"));
    }
    failQueuedSendsWithReason(QStringLiteral("not_connected"),
                              tr("串口未连接，无法发送"));

    m_serialManager.close();
    m_dispatcher.reset();
    m_sendQueueFrozen = true;
    m_sendInFlight = false;
    m_sendContext = SendRetryContext();
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
    enqueueSend(trimmedCommand,
                encodeResult.normalizedMode,
                encodeResult.frame,
                retryCount,
                retryDelayMs);
}

void SerialStationController::enqueueSend(const QString& command,
                                         const QString& mode,
                                         const QByteArray& frame,
                                         int retryCount,
                                         int retryDelayMs)
{
    const int maxRetryCount = qMax(0, retryCount);
    const int intervalMs = qMax(0, retryDelayMs);

    SendRetryContext context;
    context.command = command;
    context.mode = mode;
    context.frame = frame;
    context.maxRetryCount = maxRetryCount;
    context.retryDelayMs = intervalMs;

    m_sendQueue.enqueue(context);
    logSystem(tr("命令发送已入队（队列长度: %1）").arg(m_sendQueue.size()),
              {{QStringLiteral("command"), command},
               {QStringLiteral("mode"), mode},
               {QStringLiteral("retryCountConfigured"), maxRetryCount},
               {QStringLiteral("retryDelayMs"), intervalMs},
               {QStringLiteral("queueSize"), m_sendQueue.size()}});

    processNextQueuedSend();
}

void SerialStationController::processNextQueuedSend()
{
    if (m_sendInFlight || m_sendQueueFrozen) {
        return;
    }

    if (!m_serialManager.session().isOpen()) {
        failQueuedSendsWithReason(QStringLiteral("not_connected"),
                                  tr("串口未连接，无法发送"));
        m_sendQueueFrozen = true;
        return;
    }

    if (m_sendQueue.isEmpty()) {
        return;
    }

    m_sendContext = m_sendQueue.dequeue();
    m_sendInFlight = true;
    processQueuedSendAttempt();
}

void SerialStationController::failQueuedSendsWithReason(const QString& reason, const QString& message)
{
    while (!m_sendQueue.isEmpty()) {
        const SendRetryContext context = m_sendQueue.dequeue();
        emitSendFailure(context.command, context.mode, message, reason);
    }
}

void SerialStationController::processQueuedSendAttempt()
{
    if (!m_sendInFlight || m_sendQueueFrozen) {
        return;
    }

    if (!m_serialManager.session().isOpen()) {
        finishSendWithFailure(m_sendContext.command,
                              m_sendContext.mode,
                              tr("串口未连接，无法发送"),
                              QStringLiteral("not_connected"));
        m_sendQueue.clear();
        processNextQueuedSend();
        return;
    }

    if (m_sendContext.attempt > 0) {
        logSystem(
            tr("命令发送重试 %1/%2（间隔 %3ms）")
                .arg(m_sendContext.attempt)
                .arg(m_sendContext.maxRetryCount)
                .arg(m_sendContext.retryDelayMs),
            {{QStringLiteral("command"), m_sendContext.command},
             {QStringLiteral("mode"), m_sendContext.mode},
             {QStringLiteral("retryAttempt"), m_sendContext.attempt},
             {QStringLiteral("retryDelayMs"), m_sendContext.retryDelayMs},
             {QStringLiteral("frameSize"), m_sendContext.frame.size()}});
    }

    const QByteArray pendingFrame =
        m_sendContext.frame.mid(static_cast<int>(m_sendContext.bytesWritten));
    m_sendContext.bytesWritten += m_serialManager.send(pendingFrame);

    if (m_sendContext.bytesWritten >= m_sendContext.frame.size()) {
        const QString logText =
            tr("%1 [%2] %3 bytes")
                .arg(m_sendContext.command, m_sendContext.mode, QString::number(m_sendContext.bytesWritten));
        emit serialTxCounted();
        logTx(logText,
              m_sendContext.frame,
              {{QStringLiteral("command"), m_sendContext.command},
               {QStringLiteral("mode"), m_sendContext.mode},
               {QStringLiteral("bytesWritten"), m_sendContext.bytesWritten},
               {QStringLiteral("retryCountConfigured"), m_sendContext.maxRetryCount},
               {QStringLiteral("retryDelayMs"), m_sendContext.retryDelayMs},
               {QStringLiteral("retriesUsed"), m_sendContext.retriesUsed}});
        emit serialCommandSent(m_sendContext.command, m_sendContext.mode, m_sendContext.bytesWritten);
        m_sendInFlight = false;
        processNextQueuedSend();
        return;
    }

    if (m_sendContext.retriesUsed >= m_sendContext.maxRetryCount) {
        const QString detail =
            m_sendContext.bytesWritten <= 0
                ? tr("串口写入失败（已重试 %1 次）").arg(m_sendContext.retriesUsed)
                : tr("串口写入不完整: %1/%2 字节（已重试 %3 次）")
                      .arg(m_sendContext.bytesWritten)
                      .arg(m_sendContext.frame.size())
                      .arg(m_sendContext.retriesUsed);
        finishSendWithFailure(m_sendContext.command, m_sendContext.mode, detail, QStringLiteral("write_failed"));
        m_sendInFlight = false;
        processNextQueuedSend();
        return;
    }

    ++m_sendContext.attempt;
    ++m_sendContext.retriesUsed;

    if (m_sendContext.retryDelayMs > 0) {
        QTimer::singleShot(m_sendContext.retryDelayMs,
                           this,
                           &SerialStationController::processQueuedSendAttempt);
        return;
    }

    QTimer::singleShot(0, this, &SerialStationController::processQueuedSendAttempt);
}

void SerialStationController::finishSendWithFailure(const QString& command,
                                                   const QString& mode,
                                                   const QString& detail,
                                                   const QString& reason)
{
    emitSendFailure(command, mode, detail, reason);
    m_sendContext = SendRetryContext();
    m_sendInFlight = false;
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

    if (!m_manualDisconnectRequested && m_autoReconnectEnabled) {
        if (m_sendInFlight && !m_sendContext.command.isEmpty()) {
            m_sendContext.bytesWritten = 0;
            m_sendContext.attempt = 0;
            m_sendContext.retriesUsed = 0;
            m_sendQueue.prepend(m_sendContext);
            m_sendContext = SendRetryContext();
            m_sendInFlight = false;
        }

        m_sendQueueFrozen = true;
        scheduleAutoReconnect();
        return;
    }

    if (m_sendInFlight && !m_sendContext.command.isEmpty()) {
        emitSendFailure(m_sendContext.command,
                        m_sendContext.mode,
                        tr("串口连接已断开，发送请求中断"),
                        QStringLiteral("not_connected"));
        m_sendContext = SendRetryContext();
        m_sendInFlight = false;
    }
    failQueuedSendsWithReason(QStringLiteral("not_connected"),
                              tr("串口连接已断开"));
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
