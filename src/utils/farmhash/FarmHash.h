/**
 * @file FarmHash.h
 * @brief FarmHash指纹 — Google快速哈希算法
 *
 * 功能: 实现Google FarmHash指纹函数，基于CityHash改进，
 *       提供32/64/128位输出，适用于数据指纹和持久化哈希。
 *
 * 协作: DataDeduplicator(数据去重) / MerkleTree(默克尔树)
 */
#ifndef FARMHASH_H
#define FARMHASH_H

#include <QObject>
#include <QByteArray>
#include <QPair>

/**
 * @brief FarmHash指纹计算器 — Google快速哈希
 */
class FarmHash : public QObject {
    Q_OBJECT

public:
    /** @brief 128位哈希值表示为两个64位 */
    using Hash128 = QPair<quint64, quint64>;

    /** @brief 运行时统计 */
    struct Stats {
        quint64 totalHashes       = 0;   ///< 累计哈希计算次数
        quint64 totalBytesProcessed = 0; ///< 累计处理字节数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit FarmHash(QObject* parent = nullptr);

    /**
     * @brief 计算64位FarmHash指纹
     * @param data 输入数据
     * @return 64位哈希值
     */
    quint64 hash64(const QByteArray& data);

    /**
     * @brief 计算32位FarmHash指纹
     * @param data 输入数据
     * @return 32位哈希值
     */
    quint32 hash32(const QByteArray& data);

    /**
     * @brief 计算128位FarmHash指纹
     * @param data 输入数据
     * @return 128位哈希值(QPair<高位, 低位>)
     */
    Hash128 hash128(const QByteArray& data);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 哈希计算完成信号 @param inputSize 输入数据大小 */
    void hashComputed(qint64 inputSize);

private:
    /** @brief 64位循环左移 */
    static quint64 rotl64(quint64 x, qint8 r);

    /** @brief 64位弱哈希混合 */
    static quint64 weakHash32Seeds(quint64 a, quint64 b, quint64 c);

    /** @brief 中等长度数据的64位哈希 (17~64字节) */
    quint64 hashMedium(const char* data, int len) const;

    /** @brief 长数据的64位哈希 (>64字节) */
    quint64 hashLong(const char* data, int len) const;

    /** @brief 短数据的32位哈希 (<=24字节) */
    static quint32 hash32Len0to24(const char* data, int len);

    /** @brief CityHash风格的混合步骤 */
    static quint64 hash128to64(const Hash128& x);

    Stats   m_stats;
    double  m_timeSumMs; ///< 累计耗时(用于计算均值)
};

#endif // FARMHASH_H
