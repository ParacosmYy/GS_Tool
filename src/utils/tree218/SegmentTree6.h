/**
 * @file SegmentTree6.h
 * @brief 线段树(区间加乘懒标记传播+持久化版本时间查询) — Segment Tree with Lazy Propagation for Range Add-Multiply and Persistent Versioning for Temporal Queries
 *
 * 功能: 实现线段树数据结构，支持区间加法和乘法操作的懒标记传播，
 *       持久化版本管理支持历史时间点查询，适用于动态序列维护。
 *
 * 协作: BTree5(B树) / FenwickTree5(树状数组) / IntervalTree4(区间树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 线段树(区间加乘+持久化版本)
 */
class SegmentTree6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int treeSize = 0;
        int numVersions = 0;
        int numLeaves = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SegmentTree6(QObject *parent = nullptr);
    ~SegmentTree6() override;

    /** @brief Build tree from initial values */
    void build(const QVector<double>& values);

    /** @brief Range add: add val to [l, r] */
    void rangeAdd(int l, int r, double val);

    /** @brief Range multiply: multiply [l, r] by val */
    void rangeMultiply(int l, int r, double val);

    /** @brief Range query: sum of [l, r] */
    double rangeQuery(int l, int r) const;

    /** @brief Point query at index */
    double pointQuery(int idx) const;

    /** @brief Save current version for persistent queries */
    int saveVersion();

    /** @brief Query range [l,r] at historical version */
    double queryVersion(int version, int l, int r) const;

    /** @brief Get number of saved versions */
    int versionCount() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int version, double timeMs);

private:
    int m_n = 0;
    int m_version = 0;

    // Node: each has sum, lazy add, lazy multiply
    struct Node {
        double sum = 0.0;
        double lazyAdd = 0.0;
        double lazyMul = 1.0;
    };

    // Current tree
    QVector<Node> m_tree;

    // Persistent snapshots: version -> tree copy
    QVector<QVector<Node>> m_versions;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Push lazy tags to children */
    void pushDown(int idx, int left, int right);

    /** @brief Pull up from children */
    void pullUp(int idx);

    /** @brief Internal range add */
    void rangeAddImpl(int idx, int left, int right, int l, int r, double val);

    /** @brief Internal range multiply */
    void rangeMulImpl(int idx, int left, int right, int l, int r, double val);

    /** @brief Internal range query */
    double rangeQueryImpl(int idx, int left, int right, int l, int r) const;

    /** @brief Build tree recursively */
    void buildImpl(int idx, int left, int right, const QVector<double>& vals);
};
