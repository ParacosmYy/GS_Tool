/**
 * @file FlatBuffersDecoder.cpp
 * @brief FlatBuffers解码器实现
 *
 * 解析.fbs Schema文本格式，提取table/struct/enum定义。
 * 基于解析后的模式对FlatBuffers二进制数据做类型化字段提取，
 * 支持标量、字符串、嵌套table、内联struct、枚举等类型。
 */
#include "protocol/protobuf/FlatBuffersDecoder.h"

#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <cstring>

// ── 匿名命名空间：FBS文本解析辅助 ──
namespace {

/** FBS基本类型字节数映射 */
static const QMap<FbsBasicType, int> kTypeSizes = {
    {FbsBasicType::Int8, 1},   {FbsBasicType::UInt8, 1},
    {FbsBasicType::Bool, 1},   {FbsBasicType::Int16, 2},
    {FbsBasicType::UInt16, 2}, {FbsBasicType::Int32, 4},
    {FbsBasicType::UInt32, 4}, {FbsBasicType::Float32, 4},
    {FbsBasicType::Int64, 8},  {FbsBasicType::UInt64, 8},
    {FbsBasicType::Float64, 8}
};

/** FBS类型名字符串 → FbsBasicType映射 */
static const QMap<QString, FbsBasicType> kTypeNameMap = {
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

/** 解析类型名字符串为FbsBasicType */
FbsBasicType resolveBasicType(const QString& typeName) {
    return kTypeNameMap.value(typeName.toLower().trimmed(),
                              FbsBasicType::Invalid);
}

/** 解析字段行: name:type [= default] */
FbsFieldDef parseFieldLine(const QString& line) {
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
    // 移除尾部注释
    int cmtPos = typeStr.indexOf(QLatin1String("//"));
    if (cmtPos >= 0) { typeStr = typeStr.left(cmtPos).trimmed(); }
    field.type = resolveBasicType(typeStr);
    if (field.type == FbsBasicType::Invalid) {
        field.typeName = typeStr;
        field.type = FbsBasicType::Table; // 自定义类型，查表区分
    }
    return field;
}

/** 提取花括号内body内容，按分号分割并解析字段 */
QList<FbsFieldDef> extractFields(const QString& block) {
    QList<FbsFieldDef> fields;
    int bStart = block.indexOf(QLatin1Char('{'));
    int bEnd = block.lastIndexOf(QLatin1Char('}'));
    if (bStart < 0 || bEnd < 0) { return fields; }
    QString body = block.mid(bStart + 1, bEnd - bStart - 1);
    for (const QString& seg : body.split(QLatin1Char(';'), Qt::SkipEmptyParts)) {
        QString trimmed = seg.trimmed();
        if (trimmed.isEmpty()) { continue; }
        FbsFieldDef f = parseFieldLine(trimmed);
        if (f.type != FbsBasicType::Invalid || !f.typeName.isEmpty()) {
            fields.append(f);
        }
    }
    return fields;
}

/** 解析enum块（内部辅助） */
FbsEnumDef parseEnumDef(const QString& block) {
    FbsEnumDef def;
    QRegularExpression re(R"(enum\s+(\w+)\s*:\s*(\w+)\s*\{)");
    auto m = re.match(block);
    if (!m.hasMatch()) { return def; }
    def.name = m.captured(1);
    def.underlyingType = resolveBasicType(m.captured(2));
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

// ───────────────────── 公开接口 ─────────────────────

FlatBuffersDecoder::FlatBuffersDecoder(QObject* parent) : QObject(parent) {}

bool FlatBuffersDecoder::loadFbsFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) { return false; }
    m_tables.clear(); m_structs.clear(); m_enums.clear();
    m_rootTypeName.clear();
    QTextStream in(&file);
    parseFbsContent(in.readAll());
    file.close();
    m_fbsFilePath = filePath;
    m_loaded = !m_tables.isEmpty();
    return m_loaded;
}

bool FlatBuffersDecoder::isLoaded() const { return m_loaded; }

QVariantMap FlatBuffersDecoder::decodeMessage(const QByteArray& data) {
    if (data.size() < 8) { return {}; }
    quint32 rootOff = readOffset(data, 0);
    const FbsTableDef* root = findRootTable();
    return parseTable(data, static_cast<int>(rootOff),
                      root ? root->name : QString());
}

// ───────────────────── FBS文本解析 ─────────────────────

void FlatBuffersDecoder::parseFbsContent(const QString& content) {
    // 移除注释
    QString cleaned = content;
    cleaned.remove(QRegularExpression(R"(/\*.*?\*/)",
        QRegularExpression::DotMatchesEverythingOption));
    cleaned.remove(QRegularExpression(R"(//[^\n]*)"));
    // root_type
    QRegularExpression rootRe(R"(root_type\s+(\w+)\s*;)");
    auto rm = rootRe.match(cleaned);
    if (rm.hasMatch()) { m_rootTypeName = rm.captured(1); }
    // 匹配table/struct/enum块
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
            FbsTableDef td; td.name = name; td.fields = extractFields(full);
            m_tables[td.name] = td;
        } else if (kw == QLatin1String("struct")) {
            FbsStructDef sd; sd.name = name; sd.fields = extractFields(full);
            for (const auto& f : std::as_const(sd.fields)) {
                if (kTypeSizes.contains(f.type)) sd.byteSize += kTypeSizes.value(f.type);
                else if (f.type == FbsBasicType::Struct) {
                    auto si = m_structs.constFind(f.typeName);
                    if (si != m_structs.constEnd()) sd.byteSize += si.value().byteSize;
                }
            }
            m_structs[sd.name] = sd;
        } else if (kw == QLatin1String("enum")) {
            FbsEnumDef ed = parseEnumDef(full);
            if (!ed.name.isEmpty()) m_enums[ed.name] = ed;
        }
    }
}

