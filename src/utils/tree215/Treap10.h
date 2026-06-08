/**
 * @file Treap10.h
 * @brief 隐式键Treap(分裂-合并数组操作+懒传播区间更新) — Treap with Implicit Key for Array Operations Supporting Split-Merge Range Update and Lazy Propagation
 *
 * 功能: 实现隐式键Treap，支持按位置分裂合并、
 *       区间更新懒传播和数组下标操作。
 *
 * 协作: WeightBalancedTree6(权重平衡树) / AVLTree5(AVL树) / SegmentTree3(线段树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 隐式键Treap(分裂-合并+懒传播区间更新)
 */
class Treap10 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int treeSize = 0;
        int treeHeight = 0;
        int splitCount = 0;
        int mergeCount = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Treap10(QObject *parent = nullptr);
    ~Treap10() override;

    /** @brief Build treap from initial values */
    void build(const QVector<double>& values);

    /** @brief Insert value at position */
    void insert(int pos, double value);

    /** @brief Remove element at position */
    void remove(int pos);

    /** @brief Get value at position */
    double get(int pos) const;

    /** @brief Set value at position */
    void set(int pos, double value);

    /** @brief Add delta to range [l, r] with lazy propagation */
    void rangeUpdate(int l, int r, double delta);

    /** @brief Query sum of range [l, r] */
    double rangeQuery(int l, int r) const;

    /** @brief Split treap at position into left and right */
    QPair<int, int> split(int root, int pos) const;

    /** @brief Merge two treaps */
    int merge(int left, int right);

    /** @brief Get all values in order */
    QVector<double> toVector() const;

    /** @brief Get tree size */
    int size() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int size, double timeMs);

private:
    struct Node {
        double value = 0.0;
        double sum = 0.0;
        double lazy = 0.0;       // Pending lazy add
        int priority = 0;
        int left = -1;
        int right = -1;
        int size = 1;
        bool hasLazy = false;
    };

    QVector<Node> m_nodes;
    int m_root = -1;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Allocate a new node */
    int allocateNode(double value);

    /** @brief Update aggregate info (sum, size) */
    void update(int node);

    /** @brief Push lazy propagation to children */
    void pushDown(int node);

    /** @brief Compute tree height */
    int computeHeight(int node) const;

    /** @brief In-order traversal into vector */
    void inOrder(int node, QVector<double>& result) const;
};
