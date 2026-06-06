/**
 * @file CartesianTree5.h
 * @brief 笛卡尔树(堆序优先级+中序键序列+范围最小查询) — Cartesian Tree with Heap-Ordered Priority and In-Order Key Sequence, Range MIN Queries
 *
 * 功能: 实现笛卡尔树，支持堆序优先级+中序键排列、Treap式插入/删除、
 *       RMQ范围最小查询和快速建树(O(n)单调栈)。
 *
 * 协作: SegmentTree4(线段树) / SparseTable3(稀疏表) / Treap4(Treap)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 笛卡尔树
 */
class CartesianTree5 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalInserts = 0;          ///< 累计插入次数
        quint64 totalDeletes = 0;          ///< 累计删除次数
        quint64 totalQueries = 0;          ///< 累计查询次数
        double avgProcessingTimeMs = 0.0;  ///< 平均耗时(ms)
        int currentSize = 0;               ///< 当前元素数
        int treeHeight = 0;                ///< 当前树高
    };

    explicit CartesianTree5(QObject *parent = nullptr);
    ~CartesianTree5() override;

    /** @brief 从键值对数组O(n)建树 */
    void build(const QVector<QPair<double, double>>& items);

    /** @brief 插入键值对 */
    bool insert(double key, double priority);

    /** @brief 删除键 */
    bool remove(double key);

    /** @brief 查找键对应的优先级 */
    bool find(double key, double& priority) const;

    /** @brief 范围最小优先级查询[keyLo, keyHi] */
    bool rangeMinQuery(double keyLo, double keyHi, double& minPriority) const;

    /** @brief 中序遍历(按键序) */
    QVector<QPair<double, double>> inOrder() const;

    bool isEmpty() const;
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void insertCompleted(double key, int size);
    void deleteCompleted(double key);
    void queryCompleted(double keyLo, double keyHi, double minPriority);

private:
    /** @brief 笛卡尔树节点 */
    struct Node {
        double key = 0.0;
        double priority = 0.0;
        int subtreeMin = 0;        ///< 子树最小优先级索引(用于RMQ)
        Node* left = nullptr;
        Node* right = nullptr;
        Node* parent = nullptr;

        Node() = default;
        Node(double k, double p) : key(k), priority(p) {}
    };

    /** @brief 单调栈O(n)建树 */
    Node* buildLinear(const QVector<QPair<double, double>>& items);

    /** @brief 右旋 */
    Node* rotateRight(Node* y);

    /** @brief 左旋 */
    Node* rotateLeft(Node* x);

    /** @brief 自底向上维护子树最小 */
    void updateSubtreeMin(Node* t);

    /** @brief 递归中序遍历 */
    void inOrderRec(Node* t, QVector<QPair<double, double>>& result) const;

    /** @brief 递归范围查询 */
    void rangeMinRec(Node* t, double lo, double hi, double& minVal) const;

    /** @brief 递归清除 */
    void clearRec(Node* t);

    /** @brief 计算树高 */
    static int computeHeight(Node* t);

    Node* m_root = nullptr;

    Stats m_stats;
    double m_timeSum = 0.0;
};
