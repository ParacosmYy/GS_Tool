/**
 * @file IntervalTree6.h
 * @brief 区间树(增强min-start/max-end+窗口查询+刺穿计数优化) — Interval Tree with Augmented Min-Start/Max-End, Window Query and Stabbing-Count Optimization
 *
 * 功能: 实现增强区间树，支持min-start/max-end增强、
 *       窗口查询和刺穿计数优化。
 *
 * 协作: SegmentTree7(线段树) / RangeTree5(范围树) / RedBlackTree10(红黑树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 区间树(增强min-start/max-end+窗口查询+刺穿计数优化)
 */
class IntervalTree6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOperations = 0;
        int nodeCount = 0;
        int treeHeight = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit IntervalTree6(QObject *parent = nullptr);
    ~IntervalTree6() override;

    /** @brief Insert interval [lo, hi] with value */
    void insert(double lo, double hi, double value);

    /** @brief Remove interval by [lo, hi] */
    bool remove(double lo, double hi);

    /** @brief Query all intervals overlapping [qlo, qhi] */
    QVector<QPair<QPair<double, double>, double>> queryWindow(double qlo, double qhi) const;

    /** @brief Count intervals stabbed by point (optimized) */
    int stabbingCount(double point) const;

    /** @brief Query all intervals stabbed by point */
    QVector<QPair<QPair<double, double>, double>> stabbingQuery(double point) const;

    int size() const;
    bool isEmpty() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void treeUpdated(const QString& op, int size, int height, double timeMs);

private:
    struct Node {
        double lo = 0.0, hi = 0.0, value = 0.0;
        double maxEnd = 0.0;       // augmented: max hi in subtree
        double minStart = 0.0;     // augmented: min lo in subtree
        int count = 1;             // subtree size for stabbing-count
        int height = 1;            // AVL height
        Node* left = nullptr;
        Node* right = nullptr;

        ~Node() { delete left; delete right; }
    };

    Node* m_root = nullptr;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief AVL balance factor */
    static int balanceFactor(Node* n);

    /** @brief Update augmented fields */
    static void updateAugment(Node* n);

    /** @brief AVL rotations */
    Node* rotateLeft(Node* x);
    Node* rotateRight(Node* y);

    /** @brief Rebalance after insert/delete */
    Node* rebalance(Node* n);

    /** @brief Recursive insert */
    Node* insertImpl(Node* n, double lo, double hi, double value);

    /** @brief Recursive remove */
    Node* removeImpl(Node* n, double lo, double hi, bool& removed);

    /** @brief Find minimum node */
    static Node* treeMin(Node* n);

    /** @brief Compute tree height */
    static int treeHeight(Node* n);
};
