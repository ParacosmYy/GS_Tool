/**
 * @file Treap9.h
 * @brief 持久化Treap(路径复制持久化+区间求和+历史快照访问) — Treap with Persistent Versioning via Path Copying, Range Sum Query and Historical Snapshot Access
 *
 * 功能: 实现持久化Treap数据结构，支持路径复制版本持久化、
 *       区间求和查询和历史快照访问。
 *
 * 协作: Rope6(Rope) / AVLTree7(AVL树) / RedBlackTree8(红黑树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 持久化Treap(路径复制持久化+区间求和+历史快照访问)
 */
class Treap9 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int currentSize = 0;
        int numVersions = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Treap9(QObject *parent = nullptr);
    ~Treap9() override;

    /** @brief Insert key-value pair at current version */
    void insert(int key, double value);

    /** @brief Remove key at current version */
    void remove(int key);

    /** @brief Look up value by key at current version */
    QPair<bool, double> lookup(int key) const;

    /** @brief Range sum query [lo, hi] at current version */
    double rangeSum(int lo, int hi) const;

    /** @brief Create a snapshot of current version, return version ID */
    int createSnapshot();

    /** @brief Restore to a historical snapshot */
    void restoreSnapshot(int versionId);

    /** @brief Get value at key in a specific version */
    QPair<bool, double> lookupVersion(int key, int versionId) const;

    /** @brief Range sum in a specific version */
    double rangeSumVersion(int lo, int hi, int versionId) const;

    /** @brief Get number of versions */
    int versionCount() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int version, double timeMs);

private:
    /** @brief Treap node with subtree sum for range queries */
    struct Node {
        int key = 0;
        double value = 0.0;
        int priority = 0;
        double subtreeSum = 0.0;
        int left = -1;   // index into m_nodes
        int right = -1;
    };

    QVector<Node> m_nodes;       // persistent node pool
    QVector<int> m_roots;        // root index per version
    int m_currentVersion = -1;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Clone a node for path copying */
    int cloneNode(int nodeIdx);

    /** @brief Update subtree sum */
    void updateSum(int nodeIdx);

    /** @brief Rotate right */
    int rotateRight(int root);

    /** @brief Rotate left */
    int rotateLeft(int root);

    /** @brief Insert into treap, return new root index */
    int insertRec(int root, int key, double value);

    /** @brief Remove from treap, return new root index */
    int removeRec(int root, int key);

    /** @brief Lookup in treap rooted at given node */
    QPair<bool, double> lookupRec(int root, int key) const;

    /** @brief Range sum in treap rooted at given node */
    double rangeSumRec(int root, int lo, int hi) const;
};
