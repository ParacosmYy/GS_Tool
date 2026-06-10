/**
 * @file WaveletDenoiser15.h
 * @brief 小波去噪(SURE自适应阈值选择与循环旋转平移不变去噪) — Wavelet Denoiser with SURE-based Adaptive Threshold Selection and Cycle-spinning for Translation-invariant Denoising
 *
 * 功能: 实现小波去噪(Wavelet denoiser)，采用SURE自适应阈值选择(SURE-based adaptive
 *       threshold selection)和循环旋转(cycle-spinning)实现平移不变去噪
 *       (translation-invariant denoising)。
 *
 * 协作: FftFilter10(FFT滤波器) / KalmanFilter11(卡尔曼滤波) / MedianFilter8(中值滤波)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 小波去噪(SURE自适应阈值选择与循环旋转平移不变去噪)
 */
class WaveletDenoiser15 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int signalLength = 0;
        int numLevels = 0;
        double threshold = 0.0;
        double inputSNR = 0.0;
        double outputSNR = 0.0;
        int numSpins = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Wavelet types */
    enum class WaveletType { Haar, D4, D6, D8 };

    explicit WaveletDenoiser15(QObject *parent = nullptr);
    ~WaveletDenoiser15() override;

    /** @brief Set denoising parameters */
    void setParameters(int decomposeLevels = 4, WaveletType wavelet = WaveletType::D4,
                       int numCycleSpins = 0, double maxThreshold = 0.0);

    /** @brief Denoise signal with SURE threshold */
    QVector<double> denoise(const QVector<double>& signal);

    /** @brief Get detail coefficients from last decomposition */
    QVector<QVector<double>> detailCoefficients() const;

    /** @brief Get computed threshold */
    double threshold() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void denoisingComplete(int length, double threshold, double timeMs);

private:
    int m_levels = 4;
    WaveletType m_wavelet = WaveletType::D4;
    int m_numSpins = 0;     // 0 = no cycle-spinning
    double m_maxThreshold = 0.0;

    QVector<QVector<double>> m_details;  // Detail coefficients per level
    QVector<double> m_approx;            // Final approximation
    double m_threshold = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Get wavelet filter coefficients */
    QVector<double> lowFilter() const;
    QVector<double> highFilter() const;

    /** @brief Forward wavelet transform (single level) */
    void forwardDWT(const QVector<double>& signal,
                    QVector<double>& approx, QVector<double>& detail) const;

    /** @brief Inverse wavelet transform (single level) */
    QVector<double> inverseDWT(const QVector<double>& approx,
                                const QVector<double>& detail) const;

    /** @brief Full forward decomposition */
    void decompose(const QVector<double>& signal);

    /** @brief Full reconstruction */
    QVector<double> reconstruct();

    /** @brief SURE-based threshold selection (VisuShrink) */
    double computeSUREThreshold(const QVector<double>& coeffs) const;

    /** @brief Soft thresholding */
    static QVector<double> softThreshold(const QVector<double>& coeffs, double t);

    /** @brief Compute signal-to-noise ratio */
    static double computeSNR(const QVector<double>& signal, const QVector<double>& noise);
};
