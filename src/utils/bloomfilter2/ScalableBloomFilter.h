/**
 * @file ScalableBloomFilter.h
 * @brief 可扩展布隆过滤器 — 自动扩容的概率集合
 *
 * 功能: 当填充率超过阈值时自动添加新布隆过滤器层，
 *       支持动态插入/查询，统计层数/元素数/假阳性估计。
 */
#ifndef SCALABLEBLOOMFILTER_H
#define SCALABLEBLOOMFILTER_H

#include <QObject>
#include <QByteArray>
#include <QVector>

class ScalableBloomFilter : public QObject {
    Q_OBJECT
public:
    /** 统计 */
    struct Stats {
        quint64 totalInsertions = 0;
        quint64 totalQueries = 0;
        quint64 totalLayers = 0;
        double  estimatedFalsePositiveRate = 0.0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit ScalableBloomFilter(double fpRate = 0.01,
                                  QObject* parent = nullptr);

    /** @brief 插入元素 @param item 元素 */
    void insert(const QByteArray& item);

    /** @brief 查询元素 @param item 元素 @return 可能存在/一定不存在 */
    bool contains(const QByteArray& item) const;

    /** @brief 当前元素数 */
    quint64 count() const { return m_totalElements; }

    /** @brief 层数 */
    int layers() const { return m_filters.size(); }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void elementInserted(quint64 totalCount);
    void layerAdded(int layerIndex);

private:
    void addLayer();

    struct BloomLayer {
        QVector<quint64> bits;
        int bitCount;
        int hashCount;
        quint64 capacity;
        quint64 inserted;
    };

    double m_fpRate;
    quint64 m_totalElements;
    quint64 m_scaleFactor;
    double m_growthRate;
    QVector<BloomLayer> m_filters;
    mutable Stats m_stats;
    mutable double m_timeSum;
};

#endif // SCALABLEBLOOMFILTER_H
