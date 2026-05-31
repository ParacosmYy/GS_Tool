/**
 * @file FrameParser.cpp
 * @brief 帧解析状态机实现 - 状态分发与各状态处理方法
 *
 * 容错性增强:
 *   - 默认帧长度上限 1024 字节（kDefaultMaxFrameLength）
 *   - 超时后 resetIntermediateState() 完整清除所有中间状态
 *   - 帧计数器用于诊断和统计
 *
 * 辅助方法实现在 FrameParserHelpers.cpp 中。
 */

#include "protocol/FrameParser.h"
#include <QDebug>

// ============================================================================
// 构造 / 配置 / 状态查询
// ============================================================================

FrameParser::FrameParser(QObject* parent)
    : QObject(parent)
    , m_timeoutCheckTimer(new QTimer(this))
{
    m_timeoutCheckTimer->setSingleShot(false);
    connect(m_timeoutCheckTimer, &QTimer::timeout,
            this, [this]() { checkTimeout(); });
}

FrameParser::~FrameParser()
{
    stopTimeoutTimer();
}

void FrameParser::setDefinition(const FrameDefinition& def)
{
    m_def = def;
    reset();

    int defMaxLen = m_def.maxFrameLength();
    if (defMaxLen > 0 && defMaxLen <= kMaxFrameSize) {
        m_maxFrameLength = defMaxLen;
    }

    startTimeoutTimer();
}

FrameDefinition FrameParser::definition() const
{
    return m_def;
}

void FrameParser::feed(const QByteArray& data)
{
    if (data.isEmpty()) {
        return;
    }

    for (int i = 0; i < data.size(); ++i) {
        processByte(static_cast<unsigned char>(data[i]));
    }
}

/** @brief 重置解析器状态（清空缓冲区，回到Idle），不清零计数器 */
void FrameParser::reset()
{
    m_state = State::Idle;
    m_buffer.clear();
    m_headerMatchPos = 0;
    m_expectedPayload = 0;
    m_footerMatchPos = 0;
    m_frameTimer.invalidate();
}

/**
 * @brief 完整重置所有中间缓冲区和匹配进度（超时恢复用）
 *
 * 比 reset() 更彻底，额外调用 squeeze() 释放内存，
 * 确保下一帧从完全干净的状态开始解析。
 */
void FrameParser::resetIntermediateState()
{
    m_state = State::Idle;
    m_buffer.clear();
    m_buffer.squeeze();
    m_headerMatchPos = 0;
    m_footerMatchPos = 0;
    m_expectedPayload = 0;
    m_frameTimer.invalidate();
}

quint64 FrameParser::frameCount() const { return m_frameCount; }
quint64 FrameParser::errorCount() const { return m_errorCount; }

void FrameParser::setMaxFrameLength(int maxLen)
{
    m_maxFrameLength = (maxLen <= 0 || maxLen > kMaxFrameSize) ? kMaxFrameSize : maxLen;
}

int FrameParser::maxFrameLength() const { return m_maxFrameLength; }

void FrameParser::setFrameTimeout(int timeoutMs)
{
    m_frameTimeoutMs = (timeoutMs < 0) ? 0 : timeoutMs;
    if (m_frameTimeoutMs > 0) {
        startTimeoutTimer();
    } else {
        stopTimeoutTimer();
    }
}

int FrameParser::frameTimeout() const { return m_frameTimeoutMs; }

// ============================================================================
// 状态机基础设施
// ============================================================================

/** @brief 检查是否超时，超时后 emit frameError 并完整重置 */
bool FrameParser::checkTimeout()
{
    if (m_frameTimeoutMs <= 0 || !m_frameTimer.isValid() || m_state == State::Idle) {
        return false;
    }

    if (m_frameTimer.hasExpired(m_frameTimeoutMs)) {
        QByteArray discarded = m_buffer;
        quint64 discardedSize = static_cast<quint64>(m_buffer.size());
        resetIntermediateState();
        m_errorCount++;

        emit frameError(
            tr("Frame timeout: received %1 bytes in %2ms, incomplete frame discarded")
                .arg(discardedSize).arg(m_frameTimeoutMs),
            discarded);

        stopTimeoutTimer();
        startTimeoutTimer();
        return true;
    }
    return false;
}

void FrameParser::stopTimeoutTimer()
{
    if (m_timeoutCheckTimer) {
        m_timeoutCheckTimer->stop();
    }
}

void FrameParser::startTimeoutTimer()
{
    if (m_timeoutCheckTimer && m_frameTimeoutMs > 0) {
        int interval = qBound(50, m_frameTimeoutMs / 2, 500);
        m_timeoutCheckTimer->start(interval);
    }
}

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
            tr("Frame exceeds max length (%1 bytes), discarded %2 bytes")
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
        // 匹配失败: 重置并检查当前字节是否为新帧头起始
        m_buffer.clear();
        m_headerMatchPos = 0;
        m_state = State::Idle;

        if (!m_def.header.isEmpty()
            && byte == static_cast<unsigned char>(m_def.header.at(0))) {
            m_buffer.append(byte);
            m_headerMatchPos = 1;
            m_state = State::HeaderMatching;
        }
    }
}

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
            tr("Invalid frame length: %1 (max allowed: %2)")
                .arg(m_expectedPayload).arg(effectiveMax),
            m_buffer);
        resetIntermediateState();
        return;
    }

    m_state = State::PayloadReceiving;
}

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
                tr("Expected total frame length (%1) exceeds max (%2)")
                    .arg(expectedTotal).arg(effectiveMax),
                m_buffer);
            resetIntermediateState();
            return;
        }

        if (expectedTotal > 0 && m_buffer.size() >= expectedTotal) {
            if (m_def.checksumType != ChecksumType::None && m_def.checksumOffset >= 0) {
                if (verifyChecksum(m_buffer)) {
                    if (!m_def.footer.isEmpty()) {
                        m_state = State::FooterMatching;
                    } else {
                        completeFrame();
                    }
                } else {
                    m_errorCount++;
                    emit frameError(tr("Checksum mismatch"), m_buffer);
                    resetIntermediateState();
                }
            } else if (!m_def.footer.isEmpty()) {
                m_state = State::FooterMatching;
            } else {
                completeFrame();
            }
        }
        return;
    }

    // 路径B: 无长度字段，有帧尾 -> 搜索帧尾
    if (!m_def.footer.isEmpty()) {
        if (m_buffer.size() > effectiveMax) {
            m_errorCount++;
            emit frameError(
                tr("Frame buffer (%1) exceeds max (%2) while searching for footer")
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
    }
}

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
        emit frameError(tr("Checksum mismatch"), m_buffer);
        resetIntermediateState();
    }
}

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
            emit frameError(tr("Checksum mismatch"), m_buffer);
            resetIntermediateState();
        }
    } else {
        m_errorCount++;
        emit frameError(tr("Footer mismatch"), m_buffer);
        resetIntermediateState();
    }
}
