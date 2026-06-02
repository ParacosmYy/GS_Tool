/**
 * @file FlatBuffersDecoder.cpp
 * @brief FlatBuffers解码器实现
 *
 * 解析FlatBuffers二进制格式，基于vtable结构提取字段值。
 * TODO: 实现完整的.fbs模式解析和类型映射。
 */
#include "protocol/protobuf/FlatBuffersDecoder.h"

FlatBuffersDecoder::FlatBuffersDecoder(QObject* parent)
    : QObject(parent)
{
}

bool FlatBuffersDecoder::loadFbsFile(const QString& filePath) {
    // TODO: 解析.fbs文件，提取table/struct/enum定义
    m_fbsFilePath = filePath;
    m_loaded = true;
    return true;
}

bool FlatBuffersDecoder::isLoaded() const {
    return m_loaded;
}

QVariantMap FlatBuffersDecoder::decodeMessage(const QByteArray& data) {
    QVariantMap result;
    if (data.size() < 8) { return result; }

    // FlatBuffers根表偏移量在数据开头4字节
    quint32 rootOffset = readOffset(data, 0);
    result = parseTable(data, static_cast<int>(rootOffset));

    return result;
}

QVariantMap FlatBuffersDecoder::parseTable(const QByteArray& data,
                                            int tableOffset) const {
    QVariantMap result;
    if (tableOffset < 4 || tableOffset >= data.size()) { return result; }

    // 读取soffset到vtable
    qint32 vtableSoffset = static_cast<qint32>(readOffset(
        data, tableOffset));
    int vtableOffset = tableOffset - vtableSoffset;
    if (vtableOffset < 0 || vtableOffset >= data.size()) { return result; }

    // vtable头: [vtable_size(2), table_size(2), field_offsets...]
    quint16 vtableSize = static_cast<quint16>(
        static_cast<quint8>(data[vtableOffset]) |
        (static_cast<quint8>(data[vtableOffset + 1]) << 8));

    // 字段数 = (vtableSize - 4) / 2
    int fieldCount = (vtableSize - 4) / 2;
    for (int i = 0; i < fieldCount; ++i) {
        int fieldOffPos = vtableOffset + 4 + i * 2;
        if (fieldOffPos + 1 >= data.size()) { break; }

        quint16 fieldOffset = static_cast<quint16>(
            static_cast<quint8>(data[fieldOffPos]) |
            (static_cast<quint8>(data[fieldOffPos + 1]) << 8));

        if (fieldOffset == 0) { continue; } // 字段不存在

        int fieldDataPos = tableOffset + fieldOffset;
        if (fieldDataPos + 4 > data.size()) { continue; }

        // 读取4字节值（简化处理，实际需根据模式定义判断类型）
        quint32 value = readOffset(data, fieldDataPos);
        result[QString("field_%1").arg(i)] = QVariant::fromValue(value);
    }

    return result;
}

quint32 FlatBuffersDecoder::readOffset(const QByteArray& data,
                                        int offset) const {
    if (offset < 0 || offset + 3 >= data.size()) { return 0; }
    return static_cast<quint32>(
        static_cast<quint8>(data[offset]) |
        (static_cast<quint8>(data[offset + 1]) << 8) |
        (static_cast<quint8>(data[offset + 2]) << 16) |
        (static_cast<quint8>(data[offset + 3]) << 24));
}
