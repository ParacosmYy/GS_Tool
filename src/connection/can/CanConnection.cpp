/**
 * @file CanConnection.cpp
 * @brief CAN/CAN-FD连接实现
 */

#include "connection/can/CanConnection.h"

CanConnection::CanConnection(QObject* parent)
    : IConnection(parent)
{
}

CanConnection::~CanConnection()
{
    close();
}

ConnectionType CanConnection::type() const
{
    // TODO: 在ConnectionType枚举中添加Can类型后修改
    return ConnectionType::Serial;
}

QString CanConnection::name() const
{
    return m_adapterName.isEmpty() ? tr("未配置") : m_adapterName;
}

ConnectionState CanConnection::state() const
{
    return m_state;
}

bool CanConnection::open()
{
    m_state = ConnectionState::Connecting;
    emit stateChanged(m_state);
    // TODO: 集成SocketCAN/PCAN/Vector等CAN适配器驱动
    m_state = ConnectionState::Connected;
    emit stateChanged(m_state);
    return true;
}

void CanConnection::close()
{
    if (m_state != ConnectionState::Disconnected) {
        m_state = ConnectionState::Disconnected;
        emit stateChanged(m_state);
    }
}

qint64 CanConnection::write(const QByteArray& data)
{
    Q_UNUSED(data)
    return -1; // TODO: 通过CAN帧发送
}

void CanConnection::configure(const QVariantMap& params)
{
    if (params.contains("bitrate")) {
        m_bitrate = params.value("bitrate").toInt();
    }
    if (params.contains("canFd")) {
        m_canFdEnabled = params.value("canFd").toBool();
    }
    if (params.contains("adapter")) {
        m_adapterName = params.value("adapter").toString();
    }
}

void CanConnection::setBitrate(int bitrate)
{
    m_bitrate = bitrate;
}

void CanConnection::setCanFdEnabled(bool enabled)
{
    m_canFdEnabled = enabled;
}

bool CanConnection::sendFrame(int id, const QByteArray& data, bool extended)
{
    Q_UNUSED(id)
    Q_UNUSED(data)
    Q_UNUSED(extended)
    return false; // TODO: 实现CAN帧发送
}
