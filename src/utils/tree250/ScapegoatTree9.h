/**
 * @file ScapegoatTree9.h
 * @brief 替罪羊树(混合权重-高度平衡准则+增量重建调度摊还代价) — Scapegoat Tree with Hybrid Weight-Height Balancing Criterion and Incremental Rebuild Scheduling for Amortized Cost
 *
 * 功能: 实现替罪羊树(Scapegoat Tree)，使用混合权重-高度平衡准则
 *       (hybrid weight-height balancing)检测不平衡节点，通过增量重建
 *       调度(incremental rebuild scheduling)将重建开销摊还到后续操作。
 *
 * 协作: AvlTree8(AVL树) / RBTree6(红黑树) / BPlusTree11(B+树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 替罪羊树(混合权重-高度+增量重建)
 */
class ScapegoatTree9 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numKeys = 0;
        int numNodes = 0;
        int treeHeight = 0;
        int numInsertions = 0;
        int numDeletions = 0;
        int numSearches = 0;
        int numRebuilds = 0;
        double alpha = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ScapegoatTree9(double alpha = 0.67, QObject *parent = nullptr);
    ~ScapegoatTree9() override;

    /** @brief Insert a key */
    void insert(int key);

    /** @brief Remove a key */
    void remove(int key);

    /** @brief Search for a key, return true if found */
    bool search(int key) const;

    /** @brief Get all keys in sorted order */
    QVector<int> inorder() const;

    /** @brief Get number of stored keys */
    int size() const;

    /** @brief Clear the tree */
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void treeRebuilt(int numKeys, int height, double timeMs);

private:
    double m_alpha;

    /** @brief Tree node */
    struct Node {
        int key = 0;
        int left = -1;
        int right = -1;
        int weight = 1;  // Subtree size including this node
        bool deleted = false; // Lazy deletion marker
    };

    QVector<Node> m_nodes;
    int m_root = -1;
    int m_count = 0;
    int m_maxHeight = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Allocate a new node */
    int allocNode(int key);

    /** @brief Compute subtree weight */
    int weight(int nodeIdx) const;

    /** @brief Check if subtree rooted at idx is alpha-weight-balanced */
    bool isBalanced(int nodeIdx) const;

    /** @brief Find scapegoat node on the insertion path */
    int findScapegoat(int nodeIdx, int key) const;

    /** @brief Flatten subtree into sorted array */
    void flatten(int nodeIdx, QVector<int>& keys) const;

    /** @brief Rebuild balanced subtree from sorted keys */
    int rebuildBalanced(const QVector<int>& keys, int lo, int hi);

    /** @brief Rebuild subtree rooted at scapegoat */
    void rebuildAt(int scapegoatIdx, int parentIdx, bool isLeft);

    /** @brief Collect inorder keys */
    void inorderHelper(int nodeIdx, QVector<int>& result) const;

    /** @brief Update height tracking */
    int computeHeight(int nodeIdx) const;

    /** @brief Log-alpha-h bound */
    int hAlpha(int n) const;
};
