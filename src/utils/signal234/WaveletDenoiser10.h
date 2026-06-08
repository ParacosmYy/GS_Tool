/**
 * @file WaveletDenoiser10.h
 * @brief 小波去噪器(第二代提升格式+自适应贝叶斯收缩空间上下文) — Wavelet Denoiser with Second-Generation Lifting Scheme and Adaptive Bayesian Shrinkage with Spatial Context
 *
 * 功能: 实现小波去噪(wavelet denoiser)处理器，采用第二代提升格式(second-generation lifting scheme)
 *       进行小波变换，并结合自适应贝叶斯收缩(adaptive Bayesian shrinkage)与空间上下文(spatial context)
 *       精确估计和抑制噪声。
 *
 * 协作: WaveletTransform5(小波变换) / WienerFilter4(维纳滤波) / KalmanFilter6(卡尔曼滤波)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 小波去噪器(第二代提升格式+自适应贝叶斯收缩空间上下文)
 */
class WaveletDenoiser10 : public QObject {
    Q_OBJECT

public:
    /** @brief Lifting step definition */
    struct LiftingStep {
        bool isPredict = true;    // true=predict, false=update
        QVector<double> taps;     // filter coefficients
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int signalLength = 0;
        int numLevels = 0;
        double noiseSigma = 0.0;
        double inputSNR = 0.0;
        double outputSNR = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit WaveletDenoiser10(QObject *parent = nullptr);
    ~WaveletDenoiser10() override;

    /** @brief Set denoising parameters */
    void setParameters(int levels = 4, const QString& wavelet = "db4",
                       double thresholdScale = 1.0);

    /** @brief Process signal: denoise */
    QVector<double> denoise(const QVector<double>& signal);

    /** @brief Estimate noise sigma from finest-level detail coefficients */
    double estimateNoiseSigma(const QVector<double>& details) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void denoiseCompleted(int length, double inputSNR, double outputSNR, double timeMs);

private:
    int m_levels = 4;
    QString m_wavelet;
    double m_thresholdScale = 1.0;

    QVector<LiftingStep> m_forwardLifting;
    QVector<LiftingStep> m_inverseLifting;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build lifting steps for given wavelet name */
    void buildLiftingSteps();

    /** @brief Forward lifting transform: returns {approx, [detail levels]} */
    void forwardLift(QVector<double>& signal, int n,
                      QVector<QVector<double>>& details) const;

    /** @brief Inverse lifting transform from approx + details */
    QVector<double> inverseLift(const QVector<double>& approx,
                                  const QVector<QVector<double>>& details) const;

    /** @brief Adaptive Bayesian shrinkage with spatial context */
    void bayesianShrink(QVector<double>& details, double sigma) const;

    /** @brief Compute spatial context variance for a coefficient */
    double spatialVariance(const QVector<double>& details, int idx) const;
};
