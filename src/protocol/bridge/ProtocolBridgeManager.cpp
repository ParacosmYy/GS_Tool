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
 */

#include "protocol/bridge/ProtocolBridgeManager.h"

#include <QMetaObject>
#include <QMetaMethod>
#include <QDebug>
#include <cstring>

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

// ============================================================================
// 模式设置 / 数据路由
// ============================================================================

/** @brief 设置协议模式(重置旧源→切换→重连信号→通知UI) @param mode 目标协议模式 */
void ProtocolBridgeManager::setProtocolMode(ChartProtocolMode mode)
{
    if (m_mode == mode) {
        return; // 模式未变，无需切换
    }

    // 重置旧源状态
    if (m_activeBridge) {
        m_activeBridge->reset();
    } else if (m_mode == ChartProtocolMode::FrameParser) {
        m_frameParser->reset();
    }

    // 切换模式并累加桥接器切换计数
    m_mode = mode;
    ++m_totalBridges;

    // 关闭自动检测(用户手动切换后停止)
    m_autoDetectEnabled = false;
    m_autoDetectBuffer.clear();

    // 重新连接信号
    switchSource();

    // 通知UI更新通道配置
    emit protocolModeChanged(m_mode);
}

/** @brief 获取当前协议模式 @return 当前活动协议模式 */
ProtocolBridgeManager::ChartProtocolMode ProtocolBridgeManager::protocolMode() const
{
    return m_mode;
}

/** @brief 接收原始串口数据并转发到当前活动的协议源(含自动检测) @param data 原始字节数据 */
void ProtocolBridgeManager::feedData(const QByteArray& data)
{
    // ---- 空数据保护 ----
    if (data.isEmpty()) {
        return;
    }

    // 累加处理字节统计
    const quint64 dataLen = static_cast<quint64>(data.size());
    m_totalBytesProcessed += dataLen;

    // ---- 自动检测模式: 采样数据推断协议类型 ----
    if (m_autoDetectEnabled && !m_lastDetectResult.detected) {
        m_autoDetectBuffer.append(data);
        // 限制采样缓冲区大小
        if (m_autoDetectBuffer.size() > kAutoDetectMaxBytes) {
            m_autoDetectBuffer = m_autoDetectBuffer.right(kAutoDetectMaxBytes);
        }

        // 采样数据足够时进行检测
        if (m_autoDetectBuffer.size() >= kAutoDetectMinBytes) {
            AutoDetectResult result = detectProtocol(m_autoDetectBuffer);
            m_lastDetectResult = result;

            if (result.detected && result.confidence >= 0.6) {
                // 置信度足够高，自动切换到检测到的模式
                m_autoDetectEnabled = false;
                m_mode = result.detectedMode;
                ++m_totalBridges;
                switchSource();
                emit autoDetectCompleted(result);
                emit protocolModeChanged(m_mode);
            }
        }
    }

    // 路由到对应协议处理器
    switch (m_mode) {
    case ChartProtocolMode::FrameParser:
        if (m_frameParser) {
            m_frameParser->feed(data);
        } else {
            qWarning() << tr("帧解析模式但解析器为空");
        }
        break;

    case ChartProtocolMode::JustFloat:
        if (m_justFloat) {
            m_justFloat->feed(data);
        } else {
            qWarning() << tr("JustFloat模式但桥接器为空");
        }
        break;

    case ChartProtocolMode::FireWater:
        if (m_fireWater) {
            m_fireWater->feed(data);
        } else {
            qWarning() << tr("FireWater模式但桥接器为空");
        }
        break;

    default:
        qWarning() << tr("未知协议模式: %1").arg(static_cast<int>(m_mode));
        break;
    }
}

// ============================================================================
// 桥接器访问器
// ============================================================================

/** @brief 获取当前活动的桥 @return 当前活动桥指针，FrameParser模式下返回nullptr */
IProtocolBridge* ProtocolBridgeManager::activeBridge() const
{
    return m_activeBridge;
}

/** @brief 获取FrameParser指针 @return 帧解析器指针 */
FrameParser* ProtocolBridgeManager::frameParser() const
{
    return m_frameParser;
}

/** @brief 获取JustFloatBridge指针 @return JustFloat协议桥指针 */
JustFloatBridge* ProtocolBridgeManager::justFloatBridge() const
{
    return m_justFloat;
}

/** @brief 获取FireWaterBridge指针 @return FireWater协议桥指针 */
FireWaterBridge* ProtocolBridgeManager::fireWaterBridge() const
{
    return m_fireWater;
}

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
// 运行时动态切换
// ============================================================================

