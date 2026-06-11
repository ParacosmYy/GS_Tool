/**
 * @file VanEmdeBoas11.h
 * @brief Van Emde Boas树(哈希簇摘要与递归y-fast前驱检索实现O(lg lg U)前驱后继查询) — Van Emde Boas with Hash-based Cluster Summary and Recursive Y-fast Trie for O(lg lg U) Predecessor/Successor Queries
 *
 * 功能: 实现Van Emde Boas树(Van Emde Boas tree)，采用哈希簇摘要(hash-based cluster summary)
 *       与递归y-fast前驱检索(recursive y-fast trie)实现O(lg lg U)前驱后继查询(O(lg lg U) predecessor/successor queries)。
 *
 * 协作: ScapegoatTree12(替罪羊树) / RedBlackTree10(红黑树) / BTree9(B树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QHash>

class VanEmdeBoas11 : public QObject {
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

    explicit VanEmdeBoas11(QObject *parent = nullptr);
    ~VanEmdeBoas11() override;

    /** @brief Set universe size (must be power of 2) */
    void setUniverse(int U);

    /** @brief Insert a key into the vEB tree */
    void insert(int key);

    /** @brief Remove a key from the vEB tree */
    void remove(int key);

    /** @brief Check if key is a member */
    bool contains(int key) const;

    /** @brief Find predecessor of key (largest element < key) */
    int predecessor(int key) const;

    /** @brief Find successor of key (smallest element > key) */
    int successor(int key) const;

    /** @brief Get minimum element */
    int minimum() const;

    /** @brief Get maximum element */
    int maximum() const;

    int size() const { return m_size; }
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationDone(const QString& op, int key, double timeMs);

private:
    static constexpr int NIL = -1;

    /** @brief vEB node with hash-based clusters */
    struct VEBNode {
        int universe = 0;
        int min = NIL;                  // Stored separately (not in clusters)
        int max = NIL;
        int summaryIdx = NIL;           // Index of summary vEB in node pool
        QHash<int, int> clusterToIdx;   // Hash-based cluster: cluster# -> node index
    };

    int m_universe = 65536;
    int m_size = 0;
    Stats m_stats;
    double m_timeSum = 0.0;

    QVector<VEBNode> m_nodes;           // Pool of vEB nodes
    int m_root = NIL;
    int m_nextFree = 0;

    /** @brief Allocate a new vEB node */
    int allocNode(int universe);

    /** @brief Get higher sqrt bits of key */
    int high(int x, int u) const;

    /** @brief Get lower sqrt bits of key */
    int low(int x, int u) const;

    /** @brief Combine high and low parts */
    int index(int high, int low, int u) const;

    /** @brief Recursive insert */
    void insertRec(int nodeIdx, int x);

    /** @brief Recursive remove */
    void removeRec(int nodeIdx, int x);

    /** @brief Recursive contains */
    bool containsRec(int nodeIdx, int x) const;

    /** @brief Recursive minimum */
    int minRec(int nodeIdx) const;

    /** @brief Recursive maximum */
    int maxRec(int nodeIdx) const;

    /** @brief Recursive successor */
    int successorRec(int nodeIdx, int x) const;

    /** @brief Recursive predecessor */
    int predecessorRec(int nodeIdx, int x) const;

    /** @brief Compute lg lg U for height estimate */
    int computeHeight() const;
};
