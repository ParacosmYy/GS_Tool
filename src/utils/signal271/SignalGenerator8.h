/**
 * @file SignalGenerator8.h
 * @brief 信号发生器(任意波形合成与DDS相位累加器精确频率控制) — Signal Generator with Arbitrary Waveform Synthesis and DDS Phase Accumulator for Precise Frequency Control
 *
 * 功能: 实现信号发生器(Signal generator)，采用任意波形合成(arbitrary waveform synthesis)
 *       与DDS相位累加器(DDS phase accumulator)实现精确频率控制(precise frequency control)。
 *
 * 协作: Reverb8(混响器) / FIRFilter7(FIR滤波器) / FFT10(快速傅里叶变换)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 信号发生器(任意波形合成与DDS相位累加器精确频率控制)
 */
class SignalGenerator8 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numSamples = 0;
        double sampleRate = 0.0;
        double frequency = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Standard waveform types */
    enum WaveformType {
        Sine = 0,
        Square,
        Triangle,
        Sawtooth,
        Arbitrary
    };

    explicit SignalGenerator8(QObject *parent = nullptr);
    ~SignalGenerator8() override;

    /** @brief Set sample rate in Hz */
    void setSampleRate(double rate);

    /** @brief Set output frequency in Hz */
    void setFrequency(double freq);

    /** @brief Set peak amplitude */
    void setAmplitude(double amp);

    /** @brief Set DC offset */
    void setOffset(double offset);

    /** @brief Set phase accumulator bit width for DDS precision */
    void setPhaseBits(int bits);

    /** @brief Set waveform type */
    void setWaveform(WaveformType type);

    /** @brief Set arbitrary waveform table (normalized -1..1) */
    void setArbitraryWaveform(const QVector<double>& table);

    /** @brief Generate N samples */
    QVector<double> generate(int numSamples);

    /** @brief Generate a chirp signal (frequency sweep) */
    QVector<double> chirp(double fStart, double fEnd, int numSamples);

    /** @brief Reset phase accumulator */
    void resetPhase();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void generationCompleted(int numSamples, double frequency, double timeMs);

private:
    double m_sampleRate = 44100.0;
    double m_frequency = 440.0;
    double m_amplitude = 1.0;
    double m_offset = 0.0;
    int m_phaseBits = 32;
    WaveformType m_waveform = Sine;

    QVector<double> m_arbTable;
    quint64 m_phaseAcc = 0;     // DDS phase accumulator
    quint64 m_phaseInc = 0;     // phase increment per sample

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Recompute phase increment from frequency and sample rate */
    void updatePhaseIncrement();

    /** @brief Generate one sample from current phase */
    double sampleFromPhase(quint64 phase) const;
};
