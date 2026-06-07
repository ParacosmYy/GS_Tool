/**
 * @file SegmentTree5.h
 * @brief 线段树(归并排序树+范围名次查询+小波树混合) — Segment Tree with Merge-Sort Tree for Range Rank Queries and Wavelet Tree Hybrid
 *
 * 功能: 实现基于归并排序树的线段树，支持范围名次查询(rank)、
 *       第k小值查询和小波树混合优化。
 *
 * 协作: FenwickTree5(树状数组) / SegmentTree6(线段树) / VanEmdeBoas4(vEB树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 线段树(归并排序树+范围名次+小波树混合)
 */
class SegmentTree5 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalQueries = 0;
        int dataSize = 0;
        int treeHeight = 0;
        int rankQueries = 0;
        int kthQueries = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SegmentTree5(QObject *parent = nullptr);
    ~SegmentTree5() override;

    /** @brief Build tree from data array */
    void build(const QVector<double>& data);

    /** @brief Rank query: count elements in [l, r] that are < value */
    int rankQuery(int l, int r, double value) const;

    /** @brief K-th smallest query in range [l, r] */
    double kthSmallest(int l, int r, int k) const;

    /** @brief Range count: elements in [l, r] within [loVal, hiVal] */
    int rangeCount(int l, int r, double loVal, double hiVal) const;

    /** @brief Get original data */
    QVector<double> data() const;

    int size() const { return m_size; }
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void queryCompleted(const QString& type, double timeMs);

private:
    int m_size = 0;
    QVector<double> m_data;
    QVector<double> m_sorted;  // global sorted for binary search bounds

    // Merge-sort tree: each node stores sorted subarray
    QVector<QVector<double>> m_tree;
    int m_treeSize = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build merge-sort tree recursively */
    void buildTree(int node, int start, int end);

    /** @brief Rank query in node's sorted array using binary search */
    int rankInNode(int node, double value) const;

    /** @brief Collect sorted vectors overlapping [l, r] */
    void collectNodes(int node, int start, int end,
                       int ql, int qr, QVector<int>& nodes) const;
};
