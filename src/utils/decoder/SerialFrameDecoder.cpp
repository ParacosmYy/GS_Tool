/**
 * @file SerialFrameDecoder.cpp
 * @brief 通用串口帧解码器实现 — 解析/校验/辅助
 * @author Serial Tool Team
 * @date 2026-06-05
 */

#include "utils/decoder/SerialFrameDecoder.h"

#include <QtMath>

// ============================================================
// 构造 / 帧定义管理
// ============================================================

SerialFrameDecoder::SerialFrameDecoder(QObject *parent)
    : QObject(parent)
{
    setObjectName("SerialFrameDecoder");
}

/**
 * @brief 设置当前帧定义
 * @param def 帧定义结构体
 */
void SerialFrameDecoder::setFrameDefinition(const FrameDef &def)
{
    m_frameDef = def;
}

/**
 * @brief 获取当前帧定义
 */
const SerialFrameDecoder::FrameDef &SerialFrameDecoder::frameDefinition() const
{
    return m_frameDef;
}

/**
 * @brief 从 QVariantMap 加载帧定义
 *
 * 期望的键名: header(QByteArray)、footer(QByteArray)、
 * fields(QVariantList，每项含 name/type/offset/length)、
 * checksumType(int)、checksumOffset(int)、checksumLength(int)、endianness(int)。
 */
SerialFrameDecoder::FrameDef SerialFrameDecoder::frameDefFromVariantMap(const QVariantMap &map) const
{
    FrameDef def;

    // 帧头帧尾
    if (map.contains("header")) {
        const QVariant v = map["header"];
        def.header = v.toByteArray();
    }
    if (map.contains("footer")) {
        const QVariant v = map["footer"];
        def.footer = v.toByteArray();
    }

    // 字节序
    def.endianness = static_cast<Endianness>(map.value("endianness", 0).toInt());

    // 校验和
    def.checksumType = static_cast<ChecksumType>(map.value("checksumType", 0).toInt());
    def.checksumOffset = map.value("checksumOffset", -1).toInt();
    def.checksumLength = map.value("checksumLength", 1).toInt();

    // 字段列表
    const QVariantList fieldsList = map.value("fields").toList();
    for (const QVariant &fv : fieldsList) {
        const QVariantMap fm = fv.toMap();
        FieldDef fd;
        fd.name = fm.value("name").toString();
        fd.type = static_cast<FieldType>(fm.value("type", 0).toInt());
        fd.offset = fm.value("offset", 0).toInt();
        fd.length = fm.value("length", fixedFieldSize(fd.type)).toInt();
        def.fields.append(fd);
    }

    return def;
}

// ============================================================
// 帧解析 (Decode)
// ============================================================

/**
 * @brief 解码一帧原始字节，提取所有字段
 *
 * 依次校验帧头 -> 帧尾 -> 校验和，然后按字段定义逐个提取值。
 * 成功时发射 frameDecoded 信号，失败时发射 decodeError 信号。
 */
