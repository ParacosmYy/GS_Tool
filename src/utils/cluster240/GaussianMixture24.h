/**
 * @file GaussianMixture24.h
 * @brief 高斯混合模型(蒙特卡洛EM+合并/分裂似然比检验模型选择) — Gaussian Mixture Model with Monte Carlo EM and Component Merge/Split Likelihood Ratio Test for Model Selection
 *
 * 功能: 实现高斯混合模型(Gaussian mixture model)参数估计，采用蒙特卡洛EM
 *       (Monte Carlo EM)进行期望最大化迭代，结合合并/分裂似然比检验
 *       (merge/split likelihood ratio test)自动确定最优分量数。
 *
 * 协作: OPTICS10(密度聚类) / KMedoids19(K-中心点) / SubspaceCluster9(子空间聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 高斯混合模型(蒙特卡洛EM+合并/分裂似然比检验模型选择)
 */
class GaussianMixture24 : public QObject {
    Q_OBJECT

public:
    /** @brief Single Gaussian component parameters */
    struct Component {
        double weight = 1.0;
        QVector<double> mean;
        double variance = 1.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numComponents = 0;
        int numDimensions = 0;
        int numSamples = 0;
        double logLikelihood = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GaussianMixture24(QObject *parent = nullptr);
    ~GaussianMixture24() override;

    /** @brief Set number of initial components */
    void setNumComponents(int k);

    /** @brief Set maximum EM iterations */
    void setMaxIterations(int iters);

    /** @brief Set convergence threshold for log-likelihood change */
    void setConvergenceThreshold(double tol);

    /** @brief Set number of Monte Carlo samples per E-step */
    void setMcSamples(int n);

    /** @brief Fit model to data, returns final components */
    QVector<Component> fit(const QVector<QVector<double>>& data);

    /** @brief Get responsibility matrix [n x k] */
    QVector<QVector<double>> responsibilities() const;

    /** @brief Predict component label for each sample */
    QVector<int> predict() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void emIterationCompleted(int iter, double logLik);
    void modelSelected(int numComponents, double bic);

private:
    int m_k = 3;
    int m_maxIter = 100;
    double m_tol = 1e-6;
    int m_mcSamples = 500;

    QVector<QVector<double>> m_data;
    QVector<Component> m_components;
    QVector<QVector<double>> m_resp;  // responsibility [n x k]

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Evaluate Gaussian pdf at x for component c */
    double gaussianPdf(const QVector<double>& x, const Component& c) const;

    /** @brief E-step: compute responsibilities via Monte Carlo integration */
    void eStep();

    /** @brief M-step: update component parameters from responsibilities */
    void mStep();

    /** @brief Compute total log-likelihood */
    double logLikelihood() const;

    /** @brief Merge test: try merging each pair, keep best split */
    bool mergeTest();

    /** @brief Split test: try splitting each component, keep best merge */
    bool splitTest();

    /** @brief Initialize components via k-means++ seeding */
    void initializeComponents();
};
