/**
 * @file RedBlackTree17.h
 * @brief 红黑树(迭代插入修复与节点池化实现缓存友好内存分配的平衡BST操作) — Red-black Tree with Iterative Insertion Fixup and Node Pooling for Cache-friendly Memory Allocation in Balanced BST Operations
 *
 * 功能: 实现红黑树(red-black tree)，采用迭代插入修复(iterative insertion fixup)
 *       与节点池化(node pooling)实现缓存友好内存分配的平衡BST操作(cache-friendly memory allocation in balanced BST operations)。
 *
 * 协作: VanEmdeBoas11(Van Emde Boas树) / ScapegoatTree12(替罪羊树) / BTree9(B树)
 */
#pragma once

#include <QObject>
#include <QVector>

class RedBlackTree17 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numNodes = 0;
        int treeHeight = 0;
        int poolCapacity = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit RedBlackTree17(QObject *parent = nullptr);
    ~RedBlackTree17() override;

    /** @brief Set initial node pool capacity */
    void setPoolCapacity(int capacity);

    /** @brief Insert key-value pair */
    void insert(int key, double value = 0.0);

    /** @brief Remove key */
    void remove(int key);

    /** @brief Search for key, returns true if found */
    bool contains(int key) const;

    /** @brief Get value by key (0.0 if not found) */
    double value(int key) const;

    /** @brief In-order traversal */
    QVector<int> keys() const;

    /** @brief Get tree height */
    int height() const;

    int size() const { return m_size; }
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationDone(const QString& op, int key, double timeMs);

private:
    enum Color { Red, Black };

    /** @brief Pooled tree node */
    struct Node {
        int key = 0;
        double value = 0.0;
        Color color = Red;
        int left = -1;
        int right = -1;
        int parent = -1;
    };

    static constexpr int NIL = -1;

    int m_root = NIL;
    int m_size = 0;
    int m_poolCap = 256;
    Stats m_stats;
    double m_timeSum = 0.0;

    QVector<Node> m_pool;           // Node pool
    QVector<int> m_freeList;        // Free node indices

    /** @brief Allocate a node from pool */
    int allocNode(int key, double value);

    /** @brief Release node back to pool */
    void freeNode(int idx);

    /** @brief Expand pool if needed */
    void expandPool();

    /** @brief Left rotation */
    void rotateLeft(int x);

    /** @brief Right rotation */
    void rotateRight(int x);

    /** @brief Iterative insertion fixup */
    void insertFixup(int z);

    /** @brief Transplant subtree */
    void transplant(int u, int v);

    /** @brief Delete fixup */
    void deleteFixup(int x);

    /** @brief Find minimum node in subtree */
    int minimum(int x) const;

    /** @brief In-order traversal helper */
    void inOrder(int node, QVector<int>& result) const;

    /** @brief Compute height recursively */
    int computeHeight(int node) const;
};
