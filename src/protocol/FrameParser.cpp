/**
 * @file FrameParser.cpp
 * @brief 帧解析状态机实现
 *
 * 实现逐字节状态机解析，包含以下安全机制:
 *   1. 帧长度上限检查 (maxFrameLength) - 防止超长帧导致内存爆炸
 *   2. 状态机超时机制 - 帧头匹配后超时自动重置
 *   3. 硬性上限 kMaxFrameSize - 最终安全阀
 */

#include "FrameParser.h"
#include "utils/HexConverter.h"
#include <QDebug>
#include <QDateTime>

/**
 * @brief 构造帧解析器
 * @param parent 父对象（通常为 ProtocolBridgeManager）
 *
 * 初始化状态为 Idle，帧计时器未启动。
 * 默认帧长度上限 = kMaxFrameSize (4096)，超时 = 500ms
 */
FrameParser::FrameParser(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置帧格式定义
 * @param def 帧结构定义
 *
 * 设置后会自动 reset()。如果 FrameDefinition::maxFrameLength() 返回有效正值，
 * 则使用该值作为帧长度上限（不超过 kMaxFrameSize）。
 */
void FrameParser::setDefinition(const FrameDefinition& def)
{
    m_def = def;
    reset();

    // 如果帧定义中计算出了最大帧长度，使用它作为上限
    int defMaxLen = m_def.maxFrameLength();
    if (defMaxLen > 0 && defMaxLen <= kMaxFrameSize) {
        m_maxFrameLength = defMaxLen;
    }
}

/**
 * @brief 获取当前帧格式定义
 * @return 帧结构定义的副本
 */
FrameDefinition FrameParser::definition() const
{
    return m_def;
}

/**
 * @brief 喂入字节流数据
 * @param data 原始字节数据（来自串口/TCP等数据源）
 *
 * 逐字节送入状态机处理。空数据直接忽略不做任何处理。
 * 每个字节处理前会检查缓冲区长度上限和状态机超时。
 */
void FrameParser::feed(const QByteArray& data)
{
    // 空数据保护：直接忽略，不触发任何状态变化
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
 * 停止帧计时器。不会清零 frameCount/errorCount 计数器。
 */
void FrameParser::reset()
{
    m_state = State::Idle;
    m_buffer.clear();
    m_headerMatchPos = 0;
    m_expectedPayload = 0;
    m_frameTimer.invalidate();
}

/**
 * @brief 获取已解析的帧计数
 * @return 成功解析的帧总数
 */
quint64 FrameParser::frameCount() const
{
    return m_frameCount;
}

/**
 * @brief 获取校验失败的帧计数
 * @return 错误帧总数
 */
quint64 FrameParser::errorCount() const
{
    return m_errorCount;
}

/**
 * @brief 设置帧长度上限
 * @param maxLen 最大允许帧长度（字节）
 *
 * 如果传入值 <= 0，使用硬性上限 kMaxFrameSize。
 * 如果传入值 > kMaxFrameSize，也被钳位到 kMaxFrameSize。
 */
void FrameParser::setMaxFrameLength(int maxLen)
{
    if (maxLen <= 0) {
        m_maxFrameLength = kMaxFrameSize;
    } else if (maxLen > kMaxFrameSize) {
        m_maxFrameLength = kMaxFrameSize;
    } else {
        m_maxFrameLength = maxLen;
    }
}

/**
 * @brief 获取当前帧长度上限
 * @return 最大允许帧长度（字节）
 */
int FrameParser::maxFrameLength() const
{
    return m_maxFrameLength;
}

/**
 * @brief 设置状态机超时阈值
 * @param timeoutMs 超时时间（毫秒），0 表示禁用超时机制
 */
void FrameParser::setFrameTimeout(int timeoutMs)
{
    m_frameTimeoutMs = (timeoutMs < 0) ? 0 : timeoutMs;
}

/**
 * @brief 获取当前超时阈值
 * @return 超时时间（毫秒），0 表示禁用
 */
int FrameParser::frameTimeout() const
{
    return m_frameTimeoutMs;
}

/**
 * @brief 检查状态机是否超时
 * @return true=已超时需要重置，false=未超时或计时未启动
 *
 * 仅在非 Idle 状态下检查。超时后发射 frameError 信号并重置状态机。
 * 超时阈值为 0 时不检查（禁用超时机制）。
 */
bool FrameParser::checkTimeout()
{
    // 超时机制禁用或计时器未启动时不检查
    if (m_frameTimeoutMs <= 0 || !m_frameTimer.isValid()) {
        return false;
    }

    // 仅在非 Idle 状态下检查超时
    if (m_state == State::Idle) {
        return false;
    }

    if (m_frameTimer.hasExpired(m_frameTimeoutMs)) {
        QByteArray discarded = m_buffer;
        emit frameError(QString("Frame timeout: received %1 bytes in %2ms, incomplete frame discarded")
                            .arg(m_buffer.size())
                            .arg(m_frameTimeoutMs),
                        discarded);
        m_errorCount++;
        reset();
        return true;
    }

    return false;
}

/**
 * @brief 处理单个字节
 * @param byte 待处理的字节值
 *
 * 处理流程:
 *   1. 检查超时 - 如果帧头匹配后超时未完成，自动重置
 *   2. 检查帧长度上限 - 缓冲区超过 maxFrameLength 时丢弃
 *   3. 根据当前状态执行对应的处理逻辑
 *
 * 帧头匹配成功时启动计时器，帧完成或重置时停止计时器。
 */
void FrameParser::processByte(unsigned char byte)
{
    // ---- 超时检查: 帧头匹配后超过一定时间未完成，自动重置 ----
    if (checkTimeout()) {
        // 超时已重置状态，当前字节需要重新处理（它是新帧的潜在起始字节）
        // 重新进入 processByte 而不是丢弃，因为该字节可能是新帧的帧头
        // 但为了避免无限递归，直接在 Idle 状态下处理
    }

    // ---- 帧长度上限检查: 防止超长帧导致内存爆炸 ----
    // 使用用户可配置的 maxFrameLength 和硬性上限 kMaxFrameSize 中的较小值
    int effectiveMax = qMin(m_maxFrameLength, kMaxFrameSize);
    if (m_buffer.size() >= effectiveMax) {
        QByteArray discarded = m_buffer;
        emit frameError(QString("Frame exceeds max length (%1 bytes), discarded %2 bytes")
                            .arg(effectiveMax)
                            .arg(m_buffer.size()),
                        discarded);
        m_errorCount++;
        reset();
        // 不 return：当前字节可能是新帧的起始字节，继续在 Idle 状态处理
    }

    switch (m_state) {
    case State::Idle:
    case State::HeaderMatching: {
        // ---- 帧头匹配阶段: 逐字节匹配帧头字节序列 ----
        m_buffer.append(byte);

        if (m_def.header.isEmpty()) {
            // 无帧头定义，直接进入长度/数据接收
            m_state = (m_def.lengthFieldOffset >= 0) ? State::LengthReceiving : State::PayloadReceiving;
            // 无帧头时立即启动计时器
            if (!m_frameTimer.isValid() && m_frameTimeoutMs > 0) {
                m_frameTimer.start();
            }
            break;
        }

        if (byte == static_cast<unsigned char>(m_def.header.at(m_headerMatchPos))) {
            m_headerMatchPos++;
            if (m_headerMatchPos >= m_def.header.size()) {
                // 帧头匹配完成 - 启动超时计时器
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
                    // 无长度字段时无法预知payload长度，需要帧尾来界定
                    m_expectedPayload = 0;
                } else {
                    // 最简帧: 只有帧头，无长度/校验/帧尾
                    m_state = State::PayloadReceiving;
                }
            }
            m_state = (m_headerMatchPos < m_def.header.size()) ? State::HeaderMatching : m_state;
            if (m_headerMatchPos == 0) {
                // 帧头已完全匹配，上面已经设置了下一个状态
            }
        } else {
            // 帧头匹配失败，重置
            m_buffer.clear();
            m_headerMatchPos = 0;
            m_state = State::Idle;

            // 检查当前字节是否是帧头第一个字节
            if (!m_def.header.isEmpty() && byte == static_cast<unsigned char>(m_def.header.at(0))) {
                m_buffer.append(byte);
                m_headerMatchPos = 1;
                m_state = State::HeaderMatching;
            }
        }
        break;
    }

    case State::LengthReceiving: {
        m_buffer.append(byte);

        // 检查是否已接收到完整的长度字段
        int lengthEnd = m_def.lengthFieldOffset + m_def.lengthFieldSize;
        if (m_buffer.size() >= lengthEnd) {
            m_expectedPayload = parseLengthField(m_buffer);
            // 长度值校验: 必须为非负且不超过帧长度上限
            if (m_expectedPayload < 0 || m_expectedPayload > effectiveMax) {
                emit frameError(QString("Invalid frame length: %1 (max allowed: %2)")
                                    .arg(m_expectedPayload)
                                    .arg(effectiveMax),
                                m_buffer);
                m_errorCount++;
                reset();
                return;
            }
            // 总帧长 = 当前已接收 + 有效数据 + 校验 + 帧尾
            m_state = State::PayloadReceiving;
        }
        break;
    }

    case State::PayloadReceiving: {
        m_buffer.append(byte);

        // 计算期望的总帧长度
        int expectedTotal = 0;
        if (m_def.lengthFieldOffset >= 0) {
            // 有长度字段: 总长 = header + lengthField + payload(=expectedPayload) + checksum + footer
            expectedTotal = m_def.lengthFieldOffset + m_def.lengthFieldSize + m_expectedPayload + m_def.checksumSize;
            if (!m_def.footer.isEmpty()) expectedTotal += m_def.footer.size();
            // 长度字段的值包含lengthAdjust
            expectedTotal += m_def.lengthAdjust;

            // 期望总长度上限检查
            if (expectedTotal > effectiveMax) {
                emit frameError(QString("Expected total frame length (%1) exceeds max (%2)")
                                    .arg(expectedTotal)
                                    .arg(effectiveMax),
                                m_buffer);
                m_errorCount++;
                reset();
                return;
            }
        } else if (!m_def.footer.isEmpty()) {
            // 无长度字段，有帧尾: 检查是否匹配帧尾
            // 帧尾搜索时也需要检查长度上限
            if (m_buffer.size() > effectiveMax) {
                emit frameError(QString("Frame buffer (%1) exceeds max (%2) while searching for footer")
                                    .arg(m_buffer.size())
                                    .arg(effectiveMax),
                                m_buffer);
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
                    // 帧尾匹配完成，进入校验
                    if (m_def.checksumType != ChecksumType::None && m_def.checksumOffset >= 0) {
                        m_state = State::ChecksumVerifying;
                    } else {
                        // 无校验，帧完成
                        QVariantMap fields = extractFields(m_buffer);
                        m_frameCount++;
                        emit frameParsed(fields, m_buffer);
                        reset();
                        return;
                    }
                }
            }
            break; // 继续接收
        }

        if (expectedTotal > 0 && m_buffer.size() >= expectedTotal) {
            // 已接收足够数据
            if (m_def.checksumType != ChecksumType::None && m_def.checksumOffset >= 0) {
                // 需要校验
                m_state = State::ChecksumVerifying;
                // 如果校验数据已在缓冲区内，直接处理
                if (m_buffer.size() >= expectedTotal) {
                    // 继续到ChecksumVerifying处理
                }
            } else {
                // 无校验，帧完成
                if (!m_def.footer.isEmpty()) {
                    m_state = State::FooterMatching;
                    break;
                }
                QVariantMap fields = extractFields(m_buffer);
                m_frameCount++;
                emit frameParsed(fields, m_buffer);
                reset();
                return;
            }
        }

        // 如果已有校验状态，继续处理
        if (m_state == State::ChecksumVerifying) {
            // 校验处理在下面
        }
        break;
    }

    case State::ChecksumVerifying: {
        m_buffer.append(byte);

        // 检查是否已接收到校验字段
        if (m_def.checksumOffset >= 0 && m_buffer.size() >= m_def.checksumOffset + m_def.checksumSize) {
            // 验证校验
            if (verifyChecksum(m_buffer)) {
                // 校验通过
                if (!m_def.footer.isEmpty()) {
                    m_state = State::FooterMatching;
                } else {
                    QVariantMap fields = extractFields(m_buffer);
                    m_frameCount++;
                    emit frameParsed(fields, m_buffer);
                    reset();
                }
            } else {
                emit frameError("Checksum mismatch", m_buffer);
                m_errorCount++;
                reset();
            }
        }
        break;
    }

    case State::FooterMatching: {
        m_buffer.append(byte);

        // 检查帧尾
        if (m_buffer.size() >=
            m_def.checksumOffset + m_def.checksumSize + m_def.footer.size()) {
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
                    QVariantMap fields = extractFields(m_buffer);
                    m_frameCount++;
                    emit frameParsed(fields, m_buffer);
                } else {
                    emit frameError("Checksum mismatch", m_buffer);
                    m_errorCount++;
                }
            } else {
                emit frameError("Footer mismatch", m_buffer);
                m_errorCount++;
            }
            reset();
        }
        break;
    }
    }

    // 处理校验状态的特殊情况（数据已在缓冲区内）
    if (m_state == State::ChecksumVerifying &&
        m_def.checksumOffset >= 0 &&
        m_buffer.size() >= m_def.checksumOffset + m_def.checksumSize &&
        !m_def.footer.isEmpty()) {
        // 校验完成但还有帧尾
        // 已在ChecksumVerifying case中处理
    }
}

