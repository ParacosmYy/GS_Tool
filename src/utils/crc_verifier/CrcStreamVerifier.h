/**
 * @file CrcStreamVerifier.h
 * @brief CRC流验证器 — 实时CRC校验数据流完整性
 *
 * 功能: 对数据流逐块计算CRC，与预期值对比，实时报告校验结果。
 *       支持8种CRC算法，滑动窗口验证，错误定位。
 *
 * 协作: IConnection(数据接收验证) / ProtocolEngine(帧校验)
 */
#ifndef CRCSTREAMVERIFIER_H
#define CRCSTREAMVERIFIER_H

#include <QObject>
#include <QByteArray>
#include <QMap>

/**
 * @brief CRC流验证器 — 实时校验数据流完整性
 */
class CrcStreamVerifier : public QObject {
    Q_OBJECT

public:
    /** @brief CRC算法 */
    enum class CrcAlgorithm {
        Crc8,           ///< CRC-8
        Crc16,          ///< CRC-16
        Crc16Modbus,    ///< CRC-16 Modbus
        Crc16Ccitt,     ///< CRC-16 CCITT
        Crc32,          ///< CRC-32
        Crc32C,         ///< CRC-32C (Castagnoli)
        Xor8,           ///< XOR-8
        Checksum8       ///< 算术和模256
    };
    Q_ENUM(CrcAlgorithm)

    /** @brief 验证结果 */
    struct VerifyResult {
        int blockIndex = 0;         ///< 块索引
        qint64 byteOffset = 0;      ///< 字节偏移
        QByteArray data;            ///< 原始数据
        quint32 computedCrc = 0;    ///< 计算的CRC
        quint32 expectedCrc = 0;    ///< 预期的CRC
        bool passed = false;        ///< 是否通过
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalBlocksVerified = 0;///< 累计验证块数
        quint64 totalBytesVerified = 0; ///< 累计验证字节数
        quint64 totalPasses = 0;        ///< 累计通过次数
        quint64 totalFailures = 0;      ///< 累计失败次数
        int     blockSize = 0;          ///< 当前块大小
        int     peakFailures = 0;       ///< 峰值连续失败数
    };

    explicit CrcStreamVerifier(QObject* parent = nullptr);

    /** @brief 设置CRC算法 @param algo 算法 */
    void setAlgorithm(CrcAlgorithm algo);

    /** @brief 设置验证块大小 @param size 块大小(字节) */
    void setBlockSize(int size);

    /** @brief 喂入数据流 @param data 数据 */
    void feedData(const QByteArray& data);

    /** @brief 设置下一个预期CRC值 @param expected 预期值 */
    void setExpectedCrc(quint32 expected);

    /** @brief 手动验证一个块 @param data 数据 @param expected 预期CRC @return 验证结果 */
    VerifyResult verifyBlock(const QByteArray& data, quint32 expected);

    /** @brief 计算CRC @param data 数据 @return CRC值 */
    quint32 computeCrc(const QByteArray& data) const;

    /** @brief 获取验证历史 @return 最近N条验证结果 */
    QList<VerifyResult> history() const;

    /** @brief 清除缓冲和状态 */
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 块验证通过 @param blockIndex 块索引 @param crc CRC值 */
    void blockVerified(int blockIndex, quint32 crc);
    /** @brief 块验证失败 @param result 验证结果 */
    void blockFailed(const VerifyResult& result);
    /** @brief 连续失败告警 @param count 连续失败次数 */
    void consecutiveFailures(int count);

private:
    quint32 computeCrc8(const QByteArray& data) const;
    quint32 computeCrc16(const QByteArray& data) const;
    quint32 computeCrc16Modbus(const QByteArray& data) const;
    quint32 computeCrc32(const QByteArray& data) const;
    quint32 computeXor8(const QByteArray& data) const;
    quint32 computeChecksum8(const QByteArray& data) const;

    CrcAlgorithm m_algorithm;       ///< 当前算法
    int m_blockSize;                ///< 块大小
    QByteArray m_buffer;            ///< 接收缓冲区
    quint32 m_nextExpected;         ///< 下一个预期CRC
    int m_blockIndex;               ///< 当前块索引
    int m_consecutiveFailures;      ///< 连续失败计数
    QList<VerifyResult> m_history;  ///< 验证历史(最近100条)

    Stats m_stats;
};

#endif // CRCSTREAMVERIFIER_H
