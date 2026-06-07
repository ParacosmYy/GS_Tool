/**
 * @file AA5.h
 * @brief AA树(自底向上删除修复+父指针增强+范围删除) — AA Tree with Bottom-Up Deletion Fixup, Parent-Pointer Augmentation and Range Deletion
 *
 * 功能: 实现AA平衡树，支持自底向上删除修复、
 *       父指针增强和范围删除。
 *
 * 协作: AVLTree7(AVL树) / RedBlackTree6(红黑树) / SplayTree4(伸展树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief AA树(自底向上删除修复+父指针)
 */
class AA5 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalOperations = 0;
        int nodeCount = 0;
        int treeHeight = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit AA5(QObject *parent = nullptr);
    ~AA5() override;

    /** @brief Insert key-value pair */
    void insert(int key, double value = 0.0);

    /** @brief Remove a single key */
    bool remove(int key);

    /** @brief Delete all keys in range [lo, hi] */
    int removeRange(int lo, int hi);

    /** @brief Find value by key, returns default if not found */
    double find(int key, double defaultVal = 0.0) const;

    /** @brief Check if key exists */
    bool contains(int key) const;

    /** @brief In-order traversal keys */
    QVector<int> keys() const;

    /** @brief In-order traversal key-value pairs */
    QVector<QPair<int, double>> entries() const;

    /** @brief Range query: all entries in [lo, hi] */
    QVector<QPair<int, double>> rangeQuery(int lo, int hi) const;

    /** @brief Get tree height */
    int height() const;

    int size() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void treeUpdated(const QString& op, int nodeCount, int height, double timeMs);

private:
    struct Node {
        int key = 0;
        double value = 0.0;
        int level = 1;
        Node* left = nullptr;
        Node* right = nullptr;
        Node* parent = nullptr;

        ~Node() { delete left; delete right; }
    };

    Node* m_root = nullptr;
    int m_count = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Skew rotation (right rotation to fix left horizontal link) */
    Node* skew(Node* n);

    /** @brief Split rotation (left rotation to fix double right horizontal link) */
    Node* split(Node* n);

    /** @brief Bottom-up rebalance after deletion */
    void rebalanceUp(Node* n);

    /** @brief Recursive insert */
    Node* insertNode(Node* n, int key, double value);

    /** @brief Recursive remove */
    Node* removeNode(Node* n, int key);

    /** @brief Find successor (leftmost in right subtree) */
    static Node* successor(Node* n);

    /** @brief Find predecessor (rightmost in left subtree) */
    static Node* predecessor(Node* n);

    /** @brief Recursive range delete */
    Node* removeRangeHelper(Node* n, int lo, int hi, int& removed);

    /** @brief In-order traversal helper */
    void inOrder(Node* n, QVector<QPair<int, double>>& result) const;

    /** @brief Range query helper */
    void rangeHelper(Node* n, int lo, int hi, QVector<QPair<int, double>>& result) const;

    /** @brief Compute height recursively */
    static int computeHeight(const Node* n);

    /** @brief Update parent pointers after rotation */
    static void updateParent(Node* child, Node* parent);
};
