/**
 * @file WaveletDenoiser12.h
 * @brief 小波去噪(SURE阈值选择+循环旋转的平移不变去噪) — Wavelet Denoiser with SURE-Based Threshold Selection and Cycle-Spinning for Translation-Invariant Denoising
 *
 * 功能: 实现小波去噪(Wavelet Denoiser)，使用SURE(Stein's Unbiased Risk
 *       Estimate)阈值选择(threshold selection)自动确定最优阈值，通过循环
 *       旋转(cycle-spinning)实现平移不变去噪(translation-invariant denoising)。
 *
 * 协作: MixedRadixFFT8(混合基FFT) / Deesser7(去齿音) / FIRFilter6(FIR滤波器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 小波去噪(SURE阈值选择+循环旋转的平移不变去噪)
 */
class WaveletDenoiser12 : public QObject {
    Q_OBJECT

public:
    /** @brief Wavelet type */
    enum Wavelet { Haar, DB2, DB4 };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int signalLength = 0;
        int decomposeLevels = 0;
        double threshold = 0.0;
        double inputSNR = 0.0;
        double outputSNR = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit WaveletDenoiser12(QObject *parent = nullptr);
    ~WaveletDenoiser12() override;

    /** @brief Set wavelet type */
    void setWavelet(Wavelet w);

    /** @brief Set decomposition levels */
    void setLevels(int levels);

    /** @brief Set number of cycle-spinning shifts */
    void setCycleSpins(int spins);

    /** @brief Denoise signal using SURE threshold + cycle-spinning */
    QVector<double> denoise(const QVector<double>& signal);

    /** @brief Multi-level wavelet decompose */
    QVector<QVector<double>> decompose(const QVector<double>& signal) const;

    /** @brief Multi-level wavelet reconstruct */
    QVector<double> reconstruct(const QVector<QVector<double>>& coeffs) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void denoisingCompleted(int length, double threshold, double timeMs);

private:
    Wavelet m_wavelet = Haar;
    int m_levels = 4;
    int m_spins = 8;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Get wavelet filter coefficients */
    QVector<double> lowDecompFilter() const;
    QVector<double> highDecompFilter() const;
    QVector<double> lowReconFilter() const;
    QVector<double> highReconFilter() const;

    /** @brief Circular convolution */
    static QVector<double> circConv(const QVector<double>& x,
                                    const QVector<double>& h);

    /** @brief Downsample by 2 */
    static QVector<double> downsample(const QVector<double>& x);

    /** @brief Upsample by 2 */
    static QVector<double> upsample(const QVector<double>& x, int targetLen);

    /** @brief SURE-based threshold estimation */
    double sureThreshold(const QVector<double>& coeffs) const;

    /** @brief Soft thresholding */
    static QVector<double> softThreshold(const QVector<double>& x, double t);

    /** @brief Circular shift */
    static QVector<double> circShift(const QVector<double>& x, int shift);

    /** @brief Estimate signal SNR in dB */
    static double estimateSNR(const QVector<double>& clean,
                              const QVector<double>& noisy);
};
