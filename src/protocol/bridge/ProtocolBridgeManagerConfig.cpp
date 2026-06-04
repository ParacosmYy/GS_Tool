/**
 * @file ProtocolBridgeManagerConfig.cpp
 * @brief 协议桥管理器 — 模式配置、数据路由与桥接器访问器(拆分自ProtocolBridgeManager.cpp)
 *
 * 实现:
 *   - 模式设置: setProtocolMode() 重置旧源→切换→重连信号→通知UI
 *   - 模式查询: protocolMode() 获取当前协议模式
 *   - 数据路由: feedData() 根据当前模式将数据转发到对应的数据源(含自动检测采样)
 *   - 桥接器访问器: activeBridge/frameParser/justFloatBridge/fireWaterBridge
 *   - 动态切换: switchActiveBridge() 不中断数据流的快速桥接器切换
 */

#include "protocol/bridge/ProtocolBridgeManager.h"

#include <QDebug>

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
