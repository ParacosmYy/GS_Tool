/**
 * @file Treap15.h
 * @brief 树堆(哈希确定性优先级与隐式分裂合并的数组索引序列操作) — Treap with Deterministic Priority via Hash and Implicit Treap Split/Merge for Array-indexed Sequence Operations
 *
 * 功能: 实现树堆(Treap)，采用哈希确定性优先级(deterministic priority via hash)
 *       与隐式分裂合并(implicit treap split/merge)实现数组索引序列操作(array-indexed sequence operations)。
 *
 * 协作: AVLTree10(AVL树) / RedBlackTree11(红黑树) / SplayTree10(伸展树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 树堆(哈希确定性优先级与隐式分裂合并)
 */
class Treap15 : public QObject {
    Q_OBJECT

public:
    /** @brief Treap node */
    struct TreapNode {
        int key = 0;
        double value = 0.0;
        quint64 priority = 0;     // Deterministic hash-based
        int left = -1;
        int right = -1;
        int parent = -1;
        int size = 1;             // Subtree size for implicit treap
        bool reversed = false;    // Lazy reverse flag
    };

    /** @brief Search result */
    struct SearchResult {
        int nodeIdx = -1;
        double value = 0.0;
        bool found = false;
        int position = 0;
    };

    /** @brief Range query result */
    struct RangeResult {
        QVector<double> values;
        double sum = 0.0;
        int count = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int treeSize = 0;
        int treeHeight = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Treap15(QObject *parent = nullptr);
    ~Treap15() override;

    /** @brief Insert at position (implicit treap) */
    bool insertAt(int pos, int key, double value);

    /** @brief Remove at position */
    bool removeAt(int pos);

    /** @brief Get value at position */
    SearchResult getAt(int pos) const;

    /** @brief Insert by key (BST treap) */
    bool insert(int key, double value);

    /** @brief Remove by key */
    bool remove(int key);

    /** @brief Search by key */
    SearchResult search(int key) const;

    /** @brief Reverse range [l, r) */
    void reverseRange(int l, int r);

    /** @brief Range query [l, r) */
    RangeResult rangeQuery(int l, int r);

    int size() const { return m_size; }
    int root() const { return m_root; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void insertDone(int key, int pos, double timeMs);
    void removeDone(int key, int pos, double timeMs);

private:
    int m_root = -1;
    int m_size = 0;
    Stats m_stats;
    double m_timeSum = 0.0;

    QVector<TreapNode> m_nodes;
    QVector<int> m_freeList;

    /** @brief Compute deterministic priority from key */
    quint64 hashPriority(int key) const;

    /** @brief Allocate a new node */
    int allocNode(int key, double value);

    /** @brief Free a node */
    void freeNode(int idx);

    /** @brief Update subtree size */
    void updateSize(int idx);

    /** @brief Push down lazy reverse flag */
    void pushDown(int idx);

    /** @brief Split treap at position into [0,pos) and [pos,end) */
    void split(int root, int pos, int& left, int& right);

    /** @brief Merge two treaps (all keys in left < right) */
    int merge(int left, int right);

    /** @brief BST insert with priority heap property */
    int bstInsert(int root, int nodeIdx);

    /** @brief Rotate to maintain heap property */
    int rotateUp(int idx);

    /** @brief Compute tree height */
    int computeHeight(int idx) const;

    /** @brief Inorder traversal */
    void inOrderHelper(int idx, QVector<QPair<int, double>>& result) const;
};
