/**
 * @file SegmentTree10.h
 * @brief 线段树(持久化版本与路径复制不可变历史范围查询快照) — Segment Tree with Persistent Versioning and Path-Copying for Immutable Historical Range Query Snapshots
 *
 * 功能: 实现线段树(segment tree)，采用持久化版本(persistent versioning)
 *       与路径复制(path-copying)实现不可变历史范围查询快照(immutable historical range query snapshots)。
 *
 * 协作: FenwickTree12(树状数组) / SparseTable11(稀疏表) / Rope11(绳索结构)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 线段树(持久化版本与路径复制不可变历史范围查询快照)
 */
class SegmentTree10 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVersions = 0;
        int numNodes = 0;
        int dataSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SegmentTree10(QObject *parent = nullptr);
    ~SegmentTree10() override;

    /** @brief Build segment tree from initial data */
    void build(const QVector<double>& data);

    /** @brief Point update: returns new version root (path-copying) */
    int update(int version, int index, double value);

    /** @brief Range sum query on a specific version */
    double rangeSum(int version, int left, int right) const;

    /** @brief Range minimum query on a specific version */
    double rangeMin(int version, int left, int right) const;

    /** @brief Get current (latest) version index */
    int latestVersion() const;

    /** @brief Get number of versions */
    int numVersions() const;

    /** @brief Extract array snapshot from a given version */
    QVector<double> snapshot(int version) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void versionCreated(int version, int numVersions, double timeMs);

private:
    /** @brief Persistent tree node */
    struct Node {
        double sum = 0.0;
        double minVal = 0.0;
        int left = -1;      // left child index in node pool
        int right = -1;     // right child index in node pool
        int rangeLeft = 0;
        int rangeRight = 0;
    };

    QVector<Node> m_nodes;          // Node pool
    QVector<int> m_roots;           // Root index per version
    int m_dataSize = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build tree recursively, returns node index */
    int buildRec(int left, int right, const QVector<double>& data);

    /** @brief Persistent update: path-copy from old root, returns new node index */
    int updateRec(int nodeIdx, int index, double value, int left, int right);

    /** @brief Range sum query */
    double querySum(int nodeIdx, int ql, int qr) const;

    /** @brief Range min query */
    double queryMin(int nodeIdx, int ql, int qr) const;

    /** @brief Extract snapshot recursively */
    void snapshotRec(int nodeIdx, QVector<double>& result) const;
};
