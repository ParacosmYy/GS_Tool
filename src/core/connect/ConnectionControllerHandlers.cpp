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

#include <QTimer>
#include <QDateTime>

#include "shared/AppConstants.h"
#include "core/send/SendController.h"
#include "ota/manager/OtaManager.h"
#include "core/recording/RecordingController.h"
#include "serial/port/PortWatcher.h"
#include "utils/log/DataLogger.h"
#include "terminal/model/TerminalModel.h"

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
            emit reconnectSucceeded(connName);
            m_reconnectAttemptCount = 0;
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

/** @brief 连接IConnection的信号到内部槽 @param conn 需要连接信号的IConnection实例 */
void ConnectionController::connectSignals(IConnection* conn)
{
    connect(conn, &IConnection::dataReceived,
            this, &ConnectionController::onDataReceived);
    connect(conn, &IConnection::stateChanged,
            this, &ConnectionController::onConnectionStateChanged);
    // 统计：累计发送数据字节数（通过 bytesWritten 信号追踪实际写入的字节数）
    connect(conn, &IConnection::bytesWritten, this, [this](qint64 bytes) {
        m_totalDataSent += static_cast<quint64>(bytes);
    });
    // 连接错误信号，转发详细错误信息到UI，同时发送 Toast 错误通知
    connect(conn, &IConnection::errorOccurred,
            this, [this](const QString& msg) {
        qWarning() << "Connection error:" << msg;
        const QString errPortName = m_connectedPortName.isEmpty()
            ? (m_currentConn ? m_currentConn->name() : tr("未知"))
            : m_connectedPortName;
        emit connectionFailed(tr("连接错误"), msg);
        emit connectionError(errPortName, msg);
    });
    // 连接错误计数更新 → 通过信号通知表现层(避免业务层直接依赖表现层)
    connect(conn, &IConnection::errorOccurred, this, [this, conn]() {
        auto counters = conn->errorCounters();
        emit errorCountersUpdated(
            counters.framingErrors, counters.parityErrors, counters.overrunErrors);
    });
}

/** @brief 统一的连接断开清理流程，停止超时定时器、缓存并清空当前连接、断开信号连接、从ConnectionManager移除并销毁连接实例、清除下游控制器连接引用 @param reason 断开原因描述，用于日志输出 */
void ConnectionController::teardownConnection(const QString& reason)
{
    stopConnectionTimeout();
    if (m_pinoutPollTimer) m_pinoutPollTimer->stop();

    if (!m_currentConn) return;

    qInfo() << "Tearing down connection:" << reason;

    // 缓存指针并立即清空成员，防止信号回调中访问
    IConnection* conn = m_currentConn;
    m_currentConn = nullptr;
    m_connectedPortName.clear();

    // 先断开信号，防止 removeConnection 内部 close() 触发的
    // stateChanged 信号进入 onConnectionStateChanged
    disconnect(conn, nullptr, this, nullptr);

    // 从管理器移除并销毁（removeConnection 内部执行 close + delete）
    m_connManager->removeConnection(conn);

    // 清除下游控制器的连接引用
    clearDownstreamConnections();
}

/** @brief 清除所有下游控制器的连接引用，在连接断开或发生错误时调用防止下游控制器持有悬空指针 */
void ConnectionController::clearDownstreamConnections()
{
    if (m_sendController) {
        m_sendController->setConnection(nullptr);
    }
    if (m_otaManager) {
        m_otaManager->setConnection(nullptr);
    }
    if (m_recordingController) {
        m_recordingController->setConnected(false);
    }
}

/** @brief 停止连接超时定时器 */
void ConnectionController::stopConnectionTimeout()
{
    if (m_connectionTimer.isActive()) {
        m_connectionTimer.stop();
    }
}

