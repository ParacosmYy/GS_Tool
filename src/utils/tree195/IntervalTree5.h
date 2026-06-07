/**
 * @file IntervalTree5.h
 * @brief 区间树(增强子树最大端点+全交集查询+优先搜索) — Interval Tree with Augmented Subtree Max-End, All-Intersections Query and Priority Search
 *
 * 功能: 实现增强型区间树，支持子树最大端点增强、
 *       全交集查询和基于优先搜索的区间检索。
 *
 * 协作: SegmentTree5(线段树) / RangeTree4(范围树) / FenwickTree5(树状数组)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 区间树(增强最大端点+全交集+优先搜索)
 */
class IntervalTree5 : public QObject {
    Q_OBJECT

public:
    /** @brief Interval type: [low, high] with data */
    using Interval = QPair<QPair<double, double>, int>;

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalQueries = 0;
        int numIntervals = 0;
        int treeHeight = 0;
        int lastResultCount = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit IntervalTree5(QObject *parent = nullptr);
    ~IntervalTree5() override;

    /** @brief Build tree from interval list */
    void build(const QVector<Interval>& intervals);

    /** @brief Insert single interval (incremental) */
    void insert(const Interval& iv);

    /** @brief Query all intervals overlapping [low, high] */
    QVector<Interval> queryAll(double low, double high) const;

    /** @brief Query single point */
    QVector<Interval> queryPoint(double point) const;

    /** @brief Priority search: find k intervals with largest overlap */
    QVector<Interval> prioritySearch(double low, double high, int k) const;

    /** @brief Count intervals overlapping [low, high] */
    int countOverlaps(double low, double high) const;

    int size() const { return m_size; }
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void queryCompleted(int results, double timeMs);

private:
    struct Node {
        double center = 0.0;
        double maxEnd = 0.0;    // augmented: max endpoint in subtree
        QVector<Interval> leftSorted;   // sorted by start (ascending)
        QVector<Interval> rightSorted;  // sorted by end (descending)
        Node *left = nullptr;
        Node *right = nullptr;
    };

    Node *m_root = nullptr;
    int m_size = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build subtree from intervals */
    Node* buildNode(QVector<Interval>& intervals);

    /** @brief Insert into subtree */
    void insertNode(Node*& node, const Interval& iv);

    /** @brief Update augmented max-end */
    void updateMaxEnd(Node* node);

    /** @brief Query overlaps in subtree */
    void queryNode(Node* node, double low, double high,
                   QVector<Interval>& result) const;

    /** @brief Count overlaps in subtree */
    int countNode(Node* node, double low, double high) const;

    /** @brief Priority search in subtree */
    void priorityNode(Node* node, double low, double high, int k,
                      QVector<QPair<double, Interval>>& heap) const;

    /** @brief Delete subtree */
    void clearNode(Node* node);
};
