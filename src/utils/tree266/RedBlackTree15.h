/**
 * @file RedBlackTree15.h
 * @brief 红黑树(自底向上插入修复与自顶向下中序后继提升删除平衡BST) — Red-Black Tree with Bottom-Up Insertion Fixup and Top-Down Deletion with In-Order Successor Promotion for Balanced BST
 *
 * 功能: 实现红黑树(red-black tree)，采用自底向上插入修复(bottom-up insertion fixup)
 *       和自顶向下中序后继提升删除(top-down deletion with in-order successor promotion)
 *       实现平衡BST(balanced BST)。
 *
 * 协作: AvlTree9(AVL树) / BTree8(B树) / VanEmdeBoas9(van Emde Boas树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 红黑树(自底向上插入修复与自顶向下中序后继提升删除平衡BST)
 */
class RedBlackTree15 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numElements = 0;
        int treeHeight = 0;
        int numRotations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit RedBlackTree15(QObject *parent = nullptr);
    ~RedBlackTree15() override;

    /** @brief Insert a key into the tree */
    void insert(int key);

    /** @brief Remove a key from the tree */
    void remove(int key);

    /** @brief Check if key exists */
    bool contains(int key) const;

    /** @brief Find minimum key, or INT_MIN if empty */
    int minimum() const;

    /** @brief Find maximum key, or INT_MAX if empty */
    int maximum() const;

    /** @brief Get successor of key, or INT_MAX if none */
    int successor(int key) const;

    /** @brief Get predecessor of key, or INT_MIN if none */
    int predecessor(int key) const;

    /** @brief Get all keys in sorted order */
    QVector<int> inOrderKeys() const;

    /** @brief Get number of elements */
    int size() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void treeUpdated(int numElements, int height, int rotations, double timeMs);

private:
    enum Color { Red, Black };

    struct Node {
        int key = 0;
        Color color = Red;
        Node* left = nullptr;
        Node* right = nullptr;
        Node* parent = nullptr;
    };

    Node* m_root = nullptr;
    Node* m_nil = nullptr;  // Sentinel node
    int m_size = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Left rotation around node x */
    void rotateLeft(Node* x);

    /** @brief Right rotation around node y */
    void rotateRight(Node* y);

    /** @brief Bottom-up fixup after insertion */
    void insertFixup(Node* z);

    /** @brief Top-down deletion with successor promotion */
    void deleteNode(Node* z);

    /** @brief Fixup after deletion */
    void deleteFixup(Node* x);

    /** @brief Transplant subtree u with v */
    void transplant(Node* u, Node* v);

    /** @brief Find minimum node in subtree */
    Node* treeMinimum(Node* node) const;

    /** @brief Find maximum node in subtree */
    Node* treeMaximum(Node* node) const;

    /** @brief Find node with given key */
    Node* search(int key) const;

    /** @brief Collect keys in-order */
    void inOrderCollect(Node* node, QVector<int>& keys) const;

    /** @brief Compute height of tree */
    int computeHeight(Node* node) const;

    /** @brief Recursively delete all nodes */
    void destroyTree(Node* node);
};
