/**
 * @file ConnectionMonitor.cpp
 * @brief 连接状态监控实现 — 状态跟踪、延迟监控、连接时间统计
 */

#include "connection/monitor/ConnectionMonitor.h"

/** @brief 构造连接监控器，初始化心跳定时器 @param parent 父QObject指针 */
ConnectionMonitor::ConnectionMonitor(QObject *parent) : QObject(parent), m_pingTimer(new QTimer(this)) {
    connect(m_pingTimer, &QTimer::timeout, this, &ConnectionMonitor::onPingTimer);
}

/** @brief 析构连接监控器，停止心跳监控 */
ConnectionMonitor::~ConnectionMonitor() { stopMonitoring(); }

/** @brief 设置连接状态，处理状态转换并发射相应信号 @param s 新的连接状态 */
void ConnectionMonitor::setState(State s) {
    if (s == m_stats.currentState) return;
    State old = m_stats.currentState;
    if (old == Connected && s == Disconnected) {
        m_stats.totalConnectedTime += QDateTime::currentMSecsSinceEpoch() - m_stats.connectedSince;
        emit connectionLost();
    }
    if (s == Connected) {
        m_stats.connectedSince = QDateTime::currentMSecsSinceEpoch();
        if (old == Reconnecting) emit connectionRestored();
    }
    if (s == Reconnecting) m_stats.reconnectCount++;
    m_stats.currentState = s;
    ++m_totalStateChanges;  ///< 统计: 状态变更次数递增
    emit stateChanged(s, old);
    emit statsUpdated(m_stats);
}

/** @brief 获取当前连接状态 @return 当前状态枚举值 */
ConnectionMonitor::State ConnectionMonitor::state() const { return m_stats.currentState; }

/** @brief 获取连接统计信息 @return 包含所有统计字段的Stats结构体 */
ConnectionMonitor::Stats ConnectionMonitor::stats() const { return m_stats; }

/** @brief 记录已发送字节数并更新统计 @param b 本次发送的字节数 */
void ConnectionMonitor::recordBytesSent(qint64 b) { m_stats.bytesSent += b; emit statsUpdated(m_stats); }

/** @brief 记录已接收字节数并更新统计 @param b 本次接收的字节数 */
void ConnectionMonitor::recordBytesReceived(qint64 b) { m_stats.bytesReceived += b; emit statsUpdated(m_stats); }

/** @brief 记录延迟测量值，超过阈值时发射latencyWarning信号 @param ms 延迟毫秒数 */
void ConnectionMonitor::recordLatency(double ms) {
    m_stats.latencyMs = ms;
    ++m_totalLatencyRecords;  ///< 统计: 延迟采样次数递增
    if (ms > m_latencyThreshold) { ++m_totalLatencyWarnings; emit latencyWarning(ms); }
    emit statsUpdated(m_stats);
}

/** @brief 记录一次错误，更新错误计数和最后错误信息 @param e 错误描述文本 */
void ConnectionMonitor::recordError(const QString &e) { m_stats.errorCount++; ++m_totalErrorsRecorded; m_stats.lastError = e; emit statsUpdated(m_stats); }

/** @brief 开始心跳监控，以指定间隔定时发射statsUpdated信号 @param interval 心跳间隔(毫秒)，默认5000ms */
void ConnectionMonitor::startMonitoring(int interval) {
    m_pingInterval = interval;
    m_pingTimer->start(interval);
}

/** @brief 停止心跳监控 */
void ConnectionMonitor::stopMonitoring() { m_pingTimer->stop(); }

/** @brief 重置所有统计数据为默认值 */
void ConnectionMonitor::resetStats() { m_stats = Stats{}; }

/** @brief 重置所有统计计数器为零 */
void ConnectionMonitor::resetMonitorStatistics() {
    m_totalStateChanges = 0;
    m_totalLatencyRecords = 0;
    m_totalLatencyWarnings = 0;
    m_totalPings = 0;
    m_totalErrorsRecorded = 0;
}

/** @brief 获取当前状态的本地化显示文本 @return 状态对应的翻译字符串 */
QString ConnectionMonitor::stateString() const {
    switch (m_stats.currentState) {
    case Disconnected: return tr("Disconnected");
    case Connecting: return tr("Connecting");
    case Connected: return tr("Connected");
    case Reconnecting: return tr("Reconnecting");
    case Error: return tr("Error");
    }
    return tr("Unknown");
}

/** @brief 心跳定时器超时回调，定时发射statsUpdated信号 */
void ConnectionMonitor::onPingTimer() { ++m_totalPings; emit statsUpdated(m_stats); }
