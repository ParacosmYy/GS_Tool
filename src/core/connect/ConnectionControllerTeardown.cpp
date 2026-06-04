/**
 * @file ConnectionControllerTeardown.cpp
 * @brief 连接控制器 - 信号连接、断连清理与下游控制器管理
 *
 * 从 ConnectionControllerHandlers.cpp 拆分而来，包含:
 *   - connectSignals():             IConnection信号到内部槽的连接
 *   - teardownConnection():         统一断连清理流程
 *   - clearDownstreamConnections(): 清除下游控制器连接引用
 *   - stopConnectionTimeout():      停止连接超时定时器
 *
 * 状态处理与错误恢复见ConnectionControllerHandlers.cpp。
 */

#include "core/connect/ConnectionController.h"

#include <QTimer>

#include "shared/AppConstants.h"
#include "core/send/SendController.h"
#include "ota/manager/OtaManager.h"
#include "core/recording/RecordingController.h"
#include "serial/port/PortWatcher.h"
#include "utils/log/DataLogger.h"
#include "terminal/model/TerminalModel.h"

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
