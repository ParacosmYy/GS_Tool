/**
 * @file SpectralGate3.h
 * @brief 频谱门控(Wiener滤波估计+最小均方误差去噪) — Spectral Gate with Wiener Filter Estimation and Minimum Mean-Square Error Denoising
 *
 * 功能: 实现频谱门控去噪，支持STFT分析、Wiener滤波器估计、
 *       MMSE增益计算和信号重建。
 *
 * 协作: WaveletDenoiser6(小波去噪) / WienerFilter5(Wiener滤波) / FFTW5(FFT引擎)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 频谱门控(Wiener滤波估计+最小均方误差去噪)
 */
class SpectralGate3 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int signalLength = 0;
        double inputSNR = 0.0;
        double outputSNR = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SpectralGate3(QObject *parent = nullptr);
    ~SpectralGate3() override;

    void setFrameSize(int size);
    void setHopSize(int hop);
    void setNoiseFloor(double floor);

    /** @brief Denoise signal using spectral gating with Wiener filter */
    QVector<double> denoise(const QVector<double>& signal);

    /** @brief Estimate noise spectrum from leading silence */
    QVector<double> estimateNoise(const QVector<double>& signal, int noiseFrames) const;

    /** @brief Compute Wiener gain for each frequency bin */
    QVector<double> wienerGain(const QVector<double>& signalPower,
                                const QVector<double>& noisePower) const;

    /** @brief MMSE short-time spectral amplitude estimator gain */
    QVector<double> mmseGain(const QVector<double>& signalPower,
                              const QVector<double>& noisePower,
                              double snrPrior) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void denoisingCompleted(int length, double inSNR, double outSNR, double timeMs);

private:
    int m_frameSize = 1024;
    int m_hopSize = 512;
    double m_noiseFloor = 1e-6;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Hann window */
    QVector<double> m_window;

    /** @brief Precompute window function */
    void precomputeWindow();

    /** @brief DFT of a real frame (magnitude + phase output) */
    void dftFrame(const QVector<double>& frame,
                   QVector<double>& magnitude, QVector<double>& phase) const;

    /** @brief Inverse DFT back to time domain */
    QVector<double> idftFrame(const QVector<double>& magnitude,
                                const QVector<double>& phase) const;

    /** @brief Compute signal power */
    static double signalPower(const QVector<double>& signal);
};
