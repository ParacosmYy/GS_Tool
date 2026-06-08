/**
 * @file Rope7.h
 * @brief 绳索数据结构(手指搜索树+增量重平衡高效字符串拼接分割) — Rope Data Structure with Finger Search Tree and Incremental Rebalancing for Efficient String Concatenation and Split
 *
 * 功能: 实现绳索数据结构，支持手指搜索树节点定位、
 *       增量重平衡、高效字符串拼接与分割操作。
 *
 * 协作: Treap10(隐式键Treap) / SplayTree8(伸展树) / PersistentSegment6(持久线段树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief 绳索数据结构(手指搜索+增量重平衡)
 */
class Rope7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int ropeLength = 0;
        int numLeaves = 0;
        int treeHeight = 0;
        int rebalanceCount = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Rope7(QObject *parent = nullptr);
    ~Rope7() override;

    /** @brief Build rope from string */
    void build(const QString& text, int leafSize = 64);

    /** @brief Get character at index */
    QChar at(int index) const;

    /** @brief Get substring [start, start+length) */
    QString mid(int start, int length) const;

    /** @brief Insert string at position */
    void insert(int pos, const QString& text);

    /** @brief Remove range [start, start+length) */
    void remove(int start, int length);

    /** @brief Concatenate another rope */
    void concat(const Rope7& other);

    /** @brief Split rope at position, return right part */
    Rope7* split(int pos);

    /** @brief Get total length */
    int length() const;

    /** @brief Convert to flat string */
    QString toString() const;

    /** @brief Trigger incremental rebalancing */
    void rebalance();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int ropeLen, double timeMs);

private:
    static const int MAX_LEAF = 64;

    struct Node {
        QString leaf;        // Non-empty only for leaf nodes
        int weight = 0;      // Length of left subtree (or leaf length)
        int totalLen = 0;    // Total length of this subtree
        int left = -1;
        int right = -1;
        int height = 1;
        bool isLeaf = false;
    };

    QVector<Node> m_nodes;
    int m_root = -1;

    // Finger search cache
    int m_finger = -1;
    int m_fingerPos = -1;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Allocate node */
    int allocNode();

    /** @brief Create leaf node from string */
    int createLeaf(const QString& text);

    /** @brief Update node aggregates */
    void update(int node);

    /** @brief Get balance factor */
    int balanceFactor(int node) const;

    /** @brief Rotate right */
    int rotateRight(int y);

    /** @brief Rotate left */
    int rotateLeft(int x);

    /** @brief AVL rebalance at node */
    int rebalanceNode(int node);

    /** @brief Split node at position, returns {left, right} */
    QPair<int, int> splitNode(int node, int pos);

    /** @brief Merge two subtrees */
    int mergeNodes(int left, int right);

    /** @brief Find node containing position, return {node, offset} */
    QPair<int, int> findPosition(int pos) const;

    /** @report In-order collection of leaves */
    void collectLeaves(int node, QVector<QString>& leaves) const;
};
