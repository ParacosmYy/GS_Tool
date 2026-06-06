/**
 * @file AA4.h
 * @brief AA树(平衡BST+skew/split重平衡+level不变量+rank操作) — AA Tree (Balanced BST) with Skew/Split Rebalancing, Level-based Invariant and Rank Operation
 *
 * 功能: 实现AA树平衡二叉搜索树，支持skew/split重平衡、level不变量维护、
 *       rank(秩)操作、插入/删除/查找。
 *
 * 协作: AVLTree4(AVL树) / RedBlackTree5(红黑树) / SplayTree6(伸展树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief AA树(平衡BST+skew/split+rank操作)
 */
class AA4 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalOperations = 0;
        int numNodes = 0;
        int treeHeight = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit AA4(QObject *parent = nullptr);
    ~AA4() override;

    /** @brief 插入键值 */
    void insert(int key);

    /** @brief 删除键值 */
    void remove(int key);

    /** @brief 查找键值是否存在 */
    bool contains(int key) const;

    /** @brief 查找第k小元素(1-indexed) */
    int kth(int k) const;

    /** @brief 求key的秩(小于key的元素个数+1) */
    int rank(int key) const;

    /** @brief 前驱(小于key的最大值) */
    int predecessor(int key) const;

    /** @brief 后继(大于key的最小值) */
    int successor(int key) const;

    /** @brief 中序遍历 */
    QVector<int> inorder() const;

    /** @brief 树中的节点数 */
    int size() const;

    /** @brief 清空树 */
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int key);

private:
    /** @brief AA树节点 */
    struct Node {
        int key = 0;
        int level = 1;    ///< AA tree level (replaces color)
        int size = 1;     ///< Subtree size for rank operations
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

    /** @brief Update subtree size */
    void updateSize(Node* node);

    /** @brief Recursive insert */
    Node* insertNode(Node* node, int key);

    /** @brief Recursive remove */
    Node* removeNode(Node* node, int key);

    /** @brief Find minimum node in subtree */
    Node* findMin(Node* node) const;

    /** @brief Recursive search */
    bool searchNode(Node* node, int key) const;

    /** @brief Recursive rank */
    int rankNode(Node* node, int key) const;

    /** @brief Recursive kth */
    int kthNode(Node* node, int k) const;

    /** @brief Decrease level if needed after deletion */
    Node* decreaseLevel(Node* node);

    /** @brief In-order traversal */
    void inorderHelper(Node* node, QVector<int>& result) const;

    /** @brief Delete subtree */
    void deleteTree(Node* node);

    /** @brief Compute tree height */
    int treeHeight(Node* node) const;
};
