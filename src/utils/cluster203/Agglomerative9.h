/**
 * @file Agglomerative9.h
 * @brief 层次聚合聚类(加权平均链接+动态树切割) — Agglomerative Clustering with Weighted-Average Linkage and Dynamic Tree Cut for Variable-Depth Cluster Extraction
 *
 * 功能: 实现加权平均链接的层次聚合聚类，支持动态树切割
 *       提取可变深度簇、合并距离矩阵和树状图构建。
 *
 * 协作: KMeans19(K均值聚类) / DBSCAN8(密度聚类) / SpectralCluster9(谱聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 层次聚合聚类(加权平均链接+动态树切割)
 */
class Agglomerative9 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int numClusters = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Agglomerative9(QObject *parent = nullptr);
    ~Agglomerative9() override;

    void setNumClusters(int k);
    void setCutHeight(double height);

    /** @brief Run agglomerative clustering, return cluster labels */
    QVector<int> fit(const QVector<QVector<double>>& data);

    /** @brief Build dendrogram linkage matrix [n-1 x 4]: (i,j,dist,size) */
    QVector<QVector<double>> linkage(const QVector<QVector<double>>& data);

    /** @brief Dynamic tree cut: extract clusters at variable depths */
    QVector<int> dynamicCut(const QVector<QVector<double>>& linkageMatrix, int n);

    /** @brief Compute weighted-average linkage distance between two clusters */
    double weightedAverageDist(int c1, int c2,
                               const QVector<QVector<double>>& distMatrix,
                               const QVector<int>& sizes) const;

    /** @brief Cut dendrogram at fixed height */
    QVector<int> cutTree(const QVector<QVector<double>>& linkageMatrix,
                         int n, double height) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int clusters, double timeMs);

private:
    int m_targetK = 2;
    double m_cutHeight = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Euclidean distance */
    static double euclidean(const QVector<double>& a, const QVector<double>& b);

    /** @brief Build pairwise distance matrix */
    static QVector<QVector<double>> buildDistMatrix(const QVector<QVector<double>>& data);
};
