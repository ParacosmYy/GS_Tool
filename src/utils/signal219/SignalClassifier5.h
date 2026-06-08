/**
 * @file SignalClassifier5.h
 * @brief 信号分类器(卷积特征提取+最近质心投票与留一交叉验证) — Signal Classifier with Convolutional Feature Extraction and Nearest-Centroid Voting with LOO Validation
 *
 * 功能: 实现信号分类器，使用卷积核提取时频特征，最近质心投票进行分类，
 *       内置留一交叉验证评估泛化性能，支持多类信号自动识别。
 *
 * 协作: SignalDetector4(信号检测) / ModulationRecognizer4(调制识别) / SpectralEstimator5(谱估计)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 信号分类器(卷积特征+最近质心+LOO验证)
 */
class SignalClassifier5 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numClasses = 0;
        int numFeatures = 0;
        int numSamples = 0;
        double looAccuracy = 0.0;
        double trainAccuracy = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SignalClassifier5(QObject *parent = nullptr);
    ~SignalClassifier5() override;

    /** @brief Set number of convolution kernels and feature dimension */
    void setParameters(int numKernels, int kernelSize);

    /** @brief Train with labeled data */
    void train(const QVector<QVector<double>>& samples,
               const QVector<int>& labels);

    /** @brief Classify a single signal */
    int classify(const QVector<double>& signal) const;

    /** @brief Batch classify signals */
    QVector<int> classifyBatch(const QVector<QVector<double>>& samples) const;

    /** @brief Run leave-one-out cross validation */
    double looValidate(const QVector<QVector<double>>& samples,
                       const QVector<int>& labels);

    /** @brief Get class centroids */
    QVector<QVector<double>> centroids() const;

    /** @brief Get per-class accuracy from LOO */
    QVector<double> perClassAccuracy() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void trainingCompleted(int classes, double accuracy, double timeMs);

private:
    int m_numKernels = 8;
    int m_kernelSize = 7;
    int m_numClasses = 0;

    // Learned convolution kernels
    QVector<QVector<double>> m_kernels;

    // Class centroids in feature space
    QVector<QVector<double>> m_centroids;

    // LOO per-class accuracy
    QVector<double> m_perClassAcc;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Extract features via convolution + pooling */
    QVector<double> extractFeatures(const QVector<double>& signal) const;

    /** @brief 1D convolution with single kernel */
    QVector<double> convolve1d(const QVector<double>& signal,
                                const QVector<double>& kernel) const;

    /** @brief Max-pooling with stride */
    QVector<double> maxPool(const QVector<double>& input, int poolSize) const;

    /** @brief Initialize kernels (Gabor-like) */
    void initKernels(int signalLength);

    /** @brief Euclidean distance in feature space */
    double featureDistance(const QVector<double>& a,
                           const QVector<double>& b) const;

    /** @brief Vote among k nearest centroids */
    int nearestCentroidVote(const QVector<double>& features) const;
};
