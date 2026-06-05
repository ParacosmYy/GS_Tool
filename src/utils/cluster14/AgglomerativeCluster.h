/**
 * @file AgglomerativeCluster.h
 * @brief 层次聚合聚类 — 自底向上构建聚类树
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

/**
 * @brief 层次聚合聚类引擎
 * 支持Single/Complete/Average/Ward链接策略，输出树状图
 */
class AgglomerativeCluster : public QObject
{
    Q_OBJECT

public:
    /** @brief 链接策略 */
    enum Linkage {
        Single,      ///< 单链接(最近邻)
        Complete,    ///< 全链接(最远邻)
        Average,     ///< 平均链接
        Ward         ///< Ward方差最小化
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalFits = 0;                ///< 累计拟合次数
        int totalMerges = 0;              ///< 累计合并次数
        int totalPointsProcessed = 0;     ///< 累计处理点数
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief 合并记录 */
    struct MergeRecord {
        int clusterA = 0;      ///< 合并的第一个簇ID
        int clusterB = 0;      ///< 合并的第二个簇ID
        double distance = 0.0; ///< 合并时的距离
        int newSize = 0;       ///< 合并后簇大小
    };

    explicit AgglomerativeCluster(Linkage linkage = Ward, QObject* parent = nullptr);

    /** @brief 拟合数据 @param data N×D矩阵(行优先) @param dims 特征维度 */
    void fit(const QVector<double>& data, int dims);

    /** @brief 获取聚类标签 @param k 目标簇数 @return 每个点的簇标签 */
    QVector<int> labels(int k) const;

    /** @brief 获取合并历史 */
    const QVector<MergeRecord>& mergeHistory() const { return m_merges; }

    /** @brief 获取指定层次的簇数 */
    int clusterCountAtDistance(double distance) const;

    /** @brief 获取惯性值(簇内平方和) */
    double inertia() const { return m_inertia; }

    /** @brief 获取数据点数 */
    int pointCount() const { return m_n; }

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 合并完成 @param mergedA 簇A @param mergedB 簇B @param dist 距离 */
    void mergeCompleted(int mergedA, int mergedB, double dist);

private:
    /** @brief 簇节点 */
    struct ClusterNode {
        int id = 0;                       ///< 簇ID
        int size = 1;                     ///< 簇大小
        int left = -1, right = -1;        ///< 子簇ID(-1为叶)
        double height = 0.0;              ///< 合并高度
        QVector<double> centroid;         ///< 质心
        QList<int> members;               ///< 成员索引
    };

    /** @brief 计算两个簇之间的距离 */
    double clusterDistance(const ClusterNode& a, const ClusterNode& b) const;

    /** @brief 计算两点欧氏距离 */
    double euclidean(const QVector<double>& p1, const QVector<double>& p2) const;

    /** @brief 从合并树提取标签 */
    void extractLabels(int nodeId, int label, QVector<int>& out) const;

    Linkage m_linkage;                    ///< 链接策略
    int m_n = 0;                          ///< 数据点数
    int m_dims = 0;                       ///< 特征维度
    QVector<ClusterNode> m_nodes;         ///< 所有簇节点
    QVector<MergeRecord> m_merges;        ///< 合并历史
    double m_inertia = 0.0;              ///< 簇内平方和
    Stats m_stats;
    double m_timeSum = 0.0;
};
