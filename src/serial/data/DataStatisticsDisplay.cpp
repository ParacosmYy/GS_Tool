/**
 * @file DataStatisticsDisplay.cpp
 * @brief 数据统计面板 — 显示格式化与查询方法实现
 *
 * 从DataStatistics.cpp拆分，负责:
 *   1. 速率格式化(formatRate): 将bytes/s转换为人类可读字符串
 *   2. 错误计数更新与显示(updateErrors)
 *   3. 连接健康状态显示(updateConnectionHealth)
 *   4. 会话统计摘要生成(sessionSummary)
 *   5. 统计值查询getter(totalRxBytes/totalTxBytes/peakRate等)
 */

#include "serial/data/DataStatistics.h"
#include "utils/data/ByteFormat.h"

#include <QChar>

// ---- 速率格式化 ----

/** @brief 格式化速率为人类可读字符串(B/s, KB/s, MB/s, GB/s) @param bytesPerSec 每秒字节数 @return 格式化字符串 */
QString DataStatistics::formatRate(double bytesPerSec) const
{
    if (bytesPerSec < 0.5) {
        return tr("0 B/s");
    } else if (bytesPerSec < 1024.0) {
        return tr("%1 B/s").arg(bytesPerSec, 0, 'f', 0);
    } else if (bytesPerSec < 1024.0 * 1024) {
        return tr("%1 KB/s").arg(bytesPerSec / 1024.0, 0, 'f', 1);
    } else if (bytesPerSec < 1024.0 * 1024 * 1024) {
        return tr("%1 MB/s").arg(bytesPerSec / (1024.0 * 1024.0), 0, 'f', 2);
    } else {
        return tr("%1 GB/s").arg(bytesPerSec / (1024.0 * 1024.0 * 1024.0), 0, 'f', 2);
    }
}

// ---- 错误显示 ----

/** @brief 更新串口通信错误计数 @param framingErrors 帧错误数 @param parityErrors 校验错误数 @param overrunErrors 溢出错误数 */
void DataStatistics::updateErrors(int framingErrors, int parityErrors, int overrunErrors)
{
    ++m_totalErrorUpdates;
    m_framingErrors = framingErrors;
    m_parityErrors = parityErrors;
    m_overrunErrors = overrunErrors;

    int totalErrors = framingErrors + parityErrors + overrunErrors;
    if (totalErrors > 0) {
        QStringList parts;
        if (framingErrors > 0)
            parts << tr("帧错误: %1").arg(framingErrors);
        if (parityErrors > 0)
            parts << tr("校验错误: %1").arg(parityErrors);
        if (overrunErrors > 0)
            parts << tr("溢出: %1").arg(overrunErrors);
        m_errorLabel->setText(tr("通信错误 - ") + parts.join(" | "));
        m_errorLabel->show();
    } else {
        m_errorLabel->hide();
    }
}

// ---- 连接健康状态 ----

/** @brief 更新连接健康状态(空闲超10秒显示警告) @param alive 连接是否存活 @param lastDataAgeMs 距上次收到数据的毫秒数 */
void DataStatistics::updateConnectionHealth(bool alive, qint64 lastDataAgeMs)
{
    ++m_totalHealthUpdates;
    if (!alive) {
        m_healthLabel->hide();
        return;
    }

    if (lastDataAgeMs < 0 || lastDataAgeMs < 10000) {
        m_healthLabel->hide();
        return;
    }

    int idleSec = static_cast<int>(lastDataAgeMs / 1000);
    if (idleSec >= 60) {
        int min = idleSec / 60;
        int sec = idleSec % 60;
        m_healthLabel->setText(tr("空闲 %1m%2s").arg(min).arg(sec));
    } else {
        m_healthLabel->setText(tr("空闲 %1s").arg(idleSec));
    }
    m_healthLabel->show();
}

// ---- 会话摘要 ----

/** @brief 生成会话统计摘要(持续时间/收发/峰值/均值/滚动速率/包计数/错误) @return 多行统计文本 */
QString DataStatistics::sessionSummary() const
{
    qint64 elapsedMs = m_stopwatch.isValid() ? m_stopwatch.elapsed() : 0;
    int sec = static_cast<int>(elapsedMs / 1000);
    QString timeStr = QString("%1:%2:%3")
                          .arg(sec / 3600, 2, 10, QChar('0'))
                          .arg((sec % 3600) / 60, 2, 10, QChar('0'))
                          .arg(sec % 60, 2, 10, QChar('0'));

    double peakRate = qMax(m_peakRxRate, m_peakTxRate);
    int totalErrors = m_framingErrors + m_parityErrors + m_overrunErrors;
    double rollingTotal = m_rollingRxBytesPerSec + m_rollingTxBytesPerSec;

    QString summary;
    summary += tr("会话统计\n");
    summary += tr("-----------------\n");
    summary += tr("持续时间: %1\n").arg(timeStr);
    summary += tr("接收: %1\n").arg(ByteFormat::formatSize(m_lastRxBytes));
    summary += tr("发送: %1\n").arg(ByteFormat::formatSize(m_lastTxBytes));
    summary += tr("峰值速率: %1\n").arg(formatRate(peakRate));
    summary += tr("平均速率: %1\n").arg(formatRate(m_avgRxRate + m_avgTxRate));
    summary += tr("滚动速率: %1\n").arg(formatRate(rollingTotal));
    summary += tr("数据包: RX %1 / TX %2\n").arg(m_rxPackets).arg(m_txPackets);
    if (totalErrors > 0) {
        summary += tr("错误: %1 (帧%2 校验%3 溢出%4)")
                       .arg(totalErrors)
                       .arg(m_framingErrors)
                       .arg(m_parityErrors)
                       .arg(m_overrunErrors);
    }
    return summary;
}

// ---- 统计值查询getter ----

/** @brief 返回接收累计字节数 */
quint64 DataStatistics::totalRxBytes() const { return m_lastRxBytes; }

/** @brief 返回发送累计字节数 */
quint64 DataStatistics::totalTxBytes() const { return m_lastTxBytes; }

quint64 DataStatistics::totalUpdates() const { return m_totalUpdates; }
double DataStatistics::peakRate() const { return qMax(m_peakRxRate, m_peakTxRate); }
quint64 DataStatistics::totalBytesCounted() const { return m_totalBytesCounted; }
quint64 DataStatistics::totalPeakUpdates() const { return m_totalPeakUpdates; }