/**
 * @brief 提取帧内各字段值
 * @param frameData 完整帧数据
 * @return 字段名→值的映射
 *
 * 提取 payload 区域后，逐字段调用 FieldDef::extractValue。
 * 元数据字段: _rawPayload, _rawFrame(HEX), _frameTime
 */
QVariantMap FrameParser::extractFields(const QByteArray& frameData) const
{
    QVariantMap result;
    // 提取payload区域（帧头之后，校验/帧尾之前）
    int payloadStart = m_def.header.size();
    if (m_def.lengthFieldOffset >= 0) {
        payloadStart = m_def.lengthFieldOffset + m_def.lengthFieldSize;
    }

    int payloadEnd = frameData.size();
    if (m_def.checksumOffset >= 0) {
        payloadEnd = m_def.checksumOffset;
    } else if (!m_def.footer.isEmpty()) {
        payloadEnd = frameData.size() - m_def.footer.size();
    }

    QByteArray payload = frameData.mid(payloadStart, payloadEnd - payloadStart);
    result["_rawPayload"] = payload;
    result["_rawFrame"] = HexConverter::toHexString(frameData);
    result["_frameTime"] = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");

    // 逐字段提取值
    for (const auto& field : m_def.fields) {
        QString key = field.name.isEmpty() ? QString("field_%1").arg(field.offset) : field.name;
        result[key] = field.formatValue(payload);
    }

    return result;
}

