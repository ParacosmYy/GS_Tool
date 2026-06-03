/**
 * @file PacketBuilder.cpp
 * @brief 数据包构建器实现
 * @author Serial Tool Team
 * @date 2026-06-02
 *
 * 根据字段定义构建二进制数据包，支持多数据类型编码、CRC16校验、
 * SettingsManager模板持久化、字段校验和十六进制互转。
 */

#include "utils/packet/PacketBuilder.h"

#include <QDataStream>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

#include "utils/settings/SettingsManager.h"

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

/**
 * @brief 从 SettingsManager 加载命名模板
 *
 * 从 "packetTemplates/<name>" 分组读取模板字段列表。
 *
 * @param name 模板名称
 * @param options 预留扩展参数 (当前未使用)
 * @return true 加载成功
 */
bool PacketBuilder::loadTemplate(const QString &name, const QVariantMap &options)
{
    Q_UNUSED(options)

    const QString group = QStringLiteral("packetTemplates/") + name;
    auto &settings = SettingsManager::instance();
    auto guard = settings.groupGuard(group);

    if (!settings.contains(QStringLiteral("fields"))) {
        return false;
    }

    const QByteArray jsonBytes = settings.get(QStringLiteral("fields")).toByteArray();
    QJsonDocument doc = QJsonDocument::fromJson(jsonBytes);
    if (!doc.isArray()) {
        return false;
    }

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

/**
 * @brief 保存命名模板到 SettingsManager
 *
 * 将当前字段列表序列化为JSON并存入 "packetTemplates/<name>" 分组。
 *
 * @param name 模板名称
 * @param fields 字段映射 (为空则使用当前 m_fields)
 * @return true 保存成功
 */
bool PacketBuilder::saveTemplate(const QString &name, const QVariantMap &fields)
{
    const QString group = QStringLiteral("packetTemplates/") + name;
    auto &settings = SettingsManager::instance();
    auto guard = settings.groupGuard(group);

    QJsonArray arr;
    if (fields.isEmpty()) {
        // 使用当前字段列表
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
        // 从 QVariantMap 转换
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

/**
 * @brief 校验所有字段的偏移和长度是否合法
 *
 * 检查规则:
 *   - 字段偏移不能为负数
 *   - 字段大小必须大于零
 *   - 字段不得超出包范围 (offset + size 不溢出)
 *   - 字段之间不允许重叠
 *
 * @return true 所有字段合法
 */
bool PacketBuilder::validate() const
{
    for (int i = 0; i < m_fields.size(); ++i) {
        const auto &field = m_fields[i];

        if (field.offset < 0) {
            return false;
        }
        if (field.size <= 0) {
            return false;
        }
        // 检查整数溢出
        if (field.offset > INT_MAX - field.size) {
            return false;
        }
    }

    // 检查字段重叠
    for (int i = 0; i < m_fields.size(); ++i) {
        const int iStart = m_fields[i].offset;
        const int iEnd = iStart + m_fields[i].size;
        for (int j = i + 1; j < m_fields.size(); ++j) {
            const int jStart = m_fields[j].offset;
            const int jEnd = jStart + m_fields[j].size;
            // 区间重叠检测: 两个区间不重叠当且仅当一个在另一个右侧
            if (!(iEnd <= jStart || jEnd <= iStart)) {
                return false;
            }
        }
    }

    return true;
}

/**
 * @brief 将构建的数据包格式化为十六进制字符串
 *
 * 调用 buildPacket() 生成二进制包，然后转换为
 * 大写十六进制字符串，字节间以空格分隔。
 *
 * @return 格式化后的十六进制字符串 (如 "AA BB CC DD")
 */
QString PacketBuilder::toHexString() const
{
    const QByteArray packet = buildPacket();
    if (packet.isEmpty()) {
        return {};
    }

    QStringList hexBytes;
    hexBytes.reserve(packet.size());
    for (char b : packet) {
        hexBytes.append(QStringLiteral("%1")
            .arg(static_cast<quint8>(b), 2, 16, QLatin1Char('0')).toUpper());
    }
    return hexBytes.join(QLatin1Char(' '));
}

/**
 * @brief 从十六进制字符串解析为字段列表
 *
 * 将十六进制字符串 (支持空格分隔或连续) 解析为字节数组，
 * 每个字节生成一个 uint8 类型的 PacketField。
 *
 * @param hex 十六进制字符串 (如 "AA BB CC" 或 "AABBCC")
 * @return 解析后的字段列表
 */
QList<PacketField> PacketBuilder::fromHexString(const QString &hex)
{
    // 移除所有空白字符
    QString cleanHex = hex;
    cleanHex.remove(QRegularExpression(QStringLiteral("\\s")));

    if (cleanHex.isEmpty() || cleanHex.size() % 2 != 0) {
        return {};
    }

    // 验证全部为合法十六进制字符
    QRegularExpression hexValidator(QStringLiteral("^[0-9A-Fa-f]+$"));
    if (!hexValidator.match(cleanHex).hasMatch()) {
        return {};
    }

    const QByteArray bytes = QByteArray::fromHex(cleanHex.toUtf8());
    QList<PacketField> result;
    result.reserve(bytes.size());

    for (int i = 0; i < bytes.size(); ++i) {
        PacketField field;
        field.name = QStringLiteral("byte_%1").arg(i);
        field.offset = i;
        field.size = 1;
        field.dataType = QStringLiteral("uint8");
        field.value = static_cast<quint8>(bytes[i]);
        result.append(field);
    }

    return result;
}
