/**
 * @file VanEmdeBoas10.h
 * @brief van Emde Boas树(哈希底层簇与x-fast trie增强的O(log log U)前驱查询空间高效数据结构) — van Emde Boas with Hash-based Bottom Clusters and X-fast Trie Augmentation for O(log log U) Predecessor with Space Efficiency
 *
 * 功能: 实现van Emde Boas树(van Emde Boas tree)，采用哈希底层簇(hash-based bottom clusters)
 *       与x-fast trie增强(x-fast trie augmentation)实现O(log log U)前驱查询空间高效数据结构(O(log log U) predecessor with space efficiency)。
 *
 * 协作: ScapegoatTree11(替罪羊树) / RedBlackTree12(红黑树) / BPlusTree13(B+树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QHash>

/**
 * @brief van Emde Boas树(哈希底层簇与x-fast trie增强)
 */
class VanEmdeBoas10 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int universeSize = 0;
        int numElements = 0;
        int treeHeight = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit VanEmdeBoas10(QObject *parent = nullptr);
    ~VanEmdeBoas10() override;

    /** @brief Set universe size (must be power of 2) */
    void setUniverse(int u);

    /** @brief Insert a value into the tree */
    void insert(int x);

    /** @brief Remove a value from the tree */
    void remove(int x);

    /** @brief Check if value exists */
    bool contains(int x) const;

    /** @brief Find successor of x (smallest element >= x), -1 if none */
    int successor(int x) const;

    /** @brief Find predecessor of x (largest element <= x), -1 if none */
    int predecessor(int x) const;

    /** @brief Find minimum element, -1 if empty */
    int minimum() const;

    /** @brief Find maximum element, -1 if empty */
    int maximum() const;

    /** @brief Get number of elements */
    int size() const;

    /** @brief Get all elements in sorted order */
    QVector<int> elements() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationDone(const QString& op, int value, int numElements, double timeMs);

private:
    /** @brief vEB node using hash-based bottom clusters */
    struct VEBNode {
        int universe = 0;          // Universe size of this subtree
        int minVal = -1;           // Minimum element (stored separately)
        int maxVal = -1;           // Maximum element
        int summaryIdx = -1;      // Index of summary structure
        QHash<int, int> cluster;  // cluster[high] -> VEBNode index for bottom clusters
    };

    QVector<VEBNode> m_nodes;
    int m_rootIdx = 0;
    int m_universe = 16;
    int m_numElements = 0;

    // X-fast trie: maps (level, prefix) -> element, for O(1) hash lookup
    QHash<QPair<int, int>, int> m_xFast;
    int m_maxBits = 4;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Allocate a new VEB node */
    int allocNode(int u);

    /** @brief High bits of x in universe u */
    int high(int x, int u) const;

    /** @brief Low bits of x in universe u */
    int low(int x, int u) const;

    /** @brief Combine high and low indices */
    int index(int high, int low, int u) const;

    /** @brief Recursive insert */
    void insertRec(int nodeIdx, int x);

    /** @brief Recursive remove */
    void removeRec(int nodeIdx, int x);

    /** @brief Recursive successor */
    int successorRec(int nodeIdx, int x) const;

    /** @brief Recursive predecessor */
    int predecessorRec(int nodeIdx, int x) const;

    /** @brief Get or create cluster node */
    int getOrCreateCluster(int nodeIdx, int clusterKey);

    /** @brief Update x-fast trie */
    void updateXFast(int x, bool add);

    /** @brief Count bits needed for universe */
    int countBits(int u) const;
};
