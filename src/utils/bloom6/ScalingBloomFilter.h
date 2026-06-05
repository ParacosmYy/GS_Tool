/**
 * @file ScalingBloomFilter.h
 * @brief 可扩展布隆过滤器(Scaling Bloom Filter)
 */

#pragma once

#include <QObject>
#include <QByteArray>
#include <QVector>

/**
 * @class ScalingBloomFilter
 * @brief 可扩展布隆过滤器 — 自动扩容的概率集合
 *
 * 支持动态扩容、计数估计、误判率跟踪。
 * 适用于大规模数据去重、缓存过滤等场景。
 */
class ScalingBloomFilter : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalInserts = 0;       /**< 总插入次数 */
        int totalQueries = 0;       /**< 总查询次数 */
        int totalFalsePositives = 0; /**< 总假阳性(估计) */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit ScalingBloomFilter(double fpRate = 0.01, int initialCapacity = 10000,
                                  QObject* parent = nullptr);

    /**
     * @brief 插入元素
     * @param item 数据
     */
    void insert(const QByteArray& item);

    /**
     * @brief 插入字符串
     * @param item 字符串
     */
    void insertString(const QString& item);

    /**
     * @brief 查询元素是否存在
     * @param item 数据
     * @return 可能存在(true)或一定不存在(false)
     */
    bool contains(const QByteArray& item) const;

    /**
     * @brief 查询字符串
     * @param item 字符串
     * @return 可能存在或一定不存在
     */
    bool containsString(const QString& item) const;

    /** @brief 估计元素数 */
    long long estimateCount() const;

    /** @brief 当前误判率估计 */
    double estimatedFalsePositiveRate() const;

    /** @brief 当前容量 */
    int capacity() const;

    /** @brief 扩容次数 */
    int scaleCount() const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 扩容信号 */
    void scaled(int newSize);

private:
    void scaleUp();
    quint64 hash(const QByteArray& data, int seed) const;
    int optimalHashCount(int bits, int items) const;

    struct Slice {
        QVector<bool> bits;
        int hashCount;
        int capacity;
        int inserts;
    };

    QVector<Slice> m_slices;
    double m_targetFpRate;
    int m_initialCapacity;
    long long m_totalInserts;

    Stats m_stats;
    double m_timeSum;
};
