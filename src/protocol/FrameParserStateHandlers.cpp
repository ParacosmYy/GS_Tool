/**
 * @file FrameParserStateHandlers.cpp
 * @brief 帧解析状态机 — 状态分发器与各状态处理方法
 *
 * 从 FrameParser.cpp 中拆分出的状态机核心逻辑，职责:
 *   1. processByte           — 逐字节状态分发器
 *   2. handleHeaderMatching  — 帧头匹配阶段（含回溯重试）
 *   3. handleLengthReceiving — 长度字段接收与解析
 *   4. handlePayloadReceiving — 载荷接收（长度/帧尾/校验三条路径）
 *   5. processCompletePayload — 长度字段模式下的帧完成处理
 *   6. handleCrcValidation   — CRC校验验证
 *   7. handleChecksumVerifying — 校验和验证阶段
 *   8. handleFooterMatching  — 帧尾匹配阶段
 *
 * 这些方法都是 FrameParser 类的 private 方法，
 * 构成状态机的完整状态流转逻辑，与构造/配置/超时机制解耦。
 */

#include "protocol/FrameParser.h"

// ============================================================================
// 状态分发器
// ============================================================================

/** @brief 逐字节状态机核心：根据当前状态分发到对应处理函数 @param byte 输入字节 */
void FrameParser::processByte(unsigned char byte)
{
    // 超时检查
    if (checkTimeout()) {
        // 超时已重置状态，当前字节继续在 Idle 状态处理
    }

    // 帧长度上限检查
    int effectiveMax = qMin(m_maxFrameLength, kMaxFrameSize);
    if (m_buffer.size() >= effectiveMax) {
        QByteArray discarded = m_buffer;
        m_errorCount++;
        resetIntermediateState();

        emit frameError(
            tr("帧超过最大长度 (%1 字节)，已丢弃 %2 字节")
                .arg(effectiveMax).arg(discarded.size()),
            discarded);
    }

    // 状态分发
    switch (m_state) {
    case State::Idle:
    case State::HeaderMatching:
        handleHeaderMatching(byte);
        break;
    case State::LengthReceiving:
        handleLengthReceiving(byte);
        break;
    case State::PayloadReceiving:
        handlePayloadReceiving(byte);
        break;
    case State::ChecksumVerifying:
        handleChecksumVerifying(byte);
        break;
    case State::FooterMatching:
        handleFooterMatching(byte);
        break;
    }
}

// ============================================================================
// 各状态处理方法
// ============================================================================

/** @brief 状态机:帧头匹配阶段(逐字节比较帧头序列) @param byte 输入字节 */
void FrameParser::handleHeaderMatching(unsigned char byte)
{
    m_buffer.append(byte);

    // 无帧头定义: 直接进入长度/数据接收
    if (m_def.header.isEmpty()) {
        m_state = (m_def.lengthFieldOffset >= 0)
            ? State::LengthReceiving : State::PayloadReceiving;
        if (!m_frameTimer.isValid() && m_frameTimeoutMs > 0) {
            m_frameTimer.start();
        }
        return;
    }

    if (byte == static_cast<unsigned char>(m_def.header.at(m_headerMatchPos))) {
        m_headerMatchPos++;

        if (m_headerMatchPos >= m_def.header.size()) {
            m_headerMatchPos = 0;
            if (m_frameTimeoutMs > 0 && !m_frameTimer.isValid()) {
                m_frameTimer.start();
            }
            if (m_def.lengthFieldOffset >= 0) {
                m_state = State::LengthReceiving;
            } else if (m_def.checksumOffset >= 0) {
                m_state = State::PayloadReceiving;
                m_expectedPayload = m_def.checksumOffset - m_buffer.size();
            } else if (!m_def.footer.isEmpty()) {
                m_state = State::PayloadReceiving;
                m_expectedPayload = 0;
            } else {
                m_state = State::PayloadReceiving;
            }
        }
    } else {
        // 匹配失败: 回溯检查缓冲区尾部是否有与帧头前缀的重叠
        // 处理 [AA,AA,55] 在流 [AA,AA,AA,55] 中的情况:
        // 原始匹配在位置2失败后，回溯到缓冲区[AA]处重新开始
        int bestOverlap = 0;
        for (int len = qMin(m_buffer.size() - 1, m_def.header.size() - 1); len >= 1; --len) {
            bool ok = true;
            for (int j = 0; j < len && ok; ++j) {
                if (static_cast<unsigned char>(m_buffer.at(m_buffer.size() - len + j))
                    != static_cast<unsigned char>(m_def.header.at(j))) {
                    ok = false;
                }
            }
            if (ok) { bestOverlap = len; break; }
        }
        if (bestOverlap > 0) {
            m_buffer = m_buffer.right(bestOverlap);
            m_headerMatchPos = bestOverlap;
            m_state = State::HeaderMatching;
        } else {
            m_buffer.clear();
            m_headerMatchPos = 0;
            m_state = State::Idle;
        }
        // 当前字节可能是新帧头的起始
        if (m_state == State::Idle && !m_def.header.isEmpty()
            && byte == static_cast<unsigned char>(m_def.header.at(0))) {
            m_buffer.append(byte);
            m_headerMatchPos = 1;
            m_state = State::HeaderMatching;
        }
    }
}

