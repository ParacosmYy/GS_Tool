/**
 * @file WaveletDenoiser7.h
 * @brief 小波降噪(循环平移平移不变阈值+BayesShrink自适应层级) — Wavelet Denoiser with Cycle-Spinning Translation-Invariant Thresholding and BayesShrink Adaptive Level
 *
 * 功能: 实现小波降噪算法，支持循环平移平移不变阈值处理、
 *       BayesShrink自适应层级阈值估计和多级小波分解/重构。
 *
 * 协作: SpectralGate4(频谱门控) / WienerFilter3(Wiener滤波) / FftEngine1(FFT引擎)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 小波降噪(循环平移平移不变阈值+BayesShrink自适应层级)
 */
class WaveletDenoiser7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int signalLength = 0;
        int numLevels = 0;
        double inputSNR = 0.0;
        double outputSNR = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit WaveletDenoiser7(QObject *parent = nullptr);
    ~WaveletDenoiser7() override;

    void setWaveletType(int type);
    void setDecompositionLevels(int levels);
    void setNumCycleSpins(int spins);
    void setThresholdMode(int mode);

    /** @brief Forward wavelet transform (Haar or D4) */
    QVector<QVector<double>> forwardWT(const QVector<double>& signal) const;

    /** @brief Inverse wavelet transform */
    QVector<double> inverseWT(const QVector<QVector<double>>& coeffs) const;

    /** @brief Estimate BayesShrink threshold for a detail level */
    double bayesShrinkThreshold(const QVector<double>& detailCoeffs) const;

    /** @brief Soft thresholding */
    static QVector<double> softThreshold(const QVector<double>& coeffs, double t);

    /** @brief Hard thresholding */
    static QVector<double> hardThreshold(const QVector<double>& coeffs, double t);

    /** @brief Denoise with cycle-spinning TI */
    QVector<double> denoise(const QVector<double>& signal);

    /** @brief Estimate noise standard deviation (MAD of finest detail) */
    double estimateNoiseStd(const QVector<double>& detailCoeffs) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void denoisingCompleted(int levels, int spins, double timeMs);

private:
    int m_waveletType = 0;   // 0=Haar, 1=D4(Daubechies-4)
    int m_numLevels = 4;
    int m_numSpins = 8;
    int m_thresholdMode = 0; // 0=Soft, 1=Hard

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Haar forward step */
    static void haarForward(QVector<double>& approx, QVector<double>& detail,
                             const QVector<double>& input);

    /** @brief Haar inverse step */
    static void haarInverse(QVector<double>& output,
                             const QVector<double>& approx,
                             const QVector<double>& detail);

    /** @brief D4 forward step */
    static void d4Forward(QVector<double>& approx, QVector<double>& detail,
                           const QVector<double>& input);

    /** @brief D4 inverse step */
    static void d4Inverse(QVector<double>& output,
                           const QVector<double>& approx,
                           const QVector<double>& detail);

    /** @brief Cycle-shift signal by offset */
    static QVector<double> cycleShift(const QVector<double>& signal, int offset);

    /** @brief Cycle-unshift signal by offset */
    static QVector<double> cycleUnshift(const QVector<double>& signal, int offset);
};
