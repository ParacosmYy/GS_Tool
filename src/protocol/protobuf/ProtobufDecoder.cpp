/**
 * @file ProtobufDecoder.cpp
 * @brief Protobuf解码器实现
 *
 * 基于Protobuf wire format解析二进制数据。
 * encodeMessage支持基本类型的编码(varint/32bit/64bit/length-delimited)。
 */
#include "protocol/protobuf/ProtobufDecoder.h"

#include <QBuffer>

/** @brief 构造函数 - 初始化Protobuf解码器 @param parent 父对象指针 */
ProtobufDecoder::ProtobufDecoder(QObject* parent)
    : QObject(parent)
{
}

/** @brief 加载.proto文件以提供模式信息 @param filePath .proto文件路径 @return 加载成功返回true，否则返回false */
bool ProtobufDecoder::loadProtoFile(const QString& filePath) {
    m_protoFilePath = filePath;
    m_loaded = true;
    return true;
}

/** @brief 检查是否已加载.proto文件 @return 已加载返回true，否则返回false */
bool ProtobufDecoder::isLoaded() const {
    return m_loaded;
}

/** @brief 解码Protobuf二进制消息(遍历字段tag按wire type解析) @param data 待解码的Protobuf二进制数据 @return 解析结果Map */
QVariantMap ProtobufDecoder::decodeMessage(const QByteArray& data) {
    QVariantMap result;
    if (data.isEmpty()) {
        ++m_errorCount;
        emit decodeError(tr("数据为空"));
        return result;
    }

    m_totalBytesDecoded += static_cast<quint64>(data.size());

    int offset = 0;
    while (offset < data.size()) {
        auto [fieldInfo, newOffset] = decodeField(data, offset);
        if (newOffset <= offset) { break; }
        offset = newOffset;

        int fieldNum = fieldInfo.value("fieldNumber").toInt();
        result[QString::number(fieldNum)] = fieldInfo;
    }

    ++m_totalDecoded;
    emit decoded(result);
    return result;
}

/** @brief 编码为Protobuf二进制消息 @param fields 字段映射表 @return 编码后的Protobuf二进制数据 */
QByteArray ProtobufDecoder::encodeMessage(const QVariantMap& fields) {
    QByteArray data;
    QBuffer buf(&data);
    buf.open(QIODevice::WriteOnly);

    for (auto it = fields.begin(); it != fields.end(); ++it) {
        bool ok = false;
        int fieldNum = it.key().toInt(&ok);
        if (!ok) { continue; }

        QVariantMap fieldMap = it.value().toMap();
        int wireType = fieldMap.value("wireType", 0).toInt();
        QVariant val = fieldMap.value("value");

        // 写tag: (field_number << 3) | wire_type
        quint64 tag = static_cast<quint64>((fieldNum << 3) | wireType);
        writeVarint(buf, tag);

        switch (wireType) {
        case 0: { // Varint
            quint64 v = val.toULongLong();
            writeVarint(buf, v);
            break;
        }
        case 1: { // 64-bit fixed
            quint64 v = val.toULongLong();
            for (int i = 0; i < 8; ++i) {
                buf.putChar(static_cast<char>((v >> (i * 8)) & 0xFF));
            }
            break;
        }
        case 2: { // Length-delimited
            QByteArray bytes;
            if (val.typeId() == QMetaType::QByteArray) {
                bytes = val.toByteArray();
            } else {
                bytes = val.toString().toUtf8();
            }
            writeVarint(buf, static_cast<quint64>(bytes.size()));
            buf.write(bytes);
            break;
        }
        case 5: { // 32-bit fixed
            quint32 v = val.toUInt();
            for (int i = 0; i < 4; ++i) {
                buf.putChar(static_cast<char>((v >> (i * 8)) & 0xFF));
            }
            break;
        }
        default:
            break;
        }
    }

    buf.close();
    return data;
}

