/**
 * @file ProtocolEngine.h
 * @brief 自定义协议解析引擎
 *
 * 接收原始串口字节流，根据 ProtocolSchema 定义的帧格式
 * 自动完成帧同步、长度解析、校验及字段提取。
 * 支持多种CRC校验算法，可通过 setChecksumAlgorithm() 运行时切换。
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
 * CRC校验: 帧接收完成后自动触发，通过crcPassCount统计，
 * 失败计入crcFailCount并发射parseError和checksumFailed信号。
 */
class ProtocolEngine : public QObject
{
    Q_OBJECT

public:
    /** @brief 校验算法枚举，用于运行时切换校验方式 */
    enum class ChecksumAlgorithm : int {
        Auto = 0,       ///< 自动：使用schema中定义的校验算法
        Crc8,           ///< CRC-8（多项式0x07）
        Crc16Modbus,    ///< CRC-16 Modbus（多项式0x8005，小端序）
        Crc16Ccitt,     ///< CRC-16 CCITT（多项式0x1021）
        Crc32,          ///< CRC-32（多项式0xEDB88320，兼容ZIP/PNG）
        Xor,            ///< 异或校验（所有字节逐个XOR）
        Sum,            ///< 累加和校验（所有字节求和取低8位）
        None            ///< 不进行校验
    };
    Q_ENUM(ChecksumAlgorithm)

    /** @brief 构造函数 @param parent 父对象指针 */
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

    /* —— 校验算法配置接口 —— */

    /** @brief 设置校验算法(覆盖schema定义) @param algo 算法名称: crc8/crc16_modbus/crc16_ccitt/crc32/xor/sum/none/auto */
    void setChecksumAlgorithm(const QString &algo);
    /** @brief 获取当前配置的校验算法名称 @return 算法名称字符串 */
    QString checksumAlgorithm() const;
    /** @brief 获取当前生效的校验算法枚举值(考虑Auto回退和schema设置) @return ChecksumAlgorithm 枚举值 */
    ChecksumAlgorithm activeChecksumAlgorithm() const;
    /**
     * @brief 独立校验接口：验证给定数据的校验和
     * @param data 包含 payload + checksum 的完整字节数据
     * @return 校验通过返回 true，校验失败或算法无效返回 false
     * @note 使用当前配置的算法进行验证，Auto时默认CRC-16 Modbus，统计不计入引擎内部计数
     */
    bool verifyChecksum(const QByteArray &data) const;

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
    /** @brief 获取解析错误总数(64位) @return 错误总数 */
    quint64 totalParseErrors() const;
    /** @brief 获取最后一次成功解析的时间戳 @return 毫秒级时间戳，未解析过返回0 */
    qint64 lastParseTimestamp() const;
    /** @brief 获取已处理的数据包总数（含成功和失败） @return 数据包总数 */
    quint64 totalPacketsProcessed() const;
    /** @brief 获取已解析的字节总数（仅成功解析的帧内字节） @return 字节总数 */
    quint64 totalBytesParsed() const;
    /** @brief 获取CRC校验错误次数 @return CRC错误计数 */
    quint64 totalCrcErrors() const;
    /** @brief 获取CRC校验通过次数 @return 校验通过计数 */
    quint64 crcPassCount() const;
    /** @brief 获取CRC校验失败次数 @return 校验失败计数 */
    quint64 crcFailCount() const;
    /** @brief 获取CRC校验通过率(0.0~1.0) @return 通过率，总验证次数为0时返回0.0 */
    double crcPassRate() const;
    /** @brief 重置所有解析统计计数器(帧数/错误/字节/时间戳/CRC统计) */
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
    /** @brief CRC校验失败信号 @param expected 期望校验值 @param actual 实际校验值 @param algorithm 算法名称 */
    void checksumFailed(quint64 expected, quint64 actual, const QString &algorithm);
    /** @brief 校验算法变更信号 @param newAlgorithm 新的算法名称 */
    void checksumAlgorithmChanged(const QString &newAlgorithm);

private:
    ProtocolSchema *m_schema = nullptr;                 ///< 当前协议定义
    QByteArray m_buffer;                                ///< 内部接收缓冲区
    int m_parseErrors = 0;                              ///< 解析错误计数(兼容旧接口)
    ChecksumAlgorithm m_checksumAlgorithm = ChecksumAlgorithm::Auto; ///< 运行时校验算法覆盖

