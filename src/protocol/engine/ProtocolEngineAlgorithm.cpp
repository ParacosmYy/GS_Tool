/**
 * @file ProtocolEngineAlgorithm.cpp
 * @brief ProtocolEngine 校验算法配置与解析实现
 *
 * 从 ProtocolEngine.cpp 拆分而来，包含校验算法的运行时配置、
 * 字符串/枚举互转、算法解析以及独立的校验验证接口。
 *
 * 支持的算法: CRC-8, CRC-16-CCITT, CRC-16-Modbus, CRC-32, XOR, Sum, None, Auto
 */

#include "protocol/engine/ProtocolEngine.h"
#include "protocol/schema/ProtocolSchema.h"

/* ============================================================================
 * 校验算法配置接口
 * ============================================================================ */

/** @brief 设置校验算法(覆盖schema定义) @param algo 算法名称字符串，如 "crc16_modbus" */
void ProtocolEngine::setChecksumAlgorithm(const QString &algo)
{
    ChecksumAlgorithm newAlgo = checksumAlgorithmFromString(algo);
    if (newAlgo == m_checksumAlgorithm) {
        return;
    }
    m_checksumAlgorithm = newAlgo;
    QString name = checksumAlgorithmToString(m_checksumAlgorithm);
    emit checksumAlgorithmChanged(name);
}

/** @brief 获取当前配置的校验算法名称 @return 算法名称字符串 */
QString ProtocolEngine::checksumAlgorithm() const
{
    return checksumAlgorithmToString(m_checksumAlgorithm);
}

/** @brief 获取当前生效的校验算法枚举(考虑Auto回退和schema设置) @return ChecksumAlgorithm 枚举值 */
ProtocolEngine::ChecksumAlgorithm ProtocolEngine::activeChecksumAlgorithm() const
{
    if (m_checksumAlgorithm != ChecksumAlgorithm::Auto) {
        return m_checksumAlgorithm;
    }
    /* Auto 模式：从schema中获取 */
    if (m_schema && m_schema->isValid()) {
        auto framing = m_schema->framing();
        switch (framing.checksumType) {
        case ProtocolSchema::ChecksumType::Crc8:       return ChecksumAlgorithm::Crc8;
        case ProtocolSchema::ChecksumType::Crc16Ccitt: return ChecksumAlgorithm::Crc16Ccitt;
        case ProtocolSchema::ChecksumType::Crc16Modbus:return ChecksumAlgorithm::Crc16Modbus;
        case ProtocolSchema::ChecksumType::Crc32:      return ChecksumAlgorithm::Crc32;
        case ProtocolSchema::ChecksumType::Xor:        return ChecksumAlgorithm::Xor;
        case ProtocolSchema::ChecksumType::Sum:        return ChecksumAlgorithm::Sum;
        case ProtocolSchema::ChecksumType::None:       return ChecksumAlgorithm::None;
        default: break;
        }
    }
    return ChecksumAlgorithm::None;
}

/** @brief 独立校验接口: 验证给定数据的校验和 @param data 完整数据(payload+checksum) @return 校验通过返回true */
bool ProtocolEngine::verifyChecksum(const QByteArray &data) const
{
    if (data.isEmpty()) {
        return false;
    }

    ChecksumAlgorithm algo = activeChecksumAlgorithm();
    if (algo == ChecksumAlgorithm::Auto || algo == ChecksumAlgorithm::None) {
        /* 无有效算法时，默认使用 CRC-16 Modbus 进行校验 */
        algo = ChecksumAlgorithm::Crc16Modbus;
    }

    int csSize = checksumSize(algo);
    if (csSize <= 0 || data.size() < csSize) {
        return false;
    }

    QByteArray payload = data.left(data.size() - csSize);
    quint64 computed = computeChecksumForAlgorithm(payload, algo);

    /* 从数据尾部提取期望的校验值(小端序) */
    QByteArray csBytes = data.right(csSize);
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
    return (computed & mask) == (expected & mask);
}

/* ============================================================================
 * 校验算法解析辅助方法
 * ============================================================================ */

/** @brief 确定当前生效的校验算法(手动覆盖优先，否则从schema获取) @param framing 帧定界规则 @return 实际生效的 ChecksumAlgorithm */
ProtocolEngine::ChecksumAlgorithm ProtocolEngine::resolveEffectiveAlgorithm(
    const ProtocolSchema::FramingRule &framing) const
{
    /* 手动覆盖优先 */
    if (m_checksumAlgorithm != ChecksumAlgorithm::Auto) {
        return m_checksumAlgorithm;
    }
    /* 从 schema 的 framing 规则中获取 */
    switch (framing.checksumType) {
    case ProtocolSchema::ChecksumType::Crc8:       return ChecksumAlgorithm::Crc8;
    case ProtocolSchema::ChecksumType::Crc16Ccitt: return ChecksumAlgorithm::Crc16Ccitt;
    case ProtocolSchema::ChecksumType::Crc16Modbus:return ChecksumAlgorithm::Crc16Modbus;
    case ProtocolSchema::ChecksumType::Crc32:      return ChecksumAlgorithm::Crc32;
    case ProtocolSchema::ChecksumType::Xor:        return ChecksumAlgorithm::Xor;
    case ProtocolSchema::ChecksumType::Sum:        return ChecksumAlgorithm::Sum;
    case ProtocolSchema::ChecksumType::None:       return ChecksumAlgorithm::None;
    default: return ChecksumAlgorithm::None;
    }
}

