/**
 * @file GaussianMixture13.h
 * @brief 高斯混合模型(对角协方差+BIC/AIC模型选择+分裂合并初始化) — Gaussian Mixture Model with Diagonal Covariance, BIC/AIC Model Selection and Split-and-Merge Initialization
 *
 * 功能: 实现GMM聚类算法，支持对角协方差矩阵、BIC/AIC准则自动选择分量数、
 *       分裂合并(split-and-merge)初始化策略和EM迭代优化。
 *
 * 协作: KMeans16(K均值) / DBSCAN10(DBSCAN) / HDBSCAN7(HDBSCAN)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 高斯混合模型聚类器(对角协方差+BIC/AIC)
 */
class GaussianMixture13 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;
        int numComponents = 0;
        int numIterations = 0;
        double finalLogLikelihood = 0.0;
        double bicScore = 0.0;
        double aicScore = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Single Gaussian component */
    struct Component {
        double weight = 1.0;                ///< Mixing weight
        QVector<double> mean;               ///< Mean vector
        QVector<double> variance;           ///< Diagonal covariance
    };

    explicit GaussianMixture13(QObject *parent = nullptr);
    ~GaussianMixture13() override;

    void setNumComponents(int k);
    void setMaxIterations(int iter);
    void setTolerance(double tol);
    void setAutoModelSelection(bool enabled);
    void setMaxComponents(int maxK);

    /** @brief 执行GMM拟合，返回样本标签 */
    QVector<int> fit(const QVector<QVector<double>>& data);

    /** @brief 计算BIC分数 */
    double computeBIC(const QVector<QVector<double>>& data, int k) const;

    /** @brief 计算AIC分数 */
    double computeAIC(const QVector<QVector<double>>& data, int k) const;

    /** @brief 自动选择最优分量数 */
    int selectBestK(const QVector<QVector<double>>& data) const;

    /** @brief 获取拟合后的分量参数 */
    QVector<Component> components() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fittingCompleted(int numComponents, double logLikelihood, double bic);

private:
    int m_numComponents = 3;
    int m_maxIterations = 100;
    double m_tolerance = 1e-6;
    bool m_autoSelection = false;
    int m_maxComponents = 10;

    QVector<Component> m_components;
    QVector<QVector<double>> m_responsibilities;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Gaussian PDF with diagonal covariance */
    double gaussianPdf(const QVector<double>& x, const Component& comp) const;

    /** @brief E-step: compute responsibilities */
    void eStep(const QVector<QVector<double>>& data);

    /** @brief M-step: update parameters */
    void mStep(const QVector<QVector<double>>& data);

    /** @brief Compute log-likelihood */
    double logLikelihood(const QVector<QVector<double>>& data) const;

    /** @brief Initialize via split-and-merge */
    void splitAndMergeInit(const QVector<QVector<double>>& data);

    /** @brief K-means initialization */
    void kmeansInit(const QVector<QVector<double>>& data);

    /** @brief Random number in [0,1) */
    double randomUniform() const;
};
