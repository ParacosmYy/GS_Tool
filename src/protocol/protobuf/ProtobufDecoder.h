/** @file ProtobufDecoder.h @brief Protobuf解码器 -- 加载.proto模式并解码/编码消息。支持Varint溢出保护/重复字段聚合/嵌套消息递归解码 */
#ifndef PROTOBUF_DECODER_H
#define PROTOBUF_DECODER_H

#include <QBuffer>
#include <QObject>
#include <QString>
#include <QByteArray>
#include <QVariantMap>
#include <QVariantList>

/** @brief Protobuf解码/编码器。加载.proto模式定义，支持重复字段和嵌套消息递归解码。协作: SchemaViewer/ProtocolBridgeManager */
class ProtobufDecoder : public QObject {
    Q_OBJECT

public:
    /** @brief 解码选项标志位 */
    enum DecodeOption {
        NoOptions       = 0x00, ///< 默认选项
        DecodeNested    = 0x01, ///< 递归解码嵌套消息(wire type 2尝试子消息解析)
        CollectRepeated = 0x02, ///< 聚合重复字段号为QVariantList
        AllOptions      = DecodeNested | CollectRepeated ///< 全部选项
    };
    Q_DECLARE_FLAGS(DecodeOptions, DecodeOption)
    Q_FLAG(DecodeOptions)

    /** @brief 解码统计快照 */
    struct DecodeStatistics {
        quint64 totalMessages = 0;      ///< 累计解码消息总数
        quint64 totalFields = 0;        ///< 累计解码字段总数(含嵌套)
        quint64 totalBytes = 0;         ///< 累计解码字节总数
        quint64 totalErrors = 0;        ///< 累计解析错误次数
        quint64 totalNestedMessages = 0;///< 累计嵌套消息解码次数
        quint64 totalRepeatedFields = 0;///< 累计重复字段聚合次数
        quint64 maxNestingDepth = 0;    ///< 历史最大嵌套深度
    };

    explicit ProtobufDecoder(QObject* parent = nullptr);
    bool loadProtoFile(const QString& filePath); ///< 加载.proto模式文件
    bool isLoaded() const;                   ///< 是否已加载模式
    QVariantMap decodeMessage(const QByteArray& data); ///< 解码Protobuf二进制消息
    QByteArray encodeMessage(const QVariantMap& fields) const; ///< 编码为Protobuf二进制

    // ---- 解码选项 ----
    void setDecodeOptions(DecodeOptions options); ///< 设置解码选项(嵌套/重复字段等)
    DecodeOptions decodeOptions() const;     ///< 获取当前解码选项
    void setMaxNestingDepth(int depth);      ///< 设置最大嵌套深度(默认8)
    int maxNestingDepth() const;             ///< 获取最大嵌套深度

    // ---- 统计接口 ----
    quint64 totalDecoded() const;            ///< 累计解码消息总数
    quint64 totalBytesDecoded() const;       ///< 累计解码字节总数
    quint64 errorCount() const;              ///< 累计解码错误次数
    quint64 totalFieldsDecoded() const;      ///< 累计解码字段总数(含嵌套)
    quint64 totalNestedDecoded() const;      ///< 累计嵌套消息解码次数
    quint64 totalRepeatedAggregated() const; ///< 累计重复字段聚合次数
    quint64 maxObservedNestingDepth() const; ///< 历史最大嵌套深度
    DecodeStatistics statistics() const;     ///< 完整统计快照
    void resetDecoderStatistics();           ///< 重置所有统计计数器

signals:
    void decoded(const QVariantMap& result); ///< 消息解码完成
    void decodeError(const QString& errorMsg); ///< 解码错误

private:
    QPair<quint64, int> parseVarint(const QByteArray& data, int offset) const; ///< 解析varint(溢出保护, 最多10字节)
    QPair<QVariantMap, int> decodeField(const QByteArray& data, int offset, int depth, QMap<int, int>& fieldTracker) const; ///< 解码单字段(含嵌套递归)
    QVariant tryDecodeNested(const QByteArray& data, int depth, bool& success) const; ///< 尝试解码嵌套消息
    void writeVarint(QBuffer& buf, quint64 value) const; ///< 写入varint编码
    void encodeSingleValue(QBuffer& buf, int wireType, const QVariant& val) const; ///< 编码单个值

    QString m_protoFilePath;  ///< .proto文件路径
    bool    m_loaded = false; ///< 是否已加载模式

    DecodeOptions m_decodeOptions = AllOptions; ///< 当前解码选项
    int m_maxNestingDepth = 8;       ///< 最大递归嵌套深度

    mutable DecodeStatistics m_stats;    ///< 累计统计快照(const方法中递增)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ProtobufDecoder::DecodeOptions)

#endif // PROTOBUF_DECODER_H
