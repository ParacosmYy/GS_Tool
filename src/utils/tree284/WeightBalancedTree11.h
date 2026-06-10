/**
 * @file WeightBalancedTree11.h
 * @brief 权重平衡树(三路重平衡与平衡因子传播的BB[alpha]树维护) — Weight-balanced Tree with 3-way Rebalancing and Balance Factor Propagation for BB[alpha] Tree Maintenance
 *
 * 功能: 实现权重平衡树(weight-balanced tree)，采用三路重平衡(3-way rebalancing)
 *       与平衡因子传播(balance factor propagation)实现BB[alpha]树维护(BB[alpha] tree maintenance)。
 *
 * 协作: AVLTree10(AVL树) / RedBlackTree11(红黑树) / SplayTree10(伸展树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 权重平衡树(三路重平衡与平衡因子传播)
 */
class WeightBalancedTree11 : public QObject {
    Q_OBJECT

public:
    /** @brief Tree node */
    struct WBNode {
        int key = 0;
        double value = 0.0;
        int left = -1;
        int right = -1;
        int parent = -1;
        int weight = 1;        // Subtree weight (size)
        double balance = 0.5;  // Balance factor: left.weight / total
    };

    /** @brief Search result */
    struct SearchResult {
        int nodeIdx = -1;
        double value = 0.0;
        bool found = false;
        int depth = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int treeSize = 0;
        int treeHeight = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit WeightBalancedTree11(QObject *parent = nullptr);
    ~WeightBalancedTree11() override;

    void setAlpha(double alpha);

    /** @brief Insert key-value pair */
    bool insert(int key, double value);

    /** @brief Remove key */
    bool remove(int key);

    /** @brief Search for key */
    SearchResult search(int key) const;

    /** @brief Get all keys in sorted order */
    QVector<int> inOrderKeys() const;

    /** @brief Get node by index */
    const WBNode& node(int idx) const;

    int root() const { return m_root; }
    int size() const { return m_size; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void insertDone(int key, int height, double timeMs);
    void removeDone(int key, int height, double timeMs);
    void rebalanceDone(int node, int oldWeight, int newWeight);

private:
    double m_alpha = 0.29;  // BB[alpha] parameter (typically 0.288..0.35)
    int m_root = -1;
    int m_size = 0;
    Stats m_stats;
    double m_timeSum = 0.0;

    QVector<WBNode> m_nodes;
    QVector<int> m_freeList;  // Free node indices for reuse

    /** @brief Allocate a new node */
    int allocNode(int key, double value);

    /** @brief Free a node */
    void freeNode(int idx);

    /** @brief Update weight and balance factor */
    void updateWeight(int idx);

    /** @brief Check if balance factor violates BB[alpha] constraint */
    bool isUnbalanced(int idx) const;

    /** @brief 3-way rebalance: select best rotation among LL, RR, LR, RL */
    void rebalance(int idx);

    /** @brief Single rotation left */
    int rotateLeft(int idx);

    /** @brief Single rotation right */
    int rotateRight(int idx);

    /** @brief Propagate balance factors up to root */
    void propagateBalance(int idx);

    /** @brief Inorder traversal helper */
    void inOrderHelper(int idx, QVector<int>& result) const;

    /** @brief Compute tree height */
    int computeHeight(int idx) const;
};
