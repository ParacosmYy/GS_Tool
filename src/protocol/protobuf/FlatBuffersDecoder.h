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

/**
 * @brief FBS基本类型枚举
 * 对应FlatBuffers支持的基础标量类型
 */
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

/**
 * @brief FBS字段定义
 * 描述单个字段的名称、类型及元信息
 */
struct FbsFieldDef {
    QString name;           ///< 字段名称
    FbsBasicType type = FbsBasicType::Invalid;  ///< 基础类型
    QString typeName;       ///< 自定义类型名（struct/enum/table引用）
    int defaultValue = 0;   ///< 默认值（标量）
    bool hasDefault = false;///< 是否有默认值
};

/**
 * @brief FBS Table定义
 * FlatBuffers中的主表结构，字段通过vtable按索引访问
 */
struct FbsTableDef {
    QString name;                   ///< 表名
    QList<FbsFieldDef> fields;      ///< 有序字段列表（对应vtable索引）
};

/**
 * @brief FBS Struct定义
 * 内联存储的固定大小复合类型
 */
struct FbsStructDef {
    QString name;                   ///< 结构体名
    QList<FbsFieldDef> fields;      ///< 有序字段列表
    int byteSize = 0;               ///< 总字节数
};

/**
 * @brief FBS Enum定义
 * 命名的整数枚举类型
 */
struct FbsEnumDef {
    QString name;                   ///< 枚举名
    FbsBasicType underlyingType = FbsBasicType::Int8;  ///< 底层类型
    QList<QPair<QString, int>> values;  ///< 枚举值列表 (name, value)
};

/**
 * @brief FlatBuffers解码器
 * 加载.fbs模式定义文件，解析schema后对二进制数据做类型化解码。
 */
class FlatBuffersDecoder : public QObject {
    Q_OBJECT

public:
    explicit FlatBuffersDecoder(QObject* parent = nullptr);

    /**
     * @brief 加载并解析.fbs模式文件
     * @param filePath .fbs文件路径
     * @return 是否加载并解析成功
     */
    bool loadFbsFile(const QString& filePath);

    /** @brief 是否已加载模式 */
    bool isLoaded() const;

    /**
     * @brief 解码FlatBuffers二进制消息
     * @param data 原始二进制数据
     * @return 解码后的字段映射（使用schema中的字段名和类型）
     */
    QVariantMap decodeMessage(const QByteArray& data);

private:
    /**
     * @brief 解析表（Table）结构
     * @param data 原始数据
     * @param tableOffset 表偏移量
     * @param rootTableName 根表类型名（空则用匿名模式）
     * @return 字段映射
     */
    QVariantMap parseTable(const QByteArray& data, int tableOffset,
                           const QString& rootTableName = QString()) const;

    /**
     * @brief 解析内联Struct字段
     * @param data 原始数据
     * @param basePos struct起始位置
     * @param structDef struct定义
     * @return 字段映射
     */
    QVariantMap parseStruct(const QByteArray& data, int basePos,
                            const FbsStructDef& structDef) const;

    /** @brief 读取指定偏移处的32位小端无符号整数 */
    quint32 readOffset(const QByteArray& data, int offset) const;

    /** @brief 读取16位小端无符号整数 */
    quint16 readUint16(const QByteArray& data, int offset) const;

    /**
     * @brief 将类型名字符串解析为FbsBasicType
     * @param typeName 类型名
     * @return 对应的基本类型枚举
     */
    FbsBasicType parseBasicType(const QString& typeName) const;

    /**
     * @brief 根据标量类型读取值
     * @param data 原始数据
     * @param pos 数据位置
     * @param type 标量类型
     * @return 解析后的值
     */
    QVariant readScalarValue(const QByteArray& data, int pos,
                             FbsBasicType type) const;

    /**
     * @brief 根据完整类型信息读取字段值（含嵌套table/struct/string/enum）
     * @param data 原始数据
     * @param pos 字段数据位置
     * @param type 字段类型
     * @param typeName 自定义类型名（struct/table/enum引用时使用）
     * @return 解析后的值
     */
    QVariant readTypedValue(const QByteArray& data, int pos,
                            FbsBasicType type, const QString& typeName) const;

    /**
     * @brief 查找指定名称的根表定义
     * 优先查找root_type声明，否则返回第一个table
     * @return 根表定义指针，无则nullptr
     */
    const FbsTableDef* findRootTable() const;

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
};

#endif // FLATBUFFERS_DECODER_H
