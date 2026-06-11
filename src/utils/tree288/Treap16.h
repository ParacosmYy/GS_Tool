/**
 * @file Treap16.h
 * @brief 隐式键Treap(惰性传播与反转操作的高效范围查询随机优先级BST) — Treap with Implicit Key and Lazy Propagation for Efficient Range Queries with Reversals on Random-priority BST
 *
 * 功能: 实现隐式键Treap(Treap)，采用惰性传播(lazy propagation)
 *       与反转操作(reversals)实现高效范围查询随机优先级BST(efficient range queries on random-priority BST)。
 *
 * 协作: Rope12(绳索) / AVLTree9(AVL树) / SplayTree8(伸展树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief 隐式键Treap(惰性传播与反转操作的高效范围查询随机优先级BST)
 */
class Treap16 : public QObject {
    Q_OBJECT

public:
    /** @brief Aggregate info for range queries */
    struct NodeInfo {
        int size = 0;
        int sum = 0;
        int minVal = 0;
        int maxVal = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numNodes = 0;
        int treeHeight = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Treap16(QObject *parent = nullptr);
    ~Treap16() override;

    /** @brief Build treap from a sequence of values */
    void build(const QVector<int>& values);

    /** @brief Insert value at position */
    void insert(int pos, int value);

    /** @brief Remove element at position */
    void remove(int pos);

    /** @brief Get value at position */
    int at(int pos) const;

    /** @brief Reverse range [l, r] */
    void reverse(int l, int r);

    /** @brief Range sum query [l, r] */
    int rangeSum(int l, int r);

    /** @brief Range min query [l, r] */
    int rangeMin(int l, int r);

    /** @brief Range max query [l, r] */
    int rangeMax(int l, int r);

    /** @brief Get all values in order */
    QVector<int> toVector() const;

    int size() const { return m_size; }
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationDone(const QString& op, int pos, double timeMs);

private:
    static constexpr int NULL_NODE = -1;

    /** @brief Treap node stored in vector */
    struct TreapNode {
        int value = 0;
        int priority = 0;
        int left = NULL_NODE;
        int right = NULL_NODE;
        int parent = NULL_NODE;
        int size = 1;
        int sum = 0;
        int minVal = 0;
        int maxVal = 0;
        bool revLazy = false;          // Lazy reversal flag
    };

    QVector<TreapNode> m_nodes;
    QVector<int> m_freeList;
    int m_root = NULL_NODE;
    int m_size = 0;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Allocate a new node */
    int allocNode(int value);

    /** @brief Free a node */
    void freeNode(int idx);

    /** @brief Push lazy reversal flag down */
    void pushDown(int idx);

    /** @brief Pull aggregate info from children */
    void pullUp(int idx);

    /** @brief Split treap at position k into [0,k) and [k,end) */
    QPair<int, int> split(int root, int k);

    /** @brief Merge two treaps (all in left < all in right) */
    int merge(int left, int right);

    /** @brief In-order traversal to collect values */
    void inOrder(int idx, QVector<int>& result) const;

    /** @brief Compute tree height */
    int computeHeight(int idx) const;
};
