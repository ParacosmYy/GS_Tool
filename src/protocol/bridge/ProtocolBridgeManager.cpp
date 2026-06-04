/**
 * @file ProtocolBridgeManager.cpp
 * @brief 协议桥管理器实现
 *
 * 实现:
 *   - 构造/析构: 持有三个数据源对象，默认使用 FrameParser 模式
 *   - 模式切换: 重置旧源 -> 断开旧信号 -> 连接新信号 -> 通知 UI
 *   - 数据路由: feedData() 根据当前模式将数据转发到对应的数据源
 *   - 空数据保护: 空数据直接忽略，避免无意义的处理开销
 *   - 状态查询: 提供桥接器解析状态、帧计数、错误统计的查询接口
 *   - 动态切换: switchActiveBridge() 不中断数据流的快速桥接器切换
 *   - 错误统计: 通过内部槽函数追踪 FrameParser 和桥接器的解析错误
 *   - 每协议统计: 各协议独立维护帧数/字节数/错误数
 *   - 吞吐量: 基于滑动窗口的实时帧率和字节率计算
 *   - 自动检测: 从数据模式推断 JustFloat / FireWater / FrameParser
 *
 * 拆分:
 *   - ProtocolBridgeManagerConfig.cpp — 模式设置/数据路由/桥接器访问器/动态切换
 *   - ProtocolBridgeManagerStats.cpp — 管理器级统计/每协议统计/吞吐量接口+滑动窗口更新
 *   - ProtocolBridgeManagerHelpers.cpp — 内部信号处理槽/信号连接切换
 *   - ProtocolBridgeManagerAutoDetect.cpp — 自动检测评分/接口
 */

#include "protocol/bridge/ProtocolBridgeManager.h"

#include <QDebug>

// ============================================================================
// 构造 / 析构
// ============================================================================

/** @brief 构造协议桥管理器，初始化每协议统计和吞吐量计时器 @param frameParser 外部创建的帧解析器 @param parent 父对象 */
ProtocolBridgeManager::ProtocolBridgeManager(FrameParser* frameParser, QObject* parent)
    : QObject(parent)
    , m_frameParser(frameParser)
    , m_justFloat(new JustFloatBridge(this))
    , m_fireWater(new FireWaterBridge(this))
    , m_activeBridge(nullptr)
    , m_mode(ChartProtocolMode::FrameParser)
{
    // FrameParser是外部创建的对象，如果尚未设置parent则归我们管理
    if (m_frameParser && !m_frameParser->parent()) {
        m_frameParser->setParent(this);
    }

    // 初始化每协议统计结构
    m_protocolStats[ChartProtocolMode::FrameParser] = {0, 0, 0};
    m_protocolStats[ChartProtocolMode::JustFloat]   = {0, 0, 0};
    m_protocolStats[ChartProtocolMode::FireWater]    = {0, 0, 0};

    // 启动吞吐量基准计时器
    m_throughputTimer.start();

    // 初始连接: 默认FrameParser模式，连接FrameParser的信号
    switchSource();
}

// ---- 模式设置/数据路由/桥接器访问器/动态切换 已拆分至 ProtocolBridgeManagerConfig.cpp ----

// ============================================================================
// 桥接器状态查询接口
// ============================================================================

/** @brief 获取当前活动桥接器的运行时统计信息 @return BridgeStats结构体 */
ProtocolBridgeManager::BridgeStats ProtocolBridgeManager::bridgeStats() const
{
    BridgeStats stats;

    if (m_mode == ChartProtocolMode::FrameParser && m_frameParser) {
        stats.totalFramesParsed = m_frameParser->frameCount();
        stats.totalErrors = m_frameParser->errorCount();
        stats.isParsing = false;
    } else {
        stats.totalFramesParsed = m_totalFramesParsed;
        stats.totalErrors = m_totalErrors;
    }

    stats.checksumErrors = m_checksumErrors;

    return stats;
}

/** @brief 查询当前活动桥接器是否正在解析 @return true=正在解析，false=空闲 */
bool ProtocolBridgeManager::isParsing() const
{
    return bridgeStats().isParsing;
}

/** @brief 获取当前活动桥接器的累计成功解析帧数 @return 帧数 */
quint64 ProtocolBridgeManager::totalFramesParsed() const
{
    return bridgeStats().totalFramesParsed;
}

/** @brief 获取当前活动桥接器的累计解析错误次数 @return 错误次数 */
quint64 ProtocolBridgeManager::totalErrors() const
{
    return bridgeStats().totalErrors;
}

/** @brief 获取当前活动桥接器的累计校验错误次数 @return 校验错误次数 */
quint64 ProtocolBridgeManager::checksumErrors() const
{
    return m_checksumErrors;
}


// ---- 管理器级统计/每协议统计/吞吐量接口 已拆分至 ProtocolBridgeManagerStats.cpp ----
// ---- 自动检测评分/接口 已拆分至 ProtocolBridgeManagerAutoDetect.cpp ----

/** @brief 重置所有桥接器和管理器的统计计数器 */
void ProtocolBridgeManager::resetStats()
{
    m_totalFramesParsed = 0;
    m_totalErrors = 0;
    m_checksumErrors = 0;

    m_totalBridges = 0;
    m_totalFramesParsedAll = 0;
    m_totalParseErrors = 0;
    m_totalBytesProcessed = 0;
    m_totalFeedDataCalls = 0;
    m_totalEmptyDataSkips = 0;
    m_totalAutoDetectAttempts = 0;
    m_totalAutoDetectSuccesses = 0;

    // 重置每协议统计
    for (auto it = m_protocolStats.begin(); it != m_protocolStats.end(); ++it) {
        *it = ProtocolStats{0, 0, 0};
    }

    // 重置吞吐量窗口
    m_throughputFrameCount = 0;
    m_throughputByteCount = 0;
    m_throughputTimer.restart();
    m_lastThroughput = ThroughputSnapshot{0.0, 0.0};

    // 重置 FrameParser 的内部计数器
    if (m_frameParser) {
        m_frameParser->resetStats();
    }

    // 重置各桥接器统计
    if (m_justFloat) {
        m_justFloat->resetStatistics();
    }
    if (m_fireWater) {
        m_fireWater->resetStatistics();
    }
}

// ---- 内部信号处理槽/信号连接切换 已拆分至 ProtocolBridgeManagerHelpers.cpp ----
