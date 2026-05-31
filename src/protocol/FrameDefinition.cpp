#include "protocol/FrameDefinition.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

// ---- FieldDef 方法实现 ----

QVariant FieldDef::extractValue(const QByteArray& payload) const
{
    if (offset < 0 || offset + size > payload.size()) {
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
            (static_cast<unsigned char>(d[0])) |
            (static_cast<unsigned char>(d[1]) << 8) |
            (static_cast<unsigned char>(d[2]) << 16) |
            (static_cast<unsigned char>(d[3]) << 24));
        break;
    case UInt32BE:
        raw = static_cast<uint32_t>(
            (static_cast<unsigned char>(d[0]) << 24) |
            (static_cast<unsigned char>(d[1]) << 16) |
            (static_cast<unsigned char>(d[2]) << 8) |
            (static_cast<unsigned char>(d[3])));
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
        // IEEE 754 float, little-endian
        if (size >= 4) {
            float val;
            memcpy(&val, d, 4);
            raw = static_cast<double>(val);
        }
        break;
    }
    case Raw:
        return QVariant(QByteArray(d, size));
    }

    return raw * scale + offsetVal;
}

QString FieldDef::formatValue(const QByteArray& payload) const
{
    QVariant val = extractValue(payload);
    if (!val.isValid()) return QString("N/A");

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
