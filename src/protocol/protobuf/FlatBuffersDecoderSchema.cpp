/**
 * @file FlatBuffersDecoderSchema.cpp
 * @brief FlatBuffers解码器 - FBS Schema加载与类型解析辅助
 *
 * 从 FlatBuffersDecoder.cpp 拆分而来，包含 .fbs 文件的加载、
 * 构造函数、isLoaded() 以及匿名命名空间的类型解析辅助函数。
 *
 * parseFbsContent() 已拆分至 FlatBuffersDecoderParse.cpp。
 * 二进制解码逻辑见 FlatBuffersDecoder.cpp。
 */

#include "protocol/protobuf/FlatBuffersDecoder.h"

#include <QFile>
#include <QTextStream>
#include <QRegularExpression>

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

/**
 * @brief 解析类型名字符串为FbsBasicType枚举值
 * @param typeName 类型名称字符串
 * @return 对应的FbsBasicType枚举值，无法识别时返回Invalid
 */
FbsBasicType resolveBasicType(const QString& typeName) {
    return kTypeNameMap.value(typeName.toLower().trimmed(),
                              FbsBasicType::Invalid);
}

/**
 * @brief 解析字段行文本为FbsFieldDef结构
 * @param line 字段行文本，格式为 "name:type [= default]"
 * @return 解析后的字段定义结构
 */
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
    int cmtPos = typeStr.indexOf(QLatin1String("//"));
    if (cmtPos >= 0) { typeStr = typeStr.left(cmtPos).trimmed(); }
    field.type = resolveBasicType(typeStr);
    if (field.type == FbsBasicType::Invalid) {
        field.typeName = typeStr;
        field.type = FbsBasicType::Table;
    }
    return field;
}

/**
 * @brief 从花括号块中提取字段定义列表
 * @param block 包含花括号的文本块
 * @return 解析出的字段定义列表
 */
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

/**
 * @brief 解析enum块为FbsEnumDef结构
 * @param block 包含enum定义的文本块
 * @return 解析后的枚举定义结构
 */
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

/** @brief 构造函数 - 初始化FlatBuffers解码器 @param parent 父对象指针 */
FlatBuffersDecoder::FlatBuffersDecoder(QObject* parent) : QObject(parent) {}

/**
 * @brief 加载并解析.fbs Schema文件
 * @param filePath .fbs文件的完整路径
 * @return 解析成功且至少包含一个table定义时返回true，否则返回false
 */
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

/** @brief 检查是否已成功加载.fbs文件 @return 已加载返回true，否则返回false */
bool FlatBuffersDecoder::isLoaded() const { return m_loaded; }

// parseFbsContent() 见 FlatBuffersDecoderParse.cpp
