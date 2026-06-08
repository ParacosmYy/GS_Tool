/**
 * @file AvlTree7.h
 * @brief AVL树(秩增强节点顺序统计查询+加权平衡范围报告) — AVL Tree with Rank-Augmented Nodes for Order-Statistic Queries and Weighted Balance for Range Reporting
 *
 * 功能: 实现AVL树(AVL tree)平衡二叉搜索树，节点增强秩(rank)字段支持顺序统计查询
 *       (order-statistic queries)，并采用加权平衡(weighted balance)策略优化范围报告
 *       (range reporting)操作。
 *
 * 协作: AA7(AA树) / RedBlackTree6(红黑树) / BTree6(B树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief AVL树(秩增强节点顺序统计查询+加权平衡范围报告)
 */
class AvlTree7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numNodes = 0;
        int treeHeight = 0;
        int numRotations = 0;
        int numRangeQueries = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit AvlTree7(QObject *parent = nullptr);
    ~AvlTree7() override;

    /** @brief Insert key-value pair */
    void insert(int key, double value);

    /** @brief Remove key */
    void remove(int key);

    /** @brief Search for key, returns value (NaN if not found) */
    double search(int key) const;

    /** @brief Check if key exists */
    bool contains(int key) const;

    /** @brief Select k-th smallest element (order statistic), returns key */
    int select(int k) const;

    /** @brief Get rank of key (1-based position in sorted order) */
    int rank(int key) const;

    /** @brief Range query: all key-value pairs in [lo, hi] */
    QVector<QPair<int, double>> rangeQuery(int lo, int hi) const;

    /** @brief Count nodes in range [lo, hi] */
    int rangeCount(int lo, int hi) const;

    /** @brief In-order traversal */
    QVector<QPair<int, double>> inorderTraversal() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void nodeInserted(int key);
    void nodeRemoved(int key);
    void rangeQueryCompleted(int lo, int hi, int count);

private:
    /** @brief Rank-augmented AVL node */
    struct Node {
        int key = 0;
        double value = 0.0;
        int height = 1;
        int subtreeSize = 1;   // rank augmentation
        double weight = 1.0;   // weighted balance
        Node* left = nullptr;
        Node* right = nullptr;
    };

    Node* m_root = nullptr;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Get height of node */
    int height(Node* node) const { return node ? node->height : 0; }

    /** @brief Get subtree size of node */
    int size(Node* node) const { return node ? node->subtreeSize : 0; }

    /** @brief Update height and rank fields */
    void update(Node* node);

    /** @brief Balance factor */
    int balanceFactor(Node* node) const;

    /** @brief Right rotation */
    Node* rotateRight(Node* y);

    /** @brief Left rotation */
    Node* rotateLeft(Node* x);

    /** @brief Rebalance node */
    Node* rebalance(Node* node);

    /** @brief Recursive insert */
    Node* insertNode(Node* node, int key, double value);

    /** @brief Recursive remove */
    Node* removeNode(Node* node, int key);

    /** @brief Find minimum in subtree */
    Node* findMin(Node* node) const;

    /** @brief Recursive search */
    double searchNode(Node* node, int key) const;

    /** @brief Select k-th smallest */
    int selectNode(Node* node, int k) const;

    /** @brief Rank of key */
    int rankNode(Node* node, int key) const;

    /** @brief Range query helper */
    void rangeHelper(Node* node, int lo, int hi,
                      QVector<QPair<int, double>>& result) const;

    /** @brief Range count helper */
    int rangeCountHelper(Node* node, int lo, int hi) const;

    /** @brief In-order traversal helper */
    void inorderHelper(Node* node, QVector<QPair<int, double>>& result) const;

    /** @brief Delete entire subtree */
    void deleteTree(Node* node);

    /** @brief Compute tree height */
    int computeHeight(Node* node) const;
};
