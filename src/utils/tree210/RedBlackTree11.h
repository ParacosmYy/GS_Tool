/**
 * @file RedBlackTree11.h
 * @brief 红黑树(顺序统计量增强+秩查询+有序敏感范围删除) — Red-Black Tree with Order-Statistic Augmentation for Rank Queries and Order-Sensitive Range Deletion
 *
 * 功能: 实现红黑树，支持顺序统计量增强、
 *       秩查询(rank/select)和有序敏感范围删除。
 *
 * 协作: VanEmdeBoas5(vEB树) / BPlusTree8(B+树) / SplayTree7(伸展树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 红黑树(顺序统计量增强+秩查询+有序敏感范围删除)
 */
class RedBlackTree11 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numNodes = 0;
        int treeHeight = 0;
        int totalRotations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit RedBlackTree11(QObject *parent = nullptr);
    ~RedBlackTree11() override;

    /** @brief Insert key */
    void insert(int key);

    /** @brief Remove key */
    void remove(int key);

    /** @brief Check if key exists */
    bool contains(int key) const;

    /** @brief Get rank of key (1-based) */
    int rank(int key) const;

    /** @brief Find k-th smallest element (1-based) */
    int select(int k) const;

    /** @brief Range deletion: remove all keys in [lo, hi] */
    void rangeDelete(int lo, int hi);

    /** @brief Range query [lo, hi] in sorted order */
    QVector<int> rangeQuery(int lo, int hi) const;

    /** @brief Get all elements in sorted order */
    QVector<int> inorder() const;

    /** @brief Get tree size */
    int size() const;

    /** @brief Get tree height */
    int height() const;

    /** @brief Clear all nodes */
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int key, double timeMs);

private:
    enum Color { Red, Black };

    /** @brief Node with order-statistic augmentation */
    struct Node {
        int key;
        Color color;
        int size;       // Subtree size for order-statistics
        Node *left, *right, *parent;

        Node(int k) : key(k), color(Red), size(1),
            left(nullptr), right(nullptr), parent(nullptr) {}
    };

    Node *m_root = nullptr;
    Node *m_nil = nullptr;  // Sentinel node

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Left rotation */
    void rotateLeft(Node *x);

    /** @brief Right rotation */
    void rotateRight(Node *x);

    /** @brief Fix tree after insert */
    void insertFixup(Node *z);

    /** @brief Fix tree after delete */
    void deleteFixup(Node *x);

    /** @brief Transplant subtree */
    void transplant(Node *u, Node *v);

    /** @brief Find minimum node */
    Node* minimum(Node *x) const;

    /** @brief Find maximum node */
    Node* maximum(Node *x) const;

    /** @brief Search for key */
    Node* search(int key) const;

    /** @brief Update subtree sizes up to root */
    void updateSizes(Node *x);

    /** @brief Recursive inorder collection */
    void inorderHelper(Node *x, QVector<int>& result) const;

    /** @brief Recursive range query */
    void rangeQueryHelper(Node *x, int lo, int hi, QVector<int>& result) const;

    /** @brief Recursive delete all nodes */
    void deleteTree(Node *x);

    /** @brief Recursive height calculation */
    int heightHelper(Node *x) const;
};
