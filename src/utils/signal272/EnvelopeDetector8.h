/**
 * @file EnvelopeDetector8.h
 * @brief 包络检测器(Hilbert变换解析信号与峰值保持衰减瞬时幅度跟踪) — Envelope Detector with Hilbert Transform Analytic Signal and Peak-Hold Decay for Instantaneous Amplitude Tracking
 *
 * 功能: 实现包络检测器(Envelope detector)，采用Hilbert变换解析信号(Hilbert transform analytic signal)
 *       与峰值保持衰减(peak-hold decay)实现瞬时幅度跟踪(instantaneous amplitude tracking)。
 *
 * 协作: Limiter11(限幅器) / Compressor10(压缩器) / AMModulator5(AM调制器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 包络检测器(Hilbert变换解析信号与峰值保持衰减瞬时幅度跟踪)
 */
class EnvelopeDetector8 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numSamples = 0;
        double peakEnvelope = 0.0;
        double rmsEnvelope = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Detection mode */
    enum Mode {
        HilbertAnalytic = 0,  // Hilbert transform analytic signal
        PeakHoldDecay,        // Peak-hold with exponential decay
        RectifySmooth         // Full-wave rectification + LPF
    };

    explicit EnvelopeDetector8(QObject *parent = nullptr);
    ~EnvelopeDetector8() override;

    /** @brief Set detection mode */
    void setMode(Mode mode);

    /** @brief Set attack time constant (ms) for peak-hold mode */
    void setAttackMs(double ms);

    /** @brief Set release/decay time constant (ms) */
    void setReleaseMs(double ms);

    /** @brief Set sample rate */
    void setSampleRate(double rate);

    /** @brief Set FIR filter length for Hilbert transform */
    void setFilterLength(int len);

    /** @brief Process input and return envelope */
    QVector<double> process(const QVector<double>& input);

    /** @brief Get instantaneous envelope */
    QVector<double> envelope() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void detectionCompleted(int samples, double peak, double timeMs);

private:
    Mode m_mode = HilbertAnalytic;
    double m_attackMs = 0.1;
    double m_releaseMs = 50.0;
    double m_sampleRate = 44100.0;
    int m_filterLen = 65;

    QVector<double> m_envelope;
    double m_prevEnv = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Hilbert transform via FIR filter */
    QVector<double> hilbertTransform(const QVector<double>& input) const;

    /** @brief Design Hilbert FIR coefficients */
    QVector<double> hilbertCoeffs() const;

    /** @brief Peak-hold with exponential decay */
    QVector<double> peakHoldDecay(const QVector<double>& input) const;

    /** @brief Full-wave rectification + smoothing */
    QVector<double> rectifySmooth(const QVector<double>& input) const;

    /** @brief Apply FIR filter */
    QVector<double> applyFIR(const QVector<double>& input,
                              const QVector<double>& coeffs) const;
};
