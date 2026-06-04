/**
 * @file FlatBuffersDecoderFields.cpp
 * @brief FlatBuffers解码器 — Table/Struct解析与类型推导
 *
 * 承载 FlatBuffersDecoder 中 table/struct 解析和类型字符串到枚举的映射:
 *   - parseTable(): 通过vtable反向引用获取字段偏移表
 *   - parseStruct(): 解析FlatBuffers内联struct
 *   - parseBasicType(): 类型名字符串转换为FbsBasicType枚举
 *
 * 顶层入口 decodeMessage() 及统计接口留在 FlatBuffersDecoder.cpp。
 * Schema 加载/解析逻辑见 FlatBuffersDecoderSchema.cpp。
 * 底层读取方法(readScalarValue/readTypedValue等)见 FlatBuffersDecoderRead.cpp。
 */

#include "protocol/protobuf/FlatBuffersDecoder.h"

// ───────────────────── Table / Struct 解析 ─────────────────────

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

// ───────────────────── 类型解析 ─────────────────────

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

// ── 底层读取方法(readOffset/readUint16/readScalarValue/readTypedValue/findRootTable)见 FlatBuffersDecoderRead.cpp ──
