/**
 * @file SpectralGate8.h
 * @brief 频谱门(Wiener滤波器与噪声估计跟踪和频谱下限的音乐噪声伪影抑制) — Spectral Gate with Wiener Filter, Noise Estimate Tracking and Spectral Floor for Musical Noise Artifact Suppression
 *
 * 功能: 实现频谱门(spectral gate)，采用Wiener滤波器(Wiener filter)与噪声估计跟踪(noise estimate tracking)
 *       和频谱下限(spectral floor)实现音乐噪声伪影抑制(musical noise artifact suppression)。
 *
 * 协作: LMSFilter7(LMS自适应滤波) / KalmanFilter9(Kalman滤波) / WienerFilter6(Wiener滤波)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 频谱门(Wiener滤波器与噪声估计跟踪)
 */
class SpectralGate8 : public QObject {
    Q_OBJECT

public:
    /** @brief Gate result for one frame */
    struct GateResult {
        QVector<double> cleanSignal;
        QVector<double> spectrum;
        QVector<double> noiseEstimate;
        QVector<double> gainCurve;
        double snr = 0.0;
        double musicalNoiseIndex = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int frameSize = 0;
        double avgProcessingTimeMs = 0.0;
        double avgSnr = 0.0;
    };

    explicit SpectralGate8(QObject *parent = nullptr);
    ~SpectralGate8() override;

    /** @brief Set FFT frame size (power of 2) */
    void setFrameSize(int size);

    /** @brief Set spectral floor level (0.0 to 1.0) */
    void setSpectralFloor(double floor);

    /** @brief Set noise estimation rate */
    void setNoiseEstimationRate(double rate);

    /** @brief Set oversubtraction factor */
    void setOversubtractionFactor(double factor);

    /** @brief Process one frame of noisy signal */
    GateResult process(const QVector<double>& frame);

    /** @brief Process entire signal frame by frame */
    QVector<GateResult> processSignal(const QVector<double>& signal);

    /** @brief Reset noise estimate */
    void resetNoiseEstimate();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void frameProcessed(int frameIdx, double snr, double timeMs);
    void noiseEstimateUpdated(double noisePower);

private:
    int m_frameSize = 512;
    double m_spectralFloor = 0.02;
    double m_noiseRate = 0.98;
    double m_oversubtraction = 1.5;
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_snrSum = 0.0;

    /** @brief Noise power estimate per bin */
    QVector<double> m_noiseEstimate;
    /** @brief Previous frame magnitude for tracking */
    QVector<double> m_prevMagnitude;
    /** @brief Wiener gain for spectral floor smoothing */
    QVector<double> m_prevGain;
    /** @brief Number of frames processed */
    int m_frameCount = 0;

    /** @brief Compute FFT (simple radix-2) */
    void fft(QVector<double>& real, QVector<double>& imag) const;

    /** @brief Compute inverse FFT */
    void ifft(QVector<double>& real, QVector<double>& imag) const;

    /** @brief Update noise estimate using minimum statistics */
    void updateNoiseEstimate(const QVector<double>& magnitude);

    /** @brief Compute Wiener gain with spectral floor */
    QVector<double> computeWienerGain(const QVector<double>& magnitude) const;

    /** @brief Compute musical noise index for quality monitoring */
    double computeMusicalNoiseIndex(const QVector<double>& gain) const;

    /** @brief Apply half-wave rectification for window */
    QVector<double> applyWindow(const QVector<double>& frame) const;
};
