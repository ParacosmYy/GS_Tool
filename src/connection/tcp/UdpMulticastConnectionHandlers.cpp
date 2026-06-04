/**
 * @file UdpMulticastConnectionHandlers.cpp
 * @brief UDP组播连接 — Socket I/O 事件处理实现
 *
 * 从 UdpMulticastConnection.cpp 拆分而来，包含数据到达回调、
 * 错误回调和连接状态更新方法。
 */

#include "connection/tcp/UdpMulticastConnection.h"

/** @brief 数据到达回调，读取所有待处理数据报并发射dataReceived信号 */
void UdpMulticastConnection::onReadyRead()
{
    if (!m_socket) return;

    while (m_socket->hasPendingDatagrams()) {
        QByteArray buffer;
        buffer.resize(static_cast<int>(m_socket->pendingDatagramSize()));
        QHostAddress sender;
        quint16 senderPort = 0;

        qint64 size = m_socket->readDatagram(buffer.data(), buffer.size(),
                                              &sender, &senderPort);
        if (size > 0) {
            ++m_dgramsRecv;
            m_rxBytes += size;
            buffer.resize(static_cast<int>(size));
            emit dataReceived(buffer);
        }
    }
}

/** @brief 网络错误回调，发射errorOccurred信号 @param error socket错误类型 */
void UdpMulticastConnection::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)
    if (m_socket) {
        emit errorOccurred(m_socket->errorString());
    }
}

/** @brief 更新连接状态，状态变化时发射stateChanged信号 @param newState 新状态 */
void UdpMulticastConnection::updateState(ConnectionState newState)
{
    if (m_state != newState) {
        m_state = newState;
        emit stateChanged(newState);
    }
}
