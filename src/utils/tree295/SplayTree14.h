/**
 * @file SplayTree14.h
 * @brief 伸展树(自顶向下zig-zig与半伸展摊还重平衡实现改进访问局部性的自调整BST) — Splay Tree with Top-down Zig-zig and Semi-splay Amortized Rebalancing for Improved Access Locality in Self-adjusting BST
 *
 * 功能: 实现伸展树(Splay tree)，采用自顶向下zig-zig(top-down zig-zig)
 *       与半伸展摊还重平衡(semi-splay amortized rebalancing)实现改进访问局部性的自调整BST(improved access locality in self-adjusting BST)。
 *
 * 协作: RedBlackTree17(红黑树) / VanEmdeBoas11(Van Emde Boas树) / BTree9(B树)
 */
#pragma once

#include <QObject>
#include <QVector>

class SplayTree14 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numNodes = 0;
        int treeHeight = 0;
        int numSplays = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SplayTree14(QObject *parent = nullptr);
    ~SplayTree14() override;

    /** @brief Insert key-value pair, splaying the new node to root */
    void insert(int key, double value = 0.0);

    /** @brief Remove key, splaying the parent of removed node */
    void remove(int key);

    /** @brief Search for key and splay it to root */
    bool contains(int key);

    /** @brief Get value by key (splays to root) */
    double value(int key);

    /** @brief In-order traversal */
    QVector<int> keys() const;

    /** @brief Get tree height */
    int height() const;

    int size() const { return m_size; }
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationDone(const QString& op, int key, int height, double timeMs);

private:
    /** @brief Tree node with pool index */
    struct Node {
        int key = 0;
        double value = 0.0;
        int left = -1;
        int right = -1;
        int parent = -1;
    };

    static constexpr int NIL = -1;

    int m_root = NIL;
    int m_size = 0;
    Stats m_stats;
    double m_timeSum = 0.0;

    QVector<Node> m_pool;
    QVector<int> m_freeList;

    /** @brief Allocate node from pool */
    int allocNode(int key, double value);

    /** @brief Free node back to pool */
    void freeNode(int idx);

    /** @brief Expand pool */
    void expandPool();

    /** @brief Top-down splay: bring key to root */
    void splay(int key);

    /** @brief Zig rotation (right) */
    void rotateRight(int x);

    /** @brief Zag rotation (left) */
    void rotateLeft(int x);

    /** @brief Semi-splay: partial rebalancing on access path */
    void semiSplay(int node);

    /** @brief Find subtree maximum */
    int findMax(int node) const;

    /** @brief In-order traversal helper */
    void inOrder(int node, QVector<int>& result) const;

    /** @brief Compute height */
    int computeHeight(int node) const;
};
