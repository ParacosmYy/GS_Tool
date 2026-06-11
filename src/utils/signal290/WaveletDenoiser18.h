/**
 * @file WaveletDenoiser18.h
 * @brief 小波去噪(贝叶斯收缩与后验中值估计的层自适应阈值) — Wavelet Denoiser with Bayesian Shrinkage and Level-dependent Threshold Adaptation Using Posterior Median Estimation
 *
 * 功能: 实现小波去噪(Wavelet denoiser)，采用贝叶斯收缩(Bayesian shrinkage)
 *       与后验中值估计(posterior median estimation)实现层自适应阈值(level-dependent threshold adaptation)。
 *
 * 协作: WaveletTransform16(小波变换) / WienerFilter12(Wiener滤波) / MedianFilter9(中值滤波)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 小波去噪(贝叶斯收缩与后验中值估计的层自适应阈值)
 */
class WaveletDenoiser18 : public QObject {
    Q_OBJECT

public:
    /** @brief Wavelet family */
    enum class Wavelet {
        Haar = 0,
        Daubechies4,
        Daubechies8,
        Symlet6
    };

    /** @brief Denoising parameters */
    struct Params {
        Wavelet wavelet = Wavelet::Daubechies4;
        int levels = 5;                   // Decomposition levels
        double noiseEstimate = 0.0;       // 0 = auto estimate via MAD
        bool posteriorMedian = true;       // Use posterior median vs soft threshold
    };

    /** @brief Denoising result */
    struct DenoiseResult {
        QVector<double> denoised;
        QVector<QVector<double>> detailCoeffs; // Per-level detail coefficients
        QVector<double> thresholds;            // Per-level thresholds
        double estimatedNoise = 0.0;
        int levelsUsed = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int inputLength = 0;
        int levelsUsed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit WaveletDenoiser18(QObject *parent = nullptr);
    ~WaveletDenoiser18() override;

    void setParams(const Params& p);

    /** @brief Denoise input signal */
    DenoiseResult denoise(const QVector<double>& input);

    /** @brief Estimate noise via Median Absolute Deviation */
    double estimateNoiseMAD(const QVector<double>& coeffs) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void denoiseDone(int n, int levels, double noise, double timeMs);

private:
    Params m_params;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Get wavelet filter coefficients (decomposition low-pass) */
    QVector<double> lowPassFilter(Wavelet w) const;

    /** @brief Get wavelet filter coefficients (decomposition high-pass) */
    QVector<double> highPassFilter(Wavelet w) const;

    /** @brief Single-level DWT: convolve and downsample by 2 */
    void dwtLevel(const QVector<double>& signal,
                  const QVector<double>& lo, const QVector<double>& hi,
                  QVector<double>& approx, QVector<double>& detail) const;

    /** @brief Single-level IDWT: upsample, convolve and reconstruct */
    QVector<double> idwtLevel(const QVector<double>& approx,
                               const QVector<double>& detail,
                               const QVector<double>& lo, const QVector<double>& hi,
                               int targetLen) const;

    /** @brief Bayesian shrinkage: posterior median estimator */
    double bayesianShrink(double coeff, double sigma, double sigmaPrior) const;

    /** @brief Level-dependent threshold via Bayesian risk */
    double levelThreshold(int level, int maxLevel, double sigma) const;

    /** @brief Estimate prior variance of coefficients at a level */
    double estimatePriorVariance(const QVector<double>& coeffs, double sigma) const;
};
