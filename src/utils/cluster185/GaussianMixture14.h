/**
 * @file GaussianMixture14.h
 * @brief 变分贝叶斯高斯混合模型(Dirichlet先验+自动剪枝) — Variational Bayesian Gaussian Mixture Model with Dirichlet Prior and Automatic Component Pruning
 *
 * 功能: 实现变分贝叶斯推断的高斯混合模型，支持Dirichlet先验浓度控制、
 *       自动分量剪枝、ELBO收敛监控和责任矩阵计算。
 *
 * 协作: OPTICS6(密度聚类) / KMeans5(初始中心) / DBSCAN10(密度基线)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 变分贝叶斯高斯混合模型(Dirichlet先验+自动剪枝)
 */
class GaussianMixture14 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;
        int numPoints = 0;
        int activeComponents = 0;
        double finalElbo = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief 模型参数 */
    struct ModelParams {
        QVector<QVector<double>> means;
        QVector<QVector<QVector<double>>> covariances;
        QVector<double> weights;
        QVector<double> dirichletAlpha;
    };

    explicit GaussianMixture14(QObject *parent = nullptr);
    ~GaussianMixture14() override;

    void setMaxComponents(int k);
    void setDirichletAlpha(double alpha);
    void setMaxIterations(int iter);
    void setTolerance(double tol);

    /** @brief 拟合模型，返回分量标签 */
    QVector<int> fit(const QVector<QVector<double>>& data);

    /** @brief 计算样本的对数似然 */
    double logLikelihood(const QVector<QVector<double>>& data) const;

    /** @brief 预测新样本的分量标签 */
    QVector<int> predict(const QVector<QVector<double>>& data) const;

    /** @brief 计算责任矩阵(N×K) */
    QVector<QVector<double>> responsibilities(const QVector<QVector<double>>& data) const;

    /** @brief 获取ELBO收敛历史 */
    QVector<double> elboHistory() const { return m_elboHistory; }

    const ModelParams& params() const { return m_params; }
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fittingCompleted(int activeComponents, double finalElbo, double timeMs);

private:
    int m_maxComponents = 10;
    double m_dirichletAlpha = 1.0;
    int m_maxIterations = 200;
    double m_tolerance = 1e-6;

    ModelParams m_params;
    Stats m_stats;
    double m_timeSum = 0.0;
    QVector<double> m_elboHistory;

    /** @brief Initialize parameters via k-means++ style seeding */
    void initialize(const QVector<QVector<double>>& data);

    /** @brief E-step: compute responsibilities */
    QVector<QVector<double>> eStep(const QVector<QVector<double>>& data) const;

    /** @brief M-step: update variational parameters */
    void mStep(const QVector<QVector<double>>& data,
               const QVector<QVector<double>>& resp);

    /** @brief Compute Evidence Lower BOund */
    double computeElbo(const QVector<QVector<double>>& data,
                       const QVector<QVector<double>>& resp) const;

    /** @brief Log-sum-exp for numerical stability */
    double logSumExp(const QVector<double>& vals) const;

    /** @brief Multivariate Gaussian log-pdf */
    double logGaussian(const QVector<double>& x, int comp) const;

    /** @brief Count active components (weight above threshold) */
    int countActive() const;

    /** @brief 2D matrix determinant */
    double determinant(const QVector<QVector<double>>& mat) const;

    /** @brief 2D matrix inverse */
    QVector<QVector<double>> inverse(const QVector<QVector<double>>& mat) const;
};
