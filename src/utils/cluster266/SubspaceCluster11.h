/**
 * @file SubspaceCluster11.h
 * @brief 子空间聚类(CLIQUE网格密度检测与Apriori候选生成高维子空间发现) — Subspace Clustering with CLIQUE Grid-Based Density Detection and Apriori Candidate Generation for High-Dimensional Subspace Discovery
 *
 * 功能: 实现子空间聚类(Subspace clustering)，采用CLIQUE网格密度检测(CLIQUE grid-based
 *       density detection)与Apriori候选生成(Apriori candidate generation)发现高维子空间。
 *
 * 协作: HierarchicalCluster13(层次聚类) / KMedoids21(K-中心点) / GaussianMixture29(高斯混合)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 子空间聚类(CLIQUE网格密度检测与Apriori候选生成高维子空间发现)
 */
class SubspaceCluster11 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int dimension = 0;
        int gridResolution = 0;
        int numDenseUnits = 0;
        int numSubspaces = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief A dense unit identified in subspace */
    struct DenseUnit {
        QVector<int> gridIndices;   // grid cell index per dimension
        QVector<int> dimensions;    // which dimensions this unit spans
        int pointCount = 0;
    };

    /** @brief A cluster in a subspace */
    struct SubspaceCluster {
        QVector<int> dimensions;    // subspace dimensions
        QVector<int> pointIndices;  // clustered point indices
        int numDenseUnits = 0;
    };

    explicit SubspaceCluster11(QObject *parent = nullptr);
    ~SubspaceCluster11() override;

    /** @brief Set grid resolution per dimension */
    void setGridResolution(int resolution);

    /** @brief Set density threshold (0..1) for dense unit detection */
    void setDensityThreshold(double threshold);

    /** @brief Run CLIQUE subspace clustering, returns clusters found */
    QVector<SubspaceCluster> fit(const QVector<QVector<double>>& data);

    /** @brief Get all dense units found in 1-D subspaces */
    QVector<DenseUnit> denseUnits() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringUpdated(int numSubspaces, int numDense, double timeMs);

private:
    int m_gridRes = 10;
    double m_densityThreshold = 0.15;
    int m_n = 0;
    int m_dim = 0;

    QVector<DenseUnit> m_denseUnits;
    QVector<SubspaceCluster> m_clusters;

    Stats m_stats;
    double m_timeSum = 0.0;

    // Per-dimension min/max for normalization
    QVector<double> m_dimMin;
    QVector<double> m_dimMax;

    /** @brief Map point value to grid cell index for a dimension */
    int toGridCell(double value, int dim) const;

    /** @brief Find dense 1-D units */
    QVector<DenseUnit> findDense1D(const QVector<QVector<double>>& data) const;

    /** @brief Generate k-D candidates from (k-1)-D dense units via Apriori */
    QVector<DenseUnit> aprioriJoin(const QVector<DenseUnit>& prev, int k) const;

    /** @brief Count points in a multi-dimensional grid unit */
    int countInUnit(const QVector<QVector<double>>& data,
                    const DenseUnit& unit) const;

    /** @brief Form connected clusters from dense units in a subspace */
    QVector<QVector<int>> formClusters(const QVector<DenseUnit>& units) const;
};
