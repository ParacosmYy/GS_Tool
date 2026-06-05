/**
 * @file WyHash.h
 * @brief WyHash 快速哈希函数
 *
 * 实现 WyHash 快速非加密哈希算法，基于 wyrand PRNG，
 * 提供极高速的 64 位哈希输出。适用于哈希表、布隆过滤器、
 * 数据去重等对性能敏感的场景。
 *
 * 协作: BloomFilter(布隆过滤器) / HashMap(哈希表)
 */
#ifndef WYHASH_H
#define WYHASH_H

#include <QByteArray>
#include <QElapsedTimer>
#include <QObject>
#include <QtGlobal>

/**
 * @class WyHash
 * @brief WyHash 快速非加密哈希引擎
 */
class WyHash : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息结构体 */
    struct Stats {
        quint64 totalHashes = 0;           ///< 累计哈希计算次数
        quint64 totalBytesProcessed = 0;   ///< 累计处理字节数
        double avgProcessingTimeMs = 0.0;  ///< 平均处理时间(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit WyHash(QObject* parent = nullptr);

    /** @brief 设置哈希种子 @param seed 64位种子值 */
    void setSeed(quint64 seed);

    /**
     * @brief 计算哈希值(WyHash 最终版)
     * @param data 输入数据
     * @return 64 位哈希值
     */
    quint64 hash(const QByteArray& data);

    /**
     * @brief 计算哈希值(64位带种子变体)
     * @param data 输入数据
     * @return 64 位哈希值
     */
    quint64 hash64(const QByteArray& data);

    /** @brief 获取统计信息 @return 统计结构体的常量引用 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 哈希计算完成信号 @param inputSize 输入数据大小 */
    void hashComputed(qint64 inputSize);

private:
    /** @brief wyrand 伪随机数生成 @param seed 种子引用 */
    static quint64 wyrand(quint64& seed);
    /** @brief 从字节数组读取小端64位整数 @param data 数据 @param offset 偏移 */
    static quint64 readLE64(const QByteArray& data, int offset);
    /** @brief 从字节数组读取小端32位整数 @param data 数据 @param offset 偏移 */
    static quint32 readLE32(const QByteArray& data, int offset);
    /** @brief 64位混合函数(wymix) @param a 值A @param b 值B @return 混合结果 */
    static quint64 wymix(quint64 a, quint64 b);

    quint64 m_seed;                 ///< 哈希种子
    QElapsedTimer m_timer;          ///< 计时器
    Stats m_stats;                  ///< 统计信息
    double m_timeSum;               ///< 累计处理时间
};

#endif // WYHASH_H
