/**
 * @file VanEmdeBoas5.h
 * @brief van Emde Boas树(聚类递归布局+溢出链表密集键范围支持) — vEB Tree with Clustered Recursive Layout and Overflow-Linked List for Dense Key Range Support
 *
 * 功能: 实现van Emde Boas树，支持聚类递归布局、
 *       溢出链表处理密集键范围和O(log log U)操作。
 *
 * 协作: ScapegoatTree6(替罪羊树) / BPlusTree8(B+树) / YFastTrie5(Y-Fast Trie)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief van Emde Boas树(聚类递归布局+溢出链表密集键范围支持)
 */
class VanEmdeBoas5 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int universeSize = 0;
        int numElements = 0;
        int numOverflow = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit VanEmdeBoas5(int universeBits = 16, QObject *parent = nullptr);
    ~VanEmdeBoas5() override;

    /** @brief Insert key */
    void insert(int key);

    /** @brief Remove key */
    void remove(int key);

    /** @brief Check if key exists */
    bool contains(int key) const;

    /** @brief Find successor of key */
    int successor(int key) const;

    /** @brief Find predecessor of key */
    int predecessor(int key) const;

    /** @brief Find minimum */
    int minimum() const;

    /** @brief Find maximum */
    int maximum() const;

    /** @brief Range query [lo, hi] */
    QVector<int> rangeQuery(int lo, int hi) const;

    /** @brief Clear all data */
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int key, double timeMs);

private:
    int m_universeBits;
    int m_universe;
    int m_sqrtU;

    /** @brief vEB cluster stored in pool */
    struct Cluster {
        int min = -1;       // Min is stored explicitly, not in sub-clusters
        int max = -1;
        int summary = -1;   // Index to summary cluster
        QVector<int> subClusters; // Indices to sub-cluster clusters
        // Overflow linked list for dense key ranges
        QVector<int> overflowKeys;
    };

    QVector<Cluster> m_clusters;
    int m_root = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Allocate a new cluster */
    int allocCluster(int subSize);

    /** @brief High bits of key */
    int high(int key) const;

    /** @brief Low bits of key */
    int low(int key) const;

    /** @brief Reconstruct key from high and low */
    int index(int high, int low) const;

    /** @brief Recursive insert helper */
    void insertImpl(int clusterIdx, int key);

    /** @brief Recursive remove helper */
    void removeImpl(int clusterIdx, int key);

    /** @brief Recursive contains helper */
    bool containsImpl(int clusterIdx, int key) const;

    /** @brief Recursive successor helper */
    int successorImpl(int clusterIdx, int key) const;

    /** @brief Recursive predecessor helper */
    int predecessorImpl(int clusterIdx, int key) const;

    /** @brief Check overflow list for dense key ranges */
    bool checkOverflow(int clusterIdx, int key) const;

    /** @brief Add to overflow list */
    void addToOverflow(int clusterIdx, int key);

    /** @brief Remove from overflow list */
    void removeFromOverflow(int clusterIdx, int key);
};
