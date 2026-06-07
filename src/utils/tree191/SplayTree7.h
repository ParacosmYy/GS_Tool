/**
 * @file SplayTree7.h
 * @brief 伸展树(zig/zig-zig/zig-zag+工作集界+手指搜索) — Splay Tree with Zig/Zig-Zig/Zig-Zag Operations, Working-Set Bound and Finger Search
 *
 * 功能: 实现伸展树(Splay Tree)，支持zig/zig-zig/zig-zag旋转操作、
 *       工作集界(Working-Set Bound)复杂度保证和手指搜索(Finger Search)。
 *
 * 协作: AvlTree4(AVL树) / RedBlackTree5(红黑树) / BPlusTree7(B+树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 伸展树(zig/zig-zig/zig-zag+工作集界+手指搜索)
 */
class SplayTree7 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalOperations = 0;
        int numKeys = 0;
        int splayCount = 0;
        int zigCount = 0;
        int zigZigCount = 0;
        int zigZagCount = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SplayTree7(QObject *parent = nullptr);
    ~SplayTree7() override;

    /** @brief Insert a key-value pair (splay to root) */
    void insert(double key, double value);

    /** @brief Remove a key */
    void remove(double key);

    /** @brief Find value for key (splay to root), qQNaN if not found */
    double find(double key);

    /** @brief Finger search: find closest key to target */
    QPair<double, double> fingerSearch(double target);

    /** @brief Range query [lo, hi] */
    QVector<QPair<double, double>> rangeQuery(double lo, double hi);

    /** @brief In-order traversal */
    QVector<QPair<double, double>> inOrder() const;

    /** @brief Clear the tree */
    void clear();

    int size() const { return m_size; }
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int size, double timeMs);

private:
    /** @brief Tree node */
    struct Node {
        double key = 0.0;
        double value = 0.0;
        Node* left = nullptr;
        Node* right = nullptr;
        Node* parent = nullptr;
    };

    Node* m_root = nullptr;
    int m_size = 0;

    // Finger for finger search
    Node* m_finger = nullptr;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Splay node to root */
    void splay(Node* n);

    /** @brief Zig rotation (single rotation) */
    void zig(Node* n);

    /** @brief Zig-zig rotation (same-side double) */
    void zigZig(Node* n);

    /** @brief Zig-zag rotation (opposite-side double) */
    void zigZag(Node* n);

    /** @brief Rotate left */
    void rotateLeft(Node* n);

    /** @brief Rotate right */
    void rotateRight(Node* n);

    /** @brief Find node without splaying */
    Node* findNode(double key) const;

    /** @brief Subtree minimum */
    Node* subtreeMin(Node* n) const;

    /** @brief Subtree maximum */
    Node* subtreeMax(Node* n) const;

    /** @brief Recursive in-order collection */
    void inOrderHelper(Node* n, QVector<QPair<double, double>>& result) const;

    /** @brief Recursive delete */
    void deleteTree(Node* n);

    /** @brief Update finger to most recently accessed node */
    void updateFinger(Node* n);
};