    quint64 m_framesParsed = 0;             ///< 成功解析帧计数
    quint64 m_framesRejected = 0;           ///< 因校验/验证失败被拒绝的帧计数
    quint64 m_totalBytesProcessed = 0;      ///< 引擎累计处理的总字节数
    quint64 m_totalValidations = 0;         ///< 校验验证执行总次数
    quint64 m_totalParseErrors = 0;         ///< 解析错误总数(64位)
    qint64 m_lastParseTimestamp = 0;        ///< 最后一次成功解析的时间戳(ms)
    quint64 m_totalCrcErrors = 0;           ///< CRC校验错误次数
    quint64 m_totalBytesParsed = 0;         ///< 已解析的字节总数（仅成功解析的帧内字节）
    quint64 m_crcPassCount = 0;             ///< CRC校验通过次数
    quint64 m_crcFailCount = 0;             ///< CRC校验失败次数

    /** @brief 尝试从缓冲区中解析一帧 @return 成功解析返回true */
    bool tryParseOneFrame();
    /** @brief 在缓冲区中搜索帧头位置 @param buffer 数据缓冲区 @param header 帧头字节序列 @return 帧头起始位置，未找到返回-1 */
    int findHeader(const QByteArray &buffer, const QVector<int> &header) const;
    /** @brief 清除缓冲区中不完整帧头之前的数据 @param header 帧头字节序列 */
    void trimBufferBeforePartialHeader(const QVector<int> &header);
    /** @brief 读取长度字段值(小端序) @param buffer 数据缓冲区 @param offset 长度字段偏移 @param size 长度字段字节数 @return 解析得到的长度值 */
    int readLengthField(const QByteArray &buffer, int offset, int size) const;
    /** @brief 从帧数据中提取单个字段值 @param frame 完整帧数据 @param field 字段定义 @return 字段值 */
    QVariant extractField(const QByteArray &frame, const ProtocolSchema::FieldDefinition &field) const;
    /** @brief 验证帧校验和(内部，支持算法覆盖，回填期望/实际值) @param frame 完整帧数据 @param framing 帧定界规则 @param expectedVal 回填期望值 @param actualVal 回填实际值 @return 校验通过返回true */
    bool validateChecksum(const QByteArray &frame, const ProtocolSchema::FramingRule &framing,
                          quint64 *expectedVal = nullptr, quint64 *actualVal = nullptr) const;
    /** @brief 确定当前生效的校验算法(手动覆盖优先) @param framing 帧定界规则 @return 实际生效的算法枚举 */
    ChecksumAlgorithm resolveEffectiveAlgorithm(const ProtocolSchema::FramingRule &framing) const;
    /** @brief 校验算法枚举→字符串 @param algo 算法枚举 @return 字符串标识 */
    static QString checksumAlgorithmToString(ChecksumAlgorithm algo);
    /** @brief 字符串→校验算法枚举 @param str 算法字符串 @return 对应枚举值，不匹配返回Auto */
    static ChecksumAlgorithm checksumAlgorithmFromString(const QString &str);
    /** @brief 获取校验算法输出的字节宽度 @param algo 算法枚举 @return 字节宽度(0/1/2/4) */
    static int checksumSize(ChecksumAlgorithm algo);
    /** @brief 计算CRC-8校验值 @param data 待计算数据 @param polynomial 生成多项式 @return CRC-8校验值 */
    static quint8 computeCrc8(const QByteArray &data, quint8 polynomial = 0x07);
    /** @brief 计算CRC-16 CCITT校验值 @param data 待计算数据 @return CRC-16校验值 */
    static quint16 computeCrc16Ccitt(const QByteArray &data);
    /** @brief 计算CRC-16 Modbus校验值 @param data 待计算数据 @return CRC-16 Modbus校验值 */
    static quint16 computeCrc16Modbus(const QByteArray &data);
    /** @brief 计算CRC-32校验值 @param data 待计算数据 @return CRC-32校验值 */
    static quint32 computeCrc32(const QByteArray &data);
    /** @brief 计算异或校验值 @param data 待计算数据 @return 异或校验值 */
    static quint8 computeXor(const QByteArray &data);
    /** @brief 计算累加和校验值 @param data 待计算数据 @return 累加和低8位 */
    static quint8 computeSum(const QByteArray &data);
    /** @brief 使用指定算法计算校验值(统一入口) @param data 待计算数据 @param algo 校验算法 @return 校验值(quint64封装) */
    static quint64 computeChecksumForAlgorithm(const QByteArray &data, ChecksumAlgorithm algo);
};

#endif // PROTOCOL_ENGINE_H
