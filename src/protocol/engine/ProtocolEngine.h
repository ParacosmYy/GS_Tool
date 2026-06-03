/**
 * @file ProtocolEngine.h
 * @brief 自定义协议解析引擎
 *
 * 接收原始串口字节流，根据 ProtocolSchema 定义的帧格式
 * 自动完成帧同步、长度解析、校验及字段提取。
 */

#ifndef PROTOCOL_ENGINE_H
#define PROTOCOL_ENGINE_H

#include <QObject>
#include <QByteArray>
#include <QVariantMap>
#include <QVector>

#include "protocol/schema/ProtocolSchema.h"

/**
 * @class ProtocolEngine
 * @brief 协议帧解析引擎
 *
 * 持续接收串口数据，按当前 ProtocolSchema 完成帧检测，
 * 解析成功后发射 frameParsed 信号，失败时发射 parseError。
 */
class ProtocolEngine : public QObject
{
    Q_OBJECT

public:
    /** @brief 构造函数 */
    explicit ProtocolEngine(QObject *parent = nullptr);

    /** @brief 析构函数 */
    ~ProtocolEngine() override;

    /**
     * @brief 设置协议帧结构定义
     * @param schema 指向已加载的 ProtocolSchema 对象
     */
    void setSchema(ProtocolSchema *schema);

    /**
     * @brief 向引擎喂入新的串口数据
     * @param data 新接收到的原始字节流
     */
    void feedData(const QByteArray &data);

    /**
     * @brief 重置解析状态，清空内部缓冲区
     */
    void reset();

    /**
     * @brief 获取当前使用的协议定义
     * @return 协议定义指针，未设置时为 nullptr
     */
    ProtocolSchema *currentSchema() const;

    /**
     * @brief 获取已成功解析的帧数
     * @return 成功解析帧计数
     */
    int framesParsed() const;

    /**
     * @brief 获取解析错误次数
     * @return 解析错误计数
     */
    int parseErrors() const;

    /**
     * @brief 获取已成功解析的帧数（64位）
     * @return 成功解析帧计数
     */
    quint64 framesParsedCount() const;

    /**
     * @brief 获取因验证失败而被拒绝的帧数
     * @return 被拒绝帧计数
     */
    quint64 framesRejected() const;

    /**
     * @brief 获取引擎处理的总字节数
     * @return 累计处理的字节总数
     */
    quint64 totalBytesProcessed() const;

    /**
     * @brief 获取最后一次成功解析的时间戳
     * @return 毫秒级时间戳（自Unix纪元起），尚未解析过时返回0
     */
    qint64 lastParseTimestamp() const;

    /**
     * @brief 重置所有解析统计计数器
     *
     * 将帧计数、拒绝计数、字节总数和时间戳全部归零。
     * 不影响当前 schema 设置和缓冲区内容。
     */
    void resetParseStatistics();

signals:
    /**
     * @brief 帧解析完成信号
     * @param fields 字段名→字段值的映射
     * @param rawData 完整的原始帧字节
     */
    void frameParsed(const QVariantMap &fields, const QByteArray &rawData);

    /**
     * @brief 解析错误信号
     * @param error 错误描述
     */
    void parseError(const QString &error);

private:
    ProtocolSchema *m_schema = nullptr;     ///< 当前协议定义
    QByteArray m_buffer;                    ///< 内部接收缓冲区
    int m_parseErrors = 0;                  ///< 解析错误计数（兼容旧接口）

    quint64 m_framesParsed = 0;             ///< 成功解析帧计数
    quint64 m_framesRejected = 0;           ///< 因校验/验证失败被拒绝的帧计数
    quint64 m_totalBytesProcessed = 0;      ///< 引擎累计处理的总字节数
    qint64 m_lastParseTimestamp = 0;        ///< 最后一次成功解析的时间戳（ms since epoch）

    /**
     * @brief 尝试从缓冲区解析一帧
     * @return true 成功提取一帧，false 数据不足
     */
    bool tryParseOneFrame();

    /**
     * @brief 在缓冲区中查找帧头字节序列
     * @param buffer 待搜索缓冲区
     * @param header 帧头字节序列
     * @return 帧头起始位置，未找到返回 -1
     */
    int findHeader(const QByteArray &buffer, const QVector<int> &header) const;

    /**
     * @brief 丢弃不可能构成帧头的垃圾数据，保留可能的尾部部分匹配
     * @param header 帧头字节序列
     */
    void trimBufferBeforePartialHeader(const QVector<int> &header);

    /**
     * @brief 从帧数据读取长度字段（小端序）
     * @param buffer 帧缓冲区
     * @param offset 偏移
     * @param size 字节数
     * @return 长度值，无效返回 -1
     */
    int readLengthField(const QByteArray &buffer, int offset, int size) const;

    /**
     * @brief 从帧数据中提取单个字段值
     * @param frame 原始帧
     * @param field 字段定义
     * @return 提取的字段值
     */
    QVariant extractField(const QByteArray &frame,
                           const ProtocolSchema::FieldDefinition &field) const;

    /**
     * @brief 验证帧校验和/CRC
     * @param frame 完整帧数据(含帧头到校验字段)
     * @param framing 帧格式定义
     * @return true=校验通过, false=校验失败
     */
    bool validateChecksum(const QByteArray &frame,
                          const ProtocolSchema::FramingRule &framing) const;

    /**
     * @brief 计算CRC-8校验值
     * @param data 待校验数据
     * @param polynomial 多项式(默认0x07)
     * @return CRC-8值
     */
    static quint8 computeCrc8(const QByteArray &data, quint8 polynomial = 0x07);

    /**
     * @brief 计算CRC-16 CCITT校验值
     * @param data 待校验数据
     * @return CRC-16 CCITT值
     */
    static quint16 computeCrc16Ccitt(const QByteArray &data);

    /**
     * @brief 计算CRC-16 Modbus校验值
     * @param data 待校验数据
     * @return CRC-16 Modbus值
     */
    static quint16 computeCrc16Modbus(const QByteArray &data);

    /**
     * @brief 计算CRC-32校验值
     * @param data 待校验数据
     * @return CRC-32值
     */
    static quint32 computeCrc32(const QByteArray &data);

    /**
     * @brief 计算异或校验值
     * @param data 待校验数据
     * @return 异或结果
     */
    static quint8 computeXor(const QByteArray &data);
};

#endif // PROTOCOL_ENGINE_H
