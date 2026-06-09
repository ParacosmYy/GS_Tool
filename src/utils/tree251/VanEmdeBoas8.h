/**
 * @file VanEmdeBoas8.h
 * @brief van Emde Boas树(哈希簇存储+破坏性合并稀疏宇宙内存优化) — van Emde Boas Tree with Hash-based Cluster Storage and Destructive Merge for Memory-Efficient Sparse Universe Support
 *
 * 功能: 实现van Emde Boas树(van Emde Boas tree)，使用哈希簇存储
 *       (hash-based cluster storage)替代固定数组以支持稀疏宇宙(sparse
 *       universe)，破坏性合并(destructive merge)在簇操作时回收内存。
 *
 * 协作: ScapegoatTree9(替罪羊树) / RBTree6(红黑树) / BPlusTree11(B+树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QHash>

/**
 * @brief van Emde Boas树(哈希簇存储+破坏性合并)
 */
class VanEmdeBoas8 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int universeSize = 0;
        int numElements = 0;
        int numClusters = 0;
        int numInsertions = 0;
        int numDeletions = 0;
        int numSearches = 0;
        int numMerges = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit VanEmdeBoas8(int universeBits = 16, QObject *parent = nullptr);
    ~VanEmdeBoas8() override;

    /** @brief Insert a key into the vEB tree */
    void insert(int key);

    /** @brief Remove a key from the vEB tree */
    void remove(int key);

    /** @brief Check if key is a member */
    bool contains(int key) const;

    /** @brief Find successor of key, return -1 if none */
    int successor(int key) const;

    /** @brief Find predecessor of key, return -1 if none */
    int predecessor(int key) const;

    /** @brief Get minimum element, return -1 if empty */
    int minimum() const;

    /** @brief Get maximum element, return -1 if empty */
    int maximum() const;

    /** @brief Get all elements in sorted order */
    QVector<int> elements() const;

    /** @brief Clear the tree */
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void mergeCompleted(int fromUniverse, int toUniverse, int elementsMoved);

private:
    /** @brief vEB node with hash-based cluster storage */
    struct VEBNode {
        int universe = 0;
        int min = -1;       // Stored separately (not in clusters)
        int max = -1;
        VEBNode* summary = nullptr;
        QHash<int, VEBNode*> clusters; // Sparse hash storage

        int high(int x) const { return x >> (universe / 2); }
        int low(int x) const { return x & ((1 << (universe / 2)) - 1); }
        int index(int high, int low) const {
            return (high << (universe / 2)) | low;
        }
        int sqrtUniverse() const { return 1 << (universe / 2); }
    };

    VEBNode* m_root = nullptr;
    int m_universeBits = 16;
    int m_universeSize = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Allocate a new vEB node */
    VEBNode* allocNode(int universeBits);

    /** @brief Recursively delete a node */
    void deleteNode(VEBNode* node);

    /** @brief Recursive insert */
    void insertRec(VEBNode* node, int key);

    /** @brief Recursive remove */
    void removeRec(VEBNode* node, int key);

    /** @brief Recursive contains */
    bool containsRec(const VEBNode* node, int key) const;

    /** @brief Recursive successor */
    int successorRec(const VEBNode* node, int key) const;

    /** @brief Recursive predecessor */
    int predecessorRec(const VEBNode* node, int key) const;

    /** @brief Destructive merge: absorb another cluster's elements */
    void destructiveMerge(VEBNode* target, VEBNode* source);

    /** @brief Collect elements recursively */
    void collectElements(const VEBNode* node,
                          QVector<int>& result) const;
};
