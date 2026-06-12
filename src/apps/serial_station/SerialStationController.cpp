#include "apps/serial_station/SerialStationController.h"

#include <QtCore/QVariantMap>

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
            this, &SerialStationController::serialErrorOccurred);
    connect(&m_serialManager, &SerialManager::bytesReceived,
            this, &SerialStationController::handleBytesReceived);
}

SerialProtocolRegistry& SerialStationController::protocols()
{
    return m_protocols;
}

SerialManager& SerialStationController::serialManager()
{
    return m_serialManager;
}

void SerialStationController::connectSerialPort(const SerialPortConfig& config)
{
    resetReceiveDispatcher();
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

    if (trimmedCommand.isEmpty()) {
        emitSendFailure(trimmedCommand, normalizedMode, tr("发送内容为空"));
        return;
    }

    if (normalizedMode != QStringLiteral("ascii") &&
        normalizedMode != QStringLiteral("protocol")) {
        emitSendFailure(trimmedCommand, normalizedMode,
                        tr("发送模式暂不支持: %1").arg(mode.trimmed()));
        return;
    }

    if (!m_serialManager.session().isOpen()) {
        emitSendFailure(trimmedCommand, normalizedMode, tr("串口未连接，无法发送"));
        return;
    }

    const QByteArray frame = buildCommandFrame(trimmedCommand, normalizedMode);
    if (frame.isEmpty()) {
        emitSendFailure(trimmedCommand, normalizedMode, tr("协议构建出的发送帧为空"));
        return;
    }

    emit serialCommandPrepared(trimmedCommand, normalizedMode, frame);

    const qint64 bytesWritten = m_serialManager.send(frame);
    if (bytesWritten <= 0) {
        emitSendFailure(trimmedCommand, normalizedMode, tr("串口写入失败"));
        return;
    }

    emit serialTxCounted();
    emit serialTxLogged(tr("%1 [%2] %3 bytes")
                            .arg(trimmedCommand, normalizedMode, QString::number(bytesWritten)));
    emit serialCommandSent(trimmedCommand, normalizedMode, bytesWritten);
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
        emit serialSystemLogged(tr("接收协议不可用，已丢弃 %1 bytes").arg(summary.inputBytes));
        return;
    }

    if (summary.status == SerialDispatcher::FeedStatus::Buffered) {
        emit serialSystemLogged(bufferedReceiveText(summary));
        return;
    }

    for (const SerialProtocolEvent& event : events) {
        processProtocolEvent(event);
    }
}

QString SerialStationController::normalizeSendMode(const QString& mode) const
{
    return mode.trimmed().toLower();
}

QByteArray SerialStationController::buildCommandFrame(const QString& command,
                                                      const QString& mode) const
{
    Q_UNUSED(mode)

    const std::unique_ptr<ISerialProtocol> protocol = m_protocols.createDefault();
    if (!protocol) {
        return {};
    }

    QVariantMap params;
    params.insert(QStringLiteral("text"), command);
    params.insert(QStringLiteral("appendNewline"), false);
    return protocol->buildCommand(command, params);
}

void SerialStationController::resetReceiveDispatcher()
{
    m_dispatcher.setProtocol(m_protocols.createDefault());
}

void SerialStationController::processProtocolEvent(const SerialProtocolEvent& event)
{
    if (event.type == serialStationConstants::kAsciiFrameType) {
        emit serialRxCounted();
        emit serialRxLogged(eventPayloadText(event));
        return;
    }

    if (event.type == serialStationConstants::kLogType) {
        const QString text = eventPayloadText(event);
        emit serialSystemLogged(text);
        return;
    }

    emit serialErrorCounted();
    emit serialSystemLogged(tr("未知接收事件: %1").arg(event.type));
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

void SerialStationController::emitSendFailure(const QString& command,
                                              const QString& mode,
                                              const QString& message)
{
    QString logText = message;
    if (!command.isEmpty()) {
        logText = tr("%1: %2").arg(command, message);
    }

    emit serialErrorCounted();
    emit serialSystemLogged(logText);
    emit serialCommandFailed(command, mode, message);
}

} // namespace serial_station
