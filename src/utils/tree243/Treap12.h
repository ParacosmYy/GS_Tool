/**
 * @file Treap12.h
 * @brief 树堆(哈希确定性优先级+分裂合并隐式键索引) — Treap with Deterministic Priority Assignment via Hashing and Split-Merge Operations for Implicit Key Indexing
 *
 * 功能: 实现树堆(Treap)数据结构，通过哈希函数确定性分配优先级(deterministic
 *       priority via hashing)避免随机数依赖，支持分裂(split)与合并(merge)操作
 *       实现隐式键索引(implicit key indexing)的有序序列维护。
 *
 * 协作: WeightBalancedTree8(权重平衡树) / AVLTree7(AVL树) / SplayTree5(伸展树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 树堆(哈希确定性优先级+分裂合并隐式键索引)
 */
class Treap12 : public QObject {
    Q_OBJECT

public:
    /** @brief Treap node */
    struct Node {
        double key = 0.0;
        int value = 0;
        quint64 priority = 0;
        int left = -1;
        int right = -1;
        int size = 1;   // subtree size for implicit indexing
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int treeSize = 0;
        int treeHeight = 0;
        int numSplits = 0;
        int numMerges = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Treap12(QObject *parent = nullptr);
    ~Treap12() override;

    /** @brief Insert key-value pair at implicit position pos */
    void insertAt(int pos, double key, int value);

    /** @brief Insert maintaining key ordering */
    void insert(double key, int value);

    /** @brief Remove node at implicit position pos */
    bool removeAt(int pos);

    /** @brief Remove by key */
    bool remove(double key);

    /** @brief Search by key, returns value or -1 */
    int search(double key) const;

    /** @brief Access element at implicit index */
    QPair<double, int> atIndex(int idx) const;

    /** @brief Get implicit index of key (rank) */
    int rankOf(double key) const;

    /** @brief In-order traversal of key-value pairs */
    QVector<QPair<double, int>> inOrder() const;

    int size() const;
    int height() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void insertCompleted(double key, int treeSize, double timeMs);
    void splitMergePerformed(int newSize, int height);

private:
    QVector<Node> m_nodes;
    int m_root = -1;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Hash-based deterministic priority */
    static quint64 hashPriority(double key, int value);

    /** @brief Allocate new node */
    int allocNode(double key, int value);

    /** @brief Update subtree size */
    void updateSize(int idx);

    /** @brief Get subtree size */
    int getSize(int idx) const;

    /** @brief Split treap by implicit position into left, right */
    void split(int idx, int pos, int& left, int& right);

    /** @brief Merge two treaps by priority */
    int merge(int left, int right);

    /** @brief Compute height recursively */
    int computeHeight(int idx) const;

    /** @brief In-order collection */
    void collectInOrder(int idx, QVector<int>& indices) const;
};
