/**
 * @file Treap8.h
 * @brief 隐式Treap(split/merge序列操作:区间插入/删除/反转) — Implicit Treap with Split/Merge for Sequence Operations: Insert/Delete/Reverse at Range
 *
 * 功能: 实现隐式Treap，支持split/merge操作的
 *       区间插入、区间删除和区间反转。
 *
 * 协作: AA5(AA树) / SplayTree4(伸展树) / RedBlackTree6(红黑树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 隐式Treap(split/merge序列操作)
 */
class Treap8 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalOperations = 0;
        int nodeCount = 0;
        int treeHeight = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Treap8(QObject *parent = nullptr);
    ~Treap8() override;

    /** @brief Build treap from initial values */
    void build(const QVector<double>& values);

    /** @brief Insert value at position */
    void insert(int pos, double value);

    /** @brief Insert multiple values at position */
    void insertRange(int pos, const QVector<double>& values);

    /** @brief Delete element at position */
    bool removeAt(int pos);

    /** @brief Delete range [l, r) */
    int removeRange(int l, int r);

    /** @brief Reverse range [l, r) */
    void reverseRange(int l, int r);

    /** @brief Get value at position */
    double at(int pos) const;

    /** @brief Set value at position */
    void setAt(int pos, double value);

    /** @brief Get all values in order */
    QVector<double> toVector() const;

    /** @brief Range sum query [l, r) */
    double rangeSum(int l, int r);

    /** @brief Size of sequence */
    int size() const;

    /** @brief Check if empty */
    bool isEmpty() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void treeUpdated(const QString& op, int size, int height, double timeMs);

private:
    struct Node {
        double value = 0.0;
        int priority = 0;
        int size = 1;
        double sum = 0.0;
        bool rev = false;
        Node* left = nullptr;
        Node* right = nullptr;

        ~Node() { delete left; delete right; }
    };

    Node* m_root = nullptr;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Update node aggregates (size, sum) */
    void update(Node* n);

    /** @brief Push down lazy reversal flag */
    void pushDown(Node* n);

    /** @brief Get size of subtree */
    static int getSize(Node* n);

    /** @brief Get sum of subtree */
    static double getSum(Node* n);

    /** @brief Split treap at position k into (left, right) */
    QPair<Node*, Node*> split(Node* n, int k);

    /** @brief Merge two treaps */
    Node* merge(Node* left, Node* right);

    /** @brief Build balanced treap from sorted array */
    Node* buildFromVec(const QVector<double>& values, int lo, int hi);

    /** @brief In-order traversal */
    void inOrder(Node* n, QVector<double>& result) const;

    /** @brief Compute tree height */
    static int computeHeight(const Node* n);

    /** @brief Generate random priority */
    static int randPriority();
};