/** @brief 动态切换活跃桥接器(不重置旧源，保留中间数据) @param mode 目标协议模式 */
void ProtocolBridgeManager::switchActiveBridge(ChartProtocolMode mode)
{
    if (m_mode == mode) {
        return;
    }

    m_mode = mode;
    ++m_totalBridges;

    switchSource();

    emit protocolModeChanged(m_mode);
}

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

// ============================================================================
// 内部信号处理槽
// ============================================================================

/** @brief 处理FrameParser帧解析成功(累加统计+每协议+吞吐量后转发) @param fields 字段映射 @param rawFrame 原始帧数据 */
void ProtocolBridgeManager::onFrameParserParsed(
    const QVariantMap& fields, const QByteArray& rawFrame)
{
    ++m_totalFramesParsedAll;

    // 更新每协议统计
    m_protocolStats[ChartProtocolMode::FrameParser].frames++;
    m_protocolStats[ChartProtocolMode::FrameParser].bytes +=
        static_cast<quint64>(rawFrame.size());

    // 更新吞吐量
    updateThroughput(static_cast<quint64>(rawFrame.size()));

    emit frameParsed(fields, rawFrame);
}

/** @brief 处理FrameParser帧解析错误(检测校验错误+每协议+累加统计+转发) @param reason 错误原因 @param rawFrame 原始帧数据 */
void ProtocolBridgeManager::onFrameParserError(
    const QString& reason, const QByteArray& rawFrame)
{
    if (reason.contains(QLatin1String("Checksum"))) {
        m_checksumErrors++;
    }
    m_totalErrors++;
    ++m_totalParseErrors;

    // 更新每协议错误统计
    m_protocolStats[ChartProtocolMode::FrameParser].errors++;

    emit frameError(reason, rawFrame);
}

/** @brief 处理桥接器帧解析成功(累加帧计数+每协议+吞吐量后转发) @param fields 字段映射 @param rawFrame 原始帧数据 */
void ProtocolBridgeManager::onBridgeParsed(
    const QVariantMap& fields, const QByteArray& rawFrame)
{
    m_totalFramesParsed++;
    ++m_totalFramesParsedAll;

    // 更新每协议统计(根据当前模式)
    m_protocolStats[m_mode].frames++;
    m_protocolStats[m_mode].bytes +=
        static_cast<quint64>(rawFrame.size());

    // 更新吞吐量
    updateThroughput(static_cast<quint64>(rawFrame.size()));

    emit frameParsed(fields, rawFrame);
}

// ============================================================================
// 信号连接切换
// ============================================================================

/** @brief 切换数据源连接(断开所有源→根据模式重连活动源→内部槽拦截统计) */
void ProtocolBridgeManager::switchSource()
{
    // ---- 先断开所有源到本manager转发的连接 ----
    disconnect(m_frameParser, &FrameParser::frameParsed,
               this, &ProtocolBridgeManager::onFrameParserParsed);
    disconnect(m_frameParser, &FrameParser::frameError,
               this, &ProtocolBridgeManager::onFrameParserError);
    disconnect(m_justFloat, &JustFloatBridge::frameParsed,
               this, &ProtocolBridgeManager::onBridgeParsed);
    disconnect(m_fireWater, &FireWaterBridge::frameParsed,
               this, &ProtocolBridgeManager::onBridgeParsed);

    // ---- 根据模式设置活动桥并连接信号 ----
    switch (m_mode) {
    case ChartProtocolMode::FrameParser:
        m_activeBridge = nullptr;
        connect(m_frameParser, &FrameParser::frameParsed,
                this, &ProtocolBridgeManager::onFrameParserParsed);
        connect(m_frameParser, &FrameParser::frameError,
                this, &ProtocolBridgeManager::onFrameParserError);
        break;

    case ChartProtocolMode::JustFloat:
        m_activeBridge = m_justFloat;
        connect(m_justFloat, &JustFloatBridge::frameParsed,
                this, &ProtocolBridgeManager::onBridgeParsed);
        break;

    case ChartProtocolMode::FireWater:
        m_activeBridge = m_fireWater;
        connect(m_fireWater, &FireWaterBridge::frameParsed,
                this, &ProtocolBridgeManager::onBridgeParsed);
        break;

    default:
        qWarning() << "ProtocolBridgeManager: unknown mode" << static_cast<int>(m_mode);
        m_activeBridge = nullptr;
        break;
    }
}
