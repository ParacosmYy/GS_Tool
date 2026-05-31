#include "FrameParser.h"
#include "utils/HexConverter.h"
#include <QDebug>

FrameParser::FrameParser(QObject* parent)
    : QObject(parent)
{
}

void FrameParser::setDefinition(const FrameDefinition& def)
{
    m_def = def;
    reset();
}

FrameDefinition FrameParser::definition() const
{
    return m_def;
}

void FrameParser::feed(const QByteArray& data)
{
    for (int i = 0; i < data.size(); ++i) {
        processByte(static_cast<unsigned char>(data[i]));
    }
}

void FrameParser::reset()
{
    m_state = State::Idle;
    m_buffer.clear();
    m_headerMatchPos = 0;
    m_expectedPayload = 0;
}

quint64 FrameParser::frameCount() const
{
    return m_frameCount;
}

quint64 FrameParser::errorCount() const
{
    return m_errorCount;
}

void FrameParser::processByte(unsigned char byte)
{
    // 缓冲区溢出保护
    if (m_buffer.size() >= kMaxFrameSize) {
        m_state = State::Idle;
        m_buffer.clear();
        m_headerMatchPos = 0;
        emit frameError("Frame exceeds max size, discarded", QByteArray());
    }

    switch (m_state) {
    case State::Idle:
    case State::HeaderMatching: {
        // 逐字节匹配帧头
        m_buffer.append(byte);

        if (m_def.header.isEmpty()) {
            // 无帧头定义，直接进入长度/数据接收
            m_state = (m_def.lengthFieldOffset >= 0) ? State::LengthReceiving : State::PayloadReceiving;
            break;
        }

        if (byte == static_cast<unsigned char>(m_def.header.at(m_headerMatchPos))) {
            m_headerMatchPos++;
            if (m_headerMatchPos >= m_def.header.size()) {
                // 帧头匹配完成
                m_headerMatchPos = 0;
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
                    // 需要外部定义payload长度，暂不处理
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
            if (m_expectedPayload < 0 || m_expectedPayload > kMaxFrameSize) {
                emit frameError("Invalid frame length", m_buffer);
                m_errorCount++;
                reset();
                return;
            }
            // 总帧长 = 当前已接收 + 有效数据 + 校验 + 帧尾
            // 计算还需要接收多少字节
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
        } else if (!m_def.footer.isEmpty()) {
            // 无长度字段，有帧尾: 检查是否匹配帧尾
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
