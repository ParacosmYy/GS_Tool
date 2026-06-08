/**
 * @file Resampler6.h
 * @brief 重采样器(有理P/Q多相FIR+CIC预滤波抗混叠异步转换) — Resampler with Rational P/Q Polyphase FIR and CIC Pre-filter for Alias-free Asynchronous Conversion
 *
 * 功能: 实现任意比率重采样，采用有理P/Q多相FIR滤波器，
 *       集成CIC预滤波器抑制混叠，支持异步转换。
 *
 * 协作: Flanger4(镶边) / Chorus3(合唱) / FIRFilter5(FIR滤波)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 重采样器(多相FIR+CIC预滤波)
 */
class Resampler6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int inputRate = 44100;
        int outputRate = 44100;
        int upFactor = 1;
        int downFactor = 1;
        int filterTaps = 0;
        int cicOrder = 0;
        quint64 inputSamples = 0;
        quint64 outputSamples = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Resampler6(QObject *parent = nullptr);
    ~Resampler6() override;

    /** @brief Configure resampler with input/output rates and optional filter parameters */
    bool configure(int inputRate, int outputRate, int numTaps = 64, int cicOrder = 4);

    /** @brief Process a block of samples, returns resampled output */
    QVector<double> process(const QVector<double>& input);

    /** @brief Flush remaining buffered samples */
    QVector<double> flush();

    /** @brief Get P/Q ratio */
    int upFactor() const { return m_up; }
    int downFactor() const { return m_down; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void resamplingCompleted(int inSamples, int outSamples, double timeMs);

private:
    int m_up = 1;
    int m_down = 1;
    int m_numTaps = 64;
    int m_cicOrder = 4;
    int m_inputRate = 44100;
    int m_outputRate = 44100;

    // Polyphase FIR filter coefficients [phase][tap]
    QVector<QVector<double>> m_polyFilter;

    // CIC pre-filter state (integrator + comb)
    QVector<double> m_integratorState;
    double m_combState = 0.0;
    int m_cicPhase = 0;

    // FIR delay line
    QVector<double> m_delayLine;
    int m_delayPos = 0;

    // Polyphase state
    double m_fracPhase = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute greatest common divisor */
    static int gcd(int a, int b);

    /** @brief Design polyphase FIR filter for P/Q resampling */
    void designPolyphaseFilter();

    /** @brief Design CIC pre-filter coefficients */
    void designCICFilter();

    /** @brief Apply CIC pre-filter to a single sample */
    double applyCIC(double sample);

    /** @brief Apply polyphase FIR branch */
    double applyPolyphase(int phase, double newSample);
};
