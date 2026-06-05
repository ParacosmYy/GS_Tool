/**
 * @file WeightedDisjointSet.h
 * @brief 加权并查集实现
 */

#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 加权并查集(Union-Find)
 *
 * 支持按秩合并和路径压缩,同时维护
 * 节点权重、集合大小和集合最值。
 */
class WeightedDisjointSet : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalUnions = 0;            ///< 总合并次数
        int totalFinds = 0;             ///< 总查找次数
        int totalComponents = 0;        ///< 当前连通分量数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit WeightedDisjointSet(int n, QObject* parent = nullptr);

    /**
     * @brief 查找根节点(带路径压缩)
     * @param x 节点索引
     * @return 根节点索引
     */
    int find(int x);

    /**
     * @brief 合并两个集合(按秩合并)
     * @param x 节点x
     * @param y 节点y
     * @return 合并后的根节点
     */
    int unite(int x, int y);

    /**
     * @brief 检查两个节点是否连通
     */
    bool connected(int x, int y);

    /**
     * @brief 获取集合大小
     */
    int componentSize(int x);

    /**
     * @brief 获取连通分量数
     */
    int componentCount() const;

    /**
     * @brief 设置节点权重
     */
    void setWeight(int x, double w);

    /**
     * @brief 获取集合权重之和
     */
    double componentWeight(int x);

    /**
     * @brief 获取集合内最大权重
     */
    double componentMaxWeight(int x);

    /**
     * @brief 重置所有数据
     */
    void reset(int n);

    Stats stats() const;
    void resetStatistics();

signals:
    /** @brief 合并完成信号 */
    void unionCompleted(int rootX, int rootY, int newRoot);

private:
    QVector<int> m_parent;       ///< 父节点
    QVector<int> m_rank;         ///< 秩
    QVector<int> m_size;         ///< 集合大小
    QVector<double> m_weight;    ///< 节点权重
    QVector<double> m_sumWeight; ///< 集合权重和
    QVector<double> m_maxWeight; ///< 集合最大权重
    int m_n;
    int m_components;
    Stats m_stats;
    double m_timeSum = 0.0;
};
