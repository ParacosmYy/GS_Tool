#include "apps/serial_station/SerialStationController.h"

#include <QtCore/QVariantMap>

namespace serial_station {

SerialStationController::SerialStationController(QObject* parent)
    : QObject(parent)
    , m_serialManager(this)
{
    m_protocols.registerBuiltInProtocols();
    connect(&m_serialManager, &SerialManager::stateChanged,
            this, &SerialStationController::serialStateChanged);
    connect(&m_serialManager, &SerialManager::errorOccurred,
            this, &SerialStationController::serialErrorOccurred);
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
    m_serialManager.configure(config);
    if (!m_serialManager.open()) {
        emit serialErrorOccurred(m_serialManager.session().errorString());
    }
}

void SerialStationController::disconnectSerialPort()
{
    m_serialManager.close();
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
