/**
 * @file DataStatisticsRefresh.cpp
 * @brief 数据统计面板 - 定时刷新回调实现
 *
 * 从 DataStatistics.cpp 拆分而来，包含1秒定时器回调:
 *   - onRefreshTimer: 刷新持续时间/速率/峰值/均值/滚动窗口/直方图/采样历史
 */

#include "serial/data/DataStatistics.h"
#include "shared/TimerConstants.h"
#include "utils/data/ByteFormat.h"
#include <QLatin1Char>

/** @brief 1秒定时器回调：刷新持续时间/速率/峰值/均值/滚动窗口/直方图/采样历史 */
void DataStatistics::onRefreshTimer()
{
    ++m_totalRefreshCycles;

    /* 1. 更新持续时间显示 */
    qint64 elapsedSec = m_stopwatch.elapsed() / 1000;
    int hours   = static_cast<int>(elapsedSec / 3600);
    int minutes = static_cast<int>((elapsedSec % 3600) / 60);
    int seconds = static_cast<int>(elapsedSec % 60);
    m_elapsedLabel->setText(
        QString("%1:%2:%3")
            .arg(hours,   2, 10, QLatin1Char('0'))
            .arg(minutes, 2, 10, QLatin1Char('0'))
            .arg(seconds, 2, 10, QLatin1Char('0'))
    );

    /* 2. 速率衰减: 超过2秒未收到update()调用，将速率归零 */
    qint64 sampleAge = m_sampleTimer.elapsed();
    if (sampleAge > 2000) {
        m_rxRate = 0.0;
        m_txRate = 0.0;
    }

    /* 3. 更新速率显示 */
    m_rxRateLabel->setText(formatRate(m_rxRate));
    m_txRateLabel->setText(formatRate(m_txRate));

    /* 4. 更新峰值速率（取历史最大值） */
    if (m_rxRate > m_peakRxRate) { m_peakRxRate = m_rxRate; ++m_totalPeakUpdates; }
    if (m_txRate > m_peakTxRate) { m_peakTxRate = m_txRate; ++m_totalPeakUpdates; }
    double peakRate = qMax(m_peakRxRate, m_peakTxRate);
    m_peakRateLabel->setText(formatRate(peakRate));

    /* 5. 计算会话平均速率（总字节/总时间） */
    double elapsedSecF = qMax(m_stopwatch.elapsed() / 1000.0, 1.0);
    m_avgRxRate = static_cast<double>(m_lastRxBytes) / elapsedSecF;
    m_avgTxRate = static_cast<double>(m_lastTxBytes) / elapsedSecF;
    double avgRate = m_avgRxRate + m_avgTxRate;
    m_avgRateLabel->setText(formatRate(avgRate));

    /* 6. 更新滚动窗口吞吐量 */
    updateRollingThroughput();

    /* 7. 更新直方图(使用当前RX+TX总速率) */
    updateHistogram(m_rxRate + m_txRate);

    /* 8. 记录吞吐量采样点 */
    ++m_totalThroughputSnapshots;
    ThroughputSample sample;
    sample.timestampMs = m_stopwatch.elapsed();
    sample.rxBytesPerSec = m_rxRate;
    sample.txBytesPerSec = m_txRate;
    sample.rxPacketsPerSec = m_rollingRxPacketsPerSec;
    sample.txPacketsPerSec = m_rollingTxPacketsPerSec;
    m_throughputHistory.push_back(sample);
    while (static_cast<int>(m_throughputHistory.size()) > kMaxThroughputSamples) {
        m_throughputHistory.pop_front();
    }

    /* 更新滚动速率显示 */
    double rollingTotal = m_rollingRxBytesPerSec + m_rollingTxBytesPerSec;
    m_rollingRateLabel->setText(formatRate(rollingTotal));

    /* 发射滚动吞吐量更新信号 */
    emit rollingThroughputUpdated(m_rollingRxBytesPerSec, m_rollingTxBytesPerSec);
}