/**
 * @brief 计算并验证校验值
 * @param frameData 完整帧数据
 * @return true=校验通过，false=校验失败
 *
 * 从 checksumStart 到 checksumEnd 区域计算校验值，
 * 与帧内 checksumOffset 处的校验字段比较。
 */
bool FrameParser::verifyChecksum(const QByteArray& frameData) const
{
    if (m_def.checksumType == ChecksumType::None || m_def.checksumOffset < 0) {
        return true;
    }

    int start = m_def.checksumStart;
    int end = (m_def.checksumEnd < 0) ? m_def.checksumOffset : m_def.checksumEnd;

    if (start < 0 || end > frameData.size() || start >= end) {
        return false;
    }

    QByteArray checkRegion = frameData.mid(start, end - start);
    QByteArray computed = computeChecksum(checkRegion);

    // 从帧数据中提取校验字段
    QByteArray actual = frameData.mid(m_def.checksumOffset, m_def.checksumSize);

    return (computed == actual);
}

/**
 * @brief 解析长度字段值
 * @param frameData 包含长度字段的帧数据
 * @return 解析出的长度值，-1 表示解析失败
 *
 * 支持 1 字节和 2 字节长度字段，大小端由 lengthBigEndian 决定。
 * 返回值 = 原始长度值 - lengthAdjust
 */
