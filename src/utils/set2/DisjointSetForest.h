/**
 * @file DisjointSetForest.h
 * @brief 并查集森林(Union-Find with Path Compression + Union by Rank)
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QMap>
#include <QString>

/**
 * @class DisjointSetForest
 * @brief 并查集森林 — 支持路径压缩和按秩合并的高效并查集
 *
 * 用于动态连通性查询，支持O(α(n)) amortized的合并和查找操作。
 * 适用于Kruskal最小生成树、等价类划分、图像连通域等场景。
 */
class DisjointSetForest : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalUnions = 0;        /**< 总合并次数 */
        int totalFinds = 0;         /**< 总查找次数 */
        int totalSetCount = 0;      /**< 初始集合数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /**
     * @brief 构造函数
     * @param n 初始元素数量(0~n-1)
     * @param parent 父对象
     */
    explicit DisjointSetForest(int n = 0, QObject* parent = nullptr);

    /** @brief 增加新元素 */
    int addElement();

    /** @brief 合并两个元素所在集合 */
    bool unionSets(int x, int y);

    /** @brief 查找元素所在集合的代表元 */
    int find(int x) const;

    /** @brief 两元素是否在同一集合 */
    bool isConnected(int x, int y) const;

    /** @brief 获取集合数量 */
    int setCount() const;

    /** @brief 获取各集合的元素分组 */
    QVector<QVector<int>> groups() const;

    /** @brief 获取元素所在集合的大小 */
    int setSize(int x) const;

    /** @brief 元素总数 */
    int size() const;

    /** @brief 重置 */
    void reset(int n);

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 集合合并信号 */
    void setsUnited(int root1, int root2, int newSetCount);

private:
    mutable QVector<int> m_parent;      /**< 父节点数组 */
    mutable QVector<int> m_rank;        /**< 秩数组 */
    int m_setCount;             /**< 当前集合数 */
    mutable Stats m_stats;              /**< 统计 */
    mutable double m_timeSum;   /**< 累计时间 */
};
