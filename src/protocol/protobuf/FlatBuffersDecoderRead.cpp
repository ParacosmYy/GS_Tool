/**
 * @file FlatBuffersDecoderRead.cpp
 * @brief FlatBuffers解码器 — 底层二进制读取方法
 *
 * 从 FlatBuffersDecoderFields.cpp 拆分而来，包含从二进制数据中
 * 读取各种类型值的底层方法:
 *   - readOffset(): 读取小端序32位偏移量
 *   - readUint16(): 读取小端序16位无符号整数
 *   - readScalarValue(): 根据标量类型读取值
 *   - readTypedValue(): 根据完整类型信息读取字段值(含嵌套/字符串/枚举)
 *   - findRootTable(): 查找根表定义
 */

#include "protocol/protobuf/FlatBuffersDecoder.h"

#include <cstring>

// ───────────────────── 底层读取 ─────────────────────

/** @brief 从数据中读取小端序32位偏移量 @param data 源数据 @param off 起始偏移 @return 读取的32位无符号值 */
quint32 FlatBuffersDecoder::readOffset(const QByteArray& data, int off) const {
    if (off + 4 > data.size()) { return 0; }
    return static_cast<quint32>(static_cast<quint8>(data[off]))
         | (static_cast<quint32>(static_cast<quint8>(data[off+1])) << 8)
         | (static_cast<quint32>(static_cast<quint8>(data[off+2])) << 16)
         | (static_cast<quint32>(static_cast<quint8>(data[off+3])) << 24);
}

/** @brief 从数据中读取小端序16位无符号整数 @param data 源数据 @param off 起始偏移 @return 读取的16位无符号值 */
quint16 FlatBuffersDecoder::readUint16(const QByteArray& data, int off) const {
    if (off + 2 > data.size()) { return 0; }
    return static_cast<quint16>(static_cast<quint8>(data[off]))
         | (static_cast<quint16>(static_cast<quint8>(data[off+1])) << 8);
}

// ───────────────────── 类型解析 ─────────────────────

/**
 * @brief 从二进制数据中读取指定类型的标量值
 * @param data 源二进制数据
 * @param pos 读取位置
 * @param type 目标标量类型
 * @return 包装为QVariant的标量值
 */
QVariant FlatBuffersDecoder::readScalarValue(const QByteArray& data,
                                              int pos, FbsBasicType type) const {
    if (pos >= data.size()) { return {}; }
    switch (type) {
    case FbsBasicType::Int8:  return QVariant::fromValue(static_cast<qint8>(data[pos]));
    case FbsBasicType::UInt8: return QVariant::fromValue(static_cast<quint8>(data[pos]));
    case FbsBasicType::Bool:  return QVariant(data[pos] != 0);
    case FbsBasicType::Int16:
        if (pos + 2 <= data.size()) return QVariant::fromValue(
            static_cast<qint16>(readUint16(data, pos)));
        break;
    case FbsBasicType::UInt16:
        if (pos + 2 <= data.size()) return QVariant::fromValue(readUint16(data, pos));
        break;
    case FbsBasicType::Int32:
        if (pos + 4 <= data.size()) return QVariant::fromValue(
            static_cast<qint32>(readOffset(data, pos)));
        break;
    case FbsBasicType::UInt32:
        if (pos + 4 <= data.size()) return QVariant::fromValue(readOffset(data, pos));
        break;
    case FbsBasicType::Float32:
        if (pos + 4 <= data.size()) {
            float f = 0.0f; std::memcpy(&f, data.constData() + pos, 4);
            return QVariant(static_cast<double>(f));
        }
        break;
    default:
        /* 未知类型默认按4字节读取，避免64位类型被截断 */
        if (pos + 4 <= data.size()) return QVariant::fromValue(readOffset(data, pos));
        break;
    case FbsBasicType::Int64: case FbsBasicType::UInt64:
        /* readScalarValue不处理64位类型，由readTypedValue专管 */
        return {};
    }
    return {};
}

/**
 * @brief 根据字段类型从二进制数据中读取类型化值
 *
 * 支持标量、64位整数、双精度浮点、字符串偏移、内联struct、
 * 嵌套table 以及枚举类型的完整解码。
 *
 * @param data 源二进制数据
 * @param pos 读取位置
 * @param type 字段类型枚举值
 * @param typeName 自定义类型名称
 * @return 包装为QVariant的字段值
 */
QVariant FlatBuffersDecoder::readTypedValue(const QByteArray& data,
                                             int pos, FbsBasicType type,
                                             const QString& typeName) const {
    switch (type) {
    case FbsBasicType::Int8: case FbsBasicType::UInt8: case FbsBasicType::Bool:
    case FbsBasicType::Int16: case FbsBasicType::UInt16:
    case FbsBasicType::Int32: case FbsBasicType::UInt32:
    case FbsBasicType::Float32:
        return readScalarValue(data, pos, type);
    case FbsBasicType::Int64: case FbsBasicType::UInt64:
        if (pos + 8 <= data.size()) {
            quint64 v = 0;
            for (int b = 0; b < 8; ++b)
                v |= static_cast<quint64>(static_cast<quint8>(data[pos + b])) << (b * 8);
            return (type == FbsBasicType::Int64)
                ? QVariant::fromValue(static_cast<qint64>(v))
                : QVariant::fromValue(v);
        }
        return {};
    case FbsBasicType::Float64:
        if (pos + 8 <= data.size()) {
            double d = 0.0; std::memcpy(&d, data.constData() + pos, 8);
            return QVariant(d);
        }
        return {};
    case FbsBasicType::String: {
        quint32 soff = readOffset(data, pos);
        int sp = pos + soff;
        if (sp + 4 <= data.size()) {
            quint32 slen = readOffset(data, sp);
            if (sp + 4 + static_cast<int>(slen) <= data.size())
                return QString::fromUtf8(data.constData() + sp + 4, slen);
        }
        return {};
    }
    case FbsBasicType::Struct:
        if (!typeName.isEmpty()) {
            auto si = m_structs.constFind(typeName);
            if (si != m_structs.constEnd()) return parseStruct(data, pos, si.value());
        }
        return {};
    case FbsBasicType::Table:
        if (!typeName.isEmpty()) {
            auto ti = m_tables.constFind(typeName);
            if (ti != m_tables.constEnd()) {
                quint32 noff = readOffset(data, pos);
                return parseTable(data, pos + noff, typeName);
            }
            auto ei = m_enums.constFind(typeName);
            if (ei != m_enums.constEnd()) {
                QVariant raw = readScalarValue(data, pos, ei.value().underlyingType);
                for (const auto& ev : ei.value().values)
                    if (ev.second == raw.toInt()) return QVariant(ev.first);
                return raw;
            }
        }
        return {};
    default:
        return readScalarValue(data, pos, type);
    }
}

/**
 * @brief 查找FlatBuffers根表定义
 *
 * 优先使用 root_type 声明的表名查找；若未声明则回退到
 * 第一个注册的 table 定义。
 *
 * @return 根表定义的指针，无可用表时返回nullptr
 */
const FbsTableDef* FlatBuffersDecoder::findRootTable() const {
    if (!m_rootTypeName.isEmpty()) {
        auto it = m_tables.constFind(m_rootTypeName);
        if (it != m_tables.constEnd()) return &it.value();
    }
    if (!m_tables.isEmpty()) return &m_tables.constBegin().value();
    return nullptr;
}
