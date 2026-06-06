/**
 * @file WAVL4.h
 * @brief 弱AVL树(秩平衡不变量+自底向上再平衡+join/split操作) — Weak AVL Tree with Rank-balanced Invariants, Bottom-up Rebalancing and Join/Split Operations
 *
 * 功能: 实现弱AVL(WAVL)树，支持秩平衡不变量维护、自底向上再平衡、
 *       join/split操作、插入/删除/查找和顺序统计。
 *
 * 协作: AVLTree4(AVL树) / RedBlackTree5(红黑树) / Treap7(Treap)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 弱AVL树(秩平衡+join/split)
 */
class WAVL4 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalOperations = 0;
        int numNodes = 0;
        int treeHeight = 0;
        int numRotations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit WAVL4(QObject *parent = nullptr);
    ~WAVL4() override;

    /** @brief 插入键值 */
    void insert(int key);

    /** @brief 删除键值 */
    void remove(int key);

    /** @brief 查找键值 */
    bool contains(int key) const;

    /** @brief 前驱 */
    int predecessor(int key) const;

    /** @brief 后继 */
    int successor(int key) const;

    /** @brief Join: combine two WAVL trees (all keys in a < all keys in b) */
    static WAVL4* join(WAVL4* a, int key, WAVL4* b);

    /** @brief Split: divide into (<key) and (>=key) trees */
    QPair<WAVL4*, WAVL4*> split(int key) const;

    /** @brief 中序遍历 */
    QVector<int> inorder() const;

    /** @brief 树中节点数 */
    int size() const;

    /** @brief 清空 */
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int key);

private:
    /** @brief WAVL node */
    struct Node {
        int key = 0;
        int rank = 0;       ///< WAVL rank
        Node* left = nullptr;
        Node* right = nullptr;
        Node* parent = nullptr;
    };

    Node* m_root = nullptr;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Get node rank (leaf sentinel = -1) */
    int nodeRank(Node* n) const;

    /** @brief Rank difference between parent and child */
    int rankDiff(Node* parent, Node* child) const;

    /** @brief Rotate right */
    Node* rotateRight(Node* x);

    /** @brief Rotate left */
    Node* rotateLeft(Node* x);

    /** @brief Promote rank */
    void promote(Node* n);

    /** @brief Demote rank */
    void demote(Node* n);

    /** @brief Bottom-up rebalance after insert */
    void rebalanceInsert(Node* n);

    /** @brief Bottom-up rebalance after remove */
    void rebalanceRemove(Node* parent);

    /** @brief Recursive insert */
    Node* insertNode(Node* n, int key, Node* parent);

    /** @brief Recursive remove */
    Node* removeNode(Node* n, int key);

    /** @brief Find minimum node */
    Node* findMin(Node* n) const;

    /** @brief Replace node in parent link */
    void transplant(Node* u, Node* v);

    /** @brief Tree height */
    int treeHeight(Node* n) const;

    /** @brief Subtree size */
    int subtreeSize(Node* n) const;

    /** @brief In-order traversal */
    void inorderHelper(Node* n, QVector<int>& result) const;

    /** @brief Delete subtree */
    void deleteTree(Node* n);
};
