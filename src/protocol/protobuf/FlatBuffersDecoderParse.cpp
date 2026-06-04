/**
 * @file FlatBuffersDecoderParse.cpp
 * @brief FlatBuffers解码器 - FBS Schema文本内容解析逻辑
 *
 * 从 FlatBuffersDecoderSchema.cpp 拆分而来，包含 parseFbsContent 方法。
 * 该方法移除注释后提取 root_type 声明，按正则匹配 table/struct/enum 块，
 * 解析为内部结构定义存入容器。
 *
 * 匿名命名空间辅助函数(类型映射/字段解析/枚举解析)定义在
 * FlatBuffersDecoderSchema.cpp 中，通过头文件可见性供本文件间接使用。
 */

#include "protocol/protobuf/FlatBuffersDecoder.h"

#include <QRegularExpression>

// 匿名命名空间辅助函数声明(定义在 FlatBuffersDecoderSchema.cpp 中)
// 由于每个翻译单元有独立匿名命名空间，此处重新声明所需辅助符号
namespace {
    /** FBS基本类型字节数映射 */
    static const QMap<FbsBasicType, int> kTypeSizesParse = {
        {FbsBasicType::Int8, 1},   {FbsBasicType::UInt8, 1},
        {FbsBasicType::Bool, 1},   {FbsBasicType::Int16, 2},
        {FbsBasicType::UInt16, 2}, {FbsBasicType::Int32, 4},
        {FbsBasicType::UInt32, 4}, {FbsBasicType::Float32, 4},
        {FbsBasicType::Int64, 8},  {FbsBasicType::UInt64, 8},
        {FbsBasicType::Float64, 8}
    };

    /** FBS类型名字符串 → FbsBasicType映射 */
    static const QMap<QString, FbsBasicType> kTypeNameMapParse = {
        {QStringLiteral("int8"),    FbsBasicType::Int8},
        {QStringLiteral("byte"),    FbsBasicType::Int8},
        {QStringLiteral("uint8"),   FbsBasicType::UInt8},
        {QStringLiteral("ubyte"),   FbsBasicType::UInt8},
        {QStringLiteral("int16"),   FbsBasicType::Int16},
        {QStringLiteral("short"),   FbsBasicType::Int16},
        {QStringLiteral("uint16"),  FbsBasicType::UInt16},
        {QStringLiteral("ushort"),  FbsBasicType::UInt16},
        {QStringLiteral("int32"),   FbsBasicType::Int32},
        {QStringLiteral("int"),     FbsBasicType::Int32},
        {QStringLiteral("uint32"),  FbsBasicType::UInt32},
        {QStringLiteral("uint"),    FbsBasicType::UInt32},
        {QStringLiteral("int64"),   FbsBasicType::Int64},
        {QStringLiteral("long"),    FbsBasicType::Int64},
        {QStringLiteral("uint64"),  FbsBasicType::UInt64},
        {QStringLiteral("ulong"),   FbsBasicType::UInt64},
        {QStringLiteral("float"),   FbsBasicType::Float32},
        {QStringLiteral("float32"), FbsBasicType::Float32},
        {QStringLiteral("double"),  FbsBasicType::Float64},
        {QStringLiteral("float64"), FbsBasicType::Float64},
        {QStringLiteral("bool"),    FbsBasicType::Bool},
        {QStringLiteral("string"),  FbsBasicType::String}
    };

    /** 解析类型名字符串为FbsBasicType枚举值 */
    FbsBasicType resolveBasicTypeParse(const QString& typeName) {
        return kTypeNameMapParse.value(typeName.toLower().trimmed(),
                                  FbsBasicType::Invalid);
    }

    /** 解析字段行文本为FbsFieldDef结构 */
    FbsFieldDef parseFieldLineParse(const QString& line) {
        FbsFieldDef field;
        int colonPos = line.indexOf(QLatin1Char(':'));
        if (colonPos <= 0) { return field; }
        field.name = line.left(colonPos).trimmed();
        QString rest = line.mid(colonPos + 1).trimmed();
        int eqPos = rest.indexOf(QLatin1Char('='));
        QString typeStr;
        if (eqPos > 0) {
            typeStr = rest.left(eqPos).trimmed();
            bool ok = false;
            field.defaultValue = rest.mid(eqPos + 1).trimmed().toInt(&ok);
            field.hasDefault = ok;
        } else {
            typeStr = rest;
        }
        int cmtPos = typeStr.indexOf(QLatin1String("//"));
        if (cmtPos >= 0) { typeStr = typeStr.left(cmtPos).trimmed(); }
        field.type = resolveBasicTypeParse(typeStr);
        if (field.type == FbsBasicType::Invalid) {
            field.typeName = typeStr;
            field.type = FbsBasicType::Table;
        }
        return field;
    }

