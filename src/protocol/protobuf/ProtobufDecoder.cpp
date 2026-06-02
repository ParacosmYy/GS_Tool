/**
 * @file ProtobufDecoder.cpp
 * @brief Protobuf解码器实现
 *
 * 基于Protobuf wire format解析二进制数据。
 * TODO: 实现完整的.proto文件解析和消息类型映射。
 */
#include "protocol/protobuf/ProtobufDecoder.h"

ProtobufDecoder::ProtobufDecoder(QObject* parent)
    : QObject(parent)
{
}

bool ProtobufDecoder::loadProtoFile(const QString& filePath) {
    // TODO: 解析.proto文件，提取message/enum/service定义
    m_protoFilePath = filePath;
    m_loaded = true;
    return true;
}

bool ProtobufDecoder::isLoaded() const {
    return m_loaded;
}

QVariantMap ProtobufDecoder::decodeMessage(const QByteArray& data) {
    QVariantMap result;
    if (data.isEmpty()) {
        emit decodeError(tr("数据为空"));
        return result;
    }

    int offset = 0;
    while (offset < data.size()) {
        auto [fieldInfo, newOffset] = decodeField(data, offset);
        if (newOffset <= offset) { break; }
        offset = newOffset;

        int fieldNum = fieldInfo.value("fieldNumber").toInt();
        result[QString::number(fieldNum)] = fieldInfo;
    }

    emit decoded(result);
    return result;
}

QByteArray ProtobufDecoder::encodeMessage(const QVariantMap& fields) {
    QByteArray data;
    // TODO: 根据模式定义编码字段
    for (auto it = fields.begin(); it != fields.end(); ++it) {
        Q_UNUSED(it)
    }
    return data;
}

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
