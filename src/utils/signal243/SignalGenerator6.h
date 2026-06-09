/**
 * @file SignalGenerator6.h
 * @brief 信号发生器(任意波形定义+多音合成可配置相位偏移) — Signal Generator with Arbitrary Waveform Definition and Multi-Tone Synthesis with Configurable Phase Offsets
 *
 * 功能: 实现信号发生器(Signal generator)，支持任意波形定义(arbitrary waveform definition)
 *       通过用户自定义采样点描述波形形状，提供多音合成(multi-tone synthesis)叠加多个
 *       正弦分量并配置各分量的独立相位偏移(phase offsets)，实现灵活信号生成。
 *
 * 协作: Reverb5(混响) / IIRFilter3(IIR滤波器) / WindowFunction4(窗函数)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 信号发生器(任意波形定义+多音合成可配置相位偏移)
 */
class SignalGenerator6 : public QObject {
    Q_OBJECT

public:
    /** @brief Single tone component */
    struct Tone {
        double frequency = 440.0;
        double amplitude = 1.0;
        double phaseOffset = 0.0;
    };

    /** @brief Waveform type enumeration */
    enum WaveformType {
        Sine = 0,
        Square = 1,
        Triangle = 2,
        Sawtooth = 3,
        Arbitrary = 4
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int sampleRate = 0;
        int numTones = 0;
        double durationMs = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SignalGenerator6(QObject *parent = nullptr);
    ~SignalGenerator6() override;

    /** @brief Set sample rate (Hz) */
    void setSampleRate(int rate);

    /** @brief Set single tone parameters */
    void setTone(const Tone& tone);

    /** @brief Set multiple tones for multi-tone synthesis */
    void setTones(const QVector<Tone>& tones);

    /** @brief Set arbitrary waveform shape (one cycle) */
    void setArbitraryWaveform(const QVector<double>& shape);

    /** @brief Generate samples for given duration (ms) */
    QVector<double> generate(double durationMs);

    /** @brief Generate N samples with current config */
    QVector<double> generateSamples(int numSamples);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void generationCompleted(int numSamples, double durationMs, double timeMs);

private:
    int m_sampleRate = 44100;
    QVector<Tone> m_tones;
    QVector<double> m_arbWaveform;
    WaveformType m_type = Sine;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Evaluate a single waveform at phase t (0..1) */
    double evalWaveform(double t) const;

    /** @brief Evaluate arbitrary waveform via linear interpolation */
    double evalArbitrary(double t) const;
};
