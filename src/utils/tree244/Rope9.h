/**
 * @file Rope9.h
 * @brief 绳索数据结构(叶节点字符串拼接+Fibonacci权重分裂准则再平衡) — Rope Data Structure with Leaf-Node String Concatenation and Rebalancing via Fibonacci-Weighted Split Criterion
 *
 * 功能: 实现绳索(Rope)数据结构，使用叶节点存储短字符串片段(leaf-node string)，
 *       通过拼接(concat)和分裂(split)操作维护有序序列，采用Fibonacci权重
 *       (Fibonacci-weighted)分裂准则检测不平衡并触发再平衡(rebalancing)。
 *
 * 协作: Treap12(树堆) / SplayTree5(伸展树) / BPlusTree7(B+树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief 绳索数据结构(叶节点字符串拼接+Fibonacci权重分裂准则再平衡)
 */
class Rope9 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numNodes = 0;
        int treeHeight = 0;
        int totalLength = 0;
        int numRebalances = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Rope9(QObject *parent = nullptr);
    ~Rope9() override;

    /** @brief Build rope from string */
    void build(const QString& text, int leafSize = 64);

    /** @brief Concatenate another rope into this one */
    void concat(const QString& text);

    /** @brief Split rope at position, returns right part */
    QString split(int pos);

    /** @brief Insert string at position */
    void insert(int pos, const QString& text);

    /** @brief Delete range [from, to) */
    void remove(int from, int to);

    /** @brief Get character at position */
    QChar at(int pos) const;

    /** @brief Extract substring [from, to) */
    QString substring(int from, int to) const;

    /** @brief Get full string */
    QString toString() const;

    /** @brief Total length of stored string */
    int length() const;

    /** @brief Check and rebalance if needed */
    void rebalance();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int length, int height, double timeMs);

private:
    int m_leafSize = 64;

    /** @brief Rope node stored in arena */
    struct Node {
        int weight = 0;      // length of left subtree (or leaf string length)
        int left = -1;       // left child index
        int right = -1;      // right child index
        int parent = -1;     // parent index
        bool isLeaf = false;
        QString leafData;    // only for leaf nodes
    };

    QVector<Node> m_nodes;
    int m_root = -1;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Allocate new node */
    int allocNode();

    /** @brief Recompute weight from children */
    void updateWeight(int idx);

    /** @brief Get total length of subtree */
    int subtreeLen(int idx) const;

    /** @brief Build balanced rope from string recursively */
    int buildRecursive(const QString& text, int start, int end);

    /** @brief Collect all leaf strings in order */
    void collectLeaves(int idx, QVector<QString>& leaves) const;

    /** @brief Check Fibonacci balance criterion */
    bool isBalanced(int idx) const;

    /** @brief Fibonacci number lookup */
    static int fibonacci(int n);

    /** @brief Build balanced rope from sorted leaf array */
    int buildFromLeaves(const QVector<QString>& leaves, int start, int end);
};
