/**
 * @file ProtocolSchemaFields.cpp
 * @brief 协议帧结构的字段访问、JSON 序列化与校验类型转换实现
 *
 * 本文件从 ProtocolSchema.cpp 拆分而来，包含:
 *   - 字段 getter（name/framing/fields/isValid/lastError）
 *   - JSON 序列化（toJson）
 *   - 校验类型枚举转换辅助（checksumTypeToString / checksumTypeFromString）
 *
 * 统计计数器接口已拆分至 ProtocolSchemaStats.cpp。
 */

#include "protocol/schema/ProtocolSchema.h"

#include <QJsonArray>
#include <QJsonObject>

// ============================================================================
// 字段 getter
// ============================================================================

/** @brief 获取协议名称 @return 协议名称字符串 */
QString ProtocolSchema::name() const
{
    return m_name;
}

/** @brief 获取帧定界规则 @return 当前帧定界规则 */
ProtocolSchema::FramingRule ProtocolSchema::framing() const
{
    return m_framing;
}

/** @brief 获取所有字段定义列表 @return 字段定义列表 */
QList<ProtocolSchema::FieldDefinition> ProtocolSchema::fields() const
{
    return m_fields;
}

/** @brief 检查当前协议定义是否有效 @return 有效返回true，否则返回false */
bool ProtocolSchema::isValid() const
{
    return m_valid;
}

/** @brief 获取最近一次解析错误描述 @return 错误描述字符串 */
QString ProtocolSchema::lastError() const
{
    return m_lastError;
}

// ============================================================================
// JSON 序列化
// ============================================================================

/** @brief 将当前协议定义序列化为JSON对象 @return 包含完整协议定义的QJsonObject */
QJsonObject ProtocolSchema::toJson() const
{
    ++m_totalSaves;  // 累计序列化保存计数

    QJsonObject root;

    /* 协议名称 */
    root[QStringLiteral("name")] = m_name;

    /* 帧定界规则 */
    QJsonObject framingObj;
    framingObj[QStringLiteral("type")] = m_framing.type;

    /* 帧头字节序列 */
    QJsonArray headerArr;
    for (int byte : m_framing.header) {
        headerArr.append(byte);
    }
    framingObj[QStringLiteral("header")] = headerArr;

    /* 长度字段 */
    QJsonObject lfObj;
    lfObj[QStringLiteral("offset")] = m_framing.lengthFieldOffset;
    lfObj[QStringLiteral("size")]   = m_framing.lengthFieldSize;
    framingObj[QStringLiteral("length_field")] = lfObj;

    /* 校验配置 */
    QJsonObject chkObj;
    chkObj[QStringLiteral("type")] = checksumTypeToString(m_framing.checksumType);
    framingObj[QStringLiteral("checksum")] = chkObj;

    root[QStringLiteral("framing")] = framingObj;

    /* 字段定义数组 */
    QJsonArray fieldsArr;
    for (const FieldDefinition &field : m_fields) {
        QJsonObject fieldObj;
        fieldObj[QStringLiteral("name")]   = field.name;
        fieldObj[QStringLiteral("offset")] = field.offset;
        fieldObj[QStringLiteral("size")]   = field.size;
        fieldObj[QStringLiteral("type")]   = field.type;
        fieldsArr.append(fieldObj);
    }
    root[QStringLiteral("fields")] = fieldsArr;

    return root;
}

// ============================================================================
// 校验类型枚举转换
// ============================================================================

/** @brief 将校验类型枚举值转换为字符串标识 @param type 校验算法枚举值 @return 对应的字符串标识 */
QString ProtocolSchema::checksumTypeToString(ChecksumType type) const
{
    switch (type) {
    case ChecksumType::None:        return QStringLiteral("none");
    case ChecksumType::Crc8:        return QStringLiteral("crc8");
    case ChecksumType::Crc16Ccitt:  return QStringLiteral("crc16_ccitt");
    case ChecksumType::Crc16Modbus: return QStringLiteral("crc16_modbus");
    case ChecksumType::Crc32:       return QStringLiteral("crc32");
    case ChecksumType::Xor:         return QStringLiteral("xor");
    case ChecksumType::Sum:         return QStringLiteral("sum");
    default:                        return QStringLiteral("none");
    }
}

/** @brief 将字符串标识转换为校验类型枚举值 @param str 校验算法字符串标识 @return 对应的枚举值 */
ProtocolSchema::ChecksumType ProtocolSchema::checksumTypeFromString(const QString &str) const
{
    if (str == QStringLiteral("none"))        return ChecksumType::None;
    if (str == QStringLiteral("crc8"))        return ChecksumType::Crc8;
    if (str == QStringLiteral("crc16_ccitt")) return ChecksumType::Crc16Ccitt;
    if (str == QStringLiteral("crc16_modbus"))return ChecksumType::Crc16Modbus;
    if (str == QStringLiteral("crc32"))       return ChecksumType::Crc32;
    if (str == QStringLiteral("xor"))         return ChecksumType::Xor;
    if (str == QStringLiteral("sum"))         return ChecksumType::Sum;
    return ChecksumType::None;
}

// 统计计数器接口见 ProtocolSchemaStats.cpp
