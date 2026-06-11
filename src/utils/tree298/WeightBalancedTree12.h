/**
 * @file WeightBalancedTree12.h
 * @brief 权重平衡树(alpha平衡旋转与秩增强节点实现保证深度界的确定性平衡BST) — Weight-balanced Tree with Alpha-balanced Rotation and Rank-augmented Node for Deterministic Balanced BST with Guaranteed Depth Bounds
 *
 * 功能: 实现权重平衡树(weight-balanced tree)，采用alpha平衡旋转(alpha-balanced rotation)
 *       与秩增强节点(rank-augmented node)实现保证深度界的确定性平衡BST(deterministic balanced BST with guaranteed depth bounds)。
 *
 * 协作: CartesianTree13(笛卡尔树) / SplayTree14(伸展树) / AVLTree(AVL树)
 */
#pragma once

#include <QObject>
#include <QVector>

class WeightBalancedTree12 : public QObject {
    Q_OBJECT

public:
    /** @brief Tree node with weight/rank augmentation */
    struct Node {
        double key = 0.0;
        int weight = 1;       // subtree size
        int rank = 0;         // black-height-like rank
        int left = -1;
        int right = -1;
        int parent = -1;
    };

    /** @brief Search result */
    struct SearchResult {
        bool found = false;
        int nodeIndex = -1;
        int depth = 0;
        int comparisons = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalInserts = 0;
        quint64 totalDeletes = 0;
        quint64 totalSearches = 0;
        int treeSize = 0;
        int treeHeight = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit WeightBalancedTree12(QObject *parent = nullptr);
    ~WeightBalancedTree12() override;

    void setAlpha(double alpha);

    /** @brief Insert a key, rebalance via alpha-balanced rotations */
    bool insert(double key);

    /** @brief Remove a key, rebalance */
    bool remove(double key);

    /** @brief Search for a key */
    SearchResult search(double key) const;

    /** @brief In-order traversal */
    QVector<double> inOrder() const;

    /** @brief Get all nodes */
    const QVector<Node>& nodes() const { return m_nodes; }

    int root() const { return m_root; }
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void insertDone(double key, int size, double timeMs);
    void removeDone(double key, int size, double timeMs);

private:
    double m_alpha = 0.29;    // Balance parameter (Niemann threshold ~0.29)
    QVector<Node> m_nodes;
    int m_root = -1;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Allocate a new node, return its index */
    int allocNode(double key);

    /** @brief Free a node (swap-remove from array) */
    void freeNode(int idx);

    /** @brief Get weight of subtree rooted at idx */
    int weight(int idx) const;

    /** @brief Update weight and rank for node */
    void updateMeta(int idx);

    /** @brief Check and rebalance if alpha-balance violated */
    int rebalance(int idx);

    /** @brief Single rotation left */
    int rotateLeft(int idx);

    /** @brief Single rotation right */
    int rotateRight(int idx);

    /** @brief Recursive in-order helper */
    void inOrderHelper(int idx, QVector<double>& result) const;
};
