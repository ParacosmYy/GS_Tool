/**
 * @file WeightBalancedTree6.h
 * @brief 权重平衡树(Alpha平衡再平衡+替罪羊式批量重建) — Weight-Balanced Tree with Alpha-Balanced Rebalancing and Scapegoat-Like Bulk Rebuild for Skewed Insertions
 *
 * 功能: 实现权重平衡树，支持Alpha平衡约束、
 *       替罪羊式批量重建和倾斜插入优化。
 *
 * 协作: CartesianTree7(笛卡尔树) / AVLTree5(AVL树) / RedBlackTree4(红黑树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 权重平衡树(Alpha平衡+替罪羊批量重建)
 */
class WeightBalancedTree6 : public QObject {
    Q_OBJECT

public:
    /** @brief Tree node */
    struct Node {
        double key = 0.0;
        int value = 0;
        int left = -1;
        int right = -1;
        int weight = 1;     // Subtree size for weight balancing
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int treeSize = 0;
        int treeHeight = 0;
        int rebuildCount = 0;
        double alpha = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit WeightBalancedTree6(QObject *parent = nullptr);
    ~WeightBalancedTree6() override;

    /** @brief Set alpha balance parameter (0.5 < alpha < 1.0) */
    void setAlpha(double alpha);

    /** @brief Insert key-value pair */
    void insert(double key, int value);

    /** @brief Remove key */
    void remove(double key);

    /** @brief Search for key, returns value or -1 */
    int search(double key) const;

    /** @brief In-order traversal */
    QVector<QPair<double, int>> inOrder() const;

    /** @brief Check if tree is alpha-balanced */
    bool isBalanced() const;

    /** @brief Bulk rebuild entire tree from sorted data */
    void bulkRebuild(const QVector<QPair<double, int>>& sortedData);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void treeUpdated(int size, int height, double timeMs);
    void rebuildTriggered(int size, double timeMs);

private:
    double m_alpha = 0.7;
    QVector<Node> m_nodes;
    int m_root = -1;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Allocate a new node, returns index */
    int allocateNode(double key, int value);

    /** @brief Recursive insert with rebalancing */
    int insertRec(int node, double key, int value, bool& rebuilt);

    /** @brief Find scapegoat node for rebuilding */
    int findScapegoat(int node, double key) const;

    /** @brief Rebuild subtree rooted at node */
    int rebuild(int node);

    /** @brief Flatten subtree into sorted array */
    void flatten(int node, QVector<int>& out) const;

    /** @brief Build balanced tree from sorted node indices */
    int buildBalanced(const QVector<int>& sorted, int start, int end);

    /** @brief Update weight of node */
    void updateWeight(int node);

    /** @brief Check alpha-balance condition */
    bool isAlphaBalanced(int node) const;

    /** @brief In-order traversal recursive */
    void inOrderRec(int node, QVector<QPair<double, int>>& result) const;

    /** @brief Compute height */
    int computeHeight(int node) const;
};