/** @brief 将校验算法枚举转换为字符串标识 @param algo 校验算法枚举 @return 字符串标识 */
QString ProtocolEngine::checksumAlgorithmToString(ChecksumAlgorithm algo)
{
    switch (algo) {
    case ChecksumAlgorithm::Auto:       return QStringLiteral("auto");
    case ChecksumAlgorithm::Crc8:       return QStringLiteral("crc8");
    case ChecksumAlgorithm::Crc16Modbus:return QStringLiteral("crc16_modbus");
    case ChecksumAlgorithm::Crc16Ccitt: return QStringLiteral("crc16_ccitt");
    case ChecksumAlgorithm::Crc32:      return QStringLiteral("crc32");
    case ChecksumAlgorithm::Xor:        return QStringLiteral("xor");
    case ChecksumAlgorithm::Sum:        return QStringLiteral("sum");
    case ChecksumAlgorithm::None:       return QStringLiteral("none");
    default:                            return QStringLiteral("auto");
    }
}

/** @brief 将字符串标识转换为校验算法枚举(支持多种分隔符格式) @param str 算法字符串 @return 对应的枚举值，不匹配时返回Auto */
ProtocolEngine::ChecksumAlgorithm ProtocolEngine::checksumAlgorithmFromString(const QString &str)
{
    QString normalized = str.toLower().trimmed();
    if (normalized == QLatin1String("crc8"))            return ChecksumAlgorithm::Crc8;
    if (normalized == QLatin1String("crc16_modbus"))    return ChecksumAlgorithm::Crc16Modbus;
    if (normalized == QLatin1String("crc16-modbus"))    return ChecksumAlgorithm::Crc16Modbus;
    if (normalized == QLatin1String("crc16modbus"))     return ChecksumAlgorithm::Crc16Modbus;
    if (normalized == QLatin1String("crc16_ccitt"))     return ChecksumAlgorithm::Crc16Ccitt;
    if (normalized == QLatin1String("crc16-ccitt"))     return ChecksumAlgorithm::Crc16Ccitt;
    if (normalized == QLatin1String("crc16ccitt"))      return ChecksumAlgorithm::Crc16Ccitt;
    if (normalized == QLatin1String("crc32"))           return ChecksumAlgorithm::Crc32;
    if (normalized == QLatin1String("xor"))             return ChecksumAlgorithm::Xor;
    if (normalized == QLatin1String("sum"))             return ChecksumAlgorithm::Sum;
    if (normalized == QLatin1String("none"))            return ChecksumAlgorithm::None;
    /* 不匹配时默认为Auto */
    return ChecksumAlgorithm::Auto;
}

/** @brief 获取校验算法输出的字节宽度 @param algo 校验算法枚举 @return 字节宽度(0/1/2/4) */
int ProtocolEngine::checksumSize(ChecksumAlgorithm algo)
{
    switch (algo) {
    case ChecksumAlgorithm::Crc8:       return 1;
    case ChecksumAlgorithm::Xor:        return 1;
    case ChecksumAlgorithm::Sum:        return 1;
    case ChecksumAlgorithm::Crc16Modbus:return 2;
    case ChecksumAlgorithm::Crc16Ccitt: return 2;
    case ChecksumAlgorithm::Crc32:      return 4;
    case ChecksumAlgorithm::None:       return 0;
    default:                            return 0;
    }
}

/** @brief 使用指定算法计算校验值(统一入口) @param data 待计算数据(不含校验字段) @param algo 校验算法 @return 校验值(封装为quint64) */
quint64 ProtocolEngine::computeChecksumForAlgorithm(const QByteArray &data, ChecksumAlgorithm algo)
{
    switch (algo) {
    case ChecksumAlgorithm::Crc8:       return static_cast<quint64>(computeCrc8(data));
    case ChecksumAlgorithm::Crc16Ccitt: return static_cast<quint64>(computeCrc16Ccitt(data));
    case ChecksumAlgorithm::Crc16Modbus:return static_cast<quint64>(computeCrc16Modbus(data));
    case ChecksumAlgorithm::Crc32:      return static_cast<quint64>(computeCrc32(data));
    case ChecksumAlgorithm::Xor:        return static_cast<quint64>(computeXor(data));
    case ChecksumAlgorithm::Sum:        return static_cast<quint64>(computeSum(data));
    default:                            return 0;
    }
}
