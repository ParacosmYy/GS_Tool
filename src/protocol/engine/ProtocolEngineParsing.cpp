/**
 * @file ProtocolEngineParsing.cpp
 * @brief ProtocolEngine 缓冲区管理与帧搜索解析实现
 *
 * 从 ProtocolEngine.cpp 拆分而来，包含:
 *   - findHeader: 在缓冲区中搜索帧头字节序列
 *   - trimBufferBeforePartialHeader: 修剪缓冲区保留可能的部分帧头
 *   - readLengthField: 从帧数据中读取长度字段(小端序)
 *   - extractField: 从帧数据中提取单个字段值(支持多种数据类型)
 */

#include "protocol/engine/ProtocolEngine.h"
#include "protocol/schema/ProtocolSchema.h"

#include <QDataStream>

/* ============================================================================
 * 缓冲区管理与帧搜索辅助方法
 * ============================================================================ */

/** @brief 在缓冲区中查找帧头字节序列 @param buffer 待搜索的缓冲区 @param header 帧头字节序列 @return 帧头起始位置，未找到返回-1 */
int ProtocolEngine::findHeader(const QByteArray &buffer,
                                const QVector<int> &header) const
{
    if (header.isEmpty() || buffer.size() < header.size()) {
        return -1;
    }

    int searchLimit = buffer.size() - header.size() + 1;
    for (int i = 0; i < searchLimit; ++i) {
        bool match = true;
        for (int j = 0; j < header.size(); ++j) {
            if (static_cast<quint8>(buffer.at(i + j)) !=
                static_cast<quint8>(header.at(j))) {
                match = false;
                break;
            }
        }
        if (match) {
            return i;
        }
    }
    return -1;
}

/** @brief 修剪缓冲区，保留末尾可能构成部分帧头的字节 @param header 帧头字节序列 */
void ProtocolEngine::trimBufferBeforePartialHeader(const QVector<int> &header)
{
    if (header.isEmpty() || m_buffer.isEmpty()) {
        return;
    }

    int maxKeep = header.size() - 1;
    if (maxKeep <= 0) {
        /* 帧头只有1字节或空，无法有部分匹配 */
        m_buffer.clear();
        return;
    }

    /* 从缓冲区末尾向前查找，看是否有部分帧头 */
    int bufferSize = m_buffer.size();
    int keepBytes = 0;

    for (int keep = 1; keep <= qMin(maxKeep, bufferSize); ++keep) {
        int startIdx = bufferSize - keep;
        bool partialMatch = true;
        for (int j = 0; j < keep; ++j) {
            if (static_cast<quint8>(m_buffer.at(startIdx + j)) !=
                static_cast<quint8>(header.at(j))) {
                partialMatch = false;
                break;
            }
        }
        if (partialMatch) {
            keepBytes = keep;
        }
    }

    /* 只保留可能是部分帧头的末尾字节 */
    int discardCount = bufferSize - keepBytes;
    if (discardCount > 0) {
        m_buffer.remove(0, discardCount);
    }
}

/** @brief 从帧数据中读取长度字段(小端序，支持1/2/4字节) @param buffer 帧数据缓冲区 @param offset 长度字段偏移 @param size 长度字段字节数 @return 解析得到的长度值 */
int ProtocolEngine::readLengthField(const QByteArray &buffer,
                                     int offset, int size) const
{
    if (size <= 0 || offset < 0 || (offset + size) > buffer.size()) {
        return -1;
    }

    quint32 value = 0;
    for (int i = 0; i < size; ++i) {
        value |= (static_cast<quint8>(buffer.at(offset + i)) << (8 * i));
    }
    return static_cast<int>(value);
}

/** @brief 从帧数据中提取单个字段值(支持uint8/uint16/int16/uint32/int32/float/double) @param frame 完整的原始帧数据 @param field 字段定义 @return 提取到的字段值，越界时返回空QByteArray */
QVariant ProtocolEngine::extractField(const QByteArray &frame,
                                       const ProtocolSchema::FieldDefinition &field) const
{
    /* 边界检查：字段是否在帧范围内 */
    if (field.offset < 0 || field.size <= 0 ||
        (field.offset + field.size) > frame.size()) {
        return QVariant(QByteArray());
    }

    QByteArray fieldBytes = frame.mid(field.offset, field.size);

    /* 按类型解析 */
    if (field.type == QLatin1String("uint8")) {
        return QVariant(static_cast<quint8>(fieldBytes.at(0)));
    }
    if (field.type == QLatin1String("uint16_le")) {
        return QVariant(static_cast<quint16>(
            static_cast<quint8>(fieldBytes.at(0)) |
            (static_cast<quint8>(fieldBytes.at(1)) << 8)));
    }
    if (field.type == QLatin1String("int16_le")) {
        quint16 raw = static_cast<quint16>(
            static_cast<quint8>(fieldBytes.at(0)) |
            (static_cast<quint8>(fieldBytes.at(1)) << 8));
        return QVariant(static_cast<qint16>(raw));
    }
    if (field.type == QLatin1String("uint32_le")) {
        quint32 val = 0;
        val |= static_cast<quint8>(fieldBytes.at(0));
        val |= static_cast<quint32>(static_cast<quint8>(fieldBytes.at(1))) << 8;
        val |= static_cast<quint32>(static_cast<quint8>(fieldBytes.at(2))) << 16;
        val |= static_cast<quint32>(static_cast<quint8>(fieldBytes.at(3))) << 24;
        return QVariant(val);
    }
    if (field.type == QLatin1String("int32_le")) {
        quint32 val = 0;
        val |= static_cast<quint8>(fieldBytes.at(0));
        val |= static_cast<quint32>(static_cast<quint8>(fieldBytes.at(1))) << 8;
        val |= static_cast<quint32>(static_cast<quint8>(fieldBytes.at(2))) << 16;
        val |= static_cast<quint32>(static_cast<quint8>(fieldBytes.at(3))) << 24;
        return QVariant(static_cast<qint32>(val));
    }
    if (field.type == QLatin1String("float_le")) {
        QDataStream ds(fieldBytes);
        ds.setByteOrder(QDataStream::LittleEndian);
        float val = 0.0f;
        ds >> val;
        return QVariant(val);
    }
    if (field.type == QLatin1String("double_le")) {
        QDataStream ds(fieldBytes);
        ds.setByteOrder(QDataStream::LittleEndian);
        double val = 0.0;
        ds >> val;
        return QVariant(val);
    }
    /* bytes 类型或未知类型：返回原始字节 */
    return QVariant(fieldBytes);
}
