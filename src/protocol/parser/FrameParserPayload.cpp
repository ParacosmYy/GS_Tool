/**
 * @file FrameParserPayload.cpp
 * @brief 帧解析状态机 — 载荷接收阶段处理
 *
 * 从 FrameParserStateHandlers.cpp 拆分而来，包含载荷接收阶段的
 * 完整处理逻辑:
 *   - handlePayloadReceiving(): 载荷接收(长度/帧尾/校验/纯header四条路径)
 *
 * 该方法是状态机中最复杂的单一状态处理器，负责在无帧尾模式下
 * 通过长度字段精确接收、帧尾搜索、校验偏移检测和纯header模式
 * 下的下一帧头边界检测四种路径完成载荷数据收集。
 */

#include "protocol/parser/FrameParser.h"

// ============================================================================
// 载荷接收阶段
// ============================================================================

/** @brief 状态机:载荷接收阶段(逐字节填充payload缓冲区) @param byte 输入字节 */
void FrameParser::handlePayloadReceiving(unsigned char byte)
{
    m_buffer.append(byte);
    int effectiveMax = qMin(m_maxFrameLength, kMaxFrameSize);

    // 路径A: 有长度字段 -> 精确总长接收
    if (m_def.lengthFieldOffset >= 0) {
        // 使用qint64防止大载荷时整数溢出
        qint64 expectedTotal64 = static_cast<qint64>(m_def.lengthFieldOffset)
                               + m_def.lengthFieldSize
                               + m_expectedPayload
                               + m_def.checksumSize
                               + m_def.lengthAdjust;
        int expectedTotal = (expectedTotal64 > effectiveMax)
                          ? effectiveMax + 1  // 强制触发溢出错误
                          : static_cast<int>(expectedTotal64);

        if (expectedTotal > effectiveMax) {
            m_errorCount++;
            ++m_totalParseErrors;  // 期望总长超出限制
            ++m_totalMalformedFrames;  ///< 统计: 期望总帧长超限视为畸形帧
            emit frameError(
                tr("期望总帧长度 (%1) 超过最大值 (%2)")
                    .arg(expectedTotal).arg(effectiveMax),
                m_buffer);
            resetIntermediateState();
            return;
        }

        if (expectedTotal > 0 && m_buffer.size() >= expectedTotal) {
            processCompletePayload();
        }
        return;
    }

    // 路径B: 无长度字段，有帧尾 -> 搜索帧尾
    if (!m_def.footer.isEmpty()) {
        if (m_buffer.size() > effectiveMax) {
            m_errorCount++;
            ++m_totalOverflows;
            ++m_totalParseErrors;  // 帧尾搜索溢出
            ++m_totalMalformedFrames;  ///< 统计: 帧尾搜索溢出视为畸形帧
            emit frameError(
                tr("帧缓冲区 (%1) 超过最大值 (%2)，搜索帧尾时溢出")
                    .arg(m_buffer.size()).arg(effectiveMax),
                m_buffer);
            resetIntermediateState();
            return;
        }

        if (m_buffer.size() >= m_def.footer.size()) {
            int footerStart = m_buffer.size() - m_def.footer.size();
            bool footerMatch = true;
            for (int i = 0; i < m_def.footer.size(); ++i) {
                if (static_cast<unsigned char>(m_buffer.at(footerStart + i)) !=
                    static_cast<unsigned char>(m_def.footer.at(i))) {
                    footerMatch = false;
                    break;
                }
            }
            if (footerMatch) {
                if (m_def.checksumType != ChecksumType::None && m_def.checksumOffset >= 0) {
                    m_state = State::ChecksumVerifying;
                } else {
                    completeFrame();
                }
            }
        }
        return;
    }

    // 路径C: 最简帧(无长度无帧尾) -> 依赖校验偏移
    if (m_def.checksumOffset >= 0) {
        if (m_buffer.size() >= m_def.checksumOffset + m_def.checksumSize) {
            m_state = State::ChecksumVerifying;
        }
        return;  // 有校验的帧走校验路径，不进入header-only逻辑
    }

    // 路径D: 纯header帧 — 检测下一个帧头出现作为当前帧结束标记
    if (m_def.header.size() > 0 && m_buffer.size() > static_cast<int>(m_def.header.size())
        && byte == static_cast<unsigned char>(m_def.header.at(0))) {
        m_buffer.chop(1);  // 回退属于下一帧的字节
        completeFrame();
        m_buffer.append(static_cast<char>(byte));
        m_headerMatchPos = 1;
        m_state = State::HeaderMatching;
        if (!m_frameTimer.isValid() && m_frameTimeoutMs > 0) m_frameTimer.start();
    }
}
