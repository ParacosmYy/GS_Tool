/**
 * @file PacketBuilderBuild.cpp
 * @brief 数据包构建器 — 包构建与字段值编码方法
 *
 * 从 PacketBuilder.cpp 拆分而来，包含:
 *   - writeFieldValue(): 根据数据类型将字段值写入包缓冲区(静态辅助)
 *   - crc16Modbus(): CRC16-Modbus校验计算(静态辅助)
 *   - buildPacket(): 根据字段定义构建完整二进制数据包
 *   - setChecksumSuffix(): CRC尾开关
 *
 * 字段管理/统计getter/resetStats保留在 PacketBuilder.cpp。
 * 模板I/O/校验/Hex转换见 PacketBuilderTemplate.cpp。
 */

#include "utils/packet/PacketBuilder.h"

#include <QDataStream>

/**
 * @brief 根据数据类型写入字段值到包缓冲区
 * @param packet 目标缓冲区
 * @param field 字段定义
 */
static void writeFieldValue(QByteArray &packet, const PacketField &field)
{
    const int base = field.offset;
    bool ok = false;

    if (field.dataType == QLatin1String("uint8")) {
        quint8 val = static_cast<quint8>(field.value.toUInt(&ok));
        if (base < packet.size()) { packet[base] = static_cast<char>(val); }
    } else if (field.dataType == QLatin1String("uint16_le")) {
        quint16 val = static_cast<quint16>(field.value.toUInt(&ok));
        if (base + 1 < packet.size()) {
            packet[base]     = static_cast<char>(val & 0xFF);
            packet[base + 1] = static_cast<char>((val >> 8) & 0xFF);
        }
    } else if (field.dataType == QLatin1String("uint16_be")) {
        quint16 val = static_cast<quint16>(field.value.toUInt(&ok));
        if (base + 1 < packet.size()) {
            packet[base]     = static_cast<char>((val >> 8) & 0xFF);
            packet[base + 1] = static_cast<char>(val & 0xFF);
        }
    } else if (field.dataType == QLatin1String("uint32_le")) {
        quint32 val = field.value.toUInt(&ok);
        for (int i = 0; i < 4; ++i) {
            if (base + i < packet.size()) {
                packet[base + i] = static_cast<char>((val >> (i * 8)) & 0xFF);
            }
        }
    } else if (field.dataType == QLatin1String("uint32_be")) {
        quint32 val = field.value.toUInt(&ok);
        for (int i = 0; i < 4; ++i) {
            if (base + i < packet.size()) {
                packet[base + i] = static_cast<char>(
                    (val >> ((3 - i) * 8)) & 0xFF);
            }
        }
    } else if (field.dataType == QLatin1String("int8")) {
        qint8 val = static_cast<qint8>(field.value.toInt(&ok));
        if (base < packet.size()) { packet[base] = val; }
    } else if (field.dataType == QLatin1String("int16_le")) {
        qint16 val = static_cast<qint16>(field.value.toInt(&ok));
        if (base + 1 < packet.size()) {
            packet[base]     = static_cast<char>(val & 0xFF);
            packet[base + 1] = static_cast<char>((val >> 8) & 0xFF);
        }
    } else if (field.dataType == QLatin1String("int32_le")) {
        qint32 val = field.value.toInt(&ok);
        for (int i = 0; i < 4; ++i) {
            if (base + i < packet.size()) {
                packet[base + i] = static_cast<char>((val >> (i * 8)) & 0xFF);
            }
        }
    } else if (field.dataType == QLatin1String("float")) {
        float fval = field.value.toFloat(&ok);
        QByteArray fb(reinterpret_cast<const char*>(&fval), 4);
        for (int i = 0; i < 4 && base + i < packet.size(); ++i) {
            packet[base + i] = fb[i];
        }
    } else if (field.dataType == QLatin1String("string")) {
        QByteArray str = field.value.toString().toUtf8();
        int copyLen = qMin(str.size(), field.size);
        for (int i = 0; i < copyLen && base + i < packet.size(); ++i) {
            packet[base + i] = str[i];
        }
    } else if (field.dataType == QLatin1String("bytes")) {
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
    ++m_totalBuilds;
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

    m_totalBytesBuilt += static_cast<quint64>(packet.size());
    return packet;
}

/**
 * @brief 设置是否添加尾部校验
 */
void PacketBuilder::setChecksumSuffix(bool enabled)
{
    m_checksumEnabled = enabled;
}
