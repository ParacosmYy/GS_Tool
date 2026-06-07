/**
 * @file ScapegoatTree6.h
 * @brief 替罪羊树(权重平衡重建触发+分数级联手指搜索) — Scapegoat Tree with Weight-Balanced Rebuild Trigger and Fractional Cascading for Finger Search
 *
 * 功能: 实现替罪羊树，支持权重平衡重建触发、
 *       分数级联加速的手指搜索和动态平衡维护。
 *
 * 协作: AvlTree5(AVL树) / RedBlackTree8(红黑树) / BPlusTree8(B+树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 替罪羊树(权重平衡重建触发+分数级联手指搜索)
 */
class ScapegoatTree6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int treeSize = 0;
        int treeHeight = 0;
        int numRebuilds = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ScapegoatTree6(QObject *parent = nullptr);
    ~ScapegoatTree6() override;

    void setAlpha(double alpha);

    /** @brief Insert key-value pair */
    void insert(int key, double value);

    /** @brief Remove a key */
    void remove(int key);

    /** @brief Look up value by key */
    QPair<bool, double> lookup(int key) const;

    /** @brief Finger search: search from a hint position */
    QPair<bool, double> fingerSearch(int key, int hintKey) const;

    /** @brief Range query [lo, hi] */
    QVector<QPair<int, double>> rangeQuery(int lo, int hi) const;

    /** @brief In-order traversal */
    QVector<QPair<int, double>> inOrderTraversal() const;

    /** @brief Check if tree is balanced */
    bool isBalanced() const;

    /** @brief Clear all data */
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int key, double timeMs);

private:
    double m_alpha = 0.6;

    /** @brief Tree node stored in pool */
    struct Node {
        int key = 0;
        double value = 0.0;
        int left = -1;
        int right = -1;
        int subtreeSize = 1;
        // Fractional cascading hints
        int cascadeLeft = -1;   // Nearest predecessor in left subtree
        int cascadeRight = -1;  // Nearest successor in right subtree
    };

    QVector<Node> m_nodes;
    int m_root = -1;
    int m_maxSize = 0;  // Track max size since last rebuild

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Allocate a new node */
    int allocNode(int key, double value);

    /** @brief Compute subtree size */
    int updateSize(int idx);

    /** @brief Check if subtree is alpha-weight-balanced */
    bool isAlphaBalanced(int idx) const;

    /** @brief Flatten subtree into sorted array */
    void flatten(int idx, QVector<int>& sorted) const;

    /** @brief Rebuild balanced subtree from sorted node indices */
    int rebuildBalanced(const QVector<int>& sorted, int lo, int hi);

    /** @brief Find scapegoat node on insertion path */
    int findScapegoat(int idx, int key) const;

    /** @brief Build fractional cascading hints */
    void buildCascadeHints(int idx);

    /** @brief Recursive range collect */
    void rangeCollect(int idx, int lo, int hi,
                       QVector<QPair<int, double>>& result) const;
};
