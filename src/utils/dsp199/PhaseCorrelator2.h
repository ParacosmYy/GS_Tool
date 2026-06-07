/**
 * @file PhaseCorrelator2.h
 * @brief 相位相关(归一化互功率谱+亚像素峰值插值) — Phase Correlator with Normalized Cross-Power Spectrum and Sub-Sample Peak Interpolation
 *
 * 功能: 实现相位相关算法，支持归一化互功率谱计算、
 *       亚像素级峰值插值和二维图像配准。
 *
 * 协作: MixedRadixFFT5(混合基FFT) / ChirpZ6(Chirp-Z变换) / WinogradFFT5(Winograd FFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 相位相关(归一化互功率谱+亚像素插值)
 */
class PhaseCorrelator2 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalCorrelations = 0;
        int lastSize = 0;
        double lastPeakValue = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Sub-pixel shift result */
    struct ShiftResult {
        double dx = 0.0;
        double dy = 0.0;
        double peakValue = 0.0;
        double confidence = 0.0;
    };

    explicit PhaseCorrelator2(QObject *parent = nullptr);
    ~PhaseCorrelator2() override;

    void setSubpixelInterpolation(bool enable);
    void setWindowSize(int size);

    /** @brief Compute cross-power spectrum of two 1D signals */
    void crossPowerSpectrum(const QVector<double>& re1, const QVector<double>& im1,
                            const QVector<double>& re2, const QVector<double>& im2,
                            QVector<double>& outRe, QVector<double>& outIm) const;

    /** @brief 1D phase correlation, returns shift in samples */
    double correlate1D(const QVector<double>& signal1, const QVector<double>& signal2);

    /** @brief 2D phase correlation, returns sub-pixel shift */
    ShiftResult correlate2D(const QVector<QVector<double>>& img1,
                            const QVector<QVector<double>>& img2);

    /** @brief Find peak position with sub-pixel interpolation using Gaussian fit */
    ShiftResult interpolatePeak(const QVector<double>& correlation) const;

    /** @brief Apply Hanning window to reduce spectral leakage */
    QVector<double> applyHanning(const QVector<double>& signal) const;

    /** @brief Apply Hamming window */
    QVector<double> applyHamming(const QVector<double>& signal) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void correlationCompleted(double shift, double peak, double timeMs);

private:
    bool m_subpixel = true;
    int m_windowSize = 256;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Simple DFT for small sizes */
    void dft(const QVector<double>& inRe, const QVector<double>& inIm,
             QVector<double>& outRe, QVector<double>& outIm, bool inverse) const;

    /** @brief Find integer peak position */
    int findPeak(const QVector<double>& data) const;
};
