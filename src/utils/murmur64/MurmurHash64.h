/**
 * @file MurmurHash64.h
 * @brief MurmurHash3 64位变体 — 高性能非加密哈希
 *
 * 功能: 实现MurmurHash3的64位输出变体，适用于哈希表、
 *       布隆过滤器和数据指纹等非密码学场景。
 *
 * 协作: BloomFilter(布隆过滤器) / DataDeduplicator(数据去重)
 */
#ifndef MURMURHASH64_H
#define MURMURHASH64_H

#include <QObject>
#include <QByteArray>

/**
 * @brief MurmurHash3 64位哈希计算器
 */
class MurmurHash64 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行时统计 */
    struct Stats {
        quint64 totalHashes       = 0;   ///< 累计哈希计算次数
        quint64 totalBytesProcessed = 0; ///< 累计处理字节数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit MurmurHash64(QObject* parent = nullptr);

    /** @brief 设置哈希种子 @param seed 32位种子值 */
    void setSeed(quint32 seed = 0);

    /**
     * @brief 计算64位MurmurHash
     * @param data 输入数据
     * @return 64位哈希值
     */
    quint64 hash64(const QByteArray& data);

    /**
     * @brief 计算32位MurmurHash
     * @param data 输入数据
     * @return 32位哈希值
     */
    quint32 hash32(const QByteArray& data);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 哈希计算完成信号 @param inputSize 输入数据大小 */
    void hashComputed(qint64 inputSize);

private:
    /**
     * @brief 64位最终混合函数
     * @param k 输入值
     * @return 混合结果
     */
    static quint64 fmix64(quint64 k);

    /**
     * @brief 64位循环左移
     * @param x 输入值
     * @param r 左移位数
     * @return 旋转结果
     */
    static quint64 rotl64(quint64 x, qint8 r);

    Stats   m_stats;
    quint32 m_seed;      ///< 哈希种子
    double  m_timeSumMs; ///< 累计耗时(用于计算均值)
};

#endif // MURMURHASH64_H
