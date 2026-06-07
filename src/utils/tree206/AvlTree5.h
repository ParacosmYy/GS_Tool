/**
 * @file AvlTree5.h
 * @brief AVL树(增强子树最小/最大键+范围搜索+最近邻查找) — AVL Tree with Augmented Min/Max Subtree Key, Range Search Query and Nearest-Neighbor Lookup
 *
 * 功能: 实现增强AVL树，支持子树最小/最大键维护、
 *       范围搜索查询和最近邻查找。
 *
 * 协作: Treap9(Treap) / RedBlackTree8(红黑树) / BPlusTree7(B+树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief AVL树(增强子树最小/最大键+范围搜索+最近邻查找)
 */
class AvlTree5 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int treeSize = 0;
        int treeHeight = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit AvlTree5(QObject *parent = nullptr);
    ~AvlTree5() override;

    /** @brief Insert key-value pair */
    void insert(int key, double value);

    /** @brief Remove key */
    void remove(int key);

    /** @brief Look up value by key */
    QPair<bool, double> lookup(int key) const;

    /** @brief Range search: find all key-value pairs in [lo, hi] */
    QVector<QPair<int, double>> rangeSearch(int lo, int hi) const;

    /** @brief Nearest neighbor: find closest key to target */
    QPair<int, double> nearestNeighbor(int target) const;

    /** @brief Get tree height */
    int height() const;

    /** @brief Check if tree is empty */
    bool isEmpty() const;

    /** @brief Clear all nodes */
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int key, double timeMs);

private:
    /** @brief AVL node with augmented min/max */
    struct Node {
        int key = 0;
        double value = 0.0;
        int height = 1;
        int left = -1;
        int right = -1;
        int subtreeMin = 0;   // min key in subtree
        int subtreeMax = 0;   // max key in subtree
    };

    QVector<Node> m_nodes;
    int m_root = -1;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Get height of node (0 for null) */
    int nodeHeight(int idx) const;

    /** @brief Get balance factor */
    int balanceFactor(int idx) const;

    /** @brief Update height and augmented data */
    void updateNode(int idx);

    /** @brief Right rotation, return new root index */
    int rotateRight(int y);

    /** @brief Left rotation, return new root index */
    int rotateLeft(int x);

    /** @brief Recursive insert */
    int insertRec(int root, int key, double value);

    /** @brief Recursive remove */
    int removeRec(int root, int key);

    /** @brief Find min key node in subtree */
    int findMin(int root) const;

    /** @brief Range search recursive */
    void rangeSearchRec(int root, int lo, int hi,
                         QVector<QPair<int, double>>& result) const;

    /** @brief Nearest neighbor recursive */
    void nearestRec(int root, int target, int& bestKey,
                     double& bestDist, double& bestVal) const;
};
