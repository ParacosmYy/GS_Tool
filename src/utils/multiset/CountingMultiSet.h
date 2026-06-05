/**
 * @file CountingMultiSet.h
 * @brief 计数多重集合 — 元素频率跟踪与集合相似度计算
 *
 * 功能: 基于哈希表的元素频率统计，支持top-K频繁元素查询，
 *       Jaccard相似度计算，集合运算(交集/并集/差集)，
 *       统计操作次数/元素数/耗时。
 */
#ifndef COUNTINGMULTISET_H
#define COUNTINGMULTISET_H

#include <QObject>
#include <QVector>
#include <QPair>
#include <QMap>

/**
 * @brief 计数多重集合 — 元素频率跟踪
 */
class CountingMultiSet : public QObject {
    Q_OBJECT

public:
    /** @brief 统计 */
    struct Stats {
        quint64 totalInserts = 0;       ///< 累计插入次数
        quint64 totalRemoves = 0;       ///< 累计删除次数
        quint64 totalQueries = 0;       ///< 累计查询次数
        int     uniqueElements = 0;     ///< 当前不同元素数
        int     totalCount = 0;         ///< 当前元素总数(含重复)
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间
    };

    explicit CountingMultiSet(QObject* parent = nullptr);

    /** @brief 插入元素 @param value 元素值 @param count 插入次数(默认1) */
    void insert(int value, int count = 1);

    /** @brief 删除元素 @param value 元素值 @param count 删除次数(默认1) @return 实际删除数 */
    int remove(int value, int count = 1);

    /** @brief 查询元素频率 @param value 元素值 @return 频率(不存在返回0) */
    int count(int value);

    /** @brief 是否包含元素 @param value 元素值 @return 是否存在 */
    bool contains(int value);

    /** @brief Top-K频繁元素 @param k 返回数量 @return (元素,频率)列表按频率降序 */
    QVector<QPair<int, int>> topK(int k) const;

    /** @brief Jaccard相似度 @param other 另一个多重集合 @return 相似度[0,1] */
    double jaccardSimilarity(const CountingMultiSet& other) const;

    /** @brief 清空集合 */
    void clear();

    /** @brief 当前不同元素数 @return 唯一元素数 */
    int size() const;

    /** @brief 元素总数(含重复) @return 总计数 */
    int totalCount() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 元素插入 @param value 元素 @param newCount 插入后频率 */
    void elementInserted(int value, int newCount);
    /** @brief 元素删除 @param value 元素 @param newCount 删除后频率 */
    void elementRemoved(int value, int newCount);

private:
    QMap<int, int> m_counts;        ///< 元素->频率映射
    Stats m_stats;
    double m_timeSum;               ///< 处理时间累加器
};

#endif // COUNTINGMULTISET_H
