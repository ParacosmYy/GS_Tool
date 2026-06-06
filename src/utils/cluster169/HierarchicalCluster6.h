/**
 * @file HierarchicalCluster6.h
 * @brief 层次聚类(凝聚树状图+动态树切割) — Hierarchical Agglomerative Clustering with Dendrogram Construction and Dynamic Tree Cutting
 *
 * 功能: 实现凝聚层次聚类，支持多种链接准则(单/全/平均/Ward)、
 *       树状图构建与动态树切割自动确定簇数。
 *
 * 协作: DBSCAN9(DBSCAN) / KMeans15(K-Means) / GaussianMixture11(GMM)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 层次聚类器
 */
class HierarchicalCluster6 : public QObject {
    Q_OBJECT

public:
    /** @brief 链接准则 */
    enum Linkage { Single, Complete, Average, Ward };

    /** @brief 树状图节点 */
    struct DendrogramNode {
        int left = -1;       ///< 左子节点索引(叶节点为原始数据索引)
        int right = -1;      ///< 右子节点索引
        double distance = 0.0; ///< 合并距离
        int size = 1;        ///< 子树样本数
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;           ///< 累计运行次数
        int lastClusters = 0;            ///< 最近簇数
        double avgProcessingTimeMs = 0.0;///< 平均耗时(ms)
    };

    explicit HierarchicalCluster6(QObject *parent = nullptr);
    ~HierarchicalCluster6() override;

    void setLinkage(Linkage method);
    void setCutHeight(double height);

    /**
     * @brief 执行层次聚类
     * @param data 数据集(每行一个样本)
     * @return 每个样本的簇标签
     */
    QVector<int> fit(const QVector<QVector<double>>& data);

    /** @brief 获取树状图 */
    QVector<DendrogramNode> dendrogram() const;

    /** @brief 按指定簇数切割树状图 */
    QVector<int> cutTree(int numClusters) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int clusters);

private:
    /** @brief 计算距离矩阵 */
    QVector<QVector<double>> computeDistanceMatrix(
        const QVector<QVector<double>>& data) const;

    /** @brief 欧氏距离 */
    static double distance(const QVector<double>& a, const QVector<double>& b);

    /** @brief 更新合并后的簇间距离 */
    void updateDistances(QVector<QVector<double>>& dist,
                         int mergedI, int mergedJ,
                         const QVector<int>& clusterSize) const;

    /** @brief 递归获取叶节点 */
    void collectLeaves(int node, QVector<int>& labels,
                       int label, const QVector<DendrogramNode>& dend) const;

    Linkage m_linkage = Ward;
    double m_cutHeight = 0.0;
    bool m_useCutHeight = false;

    QVector<DendrogramNode> m_dendrogram;
    int m_n = 0;

    Stats m_stats;
    double m_timeSum = 0.0;
};