QVariantMap SerialFrameDecoder::decodeFrame(const QByteArray &data)
{
    QVariantMap result;

    // 最小长度检查
    const int minLen = minimumFrameLength();
    if (data.size() < minLen) {
        m_stats.totalDecodeErrors++;
        emit decodeError(tr("帧长度不足: 需要 %1 字节, 实际 %2 字节")
                             .arg(minLen)
                             .arg(data.size()));
        return result;
    }

    // 帧头校验
    if (!m_frameDef.header.isEmpty()
        && !data.startsWith(m_frameDef.header)) {
        m_stats.totalDecodeErrors++;
        emit decodeError(tr("帧头不匹配: 期望 %1, 实际 %2")
                             .arg(QString(m_frameDef.header.toHex()))
                             .arg(QString(data.left(m_frameDef.header.size()).toHex())));
        return result;
    }

    // 帧尾校验
    if (!m_frameDef.footer.isEmpty()
        && !data.endsWith(m_frameDef.footer)) {
        m_stats.totalDecodeErrors++;
        emit decodeError(tr("帧尾不匹配: 期望 %1, 实际 %2")
                             .arg(QString(m_frameDef.footer.toHex()))
                             .arg(QString(data.right(m_frameDef.footer.size()).toHex())));
        return result;
    }

    // 校验和验证
    if (m_frameDef.checksumType != ChecksumType::None && m_frameDef.checksumOffset >= 0) {
        const int csLen = qMax(1, m_frameDef.checksumLength);
        const QPair<int, int> range = checksumRange(data.size());
        const QByteArray payloadSlice = data.mid(range.first, range.second - range.first);
        const QByteArray expected = computeChecksum(payloadSlice, m_frameDef.checksumType, csLen);
        const QByteArray actual = data.mid(m_frameDef.checksumOffset, csLen);
        if (expected != actual) {
            m_stats.totalChecksumErrors++;
            m_stats.totalDecodeErrors++;
            emit decodeError(tr("校验和错误: 期望 %1, 实际 %2")
                                 .arg(QString(expected.toHex()))
                                 .arg(QString(actual.toHex())));
            return result;
        }
    }

    // 逐字段提取
    for (const FieldDef &field : m_frameDef.fields) {
        const int ftIndex = static_cast<int>(field.type);
        if (ftIndex >= 0 && ftIndex < 10) {
            m_stats.framesByFieldType[ftIndex]++;
        }
        result[field.name] = extractFieldValue(data, field);
    }

    // 更新统计
    m_stats.totalFramesDecoded++;
    m_totalFrameSizeForAvg += data.size();
    m_stats.avgFrameSize = static_cast<double>(m_totalFrameSizeForAvg)
                           / static_cast<double>(m_stats.totalFramesDecoded);

    emit frameDecoded(result);
    return result;
}

// ============================================================
// 帧校验 (Validate)
// ============================================================

/**
 * @brief 校验帧头、帧尾、校验和的合法性
 */
bool SerialFrameDecoder::validateFrame(const QByteArray &data) const
{
    const int minLen = minimumFrameLength();
    if (data.size() < minLen) {
        return false;
    }

    // 帧头
    if (!m_frameDef.header.isEmpty() && !data.startsWith(m_frameDef.header)) {
        return false;
    }

    // 帧尾
    if (!m_frameDef.footer.isEmpty() && !data.endsWith(m_frameDef.footer)) {
        return false;
    }

    // 校验和
    if (m_frameDef.checksumType != ChecksumType::None && m_frameDef.checksumOffset >= 0) {
        const int csLen = qMax(1, m_frameDef.checksumLength);
        const QPair<int, int> range = checksumRange(data.size());
        const QByteArray payloadSlice = data.mid(range.first, range.second - range.first);
        const QByteArray expected = computeChecksum(payloadSlice, m_frameDef.checksumType, csLen);
        const QByteArray actual = data.mid(m_frameDef.checksumOffset, csLen);
        if (expected != actual) {
            return false;
        }
    }

    return true;
}

// ============================================================
// 辅助接口
// ============================================================

/**
 * @brief 计算最小帧长度
 */
int SerialFrameDecoder::minimumFrameLength() const
{
    int len = m_frameDef.header.size() + m_frameDef.footer.size();

    // 字段区域 — 取最大结束偏移
    int maxEnd = 0;
    for (const FieldDef &field : m_frameDef.fields) {
        const int end = field.offset + field.length;
        if (end > maxEnd) {
            maxEnd = end;
        }
    }
    len += maxEnd;

    // 校验和区域
    if (m_frameDef.checksumType != ChecksumType::None && m_frameDef.checksumOffset >= 0) {
        const int csEnd = m_frameDef.checksumOffset + qMax(1, m_frameDef.checksumLength);
        if (csEnd > maxEnd) {
            len += (csEnd - maxEnd);
        }
    }

    return len;
}

/**
 * @brief 获取字段类型的固定字节长度
 */
int SerialFrameDecoder::fixedFieldSize(FieldType type)
{
    switch (type) {
    case FieldType::UInt8:   return 1;
    case FieldType::UInt16:  return 2;
    case FieldType::UInt32:  return 4;
    case FieldType::Int8:    return 1;
    case FieldType::Int16:   return 2;
    case FieldType::Int32:   return 4;
    case FieldType::Float32: return 4;
    case FieldType::Float64: return 8;
    case FieldType::Bool:    return 1;
    case FieldType::Bytes:   return 0; // 可变长度
    }
    return 0;
}

/**
 * @brief 获取统计快照
 */
