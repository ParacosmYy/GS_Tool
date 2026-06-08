/**
 * @file HexFFT6.h
 * @brief 六角FFT(6点蝶形核+radix-6 Cooley-Tukey分解) — Hex FFT with 6-Point Butterfly Kernel and Radix-6 Cooley-Tukey Decomposition for Composite Lengths
 *
 * 功能: 实现基于6点蝶形核的FFT算法，采用radix-6 Cooley-Tukey分解，
 *       支持长度为6的幂次复合长度，提供前向/逆向变换和频谱分析。
 *
 * 协作: NumberTheoreticTransform3(数论变换) / WinogradFFT4(Winograd FFT) / SlidingDFT6(滑动DFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 六角FFT(6点蝶形核+radix-6分解)
 */
class HexFFT6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalTransforms = 0;
        int transformSize = 0;
        int radix = 6;
        int stages = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit HexFFT6(QObject *parent = nullptr);
    ~HexFFT6() override;

    /** @brief Set transform size (must be power of 6) */
    void setParameters(int size);

    /** @brief Forward transform, returns complex [re,im,re,im,...] */
    QVector<double> forward(const QVector<double>& realInput);

    /** @brief Inverse transform from complex interleaved */
    QVector<double> inverse(const QVector<double>& complexInterleaved);

    /** @brief Compute magnitude spectrum */
    QVector<double> magnitudeSpectrum(const QVector<double>& realInput);

    /** @brief Compute power spectrum in dB */
    QVector<double> powerSpectrumDb(const QVector<double>& realInput);

    /** @brief 6-point DFT butterfly kernel */
    void butterfly6(double& r0, double& i0, double& r1, double& i1,
                     double& r2, double& i2, double& r3, double& i3,
                     double& r4, double& i4, double& r5, double& i5) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int size, int stages, double timeMs);

private:
    int m_size = 216;    // 6^3
    int m_stages = 3;

    // Precomputed twiddle factors [re,im] pairs
    QVector<double> m_twiddles;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Precompute all twiddle factors for radix-6 stages */
    void computeTwiddles();

    /** @brief Radix-6 Cooley-Tukey decomposition */
    void radix6CT(QVector<double>& re, QVector<double>& im, bool inverse);

    /** @brief Digit-reverse permutation for radix-6 */
    void digitReverse(QVector<double>& re, QVector<double>& im);

    /** @brief Apply twiddle factor multiply */
    void applyTwiddles(QVector<double>& re, QVector<double>& im,
                        int stage, int group, int offset);
};
