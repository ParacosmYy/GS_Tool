/**
 * @file PacketBuilderTemplate.cpp
 * @brief PacketBuilder 模板I/O、校验和Hex格式化
 *
 * 从 PacketBuilder.cpp 拆分而来，包含JSON文件模板读写、
 * SettingsManager命名模板存取、字段校验和十六进制转换。
 */

#include "utils/packet/PacketBuilder.h"
#include "utils/settings/SettingsManager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <climits>

/** @brief 从JSON文件加载模板 @param filePath JSON文件路径 @return true加载成功 */
bool PacketBuilder::loadTemplate(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return false;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isArray()) return false;

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

/** @brief 保存模板到JSON文件 @param filePath 输出路径 @return true保存成功 */
bool PacketBuilder::saveTemplate(const QString &filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) return false;

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

/** @brief 从SettingsManager加载命名模板 @param name 模板名称 @param options 预留参数 @return true加载成功 */
bool PacketBuilder::loadTemplate(const QString &name, const QVariantMap &options)
{
    Q_UNUSED(options)

    const QString group = QStringLiteral("packetTemplates/") + name;
    auto &settings = SettingsManager::instance();
    auto guard = settings.groupGuard(group);

    if (!settings.contains(QStringLiteral("fields"))) return false;

    const QByteArray jsonBytes = settings.get(QStringLiteral("fields")).toByteArray();
    QJsonDocument doc = QJsonDocument::fromJson(jsonBytes);
    if (!doc.isArray()) return false;

    m_fields.clear();
    const QJsonArray arr = doc.array();
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

/** @brief 保存命名模板到SettingsManager @param name 模板名称 @param fields 字段映射 @return true保存成功 */
bool PacketBuilder::saveTemplate(const QString &name, const QVariantMap &fields)
{
    const QString group = QStringLiteral("packetTemplates/") + name;
    auto &settings = SettingsManager::instance();
    auto guard = settings.groupGuard(group);

    QJsonArray arr;
    if (fields.isEmpty()) {
        for (const auto &field : m_fields) {
            QJsonObject obj;
            obj[QStringLiteral("name")] = field.name;
            obj[QStringLiteral("offset")] = field.offset;
            obj[QStringLiteral("size")] = field.size;
            obj[QStringLiteral("dataType")] = field.dataType;
            obj[QStringLiteral("value")] = QJsonValue::fromVariant(field.value);
            arr.append(obj);
        }
    } else {
        for (auto it = fields.constBegin(); it != fields.constEnd(); ++it) {
            QJsonObject obj;
            obj[QStringLiteral("name")] = it.key();
            obj[QStringLiteral("value")] = QJsonValue::fromVariant(it.value());
            arr.append(obj);
        }
    }

    QJsonDocument doc(arr);
    settings.set(QStringLiteral("fields"), doc.toJson(QJsonDocument::Compact));
    settings.sync();
    return true;
}

/** @brief 校验所有字段的偏移和长度是否合法(无负偏移/正大小/无溢出/无重叠) */
bool PacketBuilder::validate() const
{
    for (int i = 0; i < m_fields.size(); ++i) {
        const auto &field = m_fields[i];
        if (field.offset < 0) return false;
        if (field.size <= 0) return false;
        if (field.offset > INT_MAX - field.size) return false;
    }

    for (int i = 0; i < m_fields.size(); ++i) {
        const int iStart = m_fields[i].offset;
        const int iEnd = iStart + m_fields[i].size;
        for (int j = i + 1; j < m_fields.size(); ++j) {
            const int jStart = m_fields[j].offset;
            const int jEnd = jStart + m_fields[j].size;
            if (!(iEnd <= jStart || jEnd <= iStart)) return false;
        }
    }
    return true;
}

/** @brief 将构建的数据包格式化为十六进制字符串(如 "AA BB CC DD") */
QString PacketBuilder::toHexString() const
{
    QByteArray packet = buildPacket();
    QString result;
    result.reserve(packet.size() * 3);
    for (int i = 0; i < packet.size(); ++i) {
        if (i > 0) result += QLatin1Char(' ');
        result += QString::number(static_cast<unsigned char>(packet[i]), 16)
                      .toUpper().rightJustified(2, QLatin1Char('0'));
    }
    return result;
}

/** @brief 从十六进制字符串解析字段列表 @param hex 十六进制字符串(空格分隔) @return 解析后的字段列表 */
QList<PacketField> PacketBuilder::fromHexString(const QString &hex)
{
    QList<PacketField> result;
    const QStringList parts = hex.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    result.reserve(parts.size());
    int offset = 0;
    for (const QString &part : parts) {
        bool ok = false;
        int value = part.toInt(&ok, 16);
        if (ok) {
            PacketField field;
            field.name = QStringLiteral("byte_%1").arg(offset);
            field.offset = offset;
            field.size = 1;
            field.dataType = QStringLiteral("uint8");
            field.value = value;
            result.append(field);
            offset++;
        }
    }
    return result;
}
