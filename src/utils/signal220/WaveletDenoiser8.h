/**
 * @file WaveletDenoiser8.h
 * @brief 小波去噪(双树复小波变换+二元收缩+尺度间依赖) — Wavelet Denoiser with Dual-Tree Complex Wavelet Transform and Bivariate Shrinkage with Interscale Dependency
 *
 * 功能: 实现小波去噪算法，采用双树复小波变换实现近似平移不变性，
 *       二元收缩函数利用尺度间系数依赖关系自适应降噪。
 *
 * 协作: WaveletTransform6(小波变换) / WienerFilter5(维纳滤波) / SignalEstimator4(信号估计)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 小波去噪(双树复小波+二元收缩+尺度间依赖)
 */
class WaveletDenoiser8 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int signalLength = 0;
        int decompLevels = 0;
        double inputSnr = 0.0;
        double outputSnr = 0.0;
        double noiseSigma = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit WaveletDenoiser8(QObject *parent = nullptr);
    ~WaveletDenoiser8() override;

    /** @brief Set parameters: decomposition levels, threshold multiplier */
    void setParameters(int levels = 4, double thresholdScale = 1.0);

    /** @brief Denoise input signal, returns cleaned signal */
    QVector<double> denoise(const QVector<double>& input);

    /** @brief Estimate noise standard deviation via MAD */
    double estimateNoiseSigma(const QVector<double>& detailCoeffs) const;

    /** @brief Get wavelet decomposition coefficients */
    QVector<QVector<double>> detailCoefficients() const;

    /** @brief Get approximation coefficients */
    QVector<double> approximationCoefficients() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void denoiseCompleted(double inputSnr, double outputSnr, double timeMs);

private:
    int m_levels = 4;
    double m_thresholdScale = 1.0;

    // Dual-tree filter coefficients (Q-shift)
    QVector<double> m_h1a, m_h1b;  // Tree A and B low-pass
    QVector<double> m_g1a, m_g1b;  // Tree A and B high-pass

    // Stored coefficients
    QVector<QVector<double>> m_detailReal;
    QVector<QVector<double>> m_detailImag;
    QVector<double> m_approxReal;
    QVector<double> m_approxImag;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Initialize dual-tree filter coefficients */
    void initFilters();

    /** @brief Forward dual-tree complex wavelet transform */
    void forwardDTCWT(const QVector<double>& input);

    /** @brief Inverse dual-tree complex wavelet transform */
    QVector<double> inverseDTCWT();

    /** @brief Bivariate shrinkage on complex coefficients */
    void bivariateShrinkage(double sigma);

    /** @brief Apply filter and downsample */
    QVector<double> filterDownsample(const QVector<double>& input,
                                     const QVector<double>& filter) const;

    /** @brief Upsample and apply filter */
    QVector<double> upsampleFilter(const QVector<double>& input,
                                   const QVector<double>& filter,
                                   int targetLen) const;
};
