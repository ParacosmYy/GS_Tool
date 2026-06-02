/**
 * @file PacketBuilder.cpp
 * @brief 数据包构建器实现
 * @author Serial Tool Team
 * @date 2026-06-02
 *
 * 根据字段定义构建二进制数据包，支持多数据类型编码和CRC16校验。
 */

#include "utils/packet/PacketBuilder.h"

#include <QDataStream>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

/**
 * @brief 构造函数
 */
PacketBuilder::PacketBuilder(QObject *parent)
    : QObject(parent)
{
}

/**
 * @brief 添加字段到列表
 */
void PacketBuilder::addField(const PacketField &field)
{
    m_fields.append(field);
    emit fieldUpdated(m_fields.size() - 1);
}

/**
 * @brief 移除指定索引的字段
 */
void PacketBuilder::removeField(int index)
{
    if (index >= 0 && index < m_fields.size()) {
        m_fields.removeAt(index);
    }
}

/**
 * @brief 获取所有字段
 */
QList<PacketField> PacketBuilder::fields() const
{
    return m_fields;
}

/**
 * @brief 根据数据类型写入字段值到包缓冲区
 * @param packet 目标缓冲区
 * @param field 字段定义
 */
static void writeFieldValue(QByteArray &packet, const PacketField &field)
{
    const int base = field.offset;
    bool ok = false;

    if (field.dataType == "uint8") {
        quint8 val = static_cast<quint8>(field.value.toUInt(&ok));
        if (base < packet.size()) { packet[base] = static_cast<char>(val); }
    } else if (field.dataType == "uint16_le") {
        quint16 val = static_cast<quint16>(field.value.toUInt(&ok));
        if (base + 1 < packet.size()) {
            packet[base]     = static_cast<char>(val & 0xFF);
            packet[base + 1] = static_cast<char>((val >> 8) & 0xFF);
        }
    } else if (field.dataType == "uint16_be") {
        quint16 val = static_cast<quint16>(field.value.toUInt(&ok));
        if (base + 1 < packet.size()) {
            packet[base]     = static_cast<char>((val >> 8) & 0xFF);
            packet[base + 1] = static_cast<char>(val & 0xFF);
        }
    } else if (field.dataType == "uint32_le") {
        quint32 val = field.value.toUInt(&ok);
        for (int i = 0; i < 4; ++i) {
            if (base + i < packet.size()) {
                packet[base + i] = static_cast<char>((val >> (i * 8)) & 0xFF);
            }
        }
    } else if (field.dataType == "uint32_be") {
        quint32 val = field.value.toUInt(&ok);
        for (int i = 0; i < 4; ++i) {
            if (base + i < packet.size()) {
                packet[base + i] = static_cast<char>(
                    (val >> ((3 - i) * 8)) & 0xFF);
            }
        }
    } else if (field.dataType == "int8") {
        qint8 val = static_cast<qint8>(field.value.toInt(&ok));
        if (base < packet.size()) { packet[base] = val; }
    } else if (field.dataType == "int16_le") {
        qint16 val = static_cast<qint16>(field.value.toInt(&ok));
        if (base + 1 < packet.size()) {
            packet[base]     = static_cast<char>(val & 0xFF);
            packet[base + 1] = static_cast<char>((val >> 8) & 0xFF);
        }
    } else if (field.dataType == "int32_le") {
        qint32 val = field.value.toInt(&ok);
        for (int i = 0; i < 4; ++i) {
            if (base + i < packet.size()) {
                packet[base + i] = static_cast<char>((val >> (i * 8)) & 0xFF);
            }
        }
    } else if (field.dataType == "float") {
        float fval = field.value.toFloat(&ok);
        QByteArray fb(reinterpret_cast<const char*>(&fval), 4);
        for (int i = 0; i < 4 && base + i < packet.size(); ++i) {
            packet[base + i] = fb[i];
        }
    } else if (field.dataType == "string") {
        QByteArray str = field.value.toString().toUtf8();
        int copyLen = qMin(str.size(), field.size);
        for (int i = 0; i < copyLen && base + i < packet.size(); ++i) {
            packet[base + i] = str[i];
        }
    } else if (field.dataType == "bytes") {
        QByteArray bytes = QByteArray::fromHex(
            field.value.toString().toUtf8());
        int copyLen = qMin(bytes.size(), field.size);
        for (int i = 0; i < copyLen && base + i < packet.size(); ++i) {
            packet[base + i] = bytes[i];
        }
    } else {
        // 通用fallback：按unsigned写入
        quint64 val = field.value.toULongLong(&ok);
        if (!ok) {
            val = field.value.toString().toULongLong(&ok, 16);
        }
        for (int i = 0; i < field.size; ++i) {
            if (base + i < packet.size()) {
                packet[base + i] = static_cast<char>((val >> (i * 8)) & 0xFF);
            }
        }
    }
}

/**
 * @brief 计算CRC16-Modbus校验 (多项式0xA001)
 */
static quint16 crc16Modbus(const QByteArray &data)
{
    quint16 crc = 0xFFFF;
    for (char b : data) {
        crc ^= static_cast<quint8>(b);
        for (int i = 0; i < 8; ++i) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

/**
 * @brief 根据字段定义构建二进制数据包
 */
QByteArray PacketBuilder::buildPacket() const
{
    if (m_fields.isEmpty()) {
        return {};
    }

    // 计算总包长度
    int totalSize = 0;
    for (const auto &field : m_fields) {
        int end = field.offset + field.size;
        if (end > totalSize) {
            totalSize = end;
        }
    }

    QByteArray packet(totalSize, 0x00);

    // 按数据类型写入各字段值
    for (const auto &field : m_fields) {
        writeFieldValue(packet, field);
    }

    // CRC16校验尾
    if (m_checksumEnabled && !packet.isEmpty()) {
        quint16 crc = crc16Modbus(packet);
        packet.append(static_cast<char>(crc & 0xFF));
        packet.append(static_cast<char>((crc >> 8) & 0xFF));
    }

    // 注意: buildPacket是const方法，不能直接emit
    // 调用方负责在构建后处理packetBuilt信号
    return packet;
}

/**
 * @brief 设置是否添加尾部校验
 */
void PacketBuilder::setChecksumSuffix(bool enabled)
{
    m_checksumEnabled = enabled;
}

/**
 * @brief 从 JSON 文件加载模板
 */
bool PacketBuilder::loadTemplate(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isArray()) {
        return false;
    }

    m_fields.clear();
    QJsonArray arr = doc.array();
    for (const auto &item : arr) {
        QJsonObject obj = item.toObject();
        PacketField field;
        field.name = obj[QStringLiteral("name")].toString();
        field.offset = obj[QStringLiteral("offset")].toInt();
        field.size = obj[QStringLiteral("size")].toInt();
        field.dataType = obj[QStringLiteral("dataType")].toString();
        field.value = obj[QStringLiteral("value")].toVariant();
        m_fields.append(field);
    }

    return true;
}

/**
 * @brief 保存模板到 JSON 文件
 */
bool PacketBuilder::saveTemplate(const QString &filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    QJsonArray arr;
    for (const auto &field : m_fields) {
        QJsonObject obj;
        obj[QStringLiteral("name")] = field.name;
        obj[QStringLiteral("offset")] = field.offset;
        obj[QStringLiteral("size")] = field.size;
        obj[QStringLiteral("dataType")] = field.dataType;
        obj[QStringLiteral("value")] = QJsonValue::fromVariant(field.value);
        arr.append(obj);
    }

    QJsonDocument doc(arr);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}