// ───────────────────── 二进制解码 ─────────────────────

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
    // 查找schema定义
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
        // 字段名与类型
        QString fname = QString("field_%1").arg(i);
        FbsBasicType ftype = FbsBasicType::UInt32;
        QString ftname;
        if (tdef && i < tdef->fields.size()) {
            const FbsFieldDef& fd = tdef->fields[i];
            fname = fd.name; ftype = fd.type; ftname = fd.typeName;
        }
        QVariant val = readTypedValue(data, fdp, ftype, ftname);
        if (val.isValid()) { result[fname] = val; }
    }
    return result;
}

QVariantMap FlatBuffersDecoder::parseStruct(const QByteArray& data,
                                             int basePos,
                                             const FbsStructDef& sdef) const {
    QVariantMap result;
    int pos = basePos;
    for (const auto& f : sdef.fields) {
        if (pos >= data.size()) { break; }
        if (f.type == FbsBasicType::Struct && !f.typeName.isEmpty()) {
            auto si = m_structs.constFind(f.typeName);
            if (si != m_structs.constEnd()) {
                result[f.name] = parseStruct(data, pos, si.value());
                pos += si.value().byteSize;
                continue;
            }
        }
        result[f.name] = readScalarValue(data, pos, f.type);
        pos += kTypeSizes.value(f.type, 4);
    }
    return result;
}

// ───────────────────── 工具方法 ─────────────────────

quint32 FlatBuffersDecoder::readOffset(const QByteArray& data, int off) const {
    if (off < 0 || off + 4 > data.size()) { return 0; }
    return static_cast<quint32>(
        static_cast<quint8>(data[off]) |
        (static_cast<quint8>(data[off + 1]) << 8) |
        (static_cast<quint8>(data[off + 2]) << 16) |
        (static_cast<quint8>(data[off + 3]) << 24));
}

quint16 FlatBuffersDecoder::readUint16(const QByteArray& data, int off) const {
    if (off < 0 || off + 2 > data.size()) { return 0; }
    return static_cast<quint16>(
        static_cast<quint8>(data[off]) |
        (static_cast<quint8>(data[off + 1]) << 8));
}

FbsBasicType FlatBuffersDecoder::parseBasicType(const QString& tn) const {
    return resolveBasicType(tn);
}

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

const FbsTableDef* FlatBuffersDecoder::findRootTable() const {
    if (!m_rootTypeName.isEmpty()) {
        auto it = m_tables.constFind(m_rootTypeName);
        if (it != m_tables.constEnd()) return &it.value();
    }
    if (!m_tables.isEmpty()) return &m_tables.constBegin().value();
    return nullptr;
}
