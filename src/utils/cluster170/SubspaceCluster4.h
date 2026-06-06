/**
 * @file SubspaceCluster4.h
 * @brief 子空间聚类(CLIQUE网格密度+Apriori候选生成) — Subspace Clustering via CLIQUE with Grid-Based Density and Apriori Candidate Generation
 *
 * 功能: 实现CLIQUE子空间聚类算法，支持网格密度估计、Apriori式候选子空间
 *       生成、自动确定密集单元和聚类提取。
 *
 * 协作: HierarchicalCluster6(层次聚类) / KMeans15(K-Means) / DBSCAN9(DBSCAN)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 子空间聚类器(CLIQUE)
 */
class SubspaceCluster4 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;            ///< 累计运行次数
        int lastClusters = 0;             ///< 最近簇数
        int lastSubspaces = 0;            ///< 最近发现的子空间数
        double avgProcessingTimeMs = 0.0; ///< 平均耗时(ms)
    };

    explicit SubspaceCluster4(QObject *parent = nullptr);
    ~SubspaceCluster4() override;

    void setGridBins(int bins);
    void setDensityThreshold(double tau);
    void setMaxSubspaceDim(int maxDim);

    /**
     * @brief 执行子空间聚类
     * @param data 数据集(每行一个样本，每列一个维度)
     * @return 每个样本的簇标签(-1为噪声)
     */
    QVector<int> fit(const QVector<QVector<double>>& data);

    /** @brief 获取发现的密集子空间(维度组合) */
    QVector<QVector<int>> denseSubspaces() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int clusters, int subspaces);

private:
    /** @brief 网格单元键 */
    struct GridCell {
        QVector<int> coords;  ///< 网格坐标
        int count = 0;         ///< 样本计数
        QVector<int> points;   ///< 样本索引
    };

    /** @brief 1维密集单元检测 */
    QVector<QVector<int>> find1DDenseUnits(
        const QVector<QVector<double>>& data, int dim) const;

    /** @brief Apriori候选生成：从k维密集单元生成(k+1)维候选 */
    QVector<QVector<int>> aprioriJoin(
        const QVector<QVector<int>>& kDense,
        int k) const;

    /** @brief 检查候选是否为密集单元 */
    bool isDense(const QVector<QVector<double>>& data,
                 const QVector<int>& dims, const QVector<int>& ranges) const;

    /** @brief 从密集单元提取聚类标签 */
    QVector<int> extractLabels(int n) const;

    /** @brief 将值映射到网格bin */
    int toBin(double value, double minVal, double maxVal) const;

    int m_bins = 10;
    double m_tau = 0.15;
    int m_maxDim = 3;

    QVector<QVector<int>> m_denseSubspaces;
    QVector<GridCell> m_denseCells;

    Stats m_stats;
    double m_timeSum = 0.0;
};
