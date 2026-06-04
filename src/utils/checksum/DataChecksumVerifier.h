/**
 * @file DataChecksumVerifier.h
 * @brief 数据校验验证引擎 -- 多算法数据完整性验证
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 支持17种校验算法(CRC/XOR/Sum/补码/Fletcher/Adler)，
 * 单算法验证或全算法批量验证，带计算耗时统计和CSV结果导出。
 */

#ifndef DATACHECKSUMVERIFIER_H
#define DATACHECKSUMVERIFIER_H

#include <QByteArray>
#include <QElapsedTimer>
#include <QList>
#include <QObject>
#include <QString>
#include <QtGlobal>

/**
 * @class DataChecksumVerifier
 * @brief 数据完整性验证引擎
 *
 * 对字节数据执行校验和计算与验证，支持17种算法。
 * 典型用法: 调试通信协议中校验和问题时，对同一帧数据
 * 运行全部算法，快速定位协议实际使用的校验方式。
 */
class DataChecksumVerifier : public QObject
{
    Q_OBJECT

public:
    /** @brief 校验算法枚举，共17种 */
    enum class Algorithm {
        CRC8,              ///< CRC-8 (多项式0x07)
        CRC16_CCITT,       ///< CRC-16/CCITT (多项式0x1021)
        CRC16_MODBUS,      ///< CRC-16/Modbus (多项式0xA001)
        CRC16_XMODEM,      ///< CRC-16/XMODEM (多项式0x1021, 初始0x0000)
        CRC32,             ///< CRC-32 (以太网/ZIP标准)
        CRC32C,            ///< CRC-32C Castagnoli (iSCSI标准)
        XOR8,              ///< 8位异或
        Sum8,              ///< 8位累加和
        Sum16_LE,          ///< 16位小端累加和
        Sum16_BE,          ///< 16位大端累加和
        Sum32_LE,          ///< 32位小端累加和
        OnesComplement16,  ///< 16位一进制反码求和(IP/TCP/UDP校验)
        TwosComplement16,  ///< 16位二进制补码求和
        Fletcher8,         ///< Fletcher-8 校验
        Fletcher16,        ///< Fletcher-16 校验
        Fletcher32,        ///< Fletcher-32 校验
        Adler32            ///< Adler-32 校验(zlib使用)
    };
    Q_ENUM(Algorithm)

    /** @brief 单次验证结果 */
    struct VerificationResult {
        Algorithm algorithm;     ///< 使用的校验算法
        quint64 expectedValue;   ///< 期望的校验值
        quint64 computedValue;   ///< 实际计算的校验值
        bool match;              ///< 是否匹配
        int dataLength;          ///< 参与计算的数据长度(字节)
        double computationTimeUs;///< 计算耗时(微秒)
    };

    /** @brief 累计统计信息 */
    struct Stats {
        quint64 totalVerifications;                    ///< 总验证次数
        quint64 totalPasses;                           ///< 匹配次数
        quint64 totalFailures;                         ///< 不匹配次数
        quint64 totalBytesProcessed;                   ///< 累计处理字节数
        double avgComputationTimeUs;                   ///< 平均单次计算耗时(微秒)
        quint64 computationsByAlgorithm[17];           ///< 按算法统计计算次数(索引=Algorithm枚举值)
    };

    /** @brief 构造数据校验验证引擎 @param parent 父对象 */
    explicit DataChecksumVerifier(QObject *parent = nullptr);

    /** @brief 静态计算校验和(无需实例，无统计) @param algo 算法 @param data 数据 @param startPos 起始偏移(-1表示0) @param length 数据长度(-1表示到末尾) @return 校验和结果 */
    static quint64 compute(Algorithm algo, const QByteArray &data,
                           int startPos = 0, int length = -1);

    /** @brief 验证单算法校验和 @param algo 算法 @param data 数据 @param expectedValue 期望值 @param startPos 起始偏移 @param length 数据长度 @return 验证结果(含耗时) */
    VerificationResult verify(Algorithm algo, const QByteArray &data,
                              quint64 expectedValue,
                              int startPos = 0, int length = -1);

    /** @brief 对同一数据运行全部17种算法验证 @param data 数据 @param expectedValue 期望值 @param startPos 起始偏移 @param length 数据长度 @return 每种算法的验证结果列表 */
    QList<VerificationResult> verifyAll(const QByteArray &data,
                                        quint64 expectedValue,
                                        int startPos = 0, int length = -1);

    /** @brief 计算校验和并返回原始字节(按算法位宽截断) @param algo 算法 @param data 数据 @param startPos 起始偏移 @param length 数据长度 @return 校验和的原始字节(大端序) */
    QByteArray computeBytes(Algorithm algo, const QByteArray &data,
                            int startPos = 0, int length = -1);

    /** @brief 获取算法的标准名称 @param algo 算法枚举 @return 如"CRC-8"/"Fletcher-16"等 */
    static QString algorithmName(Algorithm algo);

    /** @brief 获取算法输出位宽 @param algo 算法枚举 @return 8/16/32 */
    static int algorithmWidth(Algorithm algo);

    /** @brief 获取全部17种算法枚举列表 @return 按定义顺序的算法列表 */
    static QList<Algorithm> allAlgorithms();

    /** @brief 导出验证结果到CSV文件 @param filePath 文件路径 @param results 验证结果列表 @return true=成功 */
    bool exportResults(const QString &filePath,
                       const QList<VerificationResult> &results);

    /** @brief 获取累计统计 @return 统计结构的const引用 */
    const Stats &stats() const;

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 单次验证完成 @param result 验证结果 */
    void verificationComplete(const VerificationResult &result);

    /** @brief 全部算法验证完成 @param results 所有验证结果 */
    void allVerificationsComplete(const QList<VerificationResult> &results);

private:
    Stats m_stats; ///< 累计统计

    /** @brief CRC-8 计算(多项式0x07, 初始0x00) */
    static quint64 computeCRC8(const uint8_t *data, int len);
    /** @brief CRC-16/CCITT 计算(多项式0x1021, 初始0xFFFF) */
    static quint64 computeCRC16_CCITT(const uint8_t *data, int len);
    /** @brief CRC-16/Modbus 计算(多项式0xA001, 初始0xFFFF) */
    static quint64 computeCRC16_MODBUS(const uint8_t *data, int len);
    /** @brief CRC-16/XMODEM 计算(多项式0x1021, 初始0x0000) */
    static quint64 computeCRC16_XMODEM(const uint8_t *data, int len);
    /** @brief CRC-32 计算(标准以太网多项式) */
    static quint64 computeCRC32(const uint8_t *data, int len);
    /** @brief CRC-32C 计算(Castagnoli多项式) */
    static quint64 computeCRC32C(const uint8_t *data, int len);
    /** @brief 8位异或计算 */
    static quint64 computeXOR8(const uint8_t *data, int len);
    /** @brief 通用累加和计算 @param bits 位宽(8/16/32) @param bigEndian 是否大端序 */
    static quint64 computeSum(const uint8_t *data, int len, int bits, bool bigEndian);
    /** @brief Fletcher校验计算 @param bits 位宽(8/16/32) */
    static quint64 computeFletcher(const uint8_t *data, int len, int bits);
    /** @brief Adler-32 校验计算 */
    static quint64 computeAdler32(const uint8_t *data, int len);
    /** @brief OnesComplement16 计算(16位反码求和) */
    static quint64 computeOnesComplement16(const uint8_t *data, int len);
    /** @brief TwosComplement16 计算(16位补码求和) */
    static quint64 computeTwosComplement16(const uint8_t *data, int len);
};

#endif // DATACHECKSUMVERIFIER_H
