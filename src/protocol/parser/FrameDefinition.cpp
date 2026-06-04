/**
 * @file FrameDefinition.cpp
 * @brief 帧格式定义和字段解析实现
 *
 * 包含:
 *   - FieldDef: 字段提取和格式化（带边界保护）
 *   - FrameDefinition: 帧结构定义的JSON序列化
 */

#include "protocol/parser/FrameDefinition.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCoreApplication>
#include <QDebug>

// ---- FieldDef 内部辅助 ----

/** @brief 验证字段边界是否在payload范围内 @param payload 帧有效数据 @param fieldOffset 字段偏移 @param fieldSize 字段大小 @param fieldName 字段名 @return true=边界合法，false=越界 */
static bool validateFieldBounds(const QByteArray& payload, int fieldOffset,
                                int fieldSize, const QString& fieldName)
{
    if (fieldOffset < 0) {
        qWarning() << "FieldDef::extractValue: offset (" << fieldOffset
                   << ") is negative, field:" << fieldName;
        return false;
    }
    if (fieldSize <= 0) {
        qWarning() << "FieldDef::extractValue: size (" << fieldSize
                   << ") is non-positive, field:" << fieldName;
        return false;
    }
    if (fieldOffset + fieldSize > payload.size()) {
        qWarning() << "FieldDef::extractValue: offset (" << fieldOffset << ") + size ("
                   << fieldSize << ") = " << (fieldOffset + fieldSize)
                   << "exceeds payload size (" << payload.size()
                   << "), field:" << fieldName;
        return false;
    }
    return true;
}

// ---- FieldDef 方法实现 ----

/** @brief 从原始字节中提取并转换字段值 @param payload 帧有效数据区域 @return 提取出的字段值(经过scale+offset变换)，越界时返回无效QVariant */
QVariant FieldDef::extractValue(const QByteArray& payload) const
{
    // 边界检查: offset 和 size 必须在 payload 范围内
    if (!validateFieldBounds(payload, offset, size, name)) {
        return QVariant();
    }

    const char* d = payload.constData() + offset;
    double raw = 0.0;

    switch (type) {
    case UInt8:
        raw = static_cast<unsigned char>(d[0]);
        break;
    case UInt16LE:
        raw = static_cast<uint16_t>(
            (static_cast<unsigned char>(d[0])) |
            (static_cast<unsigned char>(d[1]) << 8));
        break;
    case UInt16BE:
        raw = static_cast<uint16_t>(
            (static_cast<unsigned char>(d[0]) << 8) |
            (static_cast<unsigned char>(d[1])));
        break;
    case UInt32LE:
        raw = static_cast<uint32_t>(
            (static_cast<uint32_t>(static_cast<unsigned char>(d[0]))) |
            (static_cast<uint32_t>(static_cast<unsigned char>(d[1])) << 8) |
            (static_cast<uint32_t>(static_cast<unsigned char>(d[2])) << 16) |
            (static_cast<uint32_t>(static_cast<unsigned char>(d[3])) << 24));
        break;
    case UInt32BE:
        raw = static_cast<uint32_t>(
            (static_cast<uint32_t>(static_cast<unsigned char>(d[0])) << 24) |
            (static_cast<uint32_t>(static_cast<unsigned char>(d[1])) << 16) |
            (static_cast<uint32_t>(static_cast<unsigned char>(d[2])) << 8) |
            (static_cast<uint32_t>(static_cast<unsigned char>(d[3]))));
        break;
    case Int8:
        raw = static_cast<signed char>(d[0]);
        break;
    case Int16LE:
        raw = static_cast<int16_t>(
            (static_cast<unsigned char>(d[0])) |
            (static_cast<unsigned char>(d[1]) << 8));
        break;
    case Int16BE:
        raw = static_cast<int16_t>(
            (static_cast<unsigned char>(d[0]) << 8) |
            (static_cast<unsigned char>(d[1])));
        break;
    case Float: {
        if (size >= 4) {
            float val;
            memcpy(&val, d, 4);
            raw = static_cast<double>(val);
        }
        break;
    }
    case Raw:
        return QVariant(QByteArray(d, size));
    default:
        qWarning() << "FieldDef::extractValue: unknown field type" << static_cast<int>(type)
                   << "field:" << name;
        return QVariant();
    }

    return raw * scale + offsetVal;
}

/** @brief 格式化显示值(带单位，Raw显示HEX) @param payload 帧有效数据区域 @return 格式化后的字符串，越界时返回"N/A" */
QString FieldDef::formatValue(const QByteArray& payload) const
{
    QVariant val = extractValue(payload);
    if (!val.isValid()) return QCoreApplication::translate("FieldDef", "N/A");

    if (type == Raw) {
        // Raw类型显示HEX
        QByteArray ba = val.toByteArray();
        QString hex;
        for (int i = 0; i < ba.size(); ++i) {
            if (i > 0) hex += ' ';
            hex += QString("%1").arg(static_cast<unsigned char>(ba[i]), 2, 16, QChar('0')).toUpper();
        }
        return hex;
    }

    double numVal = val.toDouble();
    QString text;
    if (numVal == qFloor(numVal)) {
        text = QString::number(static_cast<qint64>(numVal));
    } else {
        text = QString::number(numVal, 'f', 2);
    }
    if (!unit.isEmpty()) {
        text += " " + unit;
    }
    return text;
}

// ---- FrameDefinition 方法实现 ----

/** @brief 计算整个帧的最大长度(用于缓冲区预分配) @return 最小帧长度估算值，-1=无帧头 */
int FrameDefinition::maxFrameLength() const
{
    if (header.isEmpty()) return -1;
    int minLen = header.size();
    if (lengthFieldOffset >= 0) minLen += lengthFieldSize;
    minLen += checksumSize;
    if (!footer.isEmpty()) minLen += footer.size();
    return minLen;
}

// FieldDef::toJson/fromJson + FrameDefinition::toJson/fromJson
// 已移至 FrameDefinitionSerialization.cpp
