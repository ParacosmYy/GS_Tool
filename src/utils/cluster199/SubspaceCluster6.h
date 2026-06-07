/**
 * @file SubspaceCluster6.h
 * @brief 子空间聚类(CLIQUE自适应网格+显著密集单元挖掘) — Subspace Clustering via CLIQUE with Adaptive Grid Resolution and Significant Dense Unit Mining
 *
 * 功能: 实现CLIQUE子空间聚类算法，支持自适应网格分辨率、
 *       显著密集单元挖掘和子空间簇合并。
 *
 * 协作: HierarchicalCluster8(层次聚类) / KMedoids16(K中心点) / DBSCAN11(DBSCAN)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 子空间聚类(CLIQUE自适应网格+显著密集单元挖掘)
 */
class SubspaceCluster6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalFits = 0;
        int numSamples = 0;
        int numClusters = 0;
        int numDenseUnits = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief A dense unit in a subspace */
    struct DenseUnit {
        QVector<int> dims;          // subspace dimensions
        QVector<int> gridIdx;       // grid cell index per dimension
        int count = 0;              // number of points in this unit
    };

    explicit SubspaceCluster6(QObject *parent = nullptr);
    ~SubspaceCluster6() override;

    void setGridResolution(int bins);
    void setDensityThreshold(double tau);
    void setMaxDimensions(int maxDims);

    /** @brief Run CLIQUE on n x d data matrix */
    QVector<int> fit(const QVector<QVector<double>>& data);

    /** @brief Get discovered dense units */
    QVector<DenseUnit> denseUnits() const;

    /** @brief Identify 1-D dense units for each dimension */
    QVector<QVector<DenseUnit>> findDense1D(const QVector<QVector<double>>& data) const;

    /** @brief Mine dense units in higher dimensions via candidate generation */
    QVector<DenseUnit> mineHigherDims(const QVector<QVector<DenseUnit>>& dense1D,
                                       const QVector<QVector<double>>& data) const;

    /** @brief Merge connected dense units into clusters */
    QVector<int> mergeClusters(const QVector<DenseUnit>& units,
                                const QVector<QVector<double>>& data) const;

    /** @brief Compute adaptive grid boundaries for a dimension */
    QVector<double> adaptiveGrid(const QVector<double>& values) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int clusters, int denseUnits, double timeMs);

private:
    int m_bins = 10;
    double m_tau = 0.15;
    int m_maxDims = 3;

    QVector<DenseUnit> m_denseUnits;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Map value to grid bin */
    int toBin(double value, const QVector<double>& boundaries) const;

    /** @brief Check if two dense units are adjacent */
    bool adjacent(const DenseUnit& a, const DenseUnit& b) const;

    /** @brief Compute coverage of dense unit on data */
    int countInUnit(const DenseUnit& unit, const QVector<QVector<double>>& data,
                     const QVector<QVector<double>>& grids) const;
};
