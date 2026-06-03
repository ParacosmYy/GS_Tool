/**
 * @file ProtobufDecoder.h
 * @brief Protobuf解码器 — 加载.proto模式并解码/编码消息
 *
 * 解析.proto文件定义的消息结构，将二进制数据解码为结构化映射，
 * 或将映射数据编码为二进制Protobuf格式。
 *
 * 增强功能:
 *   1. Varint溢出保护（最多10字节，超过视为损坏数据）
 *   2. 重复字段支持（相同fieldNumber自动聚合为QVariantList）
 *   3. 嵌套消息支持（wire type 2可递归解码子消息）
 *   4. 详细统计: 解码消息数、解码字段数、解析错误数、嵌套深度
 */
#ifndef PROTOBUF_DECODER_H
#define PROTOBUF_DECODER_H

#include <QBuffer>
#include <QObject>
#include <QString>
#include <QByteArray>
#include <QVariantMap>
#include <QVariantList>

/**
 * @brief Protobuf解码/编码器
 * 加载.proto模式定义，提供消息的解码和编码功能。
 * 支持重复字段和嵌套消息的递归解码。
 *
 * 协作关系:
 *   - SchemaViewer: 提供模式定义可视化
 *   - ProtocolBridgeManager: 作为桥接协议的数据消费端
 */
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

    /**
     * @brief 加载.proto模式文件
     * @param filePath .proto文件路径
     * @return 是否加载成功
     */
    bool loadProtoFile(const QString& filePath);

    /** @brief 是否已加载模式 */
    bool isLoaded() const;

    /**
     * @brief 解码Protobuf二进制消息
     * @param data 原始二进制数据
     * @return 解码后的字段映射 {fieldNumber: {name, type, value}}
     */
    QVariantMap decodeMessage(const QByteArray& data);

    /**
     * @brief 编码为Protobuf二进制消息
     * @param fields 字段映射
     * @return 编码后的二进制数据
     */
    QByteArray encodeMessage(const QVariantMap& fields) const;

    // ---- 解码选项 ----

    /** @brief 设置解码选项(嵌套/重复字段等) @param options 选项位掩码 */
    void setDecodeOptions(DecodeOptions options);

    /** @brief 获取当前解码选项 @return 选项位掩码 */
    DecodeOptions decodeOptions() const;

    /** @brief 设置最大递归嵌套深度(防止栈溢出，默认8) @param depth 最大深度 */
    void setMaxNestingDepth(int depth);

    /** @brief 获取最大递归嵌套深度 @return 最大深度 */
    int maxNestingDepth() const;

    // ---- 统计接口 ----

    /** @brief 获取累计解码的消息总数 */
    quint64 totalDecoded() const;

    /** @brief 获取累计解码的字节总数 */
    quint64 totalBytesDecoded() const;

    /** @brief 获取累计解码错误次数 */
    quint64 errorCount() const;

    /** @brief 获取累计解码字段总数(含嵌套字段) */
    quint64 totalFieldsDecoded() const;

    /** @brief 获取累计嵌套消息解码次数 */
    quint64 totalNestedDecoded() const;

    /** @brief 获取累计重复字段聚合次数 */
    quint64 totalRepeatedAggregated() const;

    /** @brief 获取历史最大嵌套深度 */
    quint64 maxObservedNestingDepth() const;

    /** @brief 获取完整统计快照 @return DecodeStatistics结构体 */
    DecodeStatistics statistics() const;

    /** @brief 重置所有统计计数器 */
    void resetDecoderStatistics();

signals:
    /** @brief 消息解码完成 */
    void decoded(const QVariantMap& result);

    /** @brief 解码错误 */
    void decodeError(const QString& errorMsg);

private:
    /**
     * @brief 解析varint编码（带溢出保护，最多10字节）
     * @param data 源数据
     * @param offset 起始偏移
     * @return {值, 消耗的字节数}，出错时消耗字节数为0
     */
    QPair<quint64, int> parseVarint(const QByteArray& data, int offset) const;

    /**
     * @brief 解码单个字段（支持嵌套递归和重复字段检测）
     * @param data 原始数据
     * @param offset 当前偏移
     * @param depth 当前嵌套深度
     * @param fieldTracker 字段号→出现次数跟踪器(用于重复字段检测)
     * @return {字段信息Map, 新偏移}
     */
    QPair<QVariantMap, int> decodeField(
        const QByteArray& data, int offset, int depth,
        QMap<int, int>& fieldTracker) const;

    /**
     * @brief 尝试将length-delimited数据解码为嵌套消息
     * @param data 字节数据
     * @param depth 当前嵌套深度
     * @param[out] success 是否成功解码为子消息
     * @return 解码后的QVariantMap（成功时），或原始字节数据（失败时）
     */
    QVariant tryDecodeNested(
        const QByteArray& data, int depth, bool& success) const;

    /**
     * @brief 写入varint编码到缓冲区
     * @param buf 目标缓冲区
     * @param value 要编码的值
     */
    void writeVarint(QBuffer& buf, quint64 value) const;

    /**
     * @brief 编码单个值到缓冲区（支持varint/32bit/64bit/length-delimited/嵌套）
     * @param buf 目标缓冲区
     * @param wireType wire类型
     * @param val 值
     */
    void encodeSingleValue(QBuffer& buf, int wireType, const QVariant& val) const;

    QString m_protoFilePath;  ///< .proto文件路径
    bool    m_loaded = false; ///< 是否已加载模式

    DecodeOptions m_decodeOptions = AllOptions; ///< 当前解码选项
    int m_maxNestingDepth = 8;       ///< 最大递归嵌套深度

    DecodeStatistics m_stats;        ///< 累计统计快照
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ProtobufDecoder::DecodeOptions)

#endif // PROTOBUF_DECODER_H
