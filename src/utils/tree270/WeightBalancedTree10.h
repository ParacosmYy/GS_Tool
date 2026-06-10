/**
 * @file WeightBalancedTree10.h
 * @brief 权重平衡树(秩平衡旋转与Alpha权重约束确定性平衡BST) — Weight-Balanced Tree with Rank-Balanced Rotations and Alpha-Weight Constraint for Deterministic Balanced BST
 *
 * 功能: 实现权重平衡树(Weight-balanced tree)，采用秩平衡旋转(rank-balanced rotations)
 *       与Alpha权重约束(alpha-weight constraint)实现确定性平衡BST(deterministic balanced BST)。
 *
 * 协作: CartesianTree11(笛卡尔树) / SplayTree12(伸展树) / AVLTree10(AVL树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 权重平衡树(秩平衡旋转与Alpha权重约束确定性平衡BST)
 */
class WeightBalancedTree10 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int treeSize = 0;
        int treeHeight = 0;
        int numRotations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Tree node */
    struct Node {
        double key = 0.0;
        int value = 0;
        int weight = 1;     // subtree size
        int left = -1;
        int right = -1;
        int parent = -1;
    };

    explicit WeightBalancedTree10(QObject *parent = nullptr);
    ~WeightBalancedTree10() override;

    /** @brief Set alpha balance factor (0.25..0.5) */
    void setAlpha(double alpha);

    /** @brief Insert a key-value pair */
    void insert(double key, int value = 0);

    /** @brief Remove a key */
    bool remove(double key);

    /** @brief Search for key, returns value (or -1 if not found) */
    int search(double key) const;

    /** @brief In-order traversal */
    QVector<double> inOrderKeys() const;

    /** @brief Get tree height */
    int height() const;

    /** @brief Get node count */
    int size() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void treeUpdated(int size, int height, int rotations, double timeMs);

private:
    double m_alpha = 0.288;  // Nievergelt-Reingold threshold (~1-1/sqrt(2))
    int m_root = -1;
    int m_rotations = 0;
    QVector<Node> m_nodes;
    QVector<int> m_freeList;     // recycled node indices

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Allocate a new node */
    int allocNode(double key, int value);

    /** @brief Free a node to recycle list */
    void freeNode(int idx);

    /** @brief Update weight from children */
    void updateWeight(int idx);

    /** @brief Check and rebalance subtree rooted at idx */
    int rebalance(int idx);

    /** @brief Perform left rotation, returns new root */
    int rotateLeft(int idx);

    /** @brief Perform right rotation, returns new root */
    int rotateRight(int idx);

    /** @brief Recursive insert helper */
    int insertRec(int idx, double key, int value);

    /** @brief Recursive remove helper */
    int removeRec(int idx, double key);

    /** @brief In-order traversal helper */
    void inOrderHelper(int idx, QVector<double>& result) const;

    /** @brief Height helper */
    int heightHelper(int idx) const;
};