/** @brief 状态机:长度字段接收阶段(组装多字节长度值) @param byte 输入字节 */
void FrameParser::handleLengthReceiving(unsigned char byte)
{
    m_buffer.append(byte);

    int lengthEnd = m_def.lengthFieldOffset + m_def.lengthFieldSize;
    if (m_buffer.size() < lengthEnd) {
        return;
    }

    int effectiveMax = qMin(m_maxFrameLength, kMaxFrameSize);
    m_expectedPayload = parseLengthField(m_buffer);

    if (m_expectedPayload < 0 || m_expectedPayload > effectiveMax) {
        m_errorCount++;
        emit frameError(
            tr("无效帧长度: %1 (最大允许: %2)")
                .arg(m_expectedPayload).arg(effectiveMax),
            m_buffer);
        resetIntermediateState();
        return;
    }

    m_state = State::PayloadReceiving;
}

/** @brief 状态机:载荷接收阶段(逐字节填充payload缓冲区) @param byte 输入字节 */
void FrameParser::handlePayloadReceiving(unsigned char byte)
{
    m_buffer.append(byte);
    int effectiveMax = qMin(m_maxFrameLength, kMaxFrameSize);

    // 路径A: 有长度字段 -> 精确总长接收
    if (m_def.lengthFieldOffset >= 0) {
        int expectedTotal = m_def.lengthFieldOffset + m_def.lengthFieldSize
                            + m_expectedPayload + m_def.checksumSize
                            + m_def.lengthAdjust;

        if (expectedTotal > effectiveMax) {
            m_errorCount++;
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

/** @brief 长度字段模式下帧接收完成后的处理(校验+帧尾判断) */
void FrameParser::processCompletePayload()
{
    if (!handleCrcValidation()) return;
    if (!m_def.footer.isEmpty()) {
        m_state = State::FooterMatching;
    } else {
        completeFrame();
    }
}

/** @brief CRC校验验证，通过返回true，失败则emit错误并重置 */
bool FrameParser::handleCrcValidation()
{
    if (m_def.checksumType != ChecksumType::None && m_def.checksumOffset >= 0) {
        if (!verifyChecksum(m_buffer)) {
            m_errorCount++;
            emit frameError(tr("校验和不匹配"), m_buffer);
            resetIntermediateState();
            return false;
        }
    }
    return true;
}

/** @brief 状态机:校验和验证阶段(累加字节计算校验和) @param byte 输入字节 */
void FrameParser::handleChecksumVerifying(unsigned char byte)
{
    m_buffer.append(byte);

    if (m_def.checksumOffset < 0) {
        return;
    }
    if (m_buffer.size() < m_def.checksumOffset + m_def.checksumSize) {
        return;
    }

    if (verifyChecksum(m_buffer)) {
        if (!m_def.footer.isEmpty()) {
            m_state = State::FooterMatching;
        } else {
            completeFrame();
        }
    } else {
        m_errorCount++;
        emit frameError(tr("校验和不匹配"), m_buffer);
        resetIntermediateState();
    }
}

/** @brief 状态机:帧尾匹配阶段(逐字节比较帧尾序列) @param byte 输入字节 */
void FrameParser::handleFooterMatching(unsigned char byte)
{
    m_buffer.append(byte);

    int expectedSize = m_def.checksumOffset + m_def.checksumSize + m_def.footer.size();
    if (m_buffer.size() < expectedSize) {
        return;
    }

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
        if (m_def.checksumType == ChecksumType::None || verifyChecksum(m_buffer)) {
            completeFrame();
        } else {
            m_errorCount++;
            emit frameError(tr("校验和不匹配"), m_buffer);
            resetIntermediateState();
        }
    } else {
        m_errorCount++;
        emit frameError(tr("帧尾不匹配"), m_buffer);
        resetIntermediateState();
    }
}
