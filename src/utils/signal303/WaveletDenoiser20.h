/**
 * @file WaveletDenoiser20.h
 * @brief 小波去噪器(SURE风险估计与自适应软硬阈值插值实现未知噪声水平最优去噪) — Wavelet Denoiser with SURE Risk Estimation and Adaptive Soft-Hard Threshold Interpolation for Optimal Denoising in Unknown Noise Levels
 *
 * 功能: 实现小波去噪器(wavelet denoiser)，采用SURE风险估计(SURE risk estimation)
 *       与自适应软硬阈值插值(adaptive soft-hard threshold interpolation)实现未知噪声水平最优去噪(optimal denoising in unknown noise levels)。
 *
 * 协作: FFTAnalyzer(FFT分析) / KalmanFilter(卡尔曼滤波) / WienerFilter(维纳滤波)
 */
#pragma once

#include <QObject>
#include <QVector>

class WaveletDenoiser20 : public QObject {
    Q_OBJECT

public:
    /** @brief Wavelet type */
    enum class WaveletType { Haar, Daubechies4, Daubechies8, Symlet4 };

    /** @brief Threshold strategy */
    enum class ThresholdMethod { Soft, Hard, Adaptive, SURE };

    /** @brief Denoising result */
    struct DenoiseResult {
        QVector<double> signal;
        QVector<double> noiseEstimate;
        double estimatedSigma = 0.0;     // Estimated noise std deviation
        double sureRisk = 0.0;           // SURE estimated risk
        int levelsUsed = 0;
        double elapsedMs = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalDenoise = 0;
        int lastSignalLength = 0;
        double avgSigma = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit WaveletDenoiser20(QObject *parent = nullptr);
    ~WaveletDenoiser20() override;

    void setWaveletType(WaveletType type);
    void setDecompositionLevels(int levels);
    void setThresholdMethod(ThresholdMethod method);

    /** @brief Denoise signal with SURE-based threshold */
    DenoiseResult denoise(const QVector<double>& signal);

    /** @brief Forward wavelet transform */
    QVector<QVector<double>> forwardTransform(const QVector<double>& signal) const;

    /** @brief Inverse wavelet transform */
    QVector<double> inverseTransform(const QVector<QVector<double>>& coefficients) const;

    /** @brief Estimate noise sigma using MAD of finest detail coefficients */
    double estimateSigma(const QVector<double>& detailCoeffs) const;

    /** @brief Compute SURE risk for a threshold value */
    double computeSURERisk(const QVector<double>& coeffs, double threshold) const;

    /** @brief Find optimal threshold via SURE minimization */
    double findOptimalThreshold(const QVector<double>& coeffs) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void denoiseDone(double sigma, double sureRisk, int levels, double timeMs);

private:
    WaveletType m_wavelet = WaveletType::Daubechies4;
    int m_maxLevels = 8;
    ThresholdMethod m_method = ThresholdMethod::SURE;
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_sigmaSum = 0.0;

    /** @brief Get wavelet filter coefficients */
    QVector<double> lowPassFilter() const;
    QVector<double> highPassFilter() const;

    /** @brief Apply soft thresholding */
    QVector<double> softThreshold(const QVector<double>& coeffs, double lambda) const;

    /** @brief Apply hard thresholding */
    QVector<double> hardThreshold(const QVector<double>& coeffs, double lambda) const;

    /** @brief Apply adaptive soft-hard interpolation threshold */
    QVector<double> adaptiveThreshold(const QVector<double>& coeffs, double lambda) const;

    /** @brief Convolution with periodic extension */
    QVector<double> convolve(const QVector<double>& signal, const QVector<double>& filter) const;
};
