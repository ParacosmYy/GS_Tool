/**
 * @file GaussianMixture33.h
 * @brief 高斯混合模型(增量Cholesky分解与充分统计量合并的在线EM更新) — Gaussian Mixture with Incremental Cholesky Factorization and Sufficient Statistics Merge for Online EM Updates
 *
 * 功能: 实现高斯混合模型(Gaussian mixture model)，采用增量Cholesky分解(incremental Cholesky factorization)
 *       与充分统计量合并(sufficient statistics merge)实现在线EM更新(online EM updates)。
 *
 * 协作: OPTICS13(密度聚类) / SubspaceCluster12(子空间聚类) / HierarchicalCluster14(层次聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 高斯混合模型(增量Cholesky分解与充分统计量合并)
 */
class GaussianMixture33 : public QObject {
    Q_OBJECT

public:
    /** @brief Single Gaussian component */
    struct Component {
        double weight = 1.0;
        QVector<double> mean;
        QVector<double> cholDiag;    // Cholesky diagonal entries
        QVector<double> cholLower;   // Lower-triangular packed Cholesky
        double logDet = 0.0;
    };

    /** @brief Sufficient statistics for online merge */
    struct SufficientStats {
        double Nk = 0.0;
        QVector<double> sumX;
        QVector<double> sumXXt;      // Packed upper triangle
    };

    /** @brief EM result */
    struct GMMResult {
        QVector<Component> components;
        QVector<int> labels;
        double logLikelihood = 0.0;
        int iterations = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numComponents = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GaussianMixture33(QObject *parent = nullptr);
    ~GaussianMixture33() override;

    void setNumComponents(int k);
    void setMaxIterations(int iters);
    void setTolerance(double tol);

    /** @brief Batch EM fitting */
    GMMResult fit(const QVector<QVector<double>>& data);

    /** @brief Online EM update with new batch */
    void onlineUpdate(const QVector<QVector<double>>& newBatch);

    /** @brief Predict component posterior for a sample */
    QVector<double> predict(const QVector<double>& sample) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fitDone(int n, int k, double ll, double timeMs);

private:
    int m_k = 3;
    int m_maxIter = 100;
    double m_tol = 1e-6;
    int m_dim = 0;
    Stats m_stats;
    double m_timeSum = 0.0;

    QVector<Component> m_components;
    QVector<SufficientStats> m_stats_acc;
    double m_totalWeight = 0.0;

    /** @brief Initialize components via k-means++ seeding */
    void initKMeansPP(const QVector<QVector<double>>& data);

    /** @brief Incremental Cholesky rank-1 update */
    void choleskyUpdate(QVector<double>& diag, QVector<double>& lower,
                        const QVector<double>& x, double alpha);

    /** @brief Compute log N(x|mu,L) for one component */
    double logGaussian(const QVector<double>& x,
                       const Component& comp) const;

    /** @brief Merge sufficient statistics into existing component */
    void mergeStats(int k, const QVector<double>& x, double resp);

    /** @brief Rebuild Cholesky from accumulated sufficient stats */
    void rebuildCholesky(int k);

    /** @brief E-step: compute responsibilities */
    void eStep(const QVector<QVector<double>>& data,
               QVector<QVector<double>>& resp, double& ll);

    /** @brief M-step: update parameters from responsibilities */
    void mStep(const QVector<QVector<double>>& data,
               const QVector<QVector<double>>& resp);
};
