/**
 * @file VanEmdeBoas4.h
 * @brief van Emde Boas树(簇层次+O(log log U)操作+前驱后继查询) — van Emde Boas Tree with Cluster Hierarchy, O(log log U) Operations and Predecessor/Successor Queries
 *
 * 功能: 实现van Emde Boas树，支持簇(cluster)层次结构、
 *       O(log log U)插入/删除/查找和前驱/后继查询。
 *
 * 协作: SplayTree7(伸展树) / AvlTree4(AVL树) / BPlusTree7(B+树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief van Emde Boas树(簇层次+O(log log U)操作)
 */
class VanEmdeBoas4 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalOperations = 0;
        int universeSize = 0;
        int numElements = 0;
        int insertCount = 0;
        int deleteCount = 0;
        int queryCount = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit VanEmdeBoas4(int universeSize = 65536,
                          QObject *parent = nullptr);
    ~VanEmdeBoas4() override;

    /** @brief Insert value into the tree */
    void insert(int x);

    /** @brief Remove value from the tree */
    void remove(int x);

    /** @brief Check if value exists */
    bool contains(int x) const;

    /** @brief Find predecessor of x (largest value < x), -1 if none */
    int predecessor(int x) const;

    /** @brief Find successor of x (smallest value > x), -1 if none */
    int successor(int x) const;

    /** @brief Get minimum value, -1 if empty */
    int minimum() const;

    /** @brief Get maximum value, -1 if empty */
    int maximum() const;

    int size() const { return m_count; }
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int value, double timeMs);

private:
    /** @brief Recursive vEB node */
    struct Node {
        int u;          // Universe size
        int min;        // Minimum element (not stored in clusters)
        int max;        // Maximum element
        Node* summary;  // Summary structure
        QVector<Node*> clusters;

        Node(int universe) : u(universe), min(-1), max(-1), summary(nullptr) {
            if (u > 2) {
                int cu = upperSqrt(u);
                clusters.resize(cu, nullptr);
            }
        }
    };

    static int upperSqrt(int u) { int s = 1; int sq = u; while (sq > 2) { sq = (sq + 1) / 2; s *= 2; } return qMax(2, s); }
    static int lowerSqrt(int u) { int s = 1; int sq = u; while (sq > 2) { sq = sq / 2; s *= 2; } return qMax(2, s); }

    int high(int x) const;
    int low(int x) const;
    int index(int high, int low) const;

    Node* m_root;
    int m_universe;
    int m_count;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Recursive insert */
    void insertRec(Node* node, int x);

    /** @brief Recursive remove */
    void removeRec(Node* node, int x);

    /** @brief Recursive contains */
    bool containsRec(Node* node, int x) const;

    /** @brief Recursive predecessor */
    int predRec(Node* node, int x) const;

    /** @brief Recursive successor */
    int succRec(Node* node, int x) const;

    /** @brief Recursive delete tree */
    void deleteNode(Node* node);
};
