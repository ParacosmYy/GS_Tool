/**
 * @file Gate4.h
 * @brief 噪声门(FFT频谱门控+时频掩码音乐噪声抑制) — Noise Gate with FFT-Based Spectral Gating and Musical Noise Suppression via Time-Frequency Masking
 *
 * 功能: 实现FFT频谱门控噪声抑制，支持自适应阈值估计、
 *       时频掩码平滑和音乐噪声抑制。
 *
 * 协作: NoiseProfile5(噪声估计) / SpectralSubtraction3(谱减法) / WienerFilter4(维纳滤波)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 噪声门(FFT频谱门控+时频掩码音乐噪声抑制)
 */
class Gate4 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalFrames = 0;
        int frameSize = 0;
        double avgProcessingTimeMs = 0.0;
        double noiseFloorDb = -60.0;
    };

    explicit Gate4(QObject *parent = nullptr);
    ~Gate4() override;

    void setFrameSize(int size);
    void setThreshold(double thresholdDb);
    void setAttack(double ms);
    void setRelease(double ms);
    void setReduction(double db);
    void setSmoothingFrames(int frames);

    /** @brief Process a frame of audio samples */
    QVector<double> process(const QVector<double>& frame);

    /** @brief Estimate noise floor from initial silence frames */
    void estimateNoiseFloor(const QVector<QVector<double>>& noiseFrames);

    /** @brief Compute FFT magnitude spectrum */
    QVector<double> magnitudeSpectrum(const QVector<double>& frame) const;

    /** @brief Apply spectral gate with time-frequency mask */
    QVector<double> applySpectralGate(const QVector<double>& spectrum,
                                       const QVector<double>& noiseEstimate) const;

    /** @brief Smooth mask to suppress musical noise */
    QVector<double> smoothMask(const QVector<double>& mask) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void frameProcessed(double rmsIn, double rmsOut, double timeMs);

private:
    int m_frameSize = 1024;
    double m_thresholdDb = -40.0;
    double m_attackMs = 1.0;
    double m_releaseMs = 50.0;
    double m_reductionDb = -60.0;
    int m_smoothingFrames = 3;

    QVector<double> m_noiseEstimate;
    QVector<double> m_prevMask;
    QVector<double> m_window;

    Stats m_stats;
    double m_timeSum = 0.0;
    double m_gain = 1.0;

    /** @brief Compute Hann window */
    QVector<double> hannWindow(int n) const;

    /** @brief Simple radix-2 FFT */
    void fft(QVector<double>& real, QVector<double>& imag) const;

    /** @brief Inverse FFT */
    void ifft(QVector<double>& real, QVector<double>& imag) const;

    /** @brief RMS energy in dB */
    static double rmsDb(const QVector<double>& samples);
};
