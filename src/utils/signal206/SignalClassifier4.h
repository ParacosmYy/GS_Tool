/**
 * @file SignalClassifier4.h
 * @brief 信号分类器(倒谱特征+高斯混合后端+BIC模型选择) — Signal Classifier with Cepstral Features, Gaussian Mixture Backend and Bayesian Information Criterion
 *
 * 功能: 实现信号分类器，支持倒谱特征提取、高斯混合模型后端
 *       和BIC模型选择。
 *
 * 协作: SignalDetector3(信号检测) / FFTW5(FFT引擎) / KalmanFilter4(卡尔曼滤波)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 信号分类器(倒谱特征+高斯混合后端+BIC模型选择)
 */
class SignalClassifier4 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalClassifications = 0;
        int numClasses = 0;
        int featureDim = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Gaussian mixture component */
    struct GMMComponent {
        QVector<double> mean;
        QVector<QVector<double>> covariance;
        double weight = 0.0;
    };

    explicit SignalClassifier4(QObject *parent = nullptr);
    ~SignalClassifier4() override;

    void setNumClasses(int k);
    void setNumMixtures(int m);
    void setFeatureDim(int dim);

    /** @brief Train classifier with labeled signal segments */
    void train(const QVector<QVector<double>>& features, const QVector<int>& labels);

    /** @brief Classify a single signal segment */
    int classify(const QVector<double>& features) const;

    /** @brief Extract cepstral features from signal frame */
    QVector<double> extractCepstral(const QVector<double>& frame) const;

    /** @brief Fit a GMM to feature vectors via EM algorithm */
    QVector<GMMComponent> fitGMM(const QVector<QVector<double>>& data,
                                   int numMixtures, int maxIter = 50) const;

    /** @brief Compute log-likelihood of data under GMM */
    double gmmLogLikelihood(const QVector<QVector<double>>& data,
                             const QVector<GMMComponent>& gmm) const;

    /** @brief Compute BIC score for model selection */
    double computeBIC(const QVector<QVector<double>>& data,
                       const QVector<GMMComponent>& gmm) const;

    /** @brief Compute multivariate Gaussian log-pdf */
    double gaussianLogPdf(const QVector<double>& x, const QVector<double>& mean,
                           const QVector<QVector<double>>& cov) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void classificationCompleted(int label, double confidence, double timeMs);

private:
    int m_numClasses = 4;
    int m_numMixtures = 3;
    int m_featureDim = 13;

    // Per-class GMMs
    QVector<QVector<GMMComponent>> m_classModels;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Initialize GMM parameters via K-means++ */
    QVector<GMMComponent> initGMM(const QVector<QVector<double>>& data, int m) const;

    /** @brief Compute diagonal covariance for numerical stability */
    static QVector<QVector<double>> diagCov(const QVector<double>& diag);

    /** @brief Cholesky decomposition determinant (log) */
    static double logDet(const QVector<QVector<double>>& cov);
};
