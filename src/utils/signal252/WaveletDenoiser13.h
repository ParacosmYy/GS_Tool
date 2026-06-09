/**
 * @file WaveletDenoiser13.h
 * @brief 小波去噪(自适应贝叶斯软阈值+尺度间父子依赖建模) — Wavelet Denoiser with Adaptive Bayesian Soft Threshold and Interscale Parent-Child Dependency Modeling
 *
 * 功能: 实现小波去噪算法(Wavelet Denoising)，使用自适应贝叶斯软阈值
 *       (adaptive Bayesian soft threshold)根据子带统计特性动态估计阈值，
 *       尺度间父子依赖建模(interscale parent-child dependency)利用跨尺度
 *       相关性提高去噪质量。
 *
 * 协作: KalmanFilter12(卡尔曼滤波) / WaveletTransform11(小波变换) / FFTFilter10(FFT滤波)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 小波去噪(自适应贝叶斯软阈值+尺度间父子依赖)
 */
class WaveletDenoiser13 : public QObject {
    Q_OBJECT

public:
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

    explicit WaveletDenoiser13(QObject *parent = nullptr);
    ~WaveletDenoiser13() override;

    /** @brief Set decomposition levels */
    void setNumLevels(int levels);

    /** @brief Set wavelet type: "haar", "db2", "db4" */
    void setWavelet(const QString& wavelet);

    /** @brief Denoise signal, return cleaned signal */
    QVector<double> denoise(const QVector<double>& signal);

    /** @brief Get wavelet coefficients from last decomposition */
    QVector<QVector<double>> coefficients() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void denoisingCompleted(int length, double snr, double timeMs);

private:
    int m_numLevels = 4;
    QString m_wavelet = "haar";

    QVector<QVector<double>> m_coeffs;  // Per-level detail coefficients
    QVector<double> m_approx;           // Final approximation
    QVector<double> m_thresholds;       // Per-level thresholds

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Get wavelet filter coefficients */
    void getFilterCoeffs(QVector<double>& loD, QVector<double>& hiD,
                          QVector<double>& loR, QVector<double>& hiR) const;

    /** @brief Wavelet decomposition */
    void decompose(const QVector<double>& signal);

    /** @brief Wavelet reconstruction */
    QVector<double> reconstruct();

    /** @brief Estimate noise sigma via MAD of finest detail coefficients */
    double estimateNoiseSigma(const QVector<double>& detail) const;

    /** @brief Compute adaptive Bayesian threshold per level */
    double bayesianThreshold(const QVector<double>& detail,
                              double sigma) const;

    /** @brief Apply soft thresholding */
    void softThreshold(QVector<double>& coeff, double threshold);

    /** @brief Apply interscale parent-child dependency modeling */
    void applyParentChildDependency();

    /** @brief Convolve signal with filter, then downsample by 2 */
    QVector<double> downsampleConvolve(const QVector<double>& sig,
                                         const QVector<double>& filter) const;

    /** @brief Upsample by 2, then convolve with filter */
    QVector<double> upsampleConvolve(const QVector<double>& sig,
                                       const QVector<double>& filter,
                                       int targetLen) const;
};
