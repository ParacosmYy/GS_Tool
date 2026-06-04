/**
 * @file DataStatisticsUpdate.cpp
 * @brief 数据统计面板 - 数据更新与重置方法
 *
 * 从 DataStatistics.cpp 拆分而来，包含:
 *   - update(): 收发字节增量速率计算与包计数
 *   - reset(): 全量重置所有统计值、滚动窗口、直方图、UI显示
 *   - rxRate()/txRate(): 速率查询getter
 *
 * UI初始化、构造函数见 DataStatistics.cpp。
 */

#include "serial/data/DataStatistics.h"
#include "utils/data/ByteFormat.h"

#include <algorithm>

/** @brief 用最新收发字节总数更新统计(计算增量速率和包计数) @param rxBytes 接收累计字节 @param txBytes 发送累计字节 */
void DataStatistics::update(quint64 rxBytes, quint64 txBytes)
{
    // 计算自上次采样以来的实际时间间隔（毫秒）
    qint64 elapsedMs = m_sampleTimer.elapsed();
    m_sampleTimer.restart();

    // 计算增量字节(防下溢: 计数器重置时归零，避免产生天文数字速率)
    quint64 rxDelta = (rxBytes >= m_lastRxBytes) ? (rxBytes - m_lastRxBytes) : 0;
    quint64 txDelta = (txBytes >= m_lastTxBytes) ? (txBytes - m_lastTxBytes) : 0;

    // 按实际时间间隔换算为 bytes/s
    double seconds = qMax(elapsedMs, static_cast<qint64>(1)) / 1000.0;
    m_rxRate = static_cast<double>(rxDelta) / seconds;
    m_txRate = static_cast<double>(txDelta) / seconds;

    // 记录本次采样值，供下次计算增量
    m_lastRxBytes = rxBytes;
    m_lastTxBytes = txBytes;

    // 更新累计字节数显示
    m_rxTotalLabel->setText(ByteFormat::formatSize(rxBytes));
    m_txTotalLabel->setText(ByteFormat::formatSize(txBytes));

    // 累加数据包计数（每次调用视为一次数据包到达）
    if (rxDelta > 0) ++m_rxPackets;
    if (txDelta > 0) ++m_txPackets;

    // 统计计数器递增
    ++m_totalUpdates;
    m_totalBytesCounted += static_cast<quint64>(rxDelta) + static_cast<quint64>(txDelta);
    ++m_totalCalculations;
}

/** @brief 重置所有统计值和UI显示，重启计时器，清空滚动窗口和直方图 */
void DataStatistics::reset()
{
    // 统计: 累计重置次数(在其他计数器清零之前递增)
    ++m_totalResets;

    // 重置累计值和速率
    m_lastRxBytes = 0;
    m_lastTxBytes = 0;
    m_rxRate = 0.0;
    m_txRate = 0.0;
    m_peakRxRate = 0.0;
    m_peakTxRate = 0.0;
    m_avgRxRate = 0.0;
    m_avgTxRate = 0.0;
    m_rxPackets = 0;
    m_txPackets = 0;

    // 重置统计计数器
    m_totalUpdates = 0;
    m_totalBytesCounted = 0;
    m_totalPeakUpdates = 0;
    m_totalErrorUpdates = 0;
    m_totalHealthUpdates = 0;
    m_totalRefreshCycles = 0;
    m_totalCalculations = 0;
    m_totalHistogramUpdates = 0;
    m_totalSlidingWindowResets = 0;
    m_totalThroughputSnapshots = 0;
    // 注意: m_totalResets 不在此处清零，保留累计重置次数
    m_totalFormatChanges = 0;

    // 重置错误计数
    m_framingErrors = 0;
    m_parityErrors = 0;
    m_overrunErrors = 0;

    // 重置滚动窗口
    m_rollingWindow.clear();
    m_prevSecondRxBytes = 0;
    m_prevSecondTxBytes = 0;
    m_prevSecondRxPackets = 0;
    m_prevSecondTxPackets = 0;
    m_rollingRxBytesPerSec = 0.0;
    m_rollingTxBytesPerSec = 0.0;
    m_rollingRxPacketsPerSec = 0.0;
    m_rollingTxPacketsPerSec = 0.0;

    // 重置直方图
    initHistogramBuckets();

    // 重置采样历史
    m_throughputHistory.clear();

    // 重置UI显示
    m_rxTotalLabel->setText(tr("0 B"));
    m_txTotalLabel->setText(tr("0 B"));
    m_rxRateLabel->setText(tr("0 B/s"));
    m_txRateLabel->setText(tr("0 B/s"));
    m_peakRateLabel->setText(tr("0 B/s"));
    m_avgRateLabel->setText(tr("0 B/s"));
    m_rollingRateLabel->setText(tr("0 B/s"));
    m_elapsedLabel->setText(tr("00:00:00"));
    m_errorLabel->hide();
    m_healthLabel->hide();

    // 重启计时器
    m_stopwatch.restart();
}

/** @brief 返回当前接收速率(bytes/s) */
double DataStatistics::rxRate() const { return m_rxRate; }

/** @brief 返回当前发送速率(bytes/s) */
double DataStatistics::txRate() const { return m_txRate; }
