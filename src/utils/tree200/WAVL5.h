/**
 * @file WAVL5.h
 * @brief 弱AVL树(秩差不变量+摊还常数再平衡) — Weak AVL Tree with Rank Difference Invariants and Amortized Constant Rebalancing per Update
 *
 * 功能: 实现弱AVL树(WAVL)，支持秩差不变量维护、
 *       摊还常数级再平衡操作和完整的有序映射接口。
 *
 * 协作: AA5(AA树) / Treap8(Treap) / RedBlackTree6(红黑树) / SplayTree4(伸展树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 弱AVL树(秩差不变量+摊还常数再平衡)
 */
class WAVL5 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOperations = 0;
        int nodeCount = 0;
        int treeHeight = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit WAVL5(QObject *parent = nullptr);
    ~WAVL5() override;

    /** @brief Insert key-value pair */
    void insert(double key, double value);

    /** @brief Remove key */
    bool remove(double key);

    /** @brief Find value by key, returns defaultValue if not found */
    double find(double key, double defaultValue = 0.0) const;

    /** @brief Check if key exists */
    bool contains(double key) const;

    /** @brief Get all key-value pairs in sorted order */
    QVector<QPair<double, double>> toVector() const;

    /** @brief Range query [lo, hi] inclusive */
    QVector<QPair<double, double>> rangeQuery(double lo, double hi) const;

    /** @brief Get minimum key */
    double minimumKey() const;

    /** @brief Get maximum key */
    double maximumKey() const;

    /** @brief Number of nodes */
    int size() const;

    /** @brief Check if empty */
    bool isEmpty() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void treeUpdated(const QString& op, int size, int height, double timeMs);

private:
    struct Node {
        double key = 0.0;
        double value = 0.0;
        int rank = 0;
        Node* left = nullptr;
        Node* right = nullptr;
        Node* parent = nullptr;

        ~Node() { delete left; delete right; }
    };

    Node* m_root = nullptr;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Get rank of node (-1 for nullptr) */
    static int getRank(const Node* n);

    /** @brief Get rank difference from parent to child */
    static int rankDiff(const Node* parent, const Node* child);

    /** @brief Rotate left around node */
    Node* rotateLeft(Node* x);

    /** @brief Rotate right around node */
    Node* rotateRight(Node* y);

    /** @brief Rebalance after insert */
    void rebalanceInsert(Node* node);

    /** @brief Rebalance after remove */
    void rebalanceRemove(Node* parent, Node* node);

    /** @brief In-order traversal */
    void inOrder(Node* n, QVector<QPair<double, double>>& result) const;

    /** @brief Range traversal */
    void rangeInOrder(Node* n, double lo, double hi,
                      QVector<QPair<double, double>>& result) const;

    /** @brief Compute tree height */
    static int computeHeight(const Node* n);

    /** @brief Find node by key */
    Node* findNode(double key) const;

    /** @brief Transplant subtree */
    void transplant(Node* u, Node* v);

    /** @brief Find minimum node in subtree */
    static Node* treeMinimum(Node* n);
};
