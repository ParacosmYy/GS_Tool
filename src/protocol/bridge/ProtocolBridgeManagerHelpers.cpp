/**
 * @file ProtocolBridgeManagerHelpers.cpp
 * @brief 协议桥管理器 — 内部信号处理槽和信号连接切换
 *
 * 从 ProtocolBridgeManager.cpp 拆分出的内部辅助功能:
 *   - 信号处理槽: onFrameParserParsed / onFrameParserError / onBridgeParsed
 *   - 信号连接切换: switchSource()
 *
 * 这些函数由 ProtocolBridgeManager 内部调用，不对外暴露。
 */

#include "protocol/bridge/ProtocolBridgeManager.h"

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
