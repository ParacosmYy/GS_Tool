/**
 * @file KMeans18.h
 * @brief 核K均值(Nyström低秩近似+Gram矩阵) — Kernel K-Means via Gram Matrix with Nyström Low-Rank Approximation
 *
 * 功能: 实现核K均值聚类，支持Gram矩阵计算、
 *       Nyström方法低秩近似加速和RBF/多项式核函数。
 *
 * 协作: GaussianMixture15(高斯混合) / SpectralCluster8(谱聚类) / FuzzyCMeans8(模糊C均值)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 核K均值(Nyström低秩近似+Gram矩阵)
 */
class KMeans18 : public QObject {
    Q_OBJECT

public:
    /** @brief Kernel type selector */
    enum Kernel { RBF, Polynomial, Linear };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;
        int numClusters = 0;
        int numPoints = 0;
        int nystromLandmarks = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit KMeans18(QObject *parent = nullptr);
    ~KMeans18() override;

    void setClusters(int k);
    void setKernel(Kernel kernel);
    void setKernelParam(double sigma);
    void setMaxIterations(int iter);
    void setLandmarkRatio(double ratio);

    /** @brief Cluster data points using kernel k-means with Nyström */
    QVector<int> fit(const QVector<QVector<double>>& data, int k);

    /** @brief Predict cluster for new points */
    QVector<int> predict(const QVector<QVector<double>>& points) const;

    /** @brief Get cluster centers in original space */
    QVector<QVector<double>> centers() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int k, int iterations, double timeMs);

private:
    int m_k = 3;
    Kernel m_kernel = RBF;
    double m_sigma = 1.0;
    int m_maxIter = 100;
    double m_landmarkRatio = 0.1;

    QVector<QVector<double>> m_data;
    QVector<QVector<double>> m_centers;
    QVector<int> m_labels;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute kernel value between two vectors */
    double kernelValue(const QVector<double>& a,
                       const QVector<double>& b) const;

    /** @brief Select Nyström landmark indices via k-means++ */
    QVector<int> selectLandmarks(int m) const;

    /** @brief Build Nyström approximation of Gram matrix */
    QVector<QVector<double>> nystromApprox(
        const QVector<int>& landmarks) const;

    /** @brief Compute distance to cluster via kernel trick */
    double kernelDistance(int point, int cluster,
                         const QVector<QVector<double>>& Knm,
                         const QVector<QVector<double>>& KmmInv,
                         const QVector<double>& clusterWeights) const;
};
