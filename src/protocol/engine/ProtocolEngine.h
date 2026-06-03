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

    /** @brief 设置协议帧结构定义 @param schema 已加载的 ProtocolSchema 指针 */
    void setSchema(ProtocolSchema *schema);

    /** @brief 向引擎喂入新的串口数据 @param data 原始字节流 */
    void feedData(const QByteArray &data);

    /** @brief 重置解析状态，清空内部缓冲区 */
    void reset();

    /** @brief 获取当前协议定义 @return 协议定义指针，未设置时为 nullptr */
    ProtocolSchema *currentSchema() const;

    /* —— 统计计数器接口 —— */

    /** @brief 获取已成功解析的帧数(兼容int) @return 成功解析帧计数 */
    int framesParsed() const;

    /** @brief 获取解析错误次数(兼容int) @return 解析错误计数 */
    int parseErrors() const;

    /** @brief 获取已成功解析的帧数(64位) @return 成功解析帧计数 */
    quint64 framesParsedCount() const;

    /** @brief 获取因校验/验证失败被拒绝的帧数 @return 被拒绝帧计数 */
    quint64 framesRejected() const;

    /** @brief 获取引擎累计处理的字节总数 @return 字节总数 */
    quint64 totalBytesProcessed() const;

    /** @brief 获取校验验证执行总次数(含通过和失败) @return 验证总次数 */
    quint64 totalValidations() const;

    /** @brief 获取解析错误总数(64位，含校验失败/格式错/溢出) @return 错误总数 */
    quint64 totalParseErrors() const;

    /** @brief 获取最后一次成功解析的时间戳 @return 毫秒级时间戳，未解析过返回0 */
    qint64 lastParseTimestamp() const;

    /** @brief 获取已处理的数据包总数（含成功和失败） @return 数据包总数 */
    quint64 totalPacketsProcessed() const;

    /** @brief 获取已解析的字节总数（仅成功解析的帧内字节） @return 字节总数 */
    quint64 totalBytesParsed() const;

    /** @brief 获取CRC校验错误次数 @return CRC错误计数 */
    quint64 totalCrcErrors() const;

    /** @brief 重置所有解析统计计数器(帧数/错误/字节/时间戳) */
    void resetParseStatistics();

    /** @brief 重置所有统计计数器(等同于resetParseStatistics) */
    void resetEngineStatistics();

    /** @brief 重置所有统计计数器(别名，调用resetEngineStatistics) */
    void resetStats();

signals:
    /** @brief 帧解析完成信号 @param fields 字段名->字段值映射 @param rawData 原始帧字节 */
    void frameParsed(const QVariantMap &fields, const QByteArray &rawData);

    /** @brief 解析错误信号 @param error 错误描述 */
    void parseError(const QString &error);

private:
    ProtocolSchema *m_schema = nullptr;     ///< 当前协议定义
    QByteArray m_buffer;                    ///< 内部接收缓冲区
    int m_parseErrors = 0;                  ///< 解析错误计数(兼容旧接口)

    quint64 m_framesParsed = 0;             ///< 成功解析帧计数
    quint64 m_framesRejected = 0;           ///< 因校验/验证失败被拒绝的帧计数
    quint64 m_totalBytesProcessed = 0;      ///< 引擎累计处理的总字节数
    quint64 m_totalValidations = 0;         ///< 校验验证执行总次数
    quint64 m_totalParseErrors = 0;         ///< 解析错误总数(64位)
    qint64 m_lastParseTimestamp = 0;        ///< 最后一次成功解析的时间戳(ms)
    quint64 m_totalCrcErrors = 0;           ///< CRC校验错误次数
    quint64 m_totalBytesParsed = 0;         ///< 已解析的字节总数（仅成功解析的帧内字节）

    bool tryParseOneFrame();
    int findHeader(const QByteArray &buffer, const QVector<int> &header) const;
    void trimBufferBeforePartialHeader(const QVector<int> &header);
    int readLengthField(const QByteArray &buffer, int offset, int size) const;
    QVariant extractField(const QByteArray &frame,
                           const ProtocolSchema::FieldDefinition &field) const;
    bool validateChecksum(const QByteArray &frame,
                          const ProtocolSchema::FramingRule &framing) const;
    static quint8 computeCrc8(const QByteArray &data, quint8 polynomial = 0x07);
    static quint16 computeCrc16Ccitt(const QByteArray &data);
    static quint16 computeCrc16Modbus(const QByteArray &data);
    static quint32 computeCrc32(const QByteArray &data);
    static quint8 computeXor(const QByteArray &data);
};

#endif // PROTOCOL_ENGINE_H
