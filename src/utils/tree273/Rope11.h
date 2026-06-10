/**
 * @file Rope11.h
 * @brief 绳索结构(平衡指树与惰性连接高效持久化字符串操作) — Rope with Balanced Finger Tree and Lazy Concatenation for Efficient Persistent String Operations
 *
 * 功能: 实现绳索结构(rope)，采用平衡指树(balanced finger tree)
 *       与惰性连接(lazy concatenation)实现高效持久化字符串操作(efficient persistent string operations)。
 *
 * 协作: BTree9(B树) / Treap14(树堆) / PersistentArray10(持久化数组)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief 绳索结构(平衡指树与惰性连接高效持久化字符串操作)
 */
class Rope11 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numLeaves = 0;
        int numNodes = 0;
        int totalLength = 0;
        int treeDepth = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Rope11(QObject *parent = nullptr);
    ~Rope11() override;

    /** @brief Build rope from string */
    void build(const QString& text);

    /** @brief Set leaf node max capacity */
    void setLeafSize(int maxSize);

    /** @brief Get character at index */
    QChar at(int index) const;

    /** @brief Get substring [start, start+length) */
    QString mid(int start, int length) const;

    /** @brief Insert string at position */
    void insert(int pos, const QString& text);

    /** @brief Remove characters [start, start+length) */
    void remove(int start, int length);

    /** @brief Concatenate with another rope (lazy) */
    void concat(Rope11& other);

    /** @brief Get total length */
    int size() const;

    /** @brief Convert entire rope to string */
    QString toString() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationDone(int totalLength, int numNodes, int depth, double timeMs);

private:
    int m_leafSize = 64;

    /** @brief Rope node stored in pool */
    struct RopeNode {
        int weight = 0;         // left subtree length (leaf: string length)
        int left = -1;          // left child index
        int right = -1;         // right child index
        int parent = -1;        // parent index
        bool isLeaf = true;
        QString data;           // leaf data
        int height = 1;         // for AVL balancing
        bool isLazy = false;    // lazy concatenation marker
    };

    int m_root = -1;
    QVector<RopeNode> m_nodes;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Allocate a new leaf node */
    int allocLeaf(const QString& data);

    /** @brief Allocate an internal node */
    int allocInternal(int left, int right);

    /** @brief Compute weight (left subtree total length) for a node */
    int computeWeight(int nodeIdx) const;

    /** @brief Get height of node */
    int nodeHeight(int nodeIdx) const;

    /** @brief Get balance factor */
    int balanceFactor(int nodeIdx) const;

    /** @brief Update height and weight up to root */
    void updatePath(int nodeIdx);

    /** @brief Rotate right at node */
    int rotateRight(int nodeIdx);

    /** @brief Rotate left at node */
    int rotateLeft(int nodeIdx);

    /** @brief Rebalance after modification */
    int rebalance(int nodeIdx);

    /** @brief Force-evaluate lazy concatenation node */
    void forceEval(int nodeIdx);

    /** @brief Split rope at position, returns two root indices */
    QPair<int, int> splitAt(int nodeIdx, int pos);

    /** @brief Collect leaves in-order into string */
    void collectLeaves(int nodeIdx, QString& result) const;
};
