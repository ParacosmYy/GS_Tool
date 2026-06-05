/**
 * @file CuckooFilter.h
 * @brief 布谷鸟过滤器 — 支持删除的概率集合成员检测
 *
 * 功能: 比布隆过滤器多支持删除操作，使用部分键布谷鸟哈希，
 *       统计插入/查询/删除次数/假阳性/耗时。
 */
#ifndef CUCKOOFILTER_H
#define CUCKOOFILTER_H

#include <QObject>
#include <QByteArray>
#include <QVector>

class CuckooFilter : public QObject {
    Q_OBJECT
public:
    /** 统计 */
    struct Stats {
        quint64 totalInsertions = 0;
        quint64 totalQueries = 0;
        quint64 totalDeletions = 0;
        quint64 totalFalsePositives = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit CuckooFilter(int capacity = 10000,
                           QObject* parent = nullptr);

    /** @brief 插入元素 @param item 元素 @return 是否成功 */
    bool insert(const QByteArray& item);

    /** @brief 查询元素 @param item 元素 @return 可能存在/一定不存在 */
    bool contains(const QByteArray& item) const;

    /** @brief 删除元素 @param item 元素 @return 是否成功删除 */
    bool remove(const QByteArray& item);

    /** @brief 当前填充率 @return 填充率[0,1] */
    double fillRatio() const;

    int capacity() const { return m_capacity; }
    int size() const { return m_size; }

    void clear();
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void elementInserted(const QByteArray& item);
    void elementRemoved(const QByteArray& item);
    void filterFull();

private:
    /** 指纹(部分键) */
    quint8 fingerprint(const QByteArray& item) const;
    /** 哈希索引 */
    quint32 hashIndex(const QByteArray& item) const;
    /** 备用索引 */
    quint32 altIndex(quint32 idx, quint8 fp) const;

    static constexpr int BUCKET_SIZE = 4;
    static constexpr int MAX_KICKS = 500;

    struct Bucket {
        quint8 fps[BUCKET_SIZE] = {};
        bool occupied[BUCKET_SIZE] = {};
    };

    int m_capacity;
    int m_numBuckets;
    int m_size;
    QVector<Bucket> m_buckets;
    mutable Stats m_stats;
    mutable double m_timeSum;
};

#endif // CUCKOOFILTER_H
