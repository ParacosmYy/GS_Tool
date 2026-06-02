/**
 * @file PacketBuilder.cpp
 * @brief 数据包构建器实现
 * @author Serial Tool Team
 * @date 2026-06-02
 */

#include "utils/packet/PacketBuilder.h"

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

    // 填充字段值
    for (const auto &field : m_fields) {
        bool ok = false;
        quint64 val = field.value.toULongLong(&ok);
        if (!ok) {
            // 尝试从十六进制字符串解析
            val = field.value.toString().toULongLong(&ok, 16);
        }

        for (int i = 0; i < field.size; ++i) {
            int shift = (field.size - 1 - i) * 8;
            if (field.offset + i < totalSize) {
                packet[field.offset + i] = static_cast<char>((val >> shift) & 0xFF);
            }
        }
    }

    // 可选：追加 XOR 校验尾
    if (m_checksumEnabled && !packet.isEmpty()) {
        quint8 checksum = 0x00;
        for (char byte : packet) {
            checksum ^= static_cast<quint8>(byte);
        }
        packet.append(static_cast<char>(checksum));
    }

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