    /** 从花括号块中提取字段定义列表 */
    QList<FbsFieldDef> extractFieldsParse(const QString& block) {
        QList<FbsFieldDef> fields;
        int bStart = block.indexOf(QLatin1Char('{'));
        int bEnd = block.lastIndexOf(QLatin1Char('}'));
        if (bStart < 0 || bEnd < 0) { return fields; }
        QString body = block.mid(bStart + 1, bEnd - bStart - 1);
        for (const QString& seg : body.split(QLatin1Char(';'), Qt::SkipEmptyParts)) {
            QString trimmed = seg.trimmed();
            if (trimmed.isEmpty()) { continue; }
            FbsFieldDef f = parseFieldLineParse(trimmed);
            if (f.type != FbsBasicType::Invalid || !f.typeName.isEmpty()) {
                fields.append(f);
            }
        }
        return fields;
    }

    /** 解析enum块为FbsEnumDef结构 */
    FbsEnumDef parseEnumDefParse(const QString& block) {
        FbsEnumDef def;
        QRegularExpression re(R"(enum\s+(\w+)\s*:\s*(\w+)\s*\{)");
        auto m = re.match(block);
        if (!m.hasMatch()) { return def; }
        def.name = m.captured(1);
        def.underlyingType = resolveBasicTypeParse(m.captured(2));
        int bStart = block.indexOf(QLatin1Char('{'));
        int bEnd = block.lastIndexOf(QLatin1Char('}'));
        if (bStart < 0 || bEnd < 0) { return def; }
        int nextVal = 0;
        for (const QString& entry :
             block.mid(bStart + 1, bEnd - bStart - 1)
                  .split(QLatin1Char(','), Qt::SkipEmptyParts)) {
            QString t = entry.trimmed();
            if (t.isEmpty()) { continue; }
            int eq = t.indexOf(QLatin1Char('='));
            if (eq > 0) {
                def.values.append({t.left(eq).trimmed(), t.mid(eq + 1).trimmed().toInt()});
                nextVal = t.mid(eq + 1).trimmed().toInt() + 1;
            } else {
                def.values.append({t, nextVal++});
            }
        }
        return def;
    }
} // anonymous namespace

/**
 * @brief 解析.fbs文件文本内容为内存模式定义
 *
 * 移除注释后，提取root_type声明，然后按正则匹配table/struct/enum块，
 * 分别解析为FbsTableDef/FbsStructDef/FbsEnumDef并存入内部容器。
 *
 * @param content .fbs文件的完整文本内容
 */
void FlatBuffersDecoder::parseFbsContent(const QString& content) {
    QString cleaned = content;
    cleaned.remove(QRegularExpression(R"(/\*.*?\*/)",
        QRegularExpression::DotMatchesEverythingOption));
    cleaned.remove(QRegularExpression(R"(//[^\n]*)"));
    QRegularExpression rootRe(R"(root_type\s+(\w+)\s*;)");
    auto rm = rootRe.match(cleaned);
    if (rm.hasMatch()) { m_rootTypeName = rm.captured(1); }
    QRegularExpression blockRe(
        R"((table|struct|enum)\s+(\w+)\s*(?::\s*(\w+)\s*)?\{([^}]*)\})");
    auto it = blockRe.globalMatch(cleaned);
    while (it.hasNext()) {
        auto m = it.next();
        QString kw = m.captured(1), name = m.captured(2);
        QString ta = m.captured(3), body = m.captured(4);
        QString full = kw + QLatin1Char(' ') + name +
            (kw == QLatin1String("enum") ? QLatin1String(":") + ta : QString()) +
            QLatin1String(" {") + body + QLatin1Char('}');
        if (kw == QLatin1String("table")) {
            FbsTableDef td; td.name = name; td.fields = extractFieldsParse(full);
            m_tables[td.name] = td;
        } else if (kw == QLatin1String("struct")) {
            FbsStructDef sd; sd.name = name; sd.fields = extractFieldsParse(full);
            for (const auto& f : std::as_const(sd.fields)) {
                if (kTypeSizesParse.contains(f.type)) sd.byteSize += kTypeSizesParse.value(f.type);
                else if (f.type == FbsBasicType::Struct) {
                    auto si = m_structs.constFind(f.typeName);
                    if (si != m_structs.constEnd()) sd.byteSize += si.value().byteSize;
                }
            }
            m_structs[sd.name] = sd;
        } else if (kw == QLatin1String("enum")) {
            FbsEnumDef ed = parseEnumDefParse(full);
            if (!ed.name.isEmpty()) m_enums[ed.name] = ed;
        }
    }
}
