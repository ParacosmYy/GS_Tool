/**
 * @file FrameParserStats.cpp
 * @brief 帧解析状态机 — 校验验证与帧尾匹配处理方法
 *
 * 从 FrameParserStateHandlers.cpp 中拆分出的校验与帧尾逻辑，职责:
 *   1. processCompletePayload  — 长度字段模式下的帧完成处理
 *   2. handleCrcValidation     — CRC校验验证
 *   3. handleChecksumVerifying — 校验和验证阶段
 *   4. handleFooterMatching    — 帧尾匹配阶段
 *
 * 这些方法都是 FrameParser 类的 private 方法，
 * 负责帧数据完整性验证（校验和/CRC）与帧尾序列匹配，
 * 校验失败时递增错误计数器并重置状态机。
 */

#include "protocol/parser/FrameParser.h"

// ============================================================================
// 校验与帧尾处理方法
// ============================================================================

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

/** @brief CRC校验验证，通过返回true，失败则递增校验错误计数并重置 */
bool FrameParser::handleCrcValidation()
{
    if (m_def.checksumType != ChecksumType::None && m_def.checksumOffset >= 0) {
        if (!verifyChecksum(m_buffer)) {
            m_errorCount++;
            m_totalChecksumErrors++;
            m_totalValidationErrors++;
            ++m_totalParseErrors;  // CRC校验失败
            ++m_totalMalformedFrames;  ///< 统计: CRC校验失败视为畸形帧
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
        m_totalChecksumErrors++;
        m_totalValidationErrors++;
        ++m_totalParseErrors;  // 校验和不匹配
        ++m_totalMalformedFrames;  ///< 统计: 校验和验证失败视为畸形帧
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
        // CRC已在handleCrcValidation或handleChecksumVerifying中验证，无需重复校验
        completeFrame();
    } else {
        m_errorCount++;
        ++m_totalParseErrors;  // 帧尾不匹配
        ++m_totalMalformedFrames;  ///< 统计: 帧尾不匹配视为畸形帧
        emit frameError(tr("帧尾不匹配"), m_buffer);
        resetIntermediateState();
    }
}
