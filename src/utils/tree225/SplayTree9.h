/**
 * @file SplayTree9.h
 * @brief 伸展树(半伸展优化+访问频率驱动偏斜负载重构) — Splay Tree with Semi-Splay Optimization and Access-Frequency-Based Restructuring for Skewed Workloads
 *
 * 功能: 实现伸展树，采用半伸展策略减少旋转次数，
 *       集成访问频率统计驱动的重构策略优化偏斜负载。
 *
 * 协作: RedBlackTree12(红黑树) / AVLTree10(AVL树) / SkipList8(跳表)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 伸展树(半伸展+访问频率重构)
 */
class SplayTree9 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numNodes = 0;
        int treeHeight = 0;
        int totalSplays = 0;
        int semiSplays = 0;
        quint64 totalAccesses = 0;
        double skewRatio = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SplayTree9(QObject *parent = nullptr);
    ~SplayTree9() override;

    /** @brief Insert key with value */
    void insert(double key, double value = 0.0);

    /** @brief Remove key */
    bool remove(double key);

    /** @brief Find key (triggers semi-splay on access) */
    bool contains(double key);

    /** @brief Get value for key */
    double value(double key, double defaultValue = 0.0) const;

    /** @brief Get all keys in sorted order */
    QVector<double> keys() const;

    /** @brief Get number of nodes */
    int size() const;

    /** @brief Force rebalance based on access frequency */
    void frequencyRestructure();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int size, double timeMs);

private:
    struct SplayNode {
        double key = 0.0;
        double value = 0.0;
        quint64 accessCount = 0;
        SplayNode* left = nullptr;
        SplayNode* right = nullptr;
        SplayNode* parent = nullptr;
    };

    SplayNode* m_root = nullptr;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Full splay: bring node to root */
    void splay(SplayNode* node);

    /** @brief Semi-splay: half-rotation for less aggressive restructuring */
    void semiSplay(SplayNode* node);

    /** @brief Right rotation */
    SplayNode* rotateRight(SplayNode* x);

    /** @brief Left rotation */
    SplayNode* rotateLeft(SplayNode* x);

    /** @brief Find node by key */
    SplayNode* findNode(double key) const;

    /** @brief Substitute subtree */
    void substitute(SplayNode* u, SplayNode* v);

    /** @brief Find maximum in subtree */
    SplayNode* subtreeMax(SplayNode* node) const;

    /** @brief Collect keys in-order */
    void collectKeys(SplayNode* node, QVector<double>& result) const;

    /** @brief Compute tree height */
    int computeHeight(SplayNode* node) const;

    /** @brief Recursive delete */
    void deleteTree(SplayNode* node);

    /** @brief Build balanced tree from sorted key-value list */
    SplayNode* buildBalanced(const QVector<QPair<double, double>>& sorted, int lo, int hi);
};
