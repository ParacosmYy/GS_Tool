/** @file ProtocolEngine.h @brief 自定义协议解析引擎。接收原始串口字节流，根据ProtocolSchema定义的帧格式自动完成帧同步、长度解析、校验及字段提取 */

#ifndef PROTOCOL_ENGINE_H
#define PROTOCOL_ENGINE_H

#include <QObject>
#include <QByteArray>
#include <QVariantMap>
#include <QVector>

#include "protocol/schema/ProtocolSchema.h"

/** @brief 协议帧解析引擎。持续接收串口数据按ProtocolSchema完成帧检测，CRC校验自动触发 */
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

    /** @brief 构造协议解析引擎 @param parent 父对象 */
    explicit ProtocolEngine(QObject *parent = nullptr);
    /** @brief 析构函数 */
    ~ProtocolEngine() override;
    /** @brief 设置协议帧结构定义 @param schema ProtocolSchema指针，定义帧格式规则 */
    void setSchema(ProtocolSchema *schema);
    /** @brief 喂入新的串口数据，内部缓冲并尝试帧解析 @param data 原始串口字节数据 */
    void feedData(const QByteArray &data);
    /** @brief 重置解析状态，清空内部缓冲区 */
    void reset();
    /** @brief 获取当前协议定义 @return 当前ProtocolSchema指针，未设置时为nullptr */
    ProtocolSchema *currentSchema() const;

    /* -- 校验算法配置 -- */
    void setChecksumAlgorithm(const QString &algo); ///< 设置校验算法(覆盖schema)
    QString checksumAlgorithm() const;       ///< 获取当前校验算法名称
    ChecksumAlgorithm activeChecksumAlgorithm() const; ///< 获取当前生效的算法(考虑Auto回退)
    bool verifyChecksum(const QByteArray &data) const; ///< 独立校验接口(统计不计入内部计数)

    /* -- 统计计数器 -- */
    int framesParsed() const;                ///< 成功解析帧数(兼容int)
    int parseErrors() const;                 ///< 解析错误次数(兼容int)
    quint64 framesParsedCount() const;       ///< 成功解析帧数(64位)
    quint64 framesRejected() const;          ///< 被拒绝帧数(校验/验证失败)
    quint64 totalBytesProcessed() const;     ///< 累计处理字节总数
    quint64 totalValidations() const;        ///< 校验验证执行总次数
    quint64 totalValidationPasses() const;   ///< 校验验证通过次数
    quint64 totalValidationFailures() const; ///< 校验验证失败次数
    quint64 totalCrcChecks() const;          ///< CRC校验执行总次数
    quint64 totalParseErrors() const;        ///< 解析错误总数(64位)
    qint64 lastParseTimestamp() const;       ///< 最后一次成功解析时间戳(ms)
    quint64 totalPacketsProcessed() const;   ///< 已处理数据包总数
    quint64 totalBytesParsed() const;        ///< 已解析字节总数(仅成功帧)
    quint64 totalCrcErrors() const;          ///< CRC校验错误次数
    quint64 crcPassCount() const;            ///< CRC校验通过次数
    quint64 crcFailCount() const;            ///< CRC校验失败次数
    double crcPassRate() const;              ///< CRC校验通过率(0.0~1.0)
    quint64 totalParses() const;             ///< 帧解析尝试总次数(tryParseOneFrame调用)
    quint64 totalChecksums() const;          ///< 校验和计算验证总次数(validateChecksum调用)
    quint64 totalMatches() const;            ///< 帧头匹配成功总次数
    quint64 totalBuilds() const;             ///< 数据注入总次数(feedData调用)
    void resetParseStatistics();             ///< 重置解析统计
    void resetEngineStatistics();            ///< 重置所有统计(等同resetParseStatistics)
    void resetStats();                       ///< 别名(调用resetEngineStatistics)

signals:
    void frameParsed(const QVariantMap &fields, const QByteArray &rawData); ///< 帧解析完成
    void parseError(const QString &error);   ///< 解析错误
    void checksumFailed(quint64 expected, quint64 actual, const QString &algorithm); ///< CRC校验失败
    void checksumAlgorithmChanged(const QString &newAlgorithm); ///< 校验算法变更

private:
    ProtocolSchema *m_schema = nullptr;                 ///< 当前协议定义
    QByteArray m_buffer;                                ///< 内部接收缓冲区
    int m_parseErrors = 0;                              ///< 解析错误计数(兼容旧接口)
    ChecksumAlgorithm m_checksumAlgorithm = ChecksumAlgorithm::Auto; ///< 运行时校验算法覆盖

    quint64 m_framesParsed = 0;             ///< 成功解析帧计数
    quint64 m_framesRejected = 0;           ///< 因校验/验证失败被拒绝的帧计数
    quint64 m_totalBytesProcessed = 0;      ///< 引擎累计处理的总字节数
    quint64 m_totalValidations = 0;         ///< 校验验证执行总次数
    quint64 m_totalValidationPasses = 0;    ///< 校验验证通过次数
    quint64 m_totalValidationFailures = 0;  ///< 校验验证失败次数
    quint64 m_totalCrcChecks = 0;           ///< CRC校验执行总次数
    quint64 m_totalParseErrors = 0;         ///< 解析错误总数(64位)
    qint64 m_lastParseTimestamp = 0;        ///< 最后一次成功解析的时间戳(ms)
    quint64 m_totalCrcErrors = 0;           ///< CRC校验错误次数
    quint64 m_totalBytesParsed = 0;         ///< 已解析的字节总数（仅成功解析的帧内字节）
    quint64 m_crcPassCount = 0;             ///< CRC校验通过次数
    quint64 m_crcFailCount = 0;             ///< CRC校验失败次数
    quint64 m_totalParses = 0;              ///< 帧解析尝试总次数(tryParseOneFrame调用)
    mutable quint64 m_totalChecksums = 0;   ///< 校验和计算验证总次数(validateChecksum调用)
    quint64 m_totalMatches = 0;             ///< 帧头匹配成功总次数
    quint64 m_totalBuilds = 0;              ///< 数据注入总次数(feedData调用)

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
