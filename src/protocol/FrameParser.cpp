/**
 * @file FrameParser.cpp
 * @brief 帧解析状态机实现 - 状态分发与各状态处理方法
 *
 * 状态机核心逻辑:
 *   processByte() 作为状态分发器，根据 m_state 调用对应的 handle* 方法。
 *   每个 handle* 方法负责单个状态的字节处理逻辑和状态转换。
 *
 * 辅助方法（校验计算、字段提取、Payload子路径）实现在 FrameParserHelpers.cpp 中。
 */

#include "FrameParser.h"
#include <QDebug>

// ============================================================================
// 构造 / 配置 / 状态查询
// ============================================================================

/** @brief 构造帧解析器，初始化状态为 Idle */
FrameParser::FrameParser(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置帧格式定义，设置后自动 reset()
 *
 * 如果 FrameDefinition::maxFrameLength() 返回有效正值，
 * 则使用该值作为帧长度上限（不超过 kMaxFrameSize）。
 */
void FrameParser::setDefinition(const FrameDefinition& def)
{
    m_def = def;
    reset();

    int defMaxLen = m_def.maxFrameLength();
    if (defMaxLen > 0 && defMaxLen <= kMaxFrameSize) {
        m_maxFrameLength = defMaxLen;
    }
}

/** @brief 获取当前帧格式定义 */
FrameDefinition FrameParser::definition() const
{
    return m_def;
}

/** @brief 喂入字节流数据，逐字节送入状态机处理。空数据直接忽略 */
void FrameParser::feed(const QByteArray& data)
{
    if (data.isEmpty()) {
        return;
    }

    for (int i = 0; i < data.size(); ++i) {
        processByte(static_cast<unsigned char>(data[i]));
    }
}

/**
 * @brief 重置解析器状态
 *
 * 清空缓冲区，回到 Idle 状态，重置帧头匹配进度和期望长度。
 * 不会清零 frameCount/errorCount 计数器。
 */
void FrameParser::reset()
{
    m_state = State::Idle;
    m_buffer.clear();
    m_headerMatchPos = 0;
    m_expectedPayload = 0;
    m_frameTimer.invalidate();
}

quint64 FrameParser::frameCount() const { return m_frameCount; }
quint64 FrameParser::errorCount() const { return m_errorCount; }

/**
 * @brief 设置帧长度上限，传入值 <= 0 或 > kMaxFrameSize 时使用 kMaxFrameSize
 * @param maxLen 最大允许帧长度（字节）
 */
void FrameParser::setMaxFrameLength(int maxLen)
{
    m_maxFrameLength = (maxLen <= 0 || maxLen > kMaxFrameSize) ? kMaxFrameSize : maxLen;
}

int FrameParser::maxFrameLength() const { return m_maxFrameLength; }

/** @brief 设置状态机超时阈值，0 表示禁用 */
void FrameParser::setFrameTimeout(int timeoutMs)
{
    m_frameTimeoutMs = (timeoutMs < 0) ? 0 : timeoutMs;
}

int FrameParser::frameTimeout() const { return m_frameTimeoutMs; }

// ============================================================================
// 状态机基础设施
// ============================================================================

/**
 * @brief 检查状态机是否超时
 * @return true=已超时需要重置，false=未超时或计时未启动
 *
 * 仅在非 Idle 状态下检查。超时后发射 frameError 信号并重置状态机。
 */
bool FrameParser::checkTimeout()
{
    if (m_frameTimeoutMs <= 0 || !m_frameTimer.isValid() || m_state == State::Idle) {
        return false;
    }

    if (m_frameTimer.hasExpired(m_frameTimeoutMs)) {
        QByteArray discarded = m_buffer;
        emit frameError(QString("Frame timeout: received %1 bytes in %2ms, incomplete frame discarded")
                            .arg(m_buffer.size()).arg(m_frameTimeoutMs),
                        discarded);
        m_errorCount++;
        reset();
        return true;
    }
    return false;
}

/**
 * @brief 帧完成后的通用处理
 *
 * 统一执行: 提取字段 -> 递增计数 -> 发射信号 -> 重置状态。
 * 所有成功解析路径最终都会调用此方法，避免重复代码。
 */
void FrameParser::completeFrame()
{
    QVariantMap fields = extractFields(m_buffer);
    m_frameCount++;
    emit frameParsed(fields, m_buffer);
    reset();
}

// ============================================================================
// 状态分发器
// ============================================================================

/**
 * @brief 处理单个字节 - 状态分发器
 * @param byte 待处理的字节值
 *
 * 处理流程:
 *   1. 检查超时 - 帧头匹配后超时未完成则自动重置
 *   2. 检查帧长度上限 - 缓冲区超过 maxFrameLength 时丢弃
 *   3. 根据当前状态分发到对应的 handle* 方法
 */
void FrameParser::processByte(unsigned char byte)
{
    // ---- 超时检查 ----
    if (checkTimeout()) {
        // 超时已重置状态，当前字节继续在 Idle 状态处理
    }

    // ---- 帧长度上限检查 ----
    int effectiveMax = qMin(m_maxFrameLength, kMaxFrameSize);
    if (m_buffer.size() >= effectiveMax) {
        QByteArray discarded = m_buffer;
        emit frameError(QString("Frame exceeds max length (%1 bytes), discarded %2 bytes")
                            .arg(effectiveMax).arg(m_buffer.size()),
                        discarded);
        m_errorCount++;
        reset();
        // 不 return：当前字节可能是新帧的起始字节
    }

    // ---- 状态分发 ----
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

/**
 * @brief 处理 Idle/HeaderMatching 状态 - 逐字节匹配帧头序列
 * @param byte 当前接收到的字节
 *
 * 帧头完全匹配后启动超时计时器，根据帧定义转入下一状态。
 * 匹配失败时重置，并检查当前字节是否是新帧头的起始字节。
 */
void FrameParser::handleHeaderMatching(unsigned char byte)
{
    m_buffer.append(byte);

    // 无帧头定义: 直接进入长度/数据接收
    if (m_def.header.isEmpty()) {
        m_state = (m_def.lengthFieldOffset >= 0) ? State::LengthReceiving : State::PayloadReceiving;
        if (!m_frameTimer.isValid() && m_frameTimeoutMs > 0) {
            m_frameTimer.start();
        }
        return;
    }

    // 帧头字节匹配
    if (byte == static_cast<unsigned char>(m_def.header.at(m_headerMatchPos))) {
        m_headerMatchPos++;

        if (m_headerMatchPos >= m_def.header.size()) {
            // 帧头完全匹配 - 启动计时器，决定下一状态
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
        // 匹配失败: 重置并检查当前字节是否为新帧头起始
        m_buffer.clear();
        m_headerMatchPos = 0;
        m_state = State::Idle;

        if (!m_def.header.isEmpty() && byte == static_cast<unsigned char>(m_def.header.at(0))) {
            m_buffer.append(byte);
            m_headerMatchPos = 1;
            m_state = State::HeaderMatching;
        }
    }
}

/**
 * @brief 处理 LengthReceiving 状态 - 接收并解析长度字段
 * @param byte 当前接收到的字节
 *
 * 长度字段完整后解析值并校验有效性，无效则报错重置。
 */
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
        emit frameError(QString("Invalid frame length: %1 (max allowed: %2)")
                            .arg(m_expectedPayload).arg(effectiveMax),
                        m_buffer);
        m_errorCount++;
        reset();
        return;
    }

    m_state = State::PayloadReceiving;
}

/**
 * @brief 处理 PayloadReceiving 状态 - 接收有效数据载荷
 * @param byte 当前接收到的字节
 *
 * 根据帧定义分三条路径:
 *   A. 有长度字段: 计算期望总长，收齐后进入校验
 *   B. 无长度有帧尾: 搜索帧尾匹配，匹配后进入校验
 *   C. 最简帧(无长度无帧尾): 依赖校验偏移或直接完成
 */
void FrameParser::handlePayloadReceiving(unsigned char byte)
{
    m_buffer.append(byte);

    int effectiveMax = qMin(m_maxFrameLength, kMaxFrameSize);

    // ---- 路径A: 有长度字段 → 精确总长接收 ----
    if (m_def.lengthFieldOffset >= 0) {
        int expectedTotal = m_def.lengthFieldOffset + m_def.lengthFieldSize
                            + m_expectedPayload + m_def.checksumSize;
        if (!m_def.footer.isEmpty()) expectedTotal += m_def.footer.size();
        expectedTotal += m_def.lengthAdjust;

        if (expectedTotal > effectiveMax) {
            emit frameError(QString("Expected total frame length (%1) exceeds max (%2)")
                                .arg(expectedTotal).arg(effectiveMax), m_buffer);
            m_errorCount++;
            reset();
            return;
        }

        if (expectedTotal > 0 && m_buffer.size() >= expectedTotal) {
            if (m_def.checksumType != ChecksumType::None && m_def.checksumOffset >= 0) {
                m_state = State::ChecksumVerifying;
            } else if (!m_def.footer.isEmpty()) {
                m_state = State::FooterMatching;
            } else {
                completeFrame();
            }
        }
        return;
    }

    // ---- 路径B: 无长度字段，有帧尾 → 搜索帧尾 ----
    if (!m_def.footer.isEmpty()) {
        if (m_buffer.size() > effectiveMax) {
            emit frameError(QString("Frame buffer (%1) exceeds max (%2) while searching for footer")
                                .arg(m_buffer.size()).arg(effectiveMax), m_buffer);
            m_errorCount++;
            reset();
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

    // ---- 路径C: 最简帧(无长度无帧尾) → 依赖校验偏移 ----
    if (m_def.checksumOffset >= 0) {
        if (m_buffer.size() >= m_def.checksumOffset + m_def.checksumSize) {
            m_state = State::ChecksumVerifying;
        }
    }
}

/**
 * @brief 处理 ChecksumVerifying 状态 - 接收并验证校验字段
 * @param byte 当前接收到的字节
 *
 * 校验通过后转入 FooterMatching 或完成帧，失败则报错重置。
 */
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
        emit frameError("Checksum mismatch", m_buffer);
        m_errorCount++;
        reset();
    }
}

/**
 * @brief 处理 FooterMatching 状态 - 匹配帧尾序列
 * @param byte 当前接收到的字节
 *
 * 帧尾匹配成功后验证校验（如果还没校验过），然后完成帧。
 */
void FrameParser::handleFooterMatching(unsigned char byte)
{
    m_buffer.append(byte);

    int expectedSize = m_def.checksumOffset + m_def.checksumSize + m_def.footer.size();
    if (m_buffer.size() < expectedSize) {
        return;
    }

    // 从缓冲区末尾提取帧尾区域进行比较
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
            emit frameError("Checksum mismatch", m_buffer);
            m_errorCount++;
            reset();
        }
    } else {
        emit frameError("Footer mismatch", m_buffer);
        m_errorCount++;
        reset();
    }
}
