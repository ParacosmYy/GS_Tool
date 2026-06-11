/**
 * @file AvlTree11.h
 * @brief AVL树(迭代再平衡与父指针增强支持锁友好的并发读路径优化) — AVL Tree with Iterative Rebalancing and Parent-pointer Augmentation for Lock-friendly Concurrent Read Path Optimization
 *
 * 功能: 实现AVL树(AVL tree)，采用迭代再平衡(iterative rebalancing)
 *       与父指针增强(parent-pointer augmentation)支持锁友好的并发读路径优化(lock-friendly concurrent read path optimization)。
 *
 * 协作: AA11(AA树) / RedBlackTree10(红黑树) / SplayTree8(伸展树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief AVL树(迭代再平衡与父指针增强支持锁友好的并发读路径优化)
 */
class AvlTree11 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numNodes = 0;
        int treeHeight = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit AvlTree11(QObject *parent = nullptr);
    ~AvlTree11() override;

    /** @brief Insert a key-value pair */
    void insert(int key, int value);

    /** @brief Remove a key */
    void remove(int key);

    /** @brief Search for a key, returns value or -1 */
    int search(int key) const;

    /** @brief Check if key exists */
    bool contains(int key) const;

    /** @brief In-order traversal: all key-value pairs */
    QVector<QPair<int,int>> toVector() const;

    /** @brief Get all keys in sorted order */
    QVector<int> keys() const;

    int size() const { return m_size; }
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationDone(const QString& op, int key, double timeMs);

private:
    static constexpr int NULL_NODE = -1;

    /** @brief AVL node with parent pointer */
    struct AVLNode {
        int key = 0;
        int value = 0;
        int height = 1;
        int balanceFactor = 0;
        int left = NULL_NODE;
        int right = NULL_NODE;
        int parent = NULL_NODE;    // Parent pointer for lock-friendly reads
    };

    QVector<AVLNode> m_nodes;
    QVector<int> m_freeList;
    int m_root = NULL_NODE;
    int m_size = 0;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Allocate a new node */
    int allocNode(int key, int value, int parent);

    /** @brief Free a node to pool */
    void freeNode(int idx);

    /** @brief Update height and balance factor */
    void updateHeight(int idx);

    /** @brief Right rotation */
    int rotateRight(int y);

    /** @brief Left rotation */
    int rotateLeft(int x);

    /** @brief Iterative rebalance from node up to root */
    void rebalanceUpward(int startIdx);

    /** @brief Iterative insert */
    void insertIter(int key, int value);

    /** @brief Iterative remove */
    void removeIter(int key);

    /** @brief Find minimum in subtree */
    int findMin(int idx) const;

    /** @brief In-order traversal helper */
    void inOrder(int idx, QVector<QPair<int,int>>& result) const;

    /** @brief Compute tree height */
    int computeHeight(int idx) const;

    /** @brief Update parent links after rotation */
    void updateChildLink(int parent, int oldChild, int newChild);
};
