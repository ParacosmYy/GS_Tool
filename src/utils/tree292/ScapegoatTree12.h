/**
 * @file ScapegoatTree12.h
 * @brief 替罪羊树(摊还权重平衡重建与α比率触发子树重构实现自调整二叉搜索树) — Scapegoat Tree with Amortized Weight-balanced Rebuilding and Alpha-ratio Triggered Subtree Reconstruction for Self-adjusting BST
 *
 * 功能: 实现替罪羊树(Scapegoat tree)，采用摊还权重平衡重建(amortized weight-balanced rebuilding)
 *       与α比率触发子树重构(alpha-ratio triggered subtree reconstruction)实现自调整二叉搜索树(self-adjusting BST)。
 *
 * 协作: AvlTree11(AVL树) / RedBlackTree10(红黑树) / SplayTree10(伸展树)
 */
#pragma once

#include <QObject>
#include <QVector>

class ScapegoatTree12 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numKeys = 0;
        int numNodes = 0;
        int numRebuilds = 0;
        int treeHeight = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ScapegoatTree12(QObject *parent = nullptr);
    ~ScapegoatTree12() override;

    void setAlpha(double alpha);       // Balance parameter (0.5 < α < 1.0)
    void setMaxSize(int maxSz);

    /** @brief Insert a key */
    void insert(int key);

    /** @brief Remove a key */
    void remove(int key);

    /** @brief Search for key, returns true if found */
    bool search(int key) const;

    /** @brief Get all keys in sorted order */
    QVector<int> inorderTraversal() const;

    /** @brief Check if tree is α-balanced */
    bool isBalanced() const;

    int size() const { return m_size; }
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationDone(const QString& op, int key, int height, double timeMs);
    void rebuildTriggered(int subtreeSize, double timeMs);

private:
    static constexpr int NULL_IDX = -1;

    /** @brief Tree node stored in array pool */
    struct Node {
        int key = 0;
        int left = NULL_IDX;
        int right = NULL_IDX;
        int subSize = 1;                // Subtree size
        bool deleted = false;           // Lazy deletion marker
    };

    double m_alpha = 0.7;
    int m_maxSize = 1000000;
    int m_root = NULL_IDX;
    int m_size = 0;
    int m_maxTreeSize = 0;              // Track max tree size for deletion rebuild
    Stats m_stats;
    double m_timeSum = 0.0;

    QVector<Node> m_nodes;
    QVector<int> m_freeList;

    /** @brief Allocate a new node */
    int allocNode(int key);

    /** @brief Free a node */
    void freeNode(int idx);

    /** @brief Compute subtree size recursively */
    int computeSize(int idx) const;

    /** @brief Update subtree sizes along path */
    void updateSizes(int idx);

    /** @brief Find scapegoat node (first α-unbalanced ancestor) */
    int findScapegoat(int idx, QVector<int>& path);

    /** @brief Rebuild subtree rooted at idx into perfectly balanced tree */
    int rebuildSubtree(int idx);

    /** @brief Flatten subtree into sorted array */
    void flatten(int idx, QVector<int>& keys);

    /** @brief Build balanced BST from sorted keys */
    int buildBalanced(const QVector<int>& keys, int lo, int hi);

    /** @brief Check if node is α-weight-balanced */
    bool isAlphaBalanced(int idx) const;

    /** @brief Collect inorder keys recursively */
    void inorderCollect(int idx, QVector<int>& result) const;

    /** @brief Compute tree height */
    int computeHeight(int idx) const;

    /** @brief Find node containing key */
    int findNode(int key) const;
};
