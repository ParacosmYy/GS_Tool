/**
 * @file ProtocolEngineChecksum.cpp
 * @brief ProtocolEngine 校验和计算与验证实现
 *
 * 从 ProtocolEngine.cpp 拆分而来，包含帧校验和验证逻辑
 * 以及 CRC-8/CRC-16-CCITT/CRC-16-Modbus/CRC-32/XOR/Sum 校验算法。
 *
 * 验证流程支持两种模式:
 * 1. Auto模式: 使用 schema 的 FramingRule 中定义的校验类型
 * 2. 手动覆盖: 通过 setChecksumAlgorithm() 设置后，强制使用指定算法
 */

#include "protocol/engine/ProtocolEngine.h"
#include "protocol/schema/ProtocolSchema.h"

/** @brief 验证帧的校验和/CRC(支持算法覆盖，可回填期望值与实际值) @param frame 完整帧数据(含校验字段) @param framing 帧格式定义 @param expectedVal 回填期望校验值(可nullptr) @param actualVal 回填实际校验值(可nullptr) @return true=校验通过，false=校验失败 */
bool ProtocolEngine::validateChecksum(const QByteArray &frame,
                                       const ProtocolSchema::FramingRule &framing,
                                       quint64 *expectedVal,
                                       quint64 *actualVal) const
{
    if (frame.isEmpty()) { return false; }

    /* 确定实际使用的校验算法 */
    ChecksumAlgorithm effectiveAlgo = resolveEffectiveAlgorithm(framing);
    if (effectiveAlgo == ChecksumAlgorithm::None) { return true; }

    /* 校验字段大小 */
    int csSize = checksumSize(effectiveAlgo);
    if (csSize <= 0) { return true; }

    /* 帧必须至少包含校验字段 */
    if (frame.size() < csSize) { return false; }

    /* 分离: 数据部分(不含校验) 和 校验字段 */
    QByteArray payload = frame.left(frame.size() - csSize);
    QByteArray csBytes = frame.right(csSize);

    /* 使用统一接口计算校验值 */
    quint64 computed = computeChecksumForAlgorithm(payload, effectiveAlgo);

    /* 从帧尾部提取期望的校验值(小端序) */
    quint64 expected = 0;
    for (int i = 0; i < csSize; ++i) {
        expected |= static_cast<quint64>(static_cast<quint8>(csBytes.at(i))) << (8 * i);
    }

    /* 比较时只取有效位宽 */
    quint64 mask = 0;
    switch (csSize) {
    case 1: mask = 0xFF; break;
    case 2: mask = 0xFFFF; break;
    case 4: mask = 0xFFFFFFFF; break;
    default: mask = 0xFFFFFFFFFFFFFFFF; break;
    }

    bool passed = (computed & mask) == (expected & mask);

    /* 回填期望值和实际值(用于信号上报) */
    if (expectedVal) { *expectedVal = expected & mask; }
    if (actualVal)   { *actualVal = computed & mask; }

    return passed;
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

/** @brief 计算累加和校验值(所有字节求和取低8位) @param data 待校验数据 @return 累加和低8位 */
quint8 ProtocolEngine::computeSum(const QByteArray &data)
{
    quint32 sum = 0;
    for (int i = 0; i < data.size(); ++i) {
        sum += static_cast<quint8>(data.at(i));
    }
    return static_cast<quint8>(sum & 0xFF);
}
