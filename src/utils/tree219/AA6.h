/**
 * @file AA6.h
 * @brief AA树(层级平衡+底向上skew-split修复) — AA Tree with Level-Based Rebalancing and Bottom-Up Skew-Split Fixup for Balanced Deletion
 *
 * 功能: 实现AA树平衡二叉搜索树，通过层级约束维护平衡，
 *       底向上skew和split操作保证插入删除后树结构正确。
 *
 * 协作: AVL5(AVL树) / RedBlackTree5(红黑树) / SplayTree4(伸展树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief AA树(层级平衡+底向上skew-split)
 */
class AA6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int nodeCount = 0;
        int treeHeight = 0;
        int numInserts = 0;
        int numDeletes = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit AA6(QObject *parent = nullptr);
    ~AA6() override;

    /** @brief Insert key-value pair */
    void insert(int key, double value);

    /** @brief Remove key */
    void remove(int key);

    /** @brief Search for key, returns value or NaN */
    double search(int key) const;

    /** @brief Check if key exists */
    bool contains(int key) const;

    /** @brief In-order traversal keys */
    QVector<int> inOrderKeys() const;

    /** @brief Clear all nodes */
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int key, double timeMs);

private:
    struct Node {
        int key = 0;
        double value = 0.0;
        int level = 1;
        Node* left = nullptr;
        Node* right = nullptr;
    };

    Node* m_root = nullptr;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Skew: right rotation to fix left-horizontal link */
    Node* skew(Node* node);

    /** @brief Split: left rotation to fix consecutive right-horizontal links */
    Node* split(Node* node);

    /** @brief Recursive insert */
    Node* insertImpl(Node* node, int key, double value);

    /** @brief Recursive remove */
    Node* removeImpl(Node* node, int key);

    /** @brief Find minimum node in subtree */
    Node* findMin(Node* node) const;

    /** @brief Recursive search */
    double searchImpl(Node* node, int key) const;

    /** @brief In-order traversal */
    void inOrderImpl(Node* node, QVector<int>& result) const;

    /** @brief Recursive delete all */
    void clearImpl(Node* node);

    /** @brief Compute tree height */
    int height(Node* node) const;
};
