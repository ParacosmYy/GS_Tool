/**
 * @file DataRateCalculatorStats.cpp
 * @brief 数据速率计算器 — 统计查询与重置
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 提供 stats() 和 resetStatistics() 方法，
 * 运行时速率计算见 @see DataRateCalculator.cpp。
 */

#include "utils/rate/DataRateCalculator.h"

/**
 * @brief 获取全局统计快照
 *
 * 返回包含当前速率、峰值、累计字节、溢出/欠载次数等
 * 完整统计信息的结构体副本。线程安全(只读)。
 *
 * @return Stats 结构体快照
 */
DataRateCalculator::Stats DataRateCalculator::stats() const
{
    Stats s;
    s.totalSamples   = m_totalSamples;
    s.totalBytesTx   = m_totalBytesTx;
    s.totalBytesRx   = m_totalBytesRx;
    s.peakRateTx     = m_peakTx;
    s.peakRateRx     = m_peakRx;
    s.currentRateTx  = m_emaRateTx;
    s.currentRateRx  = m_emaRateRx;
    s.windowSizeMs   = static_cast<quint64>(static_cast<int>(m_windowSize)) * 1000ULL;
    s.totalOverflows = m_totalOverflows;
    s.totalUnderruns = m_totalUnderruns;
    return s;
}

/**
 * @brief 重置所有计数器、历史缓冲区和统计信息
 *
 * 清空 TX/RX 环形缓冲区、EMA 速率、峰值、累计字节和所有计数器。
 * 不影响窗口大小配置和 EMA 平滑系数。
 * 重置后定时器继续运行，从零开始重新采样。
 */
void DataRateCalculator::resetStatistics()
{
    // 清空环形缓冲区和窗口累计
    m_ringTx.clear();
    m_ringRx.clear();
    m_accumTx   = 0;
    m_accumRx   = 0;

    // 清空 EMA 和峰值
    m_emaRateTx = 0.0;
    m_emaRateRx = 0.0;
    m_peakTx    = 0.0;
    m_peakRx    = 0.0;

    // 清空全局统计计数器
    m_totalSamples   = 0;
    m_totalBytesTx   = 0;
    m_totalBytesRx   = 0;
    m_totalOverflows = 0;
    m_totalUnderruns = 0;
}
