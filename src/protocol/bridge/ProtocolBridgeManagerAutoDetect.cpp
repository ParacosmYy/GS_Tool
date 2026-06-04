/**
 * @file ProtocolBridgeManagerAutoDetect.cpp
 * @brief 协议桥自动检测实现 — 从数据流推断协议类型
 *
 * 从 ProtocolBridgeManagerHelpers.cpp 拆分出的自动检测功能:
 *   - 自动检测评分函数 (scoreJustFloat / scoreFireWater)
 *   - 自动检测主接口 (detectProtocol)
 *   - 自动检测控制 (setAutoDetectEnabled / isAutoDetectEnabled / lastAutoDetectResult)
 */

#include "protocol/bridge/ProtocolBridgeManager.h"

#include <QtGlobal>

// ============================================================================
// 自动检测评分函数
// ============================================================================

/** @brief 评估数据匹配 JustFloat 协议的程度 @param data 采样数据 @return 匹配分值 [0.0, 1.0] */
double ProtocolBridgeManager::scoreJustFloat(const QByteArray& data) const
{
    static constexpr unsigned char kTail[4] = {0x00, 0x00, 0x80, 0x7F};
    int tailMatches = 0;
    int totalAlignments = 0;

    for (int i = 0; i <= data.size() - 4; ++i) {
        if (static_cast<unsigned char>(data[i])     == kTail[0] &&
            static_cast<unsigned char>(data[i + 1]) == kTail[1] &&
            static_cast<unsigned char>(data[i + 2]) == kTail[2] &&
            static_cast<unsigned char>(data[i + 3]) == kTail[3]) {
            ++tailMatches;
            if (i % 4 == 0) { ++totalAlignments; }
        }
    }

    if (tailMatches == 0) { return 0.0; }

    double alignmentRatio = static_cast<double>(totalAlignments) /
                            static_cast<double>(tailMatches);
    double countBonus = qMin(1.0, static_cast<double>(tailMatches) / 2.0);
    return alignmentRatio * countBonus;
}

/** @brief 评估数据匹配 FireWater 协议的程度 @param data 采样数据 @return 匹配分值 [0.0, 1.0] */
double ProtocolBridgeManager::scoreFireWater(const QByteArray& data) const
{
    int printableCount = 0;
    int newlineCount = 0;
    int separatorCount = 0;
    int digitDotCount = 0;

    for (int i = 0; i < data.size(); ++i) {
        unsigned char ch = static_cast<unsigned char>(data[i]);
        if (ch == '\n' || ch == '\r') { ++newlineCount; }
        else if (ch == ',' || ch == '\t' || ch == ' ') { ++separatorCount; }
        else if ((ch >= '0' && ch <= '9') || ch == '.' || ch == '-' || ch == '+') { ++digitDotCount; }
        else if (ch >= 0x20 && ch < 0x7F) { ++printableCount; }
    }

    int totalChars = data.size();
    if (totalChars == 0 || newlineCount == 0) { return 0.0; }

    double printableRatio =
        static_cast<double>(printableCount + newlineCount +
                            separatorCount + digitDotCount) / totalChars;
    double separatorRatio = static_cast<double>(separatorCount) / totalChars;
    double numericRatio = static_cast<double>(digitDotCount) / totalChars;

    double score = printableRatio * 0.3 +
                   qMin(separatorRatio * 5.0, 1.0) * 0.3 +
                   qMin(numericRatio * 3.0, 1.0) * 0.2 +
                   qMin(static_cast<double>(newlineCount) / 2.0, 1.0) * 0.2;
    return qBound(0.0, score, 1.0);
}

// ============================================================================
// 自动检测接口
// ============================================================================

/** @brief 从数据流自动检测协议类型 @param data 采样数据 @return 检测结果(模式+置信度) */
ProtocolBridgeManager::AutoDetectResult
ProtocolBridgeManager::detectProtocol(const QByteArray& data) const
{
    AutoDetectResult result;
    result.detectedMode = ChartProtocolMode::FrameParser;
    result.confidence = 0.0;
    result.detected = false;

    if (data.size() < kAutoDetectMinBytes) { return result; }

    double jfScore = scoreJustFloat(data);
    double fwScore = scoreFireWater(data);

    if (jfScore > fwScore && jfScore >= 0.5) {
        result.detectedMode = ChartProtocolMode::JustFloat;
        result.confidence = jfScore;
        result.detected = true;
    } else if (fwScore >= 0.5) {
        result.detectedMode = ChartProtocolMode::FireWater;
        result.confidence = fwScore;
        result.detected = true;
    } else {
        result.detectedMode = ChartProtocolMode::FrameParser;
        result.confidence = 1.0 - qMax(jfScore, fwScore);
        result.detected = true;
    }

    return result;
}

/** @brief 启用/禁用自动检测模式 @param enable true启用 */
void ProtocolBridgeManager::setAutoDetectEnabled(bool enable)
{
    m_autoDetectEnabled = enable;
    m_autoDetectBuffer.clear();
    m_lastDetectResult = AutoDetectResult();
}

/** @brief 查询自动检测是否启用 @return true已启用 */
bool ProtocolBridgeManager::isAutoDetectEnabled() const
{
    return m_autoDetectEnabled;
}

/** @brief 获取最后一次自动检测结果 @return 检测结果快照 */
ProtocolBridgeManager::AutoDetectResult
ProtocolBridgeManager::lastAutoDetectResult() const
{
    return m_lastDetectResult;
}
