/**
 * @file ScapegoatTree8.h
 * @brief 替罪羊树(权重平衡重建触发+自顶向下搜索路径重平衡) — Scapegoat Tree with Weight-Balanced Rebuild Triggers and Top-Down Search-Path Rebalancing for Amortized O(log n)
 *
 * 功能: 实现替罪羊树(Scapegoat tree)，采用权重平衡(weight-balanced)策略触发子树重建
 *       (subtree rebuild)，结合自顶向下(top-down)搜索路径(search path)重平衡机制，
 *       实现均摊O(log n)时间复杂度的二叉搜索树。
 *
 * 协作: AvlTree7(AVL树) / RedBlackTree6(红黑树) / BPlusTree10(B+树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 替罪羊树(权重平衡重建+自顶向下搜索路径重平衡)
 */
class ScapegoatTree8 : public QObject {
    Q_OBJECT

public:
    /** @brief Key-value pair for iteration */
    using KVPair = QPair<int, double>;

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numKeys = 0;
        int treeHeight = 0;
        int numRebuilds = 0;
        int maxNodesBeforeRebuild = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ScapegoatTree8(QObject *parent = nullptr);
    ~ScapegoatTree8() override;

    /** @brief Set balance factor alpha (0.5 < alpha < 1.0) */
    void setAlpha(double alpha);

    /** @brief Insert key-value pair */
    void insert(int key, double value);

    /** @brief Remove key */
    void remove(int key);

    /** @brief Search for key, returns value (NaN if not found) */
    double search(int key) const;

    /** @brief Check if key exists */
    bool contains(int key) const;

    /** @brief Get all key-value pairs (in-order) */
    QVector<KVPair> allPairs() const;

    /** @brief Get tree height */
    int height() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void nodeInserted(int key);
    void nodeRemoved(int key);
    void treeRebuilt(int numNodes, double timeMs);

private:
    /** @brief Tree node */
    struct Node {
        int key = 0;
        double value = 0.0;
        int size = 1;     // subtree size for weight balancing
        Node* left = nullptr;
        Node* right = nullptr;
    };

    double m_alpha = 0.7;
    int m_maxSize = 0;    // max tree size since last rebuild
    Node* m_root = nullptr;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute h_alpha(n) = floor(log_{1/alpha}(n)) */
    int hAlpha(int n) const;

    /** @brief Check if node at depth d is unbalanced */
    bool isUnbalanced(Node* node, int depth) const;

    /** @brief Find scapegoat node on search path */
    Node** findScapegoat(Node** rootPtr, int key);

    /** @brief Flatten subtree into sorted array */
    void flatten(Node* node, QVector<Node*>& nodes) const;

    /** @brief Rebuild balanced subtree from sorted array */
    Node* rebuildBalanced(const QVector<Node*>& nodes, int lo, int hi);

    /** @brief Update size of node */
    void updateSize(Node* node);

    /** @brief Get subtree size */
    int nodeSize(Node* node) const;

    /** @brief Recursive insert helper */
    Node** insertHelper(Node** ptr, int key, double value, int depth);

    /** @brief Recursive search helper */
    Node* searchHelper(Node* node, int key) const;

    /** @brief Delete all nodes */
    void deleteTree(Node* node);

    /** @brief In-order traversal */
    void inOrder(Node* node, QVector<KVPair>& result) const;

    /** @brief Compute height */
    int computeHeight(Node* node) const;
};