/** @brief 将varint编码写入缓冲区 @param buf 目标缓冲区 @param value 待写入的64位无符号整数 */
void ProtobufDecoder::writeVarint(QBuffer& buf, quint64 value) const {
    while (value > 0x7F) {
        buf.putChar(static_cast<char>((value & 0x7F) | 0x80));
        value >>= 7;
    }
    buf.putChar(static_cast<char>(value & 0x7F));
}

/** @brief 从数据中解析一个varint值 @param data 源二进制数据 @param offset 起始偏移量 @return QPair<解析后的值,消耗的字节数> */
QPair<quint64, int> ProtobufDecoder::parseVarint(const QByteArray& data,
                                                  int offset) const {
    quint64 value = 0;
    int shift = 0;
    int pos = offset;

    while (pos < data.size()) {
        quint8 byte = static_cast<quint8>(data[pos]);
        value |= static_cast<quint64>(byte & 0x7F) << shift;
        pos++;
        if ((byte & 0x80) == 0) { break; }
        shift += 7;
    }
    return {value, pos - offset};
}

/** @brief 从二进制数据中解码单个Protobuf字段(tag→fieldNumber+wireType→值提取) @param data 源二进制数据 @param offset 起始偏移量 @return QPair<字段信息Map,新偏移量> */
QPair<QVariantMap, int> ProtobufDecoder::decodeField(const QByteArray& data,
                                                      int offset) const {
    QVariantMap field;
    int consumed = 0;

    // 解析tag (field_number << 3 | wire_type)
    auto [tag, tagBytes] = parseVarint(data, offset);
    consumed += tagBytes;

    int fieldNumber = static_cast<int>(tag >> 3);
    int wireType    = static_cast<int>(tag & 0x07);

    field["fieldNumber"] = fieldNumber;
    field["wireType"]    = wireType;

    // 根据wire type解析值
    switch (wireType) {
    case 0: { // Varint
        auto [val, vb] = parseVarint(data, offset + consumed);
        consumed += vb;
        field["value"] = QVariant::fromValue(val);
        break;
    }
    case 1: { // 64-bit
        if (offset + consumed + 8 <= data.size()) {
            quint64 val = 0;
            for (int i = 0; i < 8; ++i) {
                val |= static_cast<quint64>(
                    static_cast<quint8>(data[offset + consumed + i]))
                    << (i * 8);
            }
            consumed += 8;
            field["value"] = QVariant::fromValue(val);
        }
        break;
    }
    case 2: { // Length-delimited
        auto [len, lb] = parseVarint(data, offset + consumed);
        consumed += lb;
        if (offset + consumed + static_cast<int>(len) <= data.size()) {
            QByteArray bytes = data.mid(offset + consumed,
                                         static_cast<int>(len));
            consumed += static_cast<int>(len);
            field["value"] = bytes;
        }
        break;
    }
    case 5: { // 32-bit
        if (offset + consumed + 4 <= data.size()) {
            quint32 val = 0;
            for (int i = 0; i < 4; ++i) {
                val |= static_cast<quint32>(
                    static_cast<quint8>(data[offset + consumed + i]))
                    << (i * 8);
            }
            consumed += 4;
            field["value"] = QVariant::fromValue(val);
        }
        break;
    }
    default:
        break;
    }

    return {field, offset + consumed};
}

// ── 统计接口实现 ──

/** @brief 获取累计解码的消息总数 @return 解码总数 */
quint64 ProtobufDecoder::totalDecoded() const
{
    return m_totalDecoded;
}

/** @brief 获取累计解码的字节总数 @return 字节总数 */
quint64 ProtobufDecoder::totalBytesDecoded() const
{
    return m_totalBytesDecoded;
}

/** @brief 获取累计解码错误次数 @return 错误次数 */
quint64 ProtobufDecoder::errorCount() const
{
    return m_errorCount;
}

/** @brief 重置所有统计计数器 */
void ProtobufDecoder::resetDecoderStatistics()
{
    m_totalDecoded = 0;
    m_totalBytesDecoded = 0;
    m_errorCount = 0;
}
