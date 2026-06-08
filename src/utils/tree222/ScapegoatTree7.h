/**
 * @file ScapegoatTree7.h
 * @brief 替罪羊树(加权失衡检测最优重建触发+批量插入支持) — Scapegoat Tree with Optimal Rebuild Trigger via Weighted Imbalance Detection and Bulk Insertion Support
 *
 * 功能: 实现替罪羊树自平衡BST，通过加权失衡检测确定最优重建节点，
 *       支持批量插入操作的高效重建策略。
 *
 * 协作: AvlTree6(AVL树) / RedBlackTree5(红黑树) / BPlusTree9(B+树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 替罪羊树(加权失衡检测+批量插入)
 */
class ScapegoatTree7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int nodeCount = 0;
        int treeHeight = 0;
        int numRebuilds = 0;
        int maxRebuildDepth = 0;
        double alpha = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ScapegoatTree7(QObject *parent = nullptr);
    ~ScapegoatTree7() override;

    /** @brief Set balance parameter alpha (0.5 < alpha < 1.0) */
    void setAlpha(double alpha = 0.67);

    /** @brief Insert a single key */
    void insert(int key);

    /** @brief Bulk insert multiple keys */
    void bulkInsert(const QVector<int>& keys);

    /** @brief Remove a key */
    bool remove(int key);

    /** @brief Search for key, returns true if found */
    bool search(int key) const;

    /** @brief In-order traversal of all keys */
    QVector<int> inOrder() const;

    /** @brief Check if tree is balanced */
    bool isBalanced() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void rebuildTriggered(int subtreeSize, int depth, double timeMs);
    void operationCompleted(const QString& op, int key, double timeMs);

private:
    struct Node {
        int key = 0;
        Node* left = nullptr;
        Node* right = nullptr;
        int weight = 1; // subtree size (used for imbalance detection)
        Node(int k) : key(k) {}
    };

    double m_alpha = 0.67;
    Node* m_root = nullptr;
    int m_maxSize = 0;  // max tree size since last rebuild

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute subtree weight */
    int weight(Node* node) const;

    /** @brief Update weight cache */
    void updateWeight(Node* node);

    /** @brief Check if node is alpha-weight-balanced */
    bool isAlphaBalanced(Node* node) const;

    /** @brief Find scapegoat node on insertion path */
    Node* findScapegoat(Node* path[], int pathLen);

    /** @brief Flatten subtree into sorted array */
    void flatten(Node* node, QVector<int>& keys) const;

    /** @brief Rebuild balanced BST from sorted array */
    Node* rebuild(const QVector<int>& keys, int lo, int hi);

    /** @brief Rebuild subtree rooted at scapegoat */
    void rebuildAt(Node*& root);

    /** @brief Recursive search helper */
    bool searchHelper(Node* node, int key) const;

    /** @brief In-order traversal helper */
    void inOrderHelper(Node* node, QVector<int>& result) const;

    /** @brief Recursive balance check */
    bool isBalancedHelper(Node* node) const;

    /** @brief Compute tree height */
    int height(Node* node) const;

    /** @brief Recursive cleanup */
    void clearNode(Node* node);
};
