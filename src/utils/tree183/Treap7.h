/**
 * @file Treap7.h
 * @brief Treap随机化平衡树(优先级堆不变量+split/merge操作+顺序统计) — Treap (Randomized BST) with Priority-based Heap Invariant, Split/Merge Operations and Order Statistics
 *
 * 功能: 实现Treap随机化平衡二叉搜索树，支持优先级堆不变量维护、
 *       split/merge操作、顺序统计(第k小/秩)、插入/删除/查找。
 *
 * 协作: AVLTree4(AVL树) / RedBlackTree5(红黑树) / SplayTree6(伸展树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Treap随机化平衡树(堆不变量+split/merge+顺序统计)
 */
class Treap7 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalOperations = 0;
        int numNodes = 0;
        int treeHeight = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Treap7(QObject *parent = nullptr);
    ~Treap7() override;

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

    /** @brief Split: divide into (<key) and (>=key) treaps */
    QPair<Treap7*, Treap7*> split(int key) const;

    /** @brief Merge: combine two treaps (all keys in a < all keys in b) */
    static Treap7* merge(Treap7* a, Treap7* b);

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
    /** @brief Treap node */
    struct Node {
        int key = 0;
        int priority = 0;   ///< Random heap priority
        int size = 1;       ///< Subtree size for order statistics
        Node* left = nullptr;
        Node* right = nullptr;
    };

    Node* m_root = nullptr;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Update subtree size */
    void updateSize(Node* node);

    /** @brief Right rotation */
    Node* rotateRight(Node* node);

    /** @brief Left rotation */
    Node* rotateLeft(Node* node);

    /** @brief Recursive insert */
    Node* insertNode(Node* node, int key);

    /** @brief Recursive remove */
    Node* removeNode(Node* node, int key);

    /** @brief Recursive search */
    bool searchNode(Node* node, int key) const;

    /** @brief Recursive split */
    QPair<Node*, Node*> splitNode(Node* node, int key);

    /** @brief Recursive merge */
    static Node* mergeNode(Node* a, Node* b);

    /** @brief Kth element */
    int kthNode(Node* node, int k) const;

    /** @brief Rank */
    int rankNode(Node* node, int key) const;

    /** @brief In-order traversal */
    void inorderHelper(Node* node, QVector<int>& result) const;

    /** @brief Delete subtree */
    void deleteTree(Node* node);

    /** @brief Tree height */
    int treeHeight(Node* node) const;

    /** @brief Generate random priority */
    int randomPriority() const;
};
