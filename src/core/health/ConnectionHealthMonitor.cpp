/**
 * @file ConnectionHealthMonitor.cpp
 * @brief 连接健康监控器实现 — 实时诊断连接质量
 *
 * 使用1秒定时器周期性采样吞吐量，综合吞吐量/错误率/稳定性三维评分。
 */

#include "core/health/ConnectionHealthMonitor.h"

#include <QDateTime>

/** @brief 构造函数，初始化监控器 @param parent 父对象 */
ConnectionHealthMonitor::ConnectionHealthMonitor(QObject* parent)
    : QObject(parent)
    , m_connected(false)
    , m_lastBytesReceived(0)
    , m_lastBytesSent(0)
    , m_consecutiveErrors(0)
    , m_currentQuality(100.0)
    , m_currentThroughput(0.0)
    , m_totalDowntimeMs(0)
{
    m_snapshotTimer.setInterval(1000);
    m_snapshotTimer.setSingleShot(false);
    connect(&m_snapshotTimer, &QTimer::timeout,
            this, &ConnectionHealthMonitor::onSnapshotTimer);
}

/** @brief 通知接收到数据，累计接收字节并递增连续无错误标记 @param bytes 接收字节数 */
void ConnectionHealthMonitor::onDataReceived(qint64 bytes)
{
    m_stats.totalBytesReceived += static_cast<quint64>(bytes);
    /* 收到数据说明连接活跃，重置连续错误计数 */
    m_consecutiveErrors = 0;
}

/** @brief 通知发送了数据，累计发送字节 @param bytes 发送字节数 */
void ConnectionHealthMonitor::onDataSent(qint64 bytes)
{
    m_stats.totalBytesSent += static_cast<quint64>(bytes);
}

/** @brief 通知发生错误，递增错误计数和连续错误计数 @param description 错误描述 */
void ConnectionHealthMonitor::onError(const QString& description)
{
    Q_UNUSED(description);
    ++m_stats.totalErrors;
    ++m_consecutiveErrors;

    /* 连续错误超过阈值时发出告警 */
    if (m_consecutiveErrors >= 5) {
        emit healthAlert(QualityLevel::Critical,
                         tr("连续 %1 次错误，连接可能不稳定").arg(m_consecutiveErrors));
    } else if (m_consecutiveErrors >= 3) {
        emit healthAlert(QualityLevel::Poor,
                         tr("连续 %1 次错误").arg(m_consecutiveErrors));
    }
}

/** @brief 通知连接已建立，启动快照定时器 */
void ConnectionHealthMonitor::onConnected()
{
    m_connected = true;
    m_consecutiveErrors = 0;
    m_connectionTimer.start();
    m_lastBytesReceived = 0;
    m_lastBytesSent = 0;
    m_throughputHistory.clear();
    m_snapshotTimer.start();
    emit connectionStateChanged(true);
}

/** @brief 通知连接已断开，停止快照定时器 */
void ConnectionHealthMonitor::onDisconnected()
{
    if (m_connected) {
        m_connected = false;
        ++m_stats.totalReconnects;

        /* 记录最长运行时间 */
        qint64 uptime = m_connectionTimer.elapsed();
        if (uptime > m_stats.longestUptime) {
            m_stats.longestUptime = uptime;
        }

        /* 开始计算断线时间 */
        m_disconnectionTimer.start();
    }
    m_snapshotTimer.stop();
    m_currentThroughput = 0.0;
    emit connectionStateChanged(false);
}

/** @brief 获取当前健康快照 @return 包含所有关键指标的快照 */
ConnectionHealthMonitor::HealthSnapshot ConnectionHealthMonitor::currentSnapshot() const
{
    HealthSnapshot snap;
    snap.qualityScore = m_currentQuality;
    snap.throughputBps = m_currentThroughput;
    snap.errorRate = m_stats.totalErrors > 0
        ? m_stats.totalErrors * 60000.0 / qMax(1LL, m_connectionTimer.elapsed())
        : 0.0;
    snap.uptimeMs = m_connected ? m_connectionTimer.elapsed() : 0;
    snap.downtimeMs = m_totalDowntimeMs;
    snap.reconnectCount = static_cast<int>(m_stats.totalReconnects);
    snap.consecutiveErrors = m_consecutiveErrors;
    return snap;
}

/** @brief 获取质量等级 @return 基于当前评分的等级枚举 */
ConnectionHealthMonitor::QualityLevel ConnectionHealthMonitor::qualityLevel() const
{
    if (m_currentQuality >= 80.0) return QualityLevel::Excellent;
    if (m_currentQuality >= 60.0) return QualityLevel::Good;
    if (m_currentQuality >= 40.0) return QualityLevel::Fair;
    if (m_currentQuality >= 20.0) return QualityLevel::Poor;
    return QualityLevel::Critical;
}

/** @brief 获取质量评分 @return 0-100分数 */
double ConnectionHealthMonitor::qualityScore() const
{
    return m_currentQuality;
}

/** @brief 1秒快照定时器回调 — 采样吞吐量并重新计算质量评分 */
void ConnectionHealthMonitor::onSnapshotTimer()
{
    ++m_stats.totalSnapshots;

    /* 采样吞吐量 */
    recordThroughputSample();

    /* 如果之前断线现在重新连接了，累计断线时间 */
    if (!m_connected && m_disconnectionTimer.isValid()) {
        m_totalDowntimeMs += m_disconnectionTimer.elapsed();
    }

    /* 重新计算质量评分 */
    calculateQuality();

    /* 检查质量阈值 */
    QualityLevel level = qualityLevel();
    emit qualityChanged(m_currentQuality, level);
    emit throughputUpdated(m_currentThroughput);

    if (m_currentQuality < 30.0 && m_connected) {
        emit healthAlert(level, tr("连接质量较差(%.1f分)，建议检查连接").arg(m_currentQuality));
    }
}

/** @brief 记录吞吐量采样值到滑动窗口(保留最近30秒) */
void ConnectionHealthMonitor::recordThroughputSample()
{
    qint64 currentTotal = m_stats.totalBytesReceived + m_stats.totalBytesSent;
    qint64 delta = currentTotal - m_lastBytesReceived - m_lastBytesSent;

    double sampleBps = qMax(0.0, static_cast<double>(delta));
    m_throughputHistory.enqueue(sampleBps);

    /* 滑动窗口保留最近30个采样 */
    while (m_throughputHistory.size() > 30) {
        m_throughputHistory.dequeue();
    }

    /* 计算平均吞吐量 */
    double sum = 0.0;
    for (double v : m_throughputHistory) {
        sum += v;
    }
    m_currentThroughput = m_throughputHistory.isEmpty() ? 0.0 : sum / m_throughputHistory.size();

    /* 更新峰值吞吐量 */
    if (m_currentThroughput > m_stats.peakThroughput) {
        m_stats.peakThroughput = m_currentThroughput;
    }

    m_lastBytesReceived = static_cast<qint64>(m_stats.totalBytesReceived);
    m_lastBytesSent = static_cast<qint64>(m_stats.totalBytesSent);
}
