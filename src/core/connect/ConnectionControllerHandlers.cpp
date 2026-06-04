/**
 * @file ConnectionControllerHandlers.cpp
 * @brief 连接控制器信号处理与错误恢复方法 - 底层IConnection信号槽与断连清理
 *
 * 从 ConnectionController.cpp 拆分而来，包含:
 *   - onConnectionStateChanged(): 底层状态变化处理(清除下游/触发重连)
 *   - onDataReceived(): 数据接收转发与状态栏刷新
 *   - onConnectionTimeout(): 连接超时中断处理
 *   - onPortRemoved(): 端口物理拔出检测与自动断开
 *   - onPortAdded(): 端口接入通知转发
 *   - connectSignals(): IConnection信号到内部槽的连接
 *   - teardownConnection(): 统一断连清理流程
 *   - clearDownstreamConnections(): 清除下游控制器连接引用
 *   - stopConnectionTimeout(): 停止连接超时定时器
 */

#include "core/connect/ConnectionController.h"
#include "core/recording/RecordingController.h"

#include <QTimer>
#include <QDateTime>

#include "shared/AppConstants.h"

/** @brief 连接状态变化内部处理，在断开/错误状态下清除下游控制器连接引用，如果启用了自动重连且非用户主动断开则启动重连定时器 @param state 新的连接状态 */
void ConnectionController::onConnectionStateChanged(ConnectionState state)
{
    // 先缓存连接名称
    QString connName = m_currentConn ? m_currentConn->name() : "";

    switch (state) {
    case ConnectionState::Connected:
        // 连接成功，停止超时定时器和重连定时器
        stopConnectionTimeout();
        m_reconnectTimer.stop();
        // 如果是重连成功，发出通知并重置计数
        if (m_reconnectAttemptCount > 0) {
            ++m_totalReconnects;  ///< 统计: 重连成功次数递增
            emit reconnectSucceeded(connName);
            m_reconnectAttemptCount = 0;
        }
        // 统计: 更新峰值并发连接数(当前有连接即1，无连接为0)
        if (m_peakConcurrentConnections < 1) {
            m_peakConcurrentConnections = 1;
        }
        if (m_recordingController) {
            m_recordingController->setConnected(true);
        }
        m_pinoutPollTimer->start();
        // 启动连接健康检测定时器
        m_lastDataTimestamp = QDateTime::currentMSecsSinceEpoch();
        m_healthTimer.start();
        break;

    case ConnectionState::Disconnected:
    case ConnectionState::Error:
        stopConnectionTimeout();
        m_pinoutPollTimer->stop();
        m_healthTimer.stop();
        clearDownstreamConnections();

        // 清除已连接端口名（连接已断开）
        m_connectedPortName.clear();

        // 错误状态: 发送 Toast 错误通知（区分 Error 和普通 Disconnected）
        if (state == ConnectionState::Error) {
            emit connectionError(connName, tr("连接发生错误"));
            ++m_errorCount;
        }

        // 自动重连: 仅在非用户主动断开且已启用时触发
        if (m_autoReconnectEnabled && !m_userInitiatedDisconnect) {
            m_reconnectTimer.start();
        }
        break;

    case ConnectionState::Connecting:
        break;

    default:
        qWarning() << "ConnectionController: unknown state" << static_cast<int>(state);
        break;
    }

    // 转发状态变化信号
    emit connectionStateChanged(state, connName);
}

/** @brief 接收数据内部处理，转发数据到上层并请求状态栏刷新，同时更新最近收到数据的时间戳用于连接健康检测 @param data 接收到的原始字节数据 */
void ConnectionController::onDataReceived(const QByteArray& data)
{
    m_lastDataTimestamp = QDateTime::currentMSecsSinceEpoch();
    // 统计：累计接收数据字节数
    m_totalDataReceived += static_cast<quint64>(data.size());
    emit dataReceived(data);
    emit statusBarUpdateRequested();
}

/** @brief 连接超时处理，当open()后超过kConnectionTimeoutMs仍未变为Connected时触发，中断当前连接并通知用户 */
void ConnectionController::onConnectionTimeout()
{
    // 标记为非用户主动断开但禁止自动重连（超时重连毫无意义）
    m_userInitiatedDisconnect = true;
    m_reconnectTimer.stop();

    // 缓存端口名称，清理后 m_connectedPortName 会被清空
    const QString timeoutName = m_connectedPortName.isEmpty()
        ? (m_currentConn ? m_currentConn->name() : tr("未知"))
        : m_connectedPortName;

    qWarning() << "Connection timeout for" << timeoutName;

    teardownConnection(tr("连接超时"));

        emit connectionFailed(tr("连接超时"),
                             tr("连接在 %1 秒后超时。 "
                             "请检查设备连接后重试。")
                             .arg(kConnectionTimeoutMs / 1000));

    // 通知 Toast: 连接超时
        emit connectionError(timeoutName, tr("连接超时"));
}

/** @brief 端口物理拔出: 匹配当前连接则自动断开 @param portName 端口名 */
void ConnectionController::onPortRemoved(const QString& portName)
{
    // 仅在当前有活跃串口连接且端口名匹配时才断开
    if (m_currentConn && m_currentConn->type() == ConnectionType::Serial
        && m_connectedPortName == portName) {
        qWarning() << "Connected port" << portName << "was removed, disconnecting...";

        // 标记为非用户主动断开（物理拔出属于意外断开，可触发自动重连）
        m_userInitiatedDisconnect = false;

        teardownConnection(tr("端口已移除: %1").arg(portName));

        // 通知 UI 连接因端口拔出而断开
        emit connectionStateChanged(ConnectionState::Disconnected, portName);
        emit connectionFailed(tr("端口移除"),
                             tr("串口 %1 已断开。 "
                                "请重新连接设备。")
                                 .arg(portName));
        // 通知 Toast: 端口被物理拔出
        emit connectionError(portName, tr("端口已被物理移除"));
    }
}

/** @brief 端口接入: 转发信号到上层, 不自动连接 @param portName 端口名 */
void ConnectionController::onPortAdded(const QString& portName) { emit portAdded(portName); }

// 信号连接、断连清理与下游控制器管理
// (connectSignals / teardownConnection / clearDownstreamConnections / stopConnectionTimeout)
// 见 ConnectionControllerTeardown.cpp

