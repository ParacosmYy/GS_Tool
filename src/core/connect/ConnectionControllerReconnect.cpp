/**
 * @file ConnectionControllerReconnect.cpp
 * @brief 连接控制器自动重连功能实现 - 指数退避重连策略
 *
 * 从 ConnectionController.cpp 拆分而来，包含:
 *   - enableAutoReconnect(): 启用/禁用自动重连
 *   - isAutoReconnectEnabled(): 查询自动重连状态
 *   - onAutoReconnect(): 自动重连定时器触发，支持指数退避
 *   - calcBackoffInterval(): 计算退避间隔
 */

#include "core/connect/ConnectionController.h"

#include <QTimer>
#include <QDateTime>

/** @brief 启用/禁用自动重连 @param enabled 是否启用 @param intervalMs 重连基础间隔(毫秒) @param maxRetries 最大重连次数(0=无限制) */
void ConnectionController::enableAutoReconnect(bool enabled, int intervalMs, int maxRetries)
{
    m_autoReconnectEnabled = enabled;
    m_reconnectMaxRetries = maxRetries;
    m_reconnectBaseIntervalMs = intervalMs;
    if (enabled) m_reconnectTimer.setInterval(intervalMs);
    else m_reconnectTimer.stop();
}

/** @brief 返回自动重连是否启用 @return true=已启用 */
bool ConnectionController::isAutoReconnectEnabled() const { return m_autoReconnectEnabled; }

/** @brief 自动重连定时器触发(支持指数退避)，检查是否仍在断开状态且未由用户主动断开则尝试重新连接，支持最大重连次数限制达到上限后停止并发出失败通知，指数退避策略: actualInterval=baseInterval*2^min(attempt,4)上限30秒 */
void ConnectionController::onAutoReconnect()
{
    // 如果已经连接或用户主动断开，停止重连
    if (m_currentConn || m_userInitiatedDisconnect) {
        m_reconnectTimer.stop();
        m_reconnectAttemptCount = 0;
        return;
    }

    // 防御: 检查上一次连接参数是否有效，无参数则无法重连
    if (m_lastConnectParams.isEmpty()) {
        m_reconnectTimer.stop();
        emit reconnectFailed(tr("无有效的重连参数"));
        m_reconnectAttemptCount = 0;
        return;
    }

    // 检查是否达到最大重连次数（0 表示无限制）
    if (m_reconnectMaxRetries > 0 && m_reconnectAttemptCount >= m_reconnectMaxRetries) {
        m_reconnectTimer.stop();
        const QString reason = tr("已达到最大重连次数 (%1)").arg(m_reconnectMaxRetries);
        emit reconnectFailed(reason);
        m_reconnectAttemptCount = 0;
        return;
    }

    m_reconnectAttemptCount++;
    ++m_totalReconnectAttempts;  ///< 统计: 每次重连尝试递增(含成功和失败)

    // 指数退避: baseInterval * 2^min(attempt, 4)，上限30秒
    int actualInterval = calcBackoffInterval(m_reconnectAttemptCount);

    // 通知UI当前重连进度和下次等待时间
    emit reconnectProgress(m_reconnectAttemptCount, m_reconnectMaxRetries, actualInterval);
    emit reconnectAttempt(m_reconnectAttemptCount, m_reconnectMaxRetries);

    qDebug() << "[" << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz") << "]"
             << "Auto-reconnect attempt" << m_reconnectAttemptCount << "/"
             << (m_reconnectMaxRetries > 0 ? QString::number(m_reconnectMaxRetries) : "unlimited")
             << "next interval:" << actualInterval << "ms";

    if (m_lastConnectType == ConnectionType::Serial) {
        connectSerial(m_lastConnectParams);
    } else {
        // 使用保存的网络参数重连，而非硬编码默认值
        connectNetwork(m_lastConnectType, m_lastConnectParams);
    }

    // 设置下次重连的间隔（指数退避）
    if (m_reconnectTimer.isActive()) {
        m_reconnectTimer.setInterval(actualInterval);
    }
}

/** @brief 计算指数退避重连间隔，策略: base*2^min(attempt,4)，上限30秒
 *  使用qint64中间变量防止int溢出(base * 2^4可能超出int范围) */
int ConnectionController::calcBackoffInterval(int attempt) const
{
    const int maxShift = 4;
    const qint64 maxMs = 30000;
    // 使用qint64防止乘法溢出: base=30000, shift=4 → 480000, 安全
    qint64 interval = static_cast<qint64>(m_reconnectBaseIntervalMs) * (1 << qMin(attempt, maxShift));
    return static_cast<int>(qMin(interval, maxMs));
}
