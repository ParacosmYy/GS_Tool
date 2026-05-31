#ifndef FRAMEDEFINITION_H
#define FRAMEDEFINITION_H

#include <QByteArray>
#include <QString>
#include <QVector>
#include <QVariantMap>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

// 单个字段定义 - 描述帧内一个数据字段的属性
struct FieldDef {
    QString name;           // 字段名称（如 "温度", "电压"）
    int offset = 0;         // 在帧payload内的字节偏移
    int size = 1;           // 字段字节数
    double scale = 1.0;     // 缩放系数: 显示值 = 原始值 × scale + offsetVal
    double offsetVal = 0.0; // 偏移值
    QString unit;           // 单位（如 "mV", "°C"）

    // 字段数据类型
    enum Type {
        UInt8,
        UInt16LE, UInt16BE,
        UInt32LE, UInt32BE,
        Int8,
        Int16LE, Int16BE,
        Float,
        Raw
    };
    Type type = UInt8;

    // 从原始字节中提取并转换字段值
    QVariant extractValue(const QByteArray& payload) const
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

    // 格式化显示值（带单位）
    QString formatValue(const QByteArray& payload) const
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

    // JSON序列化
    QJsonObject toJson() const
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

    static FieldDef fromJson(const QJsonObject& obj)
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
};

// 校验类型枚举
enum class ChecksumType {
    None,
    Sum8,
    CRC8,
    CRC16CCITT,
    CRC16Modbus,
    CRC32
};

// 完整帧格式定义 - 描述一帧数据的结构
struct FrameDefinition {
    QByteArray header;          // 帧头（如 AA 55）
    QByteArray footer;          // 帧尾（可选，如 0D 0A）
    int lengthFieldOffset = -1; // 长度字段在帧内的偏移（-1=无长度字段）
    int lengthFieldSize = 1;    // 长度字段字节数（1或2）
    bool lengthBigEndian = false; // 长度字段大小端
    int lengthAdjust = 0;       // 长度字段值需要加的偏移（长度值=有效数据长度+lengthAdjust）
    int checksumOffset = -1;    // 校验字段在帧内的偏移（-1=无校验）
    ChecksumType checksumType = ChecksumType::None;
    int checksumSize = 0;       // 校验字段字节数
    int checksumStart = 0;      // 校验计算起始偏移（通常为0，从帧头开始）
    int checksumEnd = -1;       // 校验计算结束偏移（-1=到校验字段前）
    QVector<FieldDef> fields;   // 数据字段列表

    // 计算整个帧的最大长度（用于缓冲区预分配）
    // 如果没有长度字段，返回-1（变长帧无法预知）
    int maxFrameLength() const
    {
        if (header.isEmpty()) return -1;
        // 最小帧: header + checksum
        int minLen = header.size();
        if (lengthFieldOffset >= 0) minLen += lengthFieldSize;
        minLen += checksumSize;
        if (!footer.isEmpty()) minLen += footer.size();
        return minLen;
    }

    // JSON序列化（保存/加载帧格式定义）
    QJsonObject toJson() const
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

    static FrameDefinition fromJson(const QJsonObject& obj)
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
};

#endif // FRAMEDEFINITION_H
