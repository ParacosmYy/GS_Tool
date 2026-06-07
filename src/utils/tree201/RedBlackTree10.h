/**
 * @file RedBlackTree10.h
 * @brief 红黑树(增强区间数据+重叠查询+中序后继线索化) — Red-Black Tree with Augmented Interval Data, Overlap Query and In-Order Successor Threading
 *
 * 功能: 实现增强红黑树，支持区间数据存储、
 *       重叠区间查询和中序后继线索化遍历。
 *
 * 协作: WAVL5(弱AVL树) / Treap8(Treap) / IntervalTree3(区间树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 红黑树(增强区间数据+重叠查询+中序后继线索化)
 */
class RedBlackTree10 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOperations = 0;
        int nodeCount = 0;
        int treeHeight = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit RedBlackTree10(QObject *parent = nullptr);
    ~RedBlackTree10() override;

    /** @brief Insert interval [lo, hi] with value */
    void insert(double lo, double hi, double value);

    /** @brief Remove interval by low endpoint */
    bool remove(double lo);

    /** @brief Find all intervals overlapping [lo, hi] */
    QVector<QPair<QPair<double, double>, double>> queryOverlap(double lo, double hi) const;

    /** @brief Get all intervals in order */
    QVector<QPair<QPair<double, double>, double>> inOrderTraversal() const;

    /** @brief Thread all nodes with in-order successor links */
    void buildThreading();

    /** @brief Traverse using successor threading */
    QVector<QPair<QPair<double, double>, double>> threadedTraversal() const;

    /** @brief Number of nodes */
    int size() const;
    bool isEmpty() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void treeUpdated(const QString& op, int size, int height, double timeMs);

private:
    enum class Color { Red, Black };

    struct Node {
        double lo = 0.0, hi = 0.0, value = 0.0;
        double maxHi = 0.0;     // augmented: max hi in subtree
        Color color = Color::Red;
        Node* left = nullptr;
        Node* right = nullptr;
        Node* parent = nullptr;
        Node* successor = nullptr;  // threaded link

        ~Node() { delete left; delete right; }
    };

    Node* m_root = nullptr;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Fix red-black properties after insert */
    void fixInsert(Node* n);

    /** @brief Fix red-black properties after delete */
    void fixDelete(Node* n);

    /** @brief Left rotation */
    void rotateLeft(Node* x);

    /** @brief Right rotation */
    void rotateRight(Node* y);

    /** @brief Update augmented maxHi field */
    void updateMaxHi(Node* n);

    /** @brief Transplant subtree */
    void transplant(Node* u, Node* v);

    /** @brief Find minimum node */
    static Node* treeMinimum(Node* n);

    /** @brief In-order successor (tree-based) */
    static Node* treeSuccessor(Node* n);

    /** @brief Build threading recursively */
    Node* buildThreadingHelper(Node* n, Node* prev);
};
