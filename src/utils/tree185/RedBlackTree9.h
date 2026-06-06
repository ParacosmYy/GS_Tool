/**
 * @file RedBlackTree9.h
 * @brief 红黑树(迭代插入/删除修复+范围查询+增强子树大小) — Red-Black Tree with Iterative Insert/Delete Fixup, Range Query and Augmented Subtree Size
 *
 * 功能: 实现红黑树，支持迭代插入/删除修复、范围查询、
 *       增强子树大小统计和顺序统计选择。
 *
 * 协作: AVLTree4(AVL树) / WAVL4(弱AVL) / Treap7(Treap)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 红黑树(迭代修复+范围查询+增强子树大小)
 */
class RedBlackTree9 : public QObject {
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

    explicit RedBlackTree9(QObject *parent = nullptr);
    ~RedBlackTree9() override;

    /** @brief 插入键值 */
    void insert(int key);

    /** @brief 删除键值 */
    void remove(int key);

    /** @brief 查找键值 */
    bool contains(int key) const;

    /** @brief 范围查询：返回[lo, hi]之间的所有键 */
    QVector<int> rangeQuery(int lo, int hi) const;

    /** @brief 顺序统计：选择第k小的元素 */
    int selectKth(int k) const;

    /** @brief 查找键的秩(小于key的元素个数) */
    int rank(int key) const;

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
    /** @brief Node color */
    enum class Color { Red, Black };

    /** @brief RB tree node with augmented subtree size */
    struct Node {
        int key = 0;
        int subtreeSize = 1;
        Color color = Color::Red;
        Node* left = nullptr;
        Node* right = nullptr;
        Node* parent = nullptr;
    };

    Node* m_root = nullptr;
    Node* m_nil = nullptr; // Sentinel node

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Left rotate */
    void rotateLeft(Node* x);

    /** @brief Right rotate */
    void rotateRight(Node* x);

    /** @brief Update subtree size */
    void updateSize(Node* n);

    /** @brief Iterative insert fixup */
    void insertFixup(Node* z);

    /** @brief Transplant node */
    void transplant(Node* u, Node* v);

    /** @brief Iterative delete fixup */
    void deleteFixup(Node* x);

    /** @brief Find minimum node */
    Node* findMin(Node* n) const;

    /** @brief Tree height */
    int treeHeight(Node* n) const;

    /** @brief In-order traversal helper */
    void inorderHelper(Node* n, QVector<int>& result) const;

    /** @brief Range query helper */
    void rangeHelper(Node* n, int lo, int hi, QVector<int>& result) const;

    /** @brief Delete subtree */
    void deleteTree(Node* n);
};
