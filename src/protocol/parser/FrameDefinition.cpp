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

/** @brief 序列化为JSON对象 @return 包含字段所有属性的JSON对象 */
QJsonObject FieldDef::toJson() const
{
    QJsonObject obj;
    obj["name"] = name;
    obj["offset"] = offset;
    obj["size"] = size;
    obj["scale"] = scale;
    obj["offsetVal"] = offsetVal;
    obj["unit"] = unit;
    obj["type"] = static_cast<int>(type);
    return obj;
}

/** @brief 从JSON对象反序列化 @param obj JSON对象 @return 反序列化后的FieldDef实例 */
FieldDef FieldDef::fromJson(const QJsonObject& obj)
{
    FieldDef f;
    f.name = obj["name"].toString();
    f.offset = obj["offset"].toInt(0);
    f.size = obj["size"].toInt(1);
    f.scale = obj["scale"].toDouble(1.0);
    f.offsetVal = obj["offsetVal"].toDouble(0.0);
    f.unit = obj["unit"].toString();
    f.type = static_cast<Type>(obj["type"].toInt(0));
    return f;
}

// ---- FrameDefinition 方法实现 ----

/** @brief 计算整个帧的最大长度(用于缓冲区预分配) @return 最小帧长度估算值，-1=无帧头 */
int FrameDefinition::maxFrameLength() const
{
    if (header.isEmpty()) return -1;
    // 最小帧: header + checksum
    int minLen = header.size();
    if (lengthFieldOffset >= 0) minLen += lengthFieldSize;
    minLen += checksumSize;
    if (!footer.isEmpty()) minLen += footer.size();
    return minLen;
}

/** @brief 序列化为JSON对象(header/footer转HEX，fields转JSON数组) @return 包含帧定义所有属性的JSON对象 */
QJsonObject FrameDefinition::toJson() const
{
    QJsonObject obj;
    // header/footer 转为HEX字符串
    QString headerHex;
    for (auto b : header) {
        headerHex += QString("%1").arg(static_cast<unsigned char>(b), 2, 16, QChar('0')).toUpper();
    }
    obj["header"] = headerHex;
    QString footerHex;
    for (auto b : footer) {
        footerHex += QString("%1").arg(static_cast<unsigned char>(b), 2, 16, QChar('0')).toUpper();
    }
    obj["footer"] = footerHex;
    obj["lengthFieldOffset"] = lengthFieldOffset;
    obj["lengthFieldSize"] = lengthFieldSize;
    obj["lengthBigEndian"] = lengthBigEndian;
    obj["lengthAdjust"] = lengthAdjust;
    obj["checksumOffset"] = checksumOffset;
    obj["checksumType"] = static_cast<int>(checksumType);
    obj["checksumSize"] = checksumSize;
    obj["checksumStart"] = checksumStart;
    obj["checksumEnd"] = checksumEnd;

    QJsonArray fieldsArr;
    for (const auto& f : fields) {
        fieldsArr.append(f.toJson());
    }
    obj["fields"] = fieldsArr;
    return obj;
}

/** @brief 从JSON对象反序列化(header/footer从HEX解析) @param obj JSON对象 @return 反序列化后的FrameDefinition实例 */
FrameDefinition FrameDefinition::fromJson(const QJsonObject& obj)
{
    FrameDefinition def;
    // 解析header HEX字符串
    QString headerHex = obj["header"].toString();
    for (int i = 0; i + 1 < headerHex.length(); i += 2) {
        bool ok;
        unsigned char b = static_cast<unsigned char>(headerHex.mid(i, 2).toUInt(&ok, 16));
        if (ok) def.header.append(b);
    }
    QString footerHex = obj["footer"].toString();
    for (int i = 0; i + 1 < footerHex.length(); i += 2) {
        bool ok;
        unsigned char b = static_cast<unsigned char>(footerHex.mid(i, 2).toUInt(&ok, 16));
        if (ok) def.footer.append(b);
    }
    def.lengthFieldOffset = obj["lengthFieldOffset"].toInt(-1);
    def.lengthFieldSize = obj["lengthFieldSize"].toInt(1);
    def.lengthBigEndian = obj["lengthBigEndian"].toBool(false);
    def.lengthAdjust = obj["lengthAdjust"].toInt(0);
    def.checksumOffset = obj["checksumOffset"].toInt(-1);
    def.checksumType = static_cast<ChecksumType>(obj["checksumType"].toInt(0));
    def.checksumSize = obj["checksumSize"].toInt(0);
    def.checksumStart = obj["checksumStart"].toInt(0);
    def.checksumEnd = obj["checksumEnd"].toInt(-1);

    QJsonArray fieldsArr = obj["fields"].toArray();
    for (const auto& fVal : fieldsArr) {
        def.fields.append(FieldDef::fromJson(fVal.toObject()));
    }
    return def;
}
