/**
 * @file BloomFilter.h
 * @brief 布隆过滤器 — 概率型集合成员检测
 *
 * 功能: 支持可配置误判率的布隆过滤器，使用多个哈希函数，
 *       统计插入/查询/误判次数。
 */
#ifndef BLOOMFILTER_H
#define BLOOMFILTER_H

#include <QObject>
#include <QVector>
#include <QByteArray>
#include <QtGlobal>

class BloomFilter : public QObject {
    Q_OBJECT
public:
    /** 过滤器统计 */
    struct Stats {
        quint64 totalInsertions = 0;
        quint64 totalQueries = 0;
        quint64 totalFalsePositives = 0;
        quint64 totalTrueNegatives = 0;
    };

    /** @brief 构造 @param expectedItems 预期元素数 @param falsePositiveRate 误判率 */
    explicit BloomFilter(quint64 expectedItems = 10000,
                         double falsePositiveRate = 0.01,
                         QObject* parent = nullptr);

    /** @brief 插入元素 @param data 数据 */
    void insert(const QByteArray& data);

    /** @brief 查询元素 @param data 数据 @return 可能存在(true)/一定不存在(false) */
    bool contains(const QByteArray& data) const;

    /** @brief 清空过滤器 */
    void clear();

    /** @brief 预期误判率 */
    double expectedFalsePositiveRate() const;

    /** @brief 当前填充率 */
    double fillRatio() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

    quint64 bitCount() const { return m_bitCount; }
    int hashCount() const { return m_hashCount; }

signals:
    void elementInserted(quint64 totalElements);

private:
    /** 计算哈希位置 */
    quint32 hashN(const QByteArray& data, int n) const;
    /** MurmurHash3变体 */
    quint32 murmurHash(const QByteArray& data, quint32 seed) const;

    QVector<quint64> m_bits;
    quint64 m_bitCount;
    int m_hashCount;
    quint64 m_insertedCount;
    mutable Stats m_stats;
};

#endif // BLOOMFILTER_H
