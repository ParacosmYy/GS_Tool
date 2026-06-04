/**
 * @file FrameDefinitionSerialization.cpp
 * @brief 帧定义 - JSON序列化与反序列化实现
 *
 * 从 FrameDefinition.cpp 拆分而来，包含FieldDef和FrameDefinition
 * 的toJson/fromJson序列化方法。
 */

#include "protocol/parser/FrameDefinition.h"

#include <QJsonArray>
#include <QJsonObject>

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
