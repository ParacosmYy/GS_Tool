/**
 * @file WaveletDenoiser6.h
 * @brief 小波去噪(非抽取提升格式+平移不变循环旋转) — Wavelet Denoiser with Non-Decimated Lifting Scheme and Shift-Invariant Cycle-Spinning
 *
 * 功能: 实现非抽取小波去噪，支持提升格式小波变换、
 *       平移不变循环旋转和多尺度阈值去噪。
 *
 * 协作: SpectralGate3(频谱门控) / WaveletDenoiser5(小波去噪) / FFTW5(FFT引擎)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 小波去噪(非抽取提升格式+平移不变循环旋转)
 */
class WaveletDenoiser6 : public QObject {
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

    explicit WaveletDenoiser6(QObject *parent = nullptr);
    ~WaveletDenoiser6() override;

    void setWavelet(const QString& name);
    void setDecompositionLevels(int levels);
    void setThresholdMethod(const QString& method);
    void setNumSpins(int spins);

    /** @brief Denoise signal using non-decimated wavelet with cycle-spinning */
    QVector<double> denoise(const QVector<double>& signal);

    /** @brief Non-decimated lifting wavelet forward transform */
    QVector<QVector<double>> forwardLifting(const QVector<double>& signal) const;

    /** @brief Non-decimated lifting wavelet inverse transform */
    QVector<double> inverseLifting(const QVector<QVector<double>>& coefficients) const;

    /** @brief Apply soft threshold to wavelet coefficients */
    QVector<double> softThreshold(const QVector<double>& coeffs, double threshold) const;

    /** @brief Compute universal threshold (VisuShrink) */
    double universalThreshold(const QVector<double>& coeffs) const;

    /** @brief Cycle-spinning: shift, denoise, unshift, average */
    QVector<double> cycleSpinDenoise(const QVector<double>& signal) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void denoisingCompleted(int length, double inSNR, double outSNR, double timeMs);

private:
    QString m_wavelet = "db2";
    int m_levels = 4;
    QString m_thresholdMethod = "visushrink";
    int m_numSpins = 8;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Predict step of lifting scheme */
    static void predictStep(QVector<double>& detail, const QVector<double>& smooth);

    /** @brief Update step of lifting scheme */
    static void updateStep(QVector<double>& smooth, const QVector<double>& detail);

    /** @brief Circular shift of signal by offset samples */
    static QVector<double> circularShift(const QVector<double>& signal, int offset);

    /** @brief Compute signal power for SNR estimation */
    static double computePower(const QVector<double>& signal);
};
