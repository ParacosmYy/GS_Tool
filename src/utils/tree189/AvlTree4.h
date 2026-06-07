/**
 * @file AvlTree4.h
 * @brief AVL平衡二叉树(高度平衡旋转+穿线中序遍历+第k小查询) — AVL Tree with Height-Balanced Rotations, Threaded Inorder Traversal and k-th Smallest Query
 *
 * 功能: 实现AVL平衡二叉搜索树，支持插入删除的高度平衡旋转、
 *       穿线(Threaded)中序遍历、第k小元素查询和范围查询。
 *
 * 协作: RedBlackTree5(红黑树) / CartesianTree6(笛卡尔树) / SegmentTree8(线段树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief AVL平衡二叉搜索树(穿线遍历+第k小查询)
 */
class AvlTree4 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalOperations = 0;
        int numNodes = 0;
        int treeHeight = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit AvlTree4(QObject *parent = nullptr);
    ~AvlTree4() override;

    /** @brief 插入键值 */
    void insert(double key);

    /** @brief 删除键值 */
    void remove(double key);

    /** @brief 查找键值是否存在 */
    bool contains(double key) const;

    /** @brief 穿线中序遍历(非递归, O(n)) */
    QVector<double> threadedInorder() const;

    /** @brief 查找第k小元素(1-indexed) */
    double kthSmallest(int k) const;

    /** @brief 范围查询[l, r]内的所有键 */
    QVector<double> rangeQuery(double lo, double hi) const;

    /** @brief 清空树 */
    void clear();

    int size() const { return m_size; }
    int height() const;
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int numNodes, double timeMs);

private:
    /** @brief Tree node with threading and subtree size */
    struct Node {
        double key = 0.0;
        int height = 1;
        int subtreeSize = 1;
        Node* left = nullptr;
        Node* right = nullptr;
        Node* parent = nullptr;
        bool leftThread = false;  // left points to inorder predecessor
        bool rightThread = false; // right points to inorder successor
    };

    Node* m_root = nullptr;
    int m_size = 0;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Get height of node */
    int nodeHeight(Node* n) const { return n ? n->height : 0; }

    /** @brief Get balance factor */
    int balanceFactor(Node* n) const;

    /** @brief Update height and subtree size */
    void updateNode(Node* n);

    /** @brief Right rotation */
    Node* rotateRight(Node* y);

    /** @brief Left rotation */
    Node* rotateLeft(Node* x);

    /** @brief Rebalance after insertion/deletion */
    Node* rebalance(Node* n);

    /** @brief Recursive insert helper */
    Node* insertNode(Node* n, double key, Node* parent);

    /** @brief Recursive remove helper */
    Node* removeNode(Node* n, double key);

    /** @brief Find minimum node in subtree */
    Node* findMin(Node* n) const;

    /** @brief Recursive find */
    Node* findNode(Node* n, double key) const;

    /** @brief Recursive delete all */
    void deleteTree(Node* n);

    /** @brief Update threading for all nodes */
    void updateThreading();

    /** @brief Recursive helper for kth smallest */
    Node* kthNode(Node* n, int k) const;
};
