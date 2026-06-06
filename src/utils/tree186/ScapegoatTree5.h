/**
 * @file ScapegoatTree5.h
 * @brief 替罪羊树(α权重平衡+插入重建+摊还O(log n)) — Scapegoat Tree with Alpha-Weight-Balanced Invariant, Rebuild-on-Insert and Amortized O(log n)
 *
 * 功能: 实现替罪羊树，支持α权重平衡不变量、插入时检测并重建、
 *       摊还O(log n)操作和子树大小统计。
 *
 * 协作: RedBlackTree9(红黑树) / AVLTree4(AVL树) / Treap7(Treap)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 替罪羊树(α权重平衡+插入重建)
 */
class ScapegoatTree5 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalOperations = 0;
        int numNodes = 0;
        int treeHeight = 0;
        int numRebuilds = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ScapegoatTree5(QObject *parent = nullptr);
    ~ScapegoatTree5() override;

    void setAlpha(double alpha);

    /** @brief 插入键值 */
    void insert(int key);

    /** @brief 删除键值(懒删除+重建) */
    void remove(int key);

    /** @brief 查找键值 */
    bool contains(int key) const;

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
    /** @brief Tree node */
    struct Node {
        int key = 0;
        int subtreeSize = 1;
        bool deleted = false;
        Node* left = nullptr;
        Node* right = nullptr;
    };

    Node* m_root = nullptr;
    int m_maxSize = 0; // Track max size since last rebuild
    double m_alpha = 0.7;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Check alpha-weight balance */
    bool isAlphaBalanced(Node* n) const;

    /** @brief Compute tree height */
    int treeHeight(Node* n) const;

    /** @brief Flatten subtree to sorted vector */
    void flatten(Node* n, QVector<Node*>& nodes) const;

    /** @brief Rebuild balanced subtree from sorted nodes */
    Node* rebuildBalanced(QVector<Node*>& nodes, int start, int end);

    /** @brief Find and rebuild scapegoat on insert path */
    Node* findScapegoat(Node* path[], int pathLen);

    /** @brief Update subtree sizes up the tree */
    void updateSizes(Node* n);

    /** @brief In-order traversal helper */
    void inorderHelper(Node* n, QVector<int>& result) const;

    /** @brief Delete subtree */
    void deleteTree(Node* n);

    /** @brief Lazy deletion cleanup */
    void cleanupDeleted();
};
