/**
 * @file SplayTree12.h
 * @brief 伸展树(自顶向下zig-zig/zig-zag重构与半伸展摊还再平衡自调整BST) — Splay Tree with Top-Down Zig-Zig/Zig-Zag Restructuring and Semi-Splay Amortized Rebalancing for Self-Adjusting BST
 *
 * 功能: 实现伸展树(Splay tree)，采用自顶向下zig-zig/zig-zag重构(top-down zig-zig/zig-zag
 *       restructuring)和半伸展摊还再平衡(semi-splay amortized rebalancing)实现自调整BST。
 *
 * 协作: RedBlackTree15(红黑树) / AvlTree9(AVL树) / Treap11(Treap)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 伸展树(自顶向下zig-zig/zig-zag重构与半伸展摊还再平衡自调整BST)
 */
class SplayTree12 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numElements = 0;
        int treeHeight = 0;
        int numSplays = 0;
        int numRotations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SplayTree12(QObject *parent = nullptr);
    ~SplayTree12() override;

    /** @brief Insert key, splay to root */
    void insert(int key);

    /** @brief Remove key */
    void remove(int key);

    /** @brief Search for key, splay if found */
    bool contains(int key);

    /** @brief Find minimum key (splay to root) */
    int minimum();

    /** @brief Find maximum key (splay to root) */
    int maximum();

    /** @brief Get all keys in sorted order */
    QVector<int> inOrderKeys() const;

    /** @brief Get number of elements */
    int size() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void treeUpdated(int numElements, int height, int splays, double timeMs);

private:
    struct Node {
        int key = 0;
        Node* left = nullptr;
        Node* right = nullptr;
    };

    Node* m_root = nullptr;
    int m_size = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Top-down splay: bring key to root */
    Node* splay(Node* root, int key);

    /** @brief Right rotation */
    Node* rotateRight(Node* y);

    /** @brief Left rotation */
    Node* rotateLeft(Node* x);

    /** @brief Semi-splay: partial restructuring for amortized balance */
    Node* semiSplay(Node* root);

    /** @brief In-order traversal */
    void inOrderCollect(Node* node, QVector<int>& keys) const;

    /** @brief Compute tree height */
    int computeHeight(Node* node) const;

    /** @brief Recursively destroy tree */
    void destroyTree(Node* node);
};
