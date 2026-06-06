/**
 * @file AA_Tree3.h
 * @brief AA树(层级skew/split重平衡+排名操作) — AA Tree with Level-based Skew/Split Rebalancing and Rank Operations
 *
 * 功能: 实现AA树(简化红黑树)，支持level-based skew/split自动重平衡、
 *       排名查询、第k小选择和范围查询，保证O(log n)最坏时间复杂度。
 *
 * 协作: RedBlackTree8(红黑树) / AVLTree5(AVL树) / SplayTree3(伸展树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief AA树
 */
class AA_Tree3 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalInserts = 0;          ///< 累计插入次数
        quint64 totalDeletes = 0;          ///< 累计删除次数
        quint64 totalRebalances = 0;       ///< 累计重平衡次数(skew+split)
        double avgProcessingTimeMs = 0.0;  ///< 平均耗时(ms)
        int currentSize = 0;               ///< 当前元素数
        int treeHeight = 0;                ///< 当前树高
    };

    explicit AA_Tree3(QObject *parent = nullptr);
    ~AA_Tree3() override;

    /** @brief 插入键值对 */
    bool insert(double key, double value);

    /** @brief 删除键 */
    bool remove(double key);

    /** @brief 精确查找 */
    bool find(double key, double& value) const;

    /** @brief 查找key的排名(1-based) */
    int rank(double key) const;

    /** @brief 查找第k小的键(1-based) */
    bool select(int k, double& key, double& value) const;

    /** @brief 范围查询[lo, hi] */
    QVector<QPair<double, double>> rangeQuery(double lo, double hi) const;

    /** @brief 中序遍历 */
    QVector<QPair<double, double>> inOrder() const;

    bool isEmpty() const;
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void insertCompleted(double key, int size);
    void deleteCompleted(double key);
    void rebalancePerformed(int skewCount, int splitCount);

private:
    /** @brief AA树节点 */
    struct Node {
        double key = 0.0;
        double value = 0.0;
        int level = 1;           ///< AA树层级
        int subtreeSize = 1;     ///< 子树大小
        Node* left = nullptr;
        Node* right = nullptr;

        Node() = default;
        Node(double k, double v) : key(k), value(v) {}
    };

    /** @brief Skew: 右旋消除左水平链接 */
    Node* skew(Node* t);

    /** @brief Split: 左旋消除连续右水平链接 */
    Node* split(Node* t);

    /** @brief 递归插入 */
    Node* insertRec(Node* t, double key, double value, bool& inserted);

    /** @brief 递归删除 */
    Node* removeRec(Node* t, double key, bool& removed);

    /** @brief 查找后继 */
    Node* successor(Node* t) const;

    /** @brief 查找前驱 */
    Node* predecessor(Node* t) const;

    /** @brief 更新子树大小 */
    static void updateSize(Node* t);
    static int getSize(Node* t);

    /** @brief 递归中序遍历 */
    void inOrderRec(Node* t, QVector<QPair<double, double>>& result) const;

    /** @brief 递归范围查询 */
    void rangeQueryRec(Node* t, double lo, double hi,
                        QVector<QPair<double, double>>& result) const;

    /** @brief 递归清除 */
    void clearRec(Node* t);

    /** @brief 计算树高 */
    static int computeHeight(Node* t);

    Node* m_root = nullptr;
    int m_skewCount = 0;
    int m_splitCount = 0;

    Stats m_stats;
    double m_timeSum = 0.0;
};
