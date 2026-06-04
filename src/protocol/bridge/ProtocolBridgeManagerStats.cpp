/**
 * @file ProtocolBridgeManagerStats.cpp
 * @brief 协议桥管理器 — 管理器级统计/每协议统计/吞吐量接口(拆分自ProtocolBridgeManager.cpp)
 *
 * 实现:
 *   - 管理器级统计: totalBridges/totalFramesParsedAll/totalParseErrors/totalBytesProcessed
 *   - 每协议统计: 各协议独立帧数/字节/错误查询
 *   - 吞吐量: 基于滑动窗口的实时帧率和字节率计算
 *   - 吞吐量滑动窗口更新(updateThroughput)
 */

#include "protocol/bridge/ProtocolBridgeManager.h"

// ============================================================================
// 管理器级统计接口
// ============================================================================

/** @brief 获取累计桥接器切换次数 @return 切换次数 */
quint64 ProtocolBridgeManager::totalBridges() const
{
    return m_totalBridges;
}

/** @brief 获取所有协议源累计解析的总帧数（含所有模式） @return 总帧数 */
quint64 ProtocolBridgeManager::totalFramesParsedAll() const
{
    return m_totalFramesParsedAll;
}

/** @brief 获取所有协议源累计解析错误总数（含所有模式） @return 总错误数 */
quint64 ProtocolBridgeManager::totalParseErrors() const
{
    return m_totalParseErrors;
}

/** @brief 获取累计处理的字节总数 @return 字节数 */
quint64 ProtocolBridgeManager::totalBytesProcessed() const
{
    return m_totalBytesProcessed;
}

/** @brief 获取累计feedData调用次数 @return 调用计数 */
quint64 ProtocolBridgeManager::totalFeedDataCalls() const
{
    return m_totalFeedDataCalls;
}

/** @brief 获取累计空数据跳过次数 @return 跳过计数 */
quint64 ProtocolBridgeManager::totalEmptyDataSkips() const
{
    return m_totalEmptyDataSkips;
}

/** @brief 获取累计自动检测尝试次数 @return 检测尝试计数 */
quint64 ProtocolBridgeManager::totalAutoDetectAttempts() const
{
    return m_totalAutoDetectAttempts;
}

/** @brief 获取累计自动检测成功次数 @return 检测成功计数 */
quint64 ProtocolBridgeManager::totalAutoDetectSuccesses() const
{
    return m_totalAutoDetectSuccesses;
}

// ============================================================================
// 每协议统计接口
// ============================================================================

/** @brief 获取指定协议的累计统计 @param mode 协议模式 @return 该协议的帧数/字节/错误 */
ProtocolBridgeManager::ProtocolStats ProtocolBridgeManager::protocolStats(
    ChartProtocolMode mode) const
{
    return m_protocolStats.value(mode, ProtocolStats{0, 0, 0});
}

/** @brief 获取所有协议的统计汇总 @return 模式→统计的Map */
QMap<ProtocolBridgeManager::ChartProtocolMode, ProtocolBridgeManager::ProtocolStats>
ProtocolBridgeManager::allProtocolStats() const
{
    return m_protocolStats;
}

// ============================================================================
// 吞吐量接口
// ============================================================================

/** @brief 获取当前吞吐量快照(基于滑动窗口) @return 帧率和字节率 */
ProtocolBridgeManager::ThroughputSnapshot ProtocolBridgeManager::throughput() const
{
    // 计算自上次重置以来经过的秒数
    qint64 elapsedMs = m_throughputTimer.elapsed();
    if (elapsedMs <= 0) {
        return m_lastThroughput;
    }

    double elapsedSec = static_cast<double>(elapsedMs) / 1000.0;
    // 避免时间窗口过短导致速率剧烈波动（至少0.5秒）
    if (elapsedSec < 0.5) {
        return m_lastThroughput;
    }

    m_lastThroughput.framesPerSec =
        static_cast<double>(m_throughputFrameCount) / elapsedSec;
    m_lastThroughput.bytesPerSec =
        static_cast<double>(m_throughputByteCount) / elapsedSec;

    return m_lastThroughput;
}

// ============================================================================
// 吞吐量滑动窗口更新
// ============================================================================

/** @brief 更新吞吐量滑动窗口 @param frameBytes 本次帧字节数 */
void ProtocolBridgeManager::updateThroughput(quint64 frameBytes)
{
    ++m_throughputFrameCount;
    m_throughputByteCount += frameBytes;

    // 每隔10秒重置滑动窗口，避免长期累积导致速率失真
    if (m_throughputTimer.elapsed() > 10000) {
        m_throughputFrameCount = 0;
        m_throughputByteCount = 0;
        m_throughputTimer.restart();
    }
}
