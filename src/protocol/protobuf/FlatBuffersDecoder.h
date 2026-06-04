/**
 * @file FlatBuffersDecoder.h
 * @brief FlatBuffers解码器 — 加载.fbs模式并解码消息
 *
 * 解析FlatBuffers Schema (.fbs)文本格式，提取table/struct/enum定义，
 * 基于解析后的模式对FlatBuffers二进制数据进行类型化字段提取。
 */
#ifndef FLATBUFFERS_DECODER_H
#define FLATBUFFERS_DECODER_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QVariantMap>
#include <QMap>
#include <QList>
#include <QPair>

/** @brief FBS基本类型枚举，对应FlatBuffers支持的基础标量类型 */
enum class FbsBasicType {
    Invalid,    ///< 无效类型
    Int8,       ///< int8 / byte
    UInt8,      ///< uint8 / ubyte
    Int16,      ///< int16 / short
    UInt16,     ///< uint16 / ushort
    Int32,      ///< int32 / int
    UInt32,     ///< uint32 / uint
    Int64,      ///< int64 / long
    UInt64,     ///< uint64 / ulong
    Float32,    ///< float
    Float64,    ///< double
    Bool,       ///< bool
    String,     ///< string（偏移量引用）
    Struct,     ///< struct（内联复合类型）
    Enum,       ///< enum（命名整数）
    Table       ///< table（偏移量引用的复合类型）
};

/** @brief FBS字段定义，描述单个字段的名称、类型及元信息 */
struct FbsFieldDef {
    QString name;           ///< 字段名称
    FbsBasicType type = FbsBasicType::Invalid;  ///< 基础类型
    QString typeName;       ///< 自定义类型名（struct/enum/table引用）
    int defaultValue = 0;   ///< 默认值（标量）
    bool hasDefault = false;///< 是否有默认值
};

/** @brief FBS Table定义，FlatBuffers中的主表结构，字段通过vtable按索引访问 */
struct FbsTableDef {
    QString name;                   ///< 表名
    QList<FbsFieldDef> fields;      ///< 有序字段列表（对应vtable索引）
};

/** @brief FBS Struct定义，内联存储的固定大小复合类型 */
struct FbsStructDef {
    QString name;                   ///< 结构体名
    QList<FbsFieldDef> fields;      ///< 有序字段列表
    int byteSize = 0;               ///< 总字节数
};

/** @brief FBS Enum定义，命名的整数枚举类型 */
struct FbsEnumDef {
    QString name;                   ///< 枚举名
    FbsBasicType underlyingType = FbsBasicType::Int8;  ///< 底层类型
    QList<QPair<QString, int>> values;  ///< 枚举值列表 (name, value)
};

/** @brief FlatBuffers解码器，加载.fbs模式定义文件，解析schema后对二进制数据做类型化解码 */
class FlatBuffersDecoder : public QObject {
    Q_OBJECT

public:
    explicit FlatBuffersDecoder(QObject* parent = nullptr);

    bool loadFbsFile(const QString& filePath);  ///< 加载并解析.fbs模式文件
    bool isLoaded() const;                       ///< 是否已加载模式
    QVariantMap decodeMessage(const QByteArray& data); ///< 解码FlatBuffers二进制消息

    // ---- 统计接口 ----

    quint64 totalDecoded() const;       ///< 累计解码的消息总数
    quint64 totalBytesDecoded() const;  ///< 累计解码的字节总数
    quint64 errorCount() const;         ///< 累计解码错误次数
    void resetDecoderStatistics();      ///< 重置所有统计计数器

private:
    QVariantMap parseTable(const QByteArray& data, int tableOffset,            ///< 解析Table结构
                           const QString& rootTableName = QString()) const;
    QVariantMap parseStruct(const QByteArray& data, int basePos,               ///< 解析内联Struct字段
                            const FbsStructDef& structDef) const;
    quint32 readOffset(const QByteArray& data, int offset) const;  ///< 读取32位小端无符号整数
    quint16 readUint16(const QByteArray& data, int offset) const;  ///< 读取16位小端无符号整数
    FbsBasicType parseBasicType(const QString& typeName) const;    ///< 将类型名字符串解析为FbsBasicType
    QVariant readScalarValue(const QByteArray& data, int pos,      ///< 根据标量类型读取值
                             FbsBasicType type) const;
    QVariant readTypedValue(const QByteArray& data, int pos,       ///< 根据完整类型信息读取字段值（含嵌套table/struct/string/enum）
                            FbsBasicType type, const QString& typeName) const;
    const FbsTableDef* findRootTable() const; ///< 查找根表定义（优先root_type声明，否则第一个table）

    // ── FBS文本解析辅助方法 ──
    void parseFbsContent(const QString& content);
    void parseTableBlock(const QString& block);
    void parseStructBlock(const QString& block);
    void parseEnumBlock(const QString& block);
    FbsFieldDef parseFieldLine(const QString& line) const;

    QString m_fbsFilePath;              ///< .fbs文件路径
    bool m_loaded = false;              ///< 是否已加载模式

    QMap<QString, FbsTableDef>  m_tables;   ///< 已解析的表定义
    QMap<QString, FbsStructDef> m_structs;  ///< 已解析的结构体定义
    QMap<QString, FbsEnumDef>   m_enums;    ///< 已解析的枚举定义
    QString m_rootTypeName;                 ///< root_type声明的主表名

    quint64 m_totalDecoded = 0;     ///< 累计解码消息总数
    quint64 m_totalBytesDecoded = 0;///< 累计解码字节总数
    quint64 m_errorCount = 0;       ///< 累计解码错误次数
};

#endif // FLATBUFFERS_DECODER_H