const SerialFrameDecoder::Stats &SerialFrameDecoder::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计计数器
 */
void SerialFrameDecoder::resetStatistics()
{
    m_stats = Stats{};
    m_totalFrameSizeForAvg = 0;
}

// ============================================================
// 内部解析辅助 — 字段提取
// ============================================================

/**
 * @brief 从字节数组提取单个字段值
 *
 * 根据字段类型和字节序将原始字节转换为 QVariant。
 */
QVariant SerialFrameDecoder::extractFieldValue(const QByteArray &data,
                                                const FieldDef &field) const
{
    if (field.offset + field.length > data.size()) {
        return QVariant();
    }

    const QByteArray slice = data.mid(field.offset, field.length);
    const bool bigEndian = (m_frameDef.endianness == Endianness::BigEndian);

    switch (field.type) {
    case FieldType::UInt8:
        return static_cast<quint8>(slice.at(0));

    case FieldType::UInt16: {
        quint16 v = 0;
        if (bigEndian) {
            v = (static_cast<quint8>(slice.at(0)) << 8)
                | static_cast<quint8>(slice.at(1));
        } else {
            v = static_cast<quint8>(slice.at(0))
                | (static_cast<quint8>(slice.at(1)) << 8);
        }
        return v;
    }

    case FieldType::UInt32: {
        quint32 v = 0;
        if (bigEndian) {
            v = (static_cast<quint32>(static_cast<quint8>(slice.at(0))) << 24)
                | (static_cast<quint32>(static_cast<quint8>(slice.at(1))) << 16)
                | (static_cast<quint32>(static_cast<quint8>(slice.at(2))) << 8)
                | static_cast<quint32>(static_cast<quint8>(slice.at(3)));
        } else {
            v = static_cast<quint32>(static_cast<quint8>(slice.at(0)))
                | (static_cast<quint32>(static_cast<quint8>(slice.at(1))) << 8)
                | (static_cast<quint32>(static_cast<quint8>(slice.at(2))) << 16)
                | (static_cast<quint32>(static_cast<quint8>(slice.at(3))) << 24);
        }
        return v;
    }

    case FieldType::Int8:
        return static_cast<qint8>(slice.at(0));

    case FieldType::Int16: {
        qint16 v = 0;
        if (bigEndian) {
            v = (static_cast<qint16>(static_cast<quint8>(slice.at(0))) << 8)
                | static_cast<quint8>(slice.at(1));
        } else {
            v = static_cast<quint8>(slice.at(0))
                | (static_cast<qint16>(static_cast<quint8>(slice.at(1))) << 8);
        }
        return v;
    }

    case FieldType::Int32: {
        qint32 v = 0;
        if (bigEndian) {
            v = (static_cast<qint32>(static_cast<quint8>(slice.at(0))) << 24)
                | (static_cast<qint32>(static_cast<quint8>(slice.at(1))) << 16)
                | (static_cast<qint32>(static_cast<quint8>(slice.at(2))) << 8)
                | static_cast<quint8>(slice.at(3));
        } else {
            v = static_cast<quint8>(slice.at(0))
                | (static_cast<qint32>(static_cast<quint8>(slice.at(1))) << 8)
                | (static_cast<qint32>(static_cast<quint8>(slice.at(2))) << 16)
                | (static_cast<qint32>(static_cast<quint8>(slice.at(3))) << 24);
        }
        return v;
    }

    case FieldType::Float32: {
        float f = 0.0f;
        if (bigEndian) {
            QByteArray tmp = slice;
            std::reverse(tmp.begin(), tmp.end());
            std::memcpy(&f, tmp.constData(), sizeof(float));
        } else {
            std::memcpy(&f, slice.constData(), sizeof(float));
        }
        return static_cast<double>(f); // QVariant 用 double 保持精度
    }

    case FieldType::Float64: {
        double d = 0.0;
        if (bigEndian) {
            QByteArray tmp = slice;
            std::reverse(tmp.begin(), tmp.end());
            std::memcpy(&d, tmp.constData(), sizeof(double));
        } else {
            std::memcpy(&d, slice.constData(), sizeof(double));
        }
        return d;
    }

    case FieldType::Bool:
        return (static_cast<quint8>(slice.at(0)) != 0);

    case FieldType::Bytes:
        return slice;
    }

    return QVariant();
}
