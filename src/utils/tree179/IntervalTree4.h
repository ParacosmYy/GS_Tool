/**
 * @file IntervalTree4.h
 * @brief 区间树(中心点分割+穿刺查询+重叠检测+区间聚合) — Interval Tree with Center-point Split, Stabbing Query, Overlap Detection and Interval Aggregation
 *
 * 功能: 实现区间树数据结构，支持中心点分割构建、
 *       穿刺查询、重叠检测和区间聚合操作。
 *
 * 协作: SegmentTree4(线段树) / RangeTree4(范围树) / KDTree4(KD树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 区间树(中心点分割+穿刺查询)
 */
class IntervalTree4 : public QObject {
    Q_OBJECT

public:
    /** @brief 区间定义 */
    struct Interval {
        double low = 0.0;
        double high = 0.0;
        int id = -1;
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalQueries = 0;
        int numIntervals = 0;
        int treeHeight = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit IntervalTree4(QObject *parent = nullptr);
    ~IntervalTree4() override;

    /** @brief 批量构建区间树 */
    void build(const QVector<Interval>& intervals);

    /** @brief 穿刺查询: 查找包含点x的所有区间 */
    QVector<Interval> stabbingQuery(double x) const;

    /** @brief 重叠检测: 查找与[low,high]重叠的所有区间 */
    QVector<Interval> overlapQuery(double low, double high) const;

    /** @brief 区间聚合: 对重叠区间执行指定操作 */
    QVector<Interval> aggregate(const QVector<Interval>& intervals) const;

    /** @brief 查找所有互相重叠的区间对 */
    QVector<QPair<Interval, Interval>> findAllOverlaps() const;

    bool isEmpty() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void queryCompleted(int resultCount);

private:
    /** @brief 树节点 */
    struct Node {
        double center = 0.0;
        QVector<Interval> byLowAsc;   ///< Intervals spanning center, sorted by low
        QVector<Interval> byHighDesc; ///< Same intervals, sorted by high descending
        Node* left = nullptr;
        Node* right = nullptr;
    };

    Node* m_root = nullptr;
    int m_height = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    Node* buildImpl(const QVector<Interval>& intervals, int depth);
    void deleteTree(Node* node);

    void stabbingImpl(Node* node, double x, QVector<Interval>& result) const;
    void overlapImpl(Node* node, double low, double high,
                     QVector<Interval>& result) const;
};
