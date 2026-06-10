/**
 * @file NoiseGate9.h
 * @brief 噪声门(双阈值迟滞与STFT频谱门控频率选择性噪声抑制) — Noise Gate with Dual-threshold Hysteresis and Spectral Gating via STFT for Frequency-selective Noise Suppression
 *
 * 功能: 实现噪声门(Noise gate)，采用双阈值迟滞(dual-threshold hysteresis)
 *       与STFT频谱门控(spectral gating via STFT)实现频率选择性噪声抑制
 *       (frequency-selective noise suppression)。
 *
 * 协作: BiquadFilter12(双二阶滤波) / AdaptiveFilter11(自适应滤波) / SpectralSubtraction9(谱减)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 噪声门(双阈值迟滞与STFT频谱门控)
 */
class NoiseGate9 : public QObject {
    Q_OBJECT

public:
    /** @brief Gate state for hysteresis */
    enum GateState { Closed = 0, Opening, Open, Closing };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int frameSize = 0;
        int numFrames = 0;
        double openRatio = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit NoiseGate9(QObject *parent = nullptr);
    ~NoiseGate9() override;

    /** @brief Set open threshold (above = signal passes) */
    void setOpenThreshold(double db);

    /** @brief Set close threshold (below = gate closes, must be < open) */
    void setCloseThreshold(double db);

    /** @brief Set attack time in milliseconds */
    void setAttackTime(double ms);

    /** @brief Set release time in milliseconds */
    void setReleaseTime(double ms);

    /** @brief Set STFT FFT size for spectral gating (0 = time-domain only) */
    void setFFTSize(int size);

    /** @brief Set spectral floor in dB for frequency bins below threshold */
    void setSpectralFloor(double db);

    /** @brief Process time-domain signal with dual-threshold hysteresis gate */
    QVector<double> processTimeDomain(const QVector<double>& input, double sampleRate);

    /** @brief Process signal with STFT spectral gating */
    QVector<double> processSpectral(const QVector<double>& input, double sampleRate);

    /** @brief Estimate noise profile from silence segment */
    void estimateNoiseProfile(const QVector<double>& noiseSegment);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingDone(int numFrames, double openRatio, double timeMs);

private:
    double m_openThresholdDb = -30.0;
    double m_closeThresholdDb = -40.0;
    double m_attackMs = 1.0;
    double m_releaseMs = 50.0;
    int m_fftSize = 0;           // 0 = time-domain only
    double m_spectralFloorDb = -60.0;

    GateState m_state = Closed;
    double m_gain = 0.0;         // Current envelope gain [0,1]

    // Noise profile for spectral gating
    QVector<double> m_noisePower;  // Per-frequency-bin noise power estimate

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute frame RMS level in dB */
    double frameRmsDb(const QVector<double>& frame) const;

    /** @brief Compute STFT of input using half-overlapped windows */
    void computeSTFT(const QVector<double>& input, int hopSize,
                     QVector<QVector<double>>& magnitude,
                     QVector<QVector<double>>& phase) const;

    /** @brief Reconstruct signal from magnitude/phase via overlap-add */
    QVector<double> inverseSTFT(const QVector<QVector<double>>& magnitude,
                                const QVector<QVector<double>>& phase,
                                int hopSize, int totalSamples) const;
};
