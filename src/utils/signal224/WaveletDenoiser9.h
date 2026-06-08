/**
 * @file WaveletDenoiser9.h
 * @brief 小波去噪器(SWT平稳小波变换+SureShrink层依赖阈值) — Wavelet Denoiser with SWT Stationary Wavelet Transform and SureShrink Level-Dependent Thresholding
 *
 * 功能: 实现基于平稳小波变换(SWT)的小波去噪，使用SureShrink
 *       层依赖阈值选取，实现信号的自适应降噪处理。
 *
 * 协作: FIRFilter7(FIR滤波器) / WaveletTransform8(小波变换) / FFTAnalyzer7(FFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 小波去噪器(SWT+SureShrink)
 */
class WaveletDenoiser9 : public QObject {
    Q_OBJECT

public:
    /** @brief Wavelet family */
    enum class Wavelet {
        Haar,
        DB2,
        DB4,
        Sym4
    };

    /** @brief Denoising result */
    struct DenoiseResult {
        QVector<double> signal;
        QVector<double> noise;
        QVector<double> thresholds;
        double inputSNR = 0.0;
        double outputSNR = 0.0;
        int levels = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int signalLength = 0;
        int decompLevels = 0;
        int waveletOrder = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit WaveletDenoiser9(QObject *parent = nullptr);
    ~WaveletDenoiser9() override;

    /** @brief Set wavelet type and decomposition levels */
    void setParameters(Wavelet wavelet = Wavelet::DB4, int levels = -1);

    /** @brief Denoise signal using SWT + SureShrink */
    DenoiseResult denoise(const QVector<double>& signal);

    /** @brief Forward SWT decomposition */
    void forwardSWT(const QVector<double>& signal,
                     QVector<QVector<double>>& approx,
                     QVector<QVector<double>>& detail) const;

    /** @brief Inverse SWT reconstruction */
    QVector<double> inverseSWT(
        const QVector<QVector<double>>& approx,
        const QVector<QVector<double>>& detail) const;

    /** @brief SureShrink threshold for a detail coefficient vector */
    double sureShrinkThreshold(const QVector<double>& coeffs) const;

    /** @brief Universal threshold (VisuShrink) */
    double universalThreshold(const QVector<double>& coeffs,
                                int n) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void denoisingCompleted(int levels, double inputSNR, double outputSNR,
                             double timeMs);

private:
    Wavelet m_wavelet = Wavelet::DB4;
    int m_levels = -1;  // auto-determine
    QVector<double> m_loD;  // decomposition low-pass
    QVector<double> m_hiD;  // decomposition high-pass
    QVector<double> m_loR;  // reconstruction low-pass
    QVector<double> m_hiR;  // reconstruction high-pass

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Load wavelet filter coefficients */
    void loadFilters();

    /** @brief Circular convolution */
    QVector<double> circConvolve(const QVector<double>& signal,
                                   const QVector<double>& filter) const;

    /** @brief Soft thresholding */
    static double softThreshold(double value, double threshold);

    /** @brief Compute SNR in dB */
    static double computeSNR(const QVector<double>& signal,
                               const QVector<double>& noise);

    /** @brief Stein's Unbiased Risk Estimate */
    double computeSURE(const QVector<double>& coeffs,
                        double threshold) const;
};
