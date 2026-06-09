/**
 * @file SegmentTree8.h
 * @brief 线段树(惰性传播+持久化版本的时间范围查询与回滚) — Segment Tree with Lazy Propagation and Persistent Versioning for Temporal Range Queries with Rollback
 *
 * 功能: 实现线段树(Segment Tree)，支持惰性传播(lazy propagation)进行高效
 *       区间更新，持久化版本(persistent versioning)支持历史状态访问，实现
 *       时间范围查询(temporal range queries)与回滚(rollback)操作。
 *
 * 协作: BTree7(B树) / FenwickTree7(树状数组) / SplayTree5(伸展树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 线段树(惰性传播+持久化版本的时间范围查询与回滚)
 */
class SegmentTree8 : public QObject {
    Q_OBJECT

public:
    /** @brief Range query result */
    struct RangeResult {
        double sum = 0.0;
        double minimum = 0.0;
        double maximum = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int treeSize = 0;
        int numVersions = 0;
        int numUpdates = 0;
        int numQueries = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SegmentTree8(QObject *parent = nullptr);
    ~SegmentTree8() override;

    /** @brief Build tree from initial data */
    void build(const QVector<double>& data);

    /** @brief Point update: set value at index */
    void update(int index, double value);

    /** @brief Range update: add delta to [l, r] with lazy propagation */
    void rangeUpdate(int l, int r, double delta);

    /** @brief Point query: get value at index */
    double query(int index) const;

    /** @brief Range query: get sum/min/max over [l, r] */
    RangeResult rangeQuery(int l, int r) const;

    /** @brief Create a persistent snapshot of current version */
    int commitVersion();

    /** @brief Rollback to a previous version */
    bool rollback(int version);

    /** @brief Query a specific historical version */
    RangeResult queryVersion(int version, int l, int r) const;

    /** @brief Get current version number */
    int currentVersion() const;

    /** @brief Get total number of versions */
    int versionCount() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void versionCreated(int version, int numVersions, double timeMs);

private:
    /** @brief Persistent node (copy-on-write) */
    struct Node {
        double sum = 0.0;
        double minVal = 0.0;
        double maxVal = 0.0;
        double lazy = 0.0;
        int left = -1;    // Index of left child
        int right = -1;   // Index of right child
    };

    QVector<Node> m_nodes;      // Node pool
    int m_root = -1;
    int m_size = 0;
    int m_currentVersion = -1;

    /** @brief Version snapshot: stores root index */
    struct Version {
        int rootIdx = -1;
        int nodeCount = 0;
    };
    QVector<Version> m_versions;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Allocate a new node (returns index) */
    int allocNode();

    /** @brief Clone a node for persistence (copy-on-write) */
    int cloneNode(int nodeIdx);

    /** @brief Build recursively */
    int buildRec(int l, int r, const QVector<double>& data);

    /** @brief Push lazy value down */
    void pushDown(int nodeIdx, int l, int r);

    /** @brief Point update recursively */
    int updateRec(int nodeIdx, int l, int r, int idx, double value);

    /** @brief Range update recursively */
    int rangeUpdateRec(int nodeIdx, int l, int r, int ql, int qr, double delta);

    /** @brief Range query recursively */
    RangeResult queryRec(int nodeIdx, int l, int r, int ql, int qr) const;
};
