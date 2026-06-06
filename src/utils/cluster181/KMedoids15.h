/**
 * @file KMedoids15.h
 * @brief K-Medoids聚类(PAM交换启发式+CLARA大规模采样) — K-Medoids Clustering with PAM Swap Heuristic and CLARA Large-Scale Sampling
 *
 * 功能: 实现K-Medoids聚类算法，支持PAM(Partition Around Medoids)交换启发式、
 *       CLARA大规模数据采样策略、轮廓系数评估和多种距离度量。
 *
 * 协作: GaussianMixture13(高斯混合) / KMeans16(K均值) / DBSCAN10(DBSCAN)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief K-Medoids聚类器(PAM交换+CLARA采样)
 */
class KMedoids15 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;
        int numClusters = 0;
        int numIterations = 0;
        double totalCost = 0.0;
        double silhouetteScore = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Distance metric type */
    enum class Metric { Euclidean, Manhattan, Cosine };

    explicit KMedoids15(QObject *parent = nullptr);
    ~KMedoids15() override;

    void setNumClusters(int k);
    void setMaxIterations(int iter);
    void setTolerance(double tol);
    void setMetric(Metric m);
    void setUseCLARA(bool enabled);
    void setClarSamples(int samples);
    void setClarSampleSize(int size);

    /** @brief 执行K-Medoids聚类，返回样本标签 */
    QVector<int> fit(const QVector<QVector<double>>& data);

    /** @brief 计算轮廓系数 */
    double silhouette(const QVector<QVector<double>>& data,
                      const QVector<int>& labels) const;

    /** @brief 获取Medoid索引 */
    QVector<int> medoidIndices() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int numClusters, double totalCost, double silhouette);

private:
    int m_numClusters = 3;
    int m_maxIterations = 100;
    double m_tolerance = 1e-6;
    Metric m_metric = Metric::Euclidean;
    bool m_useCLARA = false;
    int m_clarSamples = 5;
    int m_clarSampleSize = 40;

    QVector<int> m_medoids;
    QVector<int> m_labels;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute distance between two points */
    double distance(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief Compute dissimilarity matrix subset */
    QVector<QVector<double>> dissimilarityMatrix(
        const QVector<QVector<double>>& data) const;

    /** @brief PAM build phase: select initial medoids */
    QVector<int> pamBuild(const QVector<QVector<double>>& dist,
                          int n, int k) const;

    /** @brief PAM swap phase: iteratively improve medoids */
    double pamSwap(const QVector<QVector<double>>& dist,
                   QVector<int>& medoids, int n, int k);

    /** @brief Assign each point to nearest medoid */
    QVector<int> assign(const QVector<QVector<double>>& dist,
                        const QVector<int>& medoids, int n) const;

    /** @brief CLARA sampling wrapper */
    QVector<int> claraFit(const QVector<QVector<double>>& data);

    /** @brief Generate random index subset */
    QVector<int> sampleIndices(int n, int size) const;

    /** @brief Map local labels back to full dataset */
    QVector<int> mapLabels(const QVector<QVector<double>>& data,
                           const QVector<int>& sampledIdx,
                           const QVector<QVector<double>>& sampledData,
                           const QVector<int>& localLabels) const;
};
