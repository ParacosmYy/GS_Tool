/**
 * @file SignalGenerator5.h
 * @brief 信号发生器(加法合成Fourier部分波+振幅/相位包络脚本) — Signal Generator with Additive Synthesis via Fourier Partials and Amplitude/Phase Envelope Scripting
 *
 * 功能: 实现加法合成(additive synthesis)信号发生器，通过Fourier部分波(partial)
 *       叠加生成波形，支持振幅/相位包络(envelope)脚本化控制。
 *
 * 协作: SignalGenerator4(基础信号) / Reverb4(混响) / IIRFilter3(滤波器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 信号发生器(加法合成Fourier部分波+包络脚本)
 */
class SignalGenerator5 : public QObject {
    Q_OBJECT

public:
    /** @brief Single Fourier partial descriptor */
    struct Partial {
        double frequency = 440.0;    // Hz
        double amplitude = 1.0;      // relative amplitude
        double phase = 0.0;          // initial phase in radians
    };

    /** @brief Envelope breakpoint (ADSR-like) */
    struct EnvelopePoint {
        double time = 0.0;           // seconds
        double value = 1.0;          // amplitude multiplier
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPartials = 0;
        int numSamples = 0;
        double sampleRate = 44100.0;
        double duration = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SignalGenerator5(QObject *parent = nullptr);
    ~SignalGenerator5() override;

    /** @brief Set sample rate */
    void setSampleRate(double sampleRate);

    /** @brief Set Fourier partials for additive synthesis */
    void setPartials(const QVector<Partial>& partials);

    /** @brief Set amplitude envelope breakpoints */
    void setAmplitudeEnvelope(const QVector<EnvelopePoint>& points);

    /** @brief Set phase envelope breakpoints */
    void setPhaseEnvelope(const QVector<EnvelopePoint>& points);

    /** @brief Generate waveform for given duration (seconds) */
    QVector<double> generate(double duration);

    /** @brief Generate N samples */
    QVector<double> generateSamples(int numSamples);

    /** @brief Add a single partial */
    void addPartial(double freq, double amp, double phase = 0.0);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void generationCompleted(int samples, double duration, double timeMs);

private:
    double m_sampleRate = 44100.0;
    QVector<Partial> m_partials;
    QVector<EnvelopePoint> m_ampEnvelope;
    QVector<EnvelopePoint> m_phaseEnvelope;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Interpolate envelope at given time */
    double interpolateEnvelope(const QVector<EnvelopePoint>& env,
                               double t) const;

    /** @brief Evaluate all partials at time t with envelopes */
    double evaluateAt(double t) const;
};
