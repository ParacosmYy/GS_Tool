/**
 * @file ProtocolEngineChecksum.cpp
 * @brief ProtocolEngine 校验和计算与验证实现
 *
 * 从 ProtocolEngine.cpp 拆分而来，包含帧校验和验证逻辑
 * 以及 CRC-8/CRC-16-CCITT/CRC-16-Modbus/CRC-32/XOR 校验算法。
 */

#include "protocol/engine/ProtocolEngine.h"
#include "protocol/schema/ProtocolSchema.h"

/** @brief 验证帧的校验和/CRC @param frame 完整帧数据(含校验字段) @param framing 帧格式定义 @return true=校验通过，false=校验失败 */
bool ProtocolEngine::validateChecksum(const QByteArray &frame,
                                       const ProtocolSchema::FramingRule &framing) const
{
    if (frame.isEmpty()) { return false; }

    /* 校验字段大小: CRC8/XOR=1, CRC16=2, CRC32=4 */
    int checksumSize = 0;
    switch (framing.checksumType) {
    case ProtocolSchema::ChecksumType::None:       return true;
    case ProtocolSchema::ChecksumType::Crc8:       checksumSize = 1; break;
    case ProtocolSchema::ChecksumType::Xor:        checksumSize = 1; break;
    case ProtocolSchema::ChecksumType::Crc16Ccitt: checksumSize = 2; break;
    case ProtocolSchema::ChecksumType::Crc16Modbus: checksumSize = 2; break;
    case ProtocolSchema::ChecksumType::Crc32:      checksumSize = 4; break;
    default: return true;
    }

    /* 帧必须至少包含校验字段 */
    if (frame.size() < checksumSize) { return false; }

    /* 分离: 数据部分(不含校验) 和 校验字段 */
    QByteArray payload = frame.left(frame.size() - checksumSize);
    QByteArray checksumBytes = frame.right(checksumSize);

    switch (framing.checksumType) {
    case ProtocolSchema::ChecksumType::Crc8: {
        quint8 expected = static_cast<quint8>(checksumBytes.at(0));
        return computeCrc8(payload) == expected;
    }
    case ProtocolSchema::ChecksumType::Xor: {
        quint8 expected = static_cast<quint8>(checksumBytes.at(0));
        return computeXor(payload) == expected;
    }
    case ProtocolSchema::ChecksumType::Crc16Ccitt: {
        quint16 expected = static_cast<quint16>(
            (static_cast<quint8>(checksumBytes.at(0))) |
            (static_cast<quint16>(static_cast<quint8>(checksumBytes.at(1))) << 8));
        return computeCrc16Ccitt(payload) == expected;
    }
    case ProtocolSchema::ChecksumType::Crc16Modbus: {
        quint16 expected = static_cast<quint16>(
            (static_cast<quint8>(checksumBytes.at(0))) |
            (static_cast<quint16>(static_cast<quint8>(checksumBytes.at(1))) << 8));
        return computeCrc16Modbus(payload) == expected;
    }
    case ProtocolSchema::ChecksumType::Crc32: {
        quint32 expected = 0;
        for (int i = 0; i < 4; ++i) {
            expected |= static_cast<quint32>(static_cast<quint8>(checksumBytes.at(i))) << (8 * i);
        }
        return computeCrc32(payload) == expected;
    }
    default: return true;
    }
}

/** @brief 计算CRC-8校验值(多项式0x07) @param data 待校验数据 @param polynomial CRC多项式 @return CRC-8校验值 */
quint8 ProtocolEngine::computeCrc8(const QByteArray &data, quint8 polynomial)
{
    quint8 crc = 0x00;
    for (int i = 0; i < data.size(); ++i) {
        crc ^= static_cast<quint8>(data.at(i));
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ polynomial;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

/** @brief 计算CRC-16 CCITT校验值(多项式0x1021) @param data 待校验数据 @return CRC-16校验值 */
quint16 ProtocolEngine::computeCrc16Ccitt(const QByteArray &data)
{
    quint16 crc = 0xFFFF;
    for (int i = 0; i < data.size(); ++i) {
        crc ^= (static_cast<quint16>(static_cast<quint8>(data.at(i))) << 8);
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

/** @brief 计算CRC-16 Modbus校验值(多项式0x8005，小端序) @param data 待校验数据 @return CRC-16校验值 */
quint16 ProtocolEngine::computeCrc16Modbus(const QByteArray &data)
{
    quint16 crc = 0xFFFF;
    for (int i = 0; i < data.size(); ++i) {
        crc ^= static_cast<quint8>(data.at(i));
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

/** @brief 计算CRC-32校验值(多项式0xEDB88320，兼容ZIP/PNG) @param data 待校验数据 @return CRC-32校验值 */
quint32 ProtocolEngine::computeCrc32(const QByteArray &data)
{
    quint32 crc = 0xFFFFFFFF;
    for (int i = 0; i < data.size(); ++i) {
        crc ^= static_cast<quint8>(data.at(i));
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x00000001) {
                crc = (crc >> 1) ^ 0xEDB88320;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc ^ 0xFFFFFFFF;
}

/** @brief 计算异或校验值(所有字节逐个XOR) @param data 待校验数据 @return 异或校验结果 */
quint8 ProtocolEngine::computeXor(const QByteArray &data)
{
    quint8 result = 0x00;
    for (int i = 0; i < data.size(); ++i) {
        result ^= static_cast<quint8>(data.at(i));
    }
    return result;
}
