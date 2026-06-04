/**
 * @file FlatBuffersDecoder.cpp
 * @brief FlatBuffers解码器 - 二进制解码与统计
 *
 * 基于解析后的Schema对FlatBuffers二进制数据做类型化字段提取，
 * 支持标量、字符串、嵌套table、内联struct、枚举等类型。
 *
 * Schema加载/解析逻辑见 FlatBuffersDecoderSchema.cpp。
 */

#include "protocol/protobuf/FlatBuffersDecoder.h"

#include <cstring>

// ───────────────────── 二进制解码 ─────────────────────

/**
 * @brief 解码FlatBuffers二进制消息
 *
 * 读取根偏移量定位根表，根据已加载的Schema定义解析各字段。
 * @param data FlatBuffers格式的二进制数据
 * @return 解析结果Map，key为字段名，value为字段值
 */
QVariantMap FlatBuffersDecoder::decodeMessage(const QByteArray& data) {
    if (data.size() < 8) {
        ++m_errorCount;
        return {};
    }
    m_totalBytesDecoded += static_cast<quint64>(data.size());
    quint32 rootOff = readOffset(data, 0);
    const FbsTableDef* root = findRootTable();
    QVariantMap result = parseTable(data, static_cast<int>(rootOff),
                      root ? root->name : QString());
    ++m_totalDecoded;
    return result;
}

/**
 * @brief 解析FlatBuffers二进制数据中的table结构
 *
 * 通过vtable反向引用获取字段偏移表，按偏移逐一读取字段值。
 * 若存在Schema定义，则使用字段名和类型信息进行类型化解析。
 *
 * @param data 完整的二进制数据
 * @param tableOffset table在数据中的起始偏移量
 * @param rootTableName 对应的table类型名称（用于查找Schema）
 * @return 解析结果Map，key为字段名，value为字段值
 */
QVariantMap FlatBuffersDecoder::parseTable(const QByteArray& data,
                                            int tableOffset,
                                            const QString& rootTableName) const {
    QVariantMap result;
    if (tableOffset < 4 || tableOffset >= data.size()) { return result; }
    qint32 vtableSoff = static_cast<qint32>(readOffset(data, tableOffset));
    int vtOff = tableOffset - vtableSoff;
    if (vtOff < 0 || vtOff + 1 >= data.size()) { return result; }
    quint16 vtSize = readUint16(data, vtOff);
    int fieldCount = (vtSize - 4) / 2;
    const FbsTableDef* tdef = nullptr;
    auto tit = m_tables.constFind(rootTableName);
    if (tit != m_tables.constEnd()) tdef = &tit.value();

    for (int i = 0; i < fieldCount; ++i) {
        int fopPos = vtOff + 4 + i * 2;
        if (fopPos + 1 >= data.size()) { break; }
        quint16 foff = readUint16(data, fopPos);
        if (foff == 0) { continue; }
        int fdp = tableOffset + foff;
        if (fdp >= data.size()) { continue; }
        QString fname = tr("field_%1").arg(i);
        FbsBasicType ftype = FbsBasicType::Int32;
        QString ftypeName;

        if (tdef && i < tdef->fields.size()) {
            const auto& fd = tdef->fields.at(i);
            fname = fd.name;
            ftype = fd.type;
            ftypeName = fd.typeName;
        }
        result[fname] = readTypedValue(data, fdp, ftype, ftypeName);
    }
    return result;
}

/**
 * @brief 解析FlatBuffers内联struct
 * @param data 源二进制数据
 * @param pos struct起始位置
 * @param sdef struct的Schema定义
 * @return 字段名→字段值的Map
 */
QVariantMap FlatBuffersDecoder::parseStruct(const QByteArray& data,
                                             int pos,
                                             const FbsStructDef& sdef) const {
    QVariantMap result;
    int off = pos;
    for (const auto& f : sdef.fields) {
        if (off >= data.size()) { break; }
        result[f.name] = readTypedValue(data, off, f.type, f.typeName);
        off += 4; // 简化：所有字段4字节对齐
    }
    return result;
}

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

/**
 * @brief 将类型名字符串转换为FbsBasicType枚举
 * @param tn 类型名字符串
 * @return 对应的枚举值
 */
FbsBasicType FlatBuffersDecoder::parseBasicType(const QString& tn) const {
    static const QMap<QString, FbsBasicType> map = {
        {"int8", FbsBasicType::Int8}, {"uint8", FbsBasicType::UInt8},
        {"int16", FbsBasicType::Int16}, {"uint16", FbsBasicType::UInt16},
        {"int32", FbsBasicType::Int32}, {"uint32", FbsBasicType::UInt32},
        {"int64", FbsBasicType::Int64}, {"uint64", FbsBasicType::UInt64},
        {"float", FbsBasicType::Float32}, {"double", FbsBasicType::Float64},
        {"bool", FbsBasicType::Bool}, {"string", FbsBasicType::String}
    };
    return map.value(tn.toLower().trimmed(), FbsBasicType::Invalid);
}

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
        if (pos + 4 <= data.size()) return QVariant::fromValue(readOffset(data, pos));
        break;
    }
    return {};
}

/**
 * @brief 根据字段类型从二进制数据中读取类型化值
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

// ── 统计接口实现 ──

/** @brief 获取累计解码的消息总数 @return 解码总数 */
quint64 FlatBuffersDecoder::totalDecoded() const
{
    return m_totalDecoded;
}

/** @brief 获取累计解码的字节总数 @return 字节总数 */
quint64 FlatBuffersDecoder::totalBytesDecoded() const
{
    return m_totalBytesDecoded;
}

/** @brief 获取累计解码错误次数 @return 错误次数 */
quint64 FlatBuffersDecoder::errorCount() const
{
    return m_errorCount;
}

/** @brief 重置所有统计计数器 */
void FlatBuffersDecoder::resetDecoderStatistics()
{
    m_totalDecoded = 0;
    m_totalBytesDecoded = 0;
    m_errorCount = 0;
}
