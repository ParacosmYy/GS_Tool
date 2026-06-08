/**
 * @file SignalGenerator4.h
 * @brief 信号发生器(带限多项式波形整形+AM/FM调制任意波形合成) — Signal Generator with Arbitrary Waveform Synthesis via Band-Limited Polynomial Waveshaper and AM/FM Modulation
 *
 * 功能: 实现信号发生器，支持多项式波形整形、
 *       AM/FM调制和带限抗混叠合成。
 *
 * 协作: Reverb3(混响) / BiquadFilter3(双二阶滤波器) / SpectrumAnalyzer2(频谱分析)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 信号发生器(带限多项式整形+AM/FM调制)
 */
class SignalGenerator4 : public QObject {
    Q_OBJECT

public:
    /** @brief Waveform type */
    enum Waveform {
        Sine = 0,
        Square,
        Sawtooth,
        Triangle,
        Pulse,
        Arbitrary
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalGenerated = 0;
        double sampleRate = 44100.0;
        double frequency = 440.0;
        double amplitude = 1.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SignalGenerator4(QObject *parent = nullptr);
    ~SignalGenerator4() override;

    /** @brief Set sample rate */
    void setSampleRate(double sampleRate);

    /** @brief Set waveform parameters */
    void setWaveform(Waveform type, double frequency, double amplitude = 1.0);

    /** @brief Set AM modulation parameters */
    void setAM(double modDepth, double modFrequency);

    /** @brief Set FM modulation parameters */
    void setFM(double modIndex, double modFrequency);

    /** @brief Set arbitrary waveform shape (one cycle) */
    void setArbitraryWaveform(const QVector<double>& shape);

    /** @brief Generate a single sample */
    double generateOne();

    /** @brief Generate N samples */
    QVector<double> generate(int numSamples);

    /** @brief Band-limited polynomial waveshaper for anti-aliased synthesis */
    double polyBlep(double t, double phaseIncrement) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void generationCompleted(int samples, double timeMs);

private:
    double m_sampleRate = 44100.0;
    double m_frequency = 440.0;
    double m_amplitude = 1.0;
    Waveform m_waveform = Sine;

    // AM parameters
    double m_amDepth = 0.0;
    double m_amFreq = 0.0;

    // FM parameters
    double m_fmIndex = 0.0;
    double m_fmFreq = 0.0;

    // Phase accumulator
    double m_phase = 0.0;

    // Arbitrary waveform table
    QVector<double> m_arbTable;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute raw waveform value at phase [0,1) */
    double rawWaveform(double phase) const;

    /** @brief Advance phase and return phase increment */
    double advancePhase();

    /** @brief Wrap phase to [0, 1) */
    static double wrapPhase(double phase);
};
