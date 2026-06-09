/**
 * @file SubspaceCluster10.h
 * @brief 子空间聚类(CLIQUE网格密度计数+Apriori候选生成轴并行子空间) — Subspace Clustering with CLIQUE Grid-Based Density Counting and Apriori Candidate Generation for Axis-Parallel Subspace
 *
 * 功能: 实现子空间聚类算法，使用CLIQUE网格密度计数(grid-based density
 *       counting)将高维空间划分为等宽网格单元，Apriori候选生成(apriori
 *       candidate generation)逐维合并密集单元发现轴平行子空间聚类。
 *
 * 协作: HierarchicalCluster12(层次聚类) / KMedoids20(K中心点) / DBSCAN19(密度聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 子空间聚类(CLIQUE网格密度+Apriori候选生成)
 */
class SubspaceCluster10 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int numDimensions = 0;
        int numGridBins = 0;
        int numDenseUnits = 0;
        int numClusters = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief A dense unit found in some subspace */
    struct DenseUnit {
        QVector<int> dimIndices;   // Dimensions of this subspace
        QVector<int> binIndices;   // Grid bin index per dimension
        int pointCount = 0;
    };

    explicit SubspaceCluster10(QObject *parent = nullptr);
    ~SubspaceCluster10() override;

    /** @brief Set number of grid bins per dimension */
    void setGridBins(int bins);

    /** @brief Set density threshold (fraction of points in a unit) */
    void setDensityThreshold(double threshold);

    /** @brief Run subspace clustering, return cluster labels */
    QVector<int> fit(const QVector<QVector<double>>& data);

    /** @brief Get dense units found in all subspaces */
    QVector<DenseUnit> denseUnits() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int numClusters, int denseUnits, double timeMs);

private:
    int m_gridBins = 10;
    double m_densityThreshold = 0.05;
    int m_n = 0;
    int m_dims = 0;

    QVector<DenseUnit> m_denseUnits;
    QVector<QVector<double>> m_data;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Map point value to grid bin index */
    int toBin(double value, double minVal, double maxVal) const;

    /** @brief Find dense units in 1-D subspaces */
    QVector<DenseUnit> findDense1D(const QVector<double>& dimMin,
                                   const QVector<double>& dimMax) const;

    /** @brief Apriori candidate generation: merge k-D dense to (k+1)-D */
    QVector<DenseUnit> aprioriMerge(const QVector<DenseUnit>& prev,
                                    int targetDim) const;

    /** @brief Count points in a dense unit */
    int countPoints(const DenseUnit& unit) const;
};
