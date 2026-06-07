/**
 * @file WaveletDenoiser5.h
 * @brief 小波去噪(双树复小波变换DTCWT+邻域相关阈值) — Wavelet Denoiser with Dual-Tree Complex Wavelet Transform (DTCWT) and Neighbor-Dependent Threshold
 *
 * 功能: 实现小波去噪，支持双树复小波变换(DTCWT)、
 *       邻域相关阈值和自适应噪声估计。
 *
 * 协作: WaveletTransform6(小波变换) / FftEngine3(FFT引擎) / KalmanFilter5(卡尔曼滤波)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 小波去噪(DTCWT+邻域阈值)
 */
class WaveletDenoiser5 : public QObject {
    Q_OBJECT

public:
    /** @brief Threshold strategy */
    enum Threshold { Universal, BayesShrink, NeighborDependent };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalDenoise = 0;
        int signalLength = 0;
        int levels = 0;
        double noiseEstimate = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit WaveletDenoiser5(QObject *parent = nullptr);
    ~WaveletDenoiser5() override;

    void setLevels(int levels);
    void setThreshold(Threshold method);
    void setWaveletLength(int len);

    /** @brief Denoise signal using DTCWT with neighbor-dependent threshold */
    QVector<double> denoise(const QVector<double>& signal);

    /** @brief Estimate noise level (MAD of finest wavelet coefficients) */
    double estimateNoise(const QVector<double>& signal) const;

    /** @brief Forward DTCWT decomposition */
    QVector<QVector<QVector<double>>> dtcwtForward(const QVector<double>& signal) const;

    /** @brief Inverse DTCWT reconstruction */
    QVector<double> dtcwtInverse(const QVector<QVector<QVector<double>>>& coeffs) const;

    /** @brief Get wavelet coefficients from last denoise */
    QVector<QVector<QVector<double>>> lastCoefficients() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void denoiseCompleted(int len, double noiseEst, double timeMs);

private:
    int m_levels = 4;
    Threshold m_threshold = NeighborDependent;
    int m_waveletLen = 10;

    QVector<QVector<QVector<double>>> m_lastCoeffs;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Tree-A filter (lowpass) */
    static const double* filterLoA();
    /** @brief Tree-A filter (highpass) */
    static const double* filterHiA();
    /** @brief Tree-B filter (lowpass) */
    static const double* filterLoB();
    /** @brief Tree-B filter (highpass) */
    static const double* filterHiB();
    static int filterLen();

    /** @brief Convolve with periodic extension */
    static QVector<double> convolve(const QVector<double>& x,
                                     const double* h, int hLen);

    /** @brief Downsample by 2 */
    static QVector<double> downsample(const QVector<double>& x);

    /** @brief Upsample by 2 */
    static QVector<double> upsample(const QVector<double>& x);

    /** @brief Apply neighbor-dependent threshold */
    static QVector<QVector<double>> neighborThreshold(
        const QVector<QVector<double>>& realTree,
        const QVector<QVector<double>>& imagTree,
        double sigma);

    /** @brief Universal threshold (VisuShrink) */
    static double universalThreshold(const QVector<double>& coeffs, double sigma);

    /** @brief BayesShrink threshold */
    static double bayesThreshold(const QVector<double>& coeffs, double sigma);
};