int FrameParser::parseLengthField(const QByteArray& frameData) const
{
    if (m_def.lengthFieldOffset < 0) return -1;

    int offset = m_def.lengthFieldOffset;
    if (offset + m_def.lengthFieldSize > frameData.size()) return -1;

    int length = 0;
    if (m_def.lengthFieldSize == 1) {
        length = static_cast<unsigned char>(frameData.at(offset));
    } else if (m_def.lengthFieldSize == 2) {
        if (m_def.lengthBigEndian) {
            length = (static_cast<unsigned char>(frameData.at(offset)) << 8) |
                     static_cast<unsigned char>(frameData.at(offset + 1));
        } else {
            length = static_cast<unsigned char>(frameData.at(offset)) |
                     (static_cast<unsigned char>(frameData.at(offset + 1)) << 8);
        }
    }

    return length - m_def.lengthAdjust;
}

/**
 * @brief 计算校验值
 * @param data 需要计算校验的数据区域
 * @return 校验结果字节数组
 *
 * 根据 checksumType 选择对应的校验算法。
 */
QByteArray FrameParser::computeChecksum(const QByteArray& data) const
{
    QByteArray result;
    switch (m_def.checksumType) {
    case ChecksumType::None:
        break;
    case ChecksumType::Sum8:
        result.append(static_cast<char>(CRC::checksum(data)));
        break;
    case ChecksumType::CRC8:
        result.append(static_cast<char>(CRC::crc8(data)));
        break;
    case ChecksumType::CRC16CCITT: {
        uint16_t crc = CRC::crc16Ccitt(data);
        result.append(static_cast<char>(crc & 0xFF));
        result.append(static_cast<char>((crc >> 8) & 0xFF));
        break;
    }
    case ChecksumType::CRC16Modbus: {
        uint16_t crc = CRC::crc16Modbus(data);
        result.append(static_cast<char>(crc & 0xFF));
        result.append(static_cast<char>((crc >> 8) & 0xFF));
        break;
    }
    case ChecksumType::CRC32: {
        uint32_t crc = CRC::crc32(data);
        result.append(static_cast<char>(crc & 0xFF));
        result.append(static_cast<char>((crc >> 8) & 0xFF));
        result.append(static_cast<char>((crc >> 16) & 0xFF));
        result.append(static_cast<char>((crc >> 24) & 0xFF));
        break;
    }
    }
    return result;
}
