/**
 * @file PhaseCorrelator.h
 * @brief 相位相关亚像素图像对齐(频域互功率谱+亚像素精化) — Phase Correlation for Sub-pixel Image Alignment via Frequency-Domain Cross-Power Spectrum
 *
 * 功能: 实现相位相关算法进行亚像素级图像对齐，支持频域互功率谱计算、
 *       亚像素精化(高斯/余弦拟合)、多分辨率金字塔和旋转/尺度不变对齐。
 *
 * 协作: SplitRadixFFT4(FFT) / MixedRadixFFT4(混合基) / WindowFunction5(窗函数)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 相位相关亚像素对齐器
 */
class PhaseCorrelator : public QObject {
    Q_OBJECT

public:
    /** @brief Alignment result */
    struct AlignResult {
        double offsetX = 0.0;       ///< Sub-pixel X offset
        double offsetY = 0.0;       ///< Sub-pixel Y offset
        double peakValue = 0.0;     ///< Cross-power peak
        double confidence = 0.0;    ///< Correlation confidence [0,1]
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalAlignments = 0;
        int imageWidth = 0;
        int imageHeight = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit PhaseCorrelator(QObject *parent = nullptr);
    ~PhaseCorrelator() override;

    void setSubpixelRefinement(bool enabled);
    void setUseWindow(bool enabled);

    /** @brief 对齐两幅图像(2D灰度)，返回亚像素偏移 */
    AlignResult align(const QVector<QVector<double>>& ref,
                      const QVector<QVector<double>>& moving) const;

    /** @brief 1D phase correlation */
    double align1D(const QVector<double>& ref,
                   const QVector<double>& moving) const;

    /** @brief Compute cross-power spectrum */
    QVector<QVector<double>> crossPowerSpectrum(
        const QVector<QVector<double>>& reA,
        const QVector<QVector<double>>& imA,
        const QVector<QVector<double>>& reB,
        const QVector<QVector<double>>& imB) const;

    /** @brief Sub-pixel refinement via Gaussian fitting */
    QPair<double, double> gaussianRefinement(
        const QVector<QVector<double>>& corr, int peakX, int peakY) const;

    /** @brief Apply Hann window to reduce spectral leakage */
    void applyHannWindow(QVector<QVector<double>>& img) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void alignmentCompleted(double offsetX, double offsetY, double confidence);

private:
    bool m_subpixel = true;
    bool m_useWindow = true;

    mutable Stats m_stats;
    mutable double m_timeSum = 0.0;

    /** @brief Find integer peak in 2D correlation */
    QPair<int, int> findPeak(const QVector<QVector<double>>& corr) const;

    /** @brief 1D FFT (Cooley-Tukey, power-of-2) */
    void fft1D(QVector<double>& re, QVector<double>& im, bool inverse) const;

    /** @brief 2D FFT via row-column decomposition */
    void fft2D(QVector<QVector<double>>& re,
               QVector<QVector<double>>& im, bool inverse) const;

    /** @brief Pad to next power of 2 */
    int nextPow2(int n) const;

    /** @brief Compute log2 */
    int log2Int(int n) const;
};
