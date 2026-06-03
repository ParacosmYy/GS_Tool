/**
 * @file DataStatisticsRolling.cpp
 * @brief 数据统计面板 — 滚动窗口吞吐量、直方图和采样历史实现
 *
 * 从DataStatistics.cpp拆分，负责:
 *   1. 滚动窗口吞吐量计算(基于最近5秒的滑动窗口)
 *   2. 吞吐量直方图数据分桶(10个桶覆盖0~10MB/s)
 *   3. 吞吐量采样历史记录(最多300个采样点，5分钟)
 *   4. 统计计数器查询和重置
 */

#include "serial/data/DataStatistics.h"
#include "utils/data/ByteFormat.h"

#include <algorithm>

// ---- 滚动窗口吞吐量 ----

/**
 * @brief 更新滚动窗口吞吐量
 *
 * 每秒调用，计算上一秒增量并推入窗口队列，最终计算窗口内平均速率。
 * 窗口大小为kRollingWindowSeconds(5秒)，自动淘汰过期数据。
 */
void DataStatistics::updateRollingThroughput()
{
    // 计算上一秒的增量
    RollingInterval interval;
    interval.rxBytesDelta = (m_lastRxBytes >= m_prevSecondRxBytes)
                            ? (m_lastRxBytes - m_prevSecondRxBytes) : 0;
    interval.txBytesDelta = (m_lastTxBytes >= m_prevSecondTxBytes)
                            ? (m_lastTxBytes - m_prevSecondTxBytes) : 0;
    interval.rxPacketDelta = static_cast<int>(
        m_rxPackets >= m_prevSecondRxPackets ? (m_rxPackets - m_prevSecondRxPackets) : 0);
    interval.txPacketDelta = static_cast<int>(
        m_txPackets >= m_prevSecondTxPackets ? (m_txPackets - m_prevSecondTxPackets) : 0);

    // 更新前一秒基准值
    m_prevSecondRxBytes = m_lastRxBytes;
    m_prevSecondTxBytes = m_lastTxBytes;
    m_prevSecondRxPackets = m_rxPackets;
    m_prevSecondTxPackets = m_txPackets;

    // 推入窗口队列
    m_rollingWindow.push_back(interval);
    while (static_cast<int>(m_rollingWindow.size()) > kRollingWindowSeconds) {
        m_rollingWindow.pop_front();
    }

    // 计算窗口内平均速率
    quint64 totalRxBytes = 0, totalTxBytes = 0;
    int totalRxPkts = 0, totalTxPkts = 0;
    for (const auto& ri : m_rollingWindow) {
        totalRxBytes += ri.rxBytesDelta;
        totalTxBytes += ri.txBytesDelta;
        totalRxPkts += ri.rxPacketDelta;
        totalTxPkts += ri.txPacketDelta;
    }

    int windowSize = qMax(static_cast<int>(m_rollingWindow.size()), 1);
    m_rollingRxBytesPerSec = static_cast<double>(totalRxBytes) / windowSize;
    m_rollingTxBytesPerSec = static_cast<double>(totalTxBytes) / windowSize;
    m_rollingRxPacketsPerSec = static_cast<double>(totalRxPkts) / windowSize;
    m_rollingTxPacketsPerSec = static_cast<double>(totalTxPkts) / windowSize;
}

// ---- 直方图 ----

/** @brief 初始化直方图桶(10个桶: 空闲/极低速/低速/中低速/中速/中高速/高速/极高速/超高速/极限) */
void DataStatistics::initHistogramBuckets()
{
    m_histogramBuckets.clear();
    m_histogramTotalSamples = 0;

    struct BucketDef { double lower; double upper; const char* lbl; };
    static const BucketDef defs[] = {
        { 0.0,    0.5,              "0 B/s"         },
        { 0.5,    100.0,            "0~100 B/s"     },
        { 100.0,  1024.0,           "100 B~1 KB/s"  },
        { 1024.0, 10240.0,          "1~10 KB/s"     },
        { 10240.0,51200.0,          "10~50 KB/s"    },
        { 51200.0,102400.0,         "50~100 KB/s"   },
        { 102400.0,512000.0,        "100~500 KB/s"  },
        { 512000.0,1048576.0,       "500KB~1 MB/s"  },
        { 1048576.0,5242880.0,      "1~5 MB/s"      },
        { 5242880.0,1e18,           ">5 MB/s"       },
    };

    for (const auto& d : defs) {
        HistogramBucket bucket;
        bucket.lowerBound = d.lower;
        bucket.upperBound = d.upper;
        bucket.sampleCount = 0;
        bucket.label = tr(d.lbl);
        m_histogramBuckets.append(bucket);
    }
}

/** @brief 更新直方图数据(将当前总速率分桶计数) @param totalBytesPerSec 当前总速率(bytes/s) */
void DataStatistics::updateHistogram(double totalBytesPerSec)
{
    ++m_histogramTotalSamples;
    for (auto& bucket : m_histogramBuckets) {
        if (totalBytesPerSec >= bucket.lowerBound && totalBytesPerSec < bucket.upperBound) {
            ++bucket.sampleCount;
            break;
        }
    }
}

// ---- 吞吐量采样历史 ----

/** @brief 获取最近N个吞吐量采样点 @param maxCount 最大数量，0=全部 @return 采样点列表 */
QVector<ThroughputSample> DataStatistics::throughputHistory(int maxCount) const
{
    if (maxCount <= 0 || maxCount >= static_cast<int>(m_throughputHistory.size()))
        return QVector<ThroughputSample>(m_throughputHistory.begin(), m_throughputHistory.end());
    int start = static_cast<int>(m_throughputHistory.size()) - maxCount;
    QVector<ThroughputSample> result;
    result.reserve(maxCount);
    for (int i = start; i < static_cast<int>(m_throughputHistory.size()); ++i)
        result.append(m_throughputHistory[i]);
    return result;
}

// ---- 统计计数器 ----

/** @brief 重置数据统计计数器(不影响面板显示) */
void DataStatistics::resetDataStatistics()
{
    m_totalUpdates = 0;
    m_totalBytesCounted = 0;
    m_totalPeakUpdates = 0;
    m_totalErrorUpdates = 0;
    m_totalHealthUpdates = 0;
    m_totalRefreshCycles = 0;
    m_totalCalculations = 0;
}
