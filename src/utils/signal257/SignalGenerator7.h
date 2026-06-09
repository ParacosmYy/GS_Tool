/**
 * @file SignalGenerator7.h
 * @brief 信号发生器(AM/FM/PM调制+预定义波形插值变形) — Signal Generator with AM/FM/PM Modulation and Waveform Morphing via Interpolation Between Predefined Shapes
 *
 * 功能: 实现信号发生器(Signal Generator)，支持AM/FM/PM调制(AM/FM/PM
 *       modulation)和波形变形(waveform morphing)，通过预定义波形之间的
 *       插值(interpolation)实现平滑的波形形状过渡。
 *
 * 协作: SignalGenerator6(基础信号) / Modulator5(调制器) / Oscillator8(振荡器)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief 信号发生器(AM/FM/PM调制+波形插值变形)
 */
class SignalGenerator7 : public QObject {
    Q_OBJECT

public:
    /** @brief Waveform shape */
    enum Shape { Sine = 0, Square = 1, Triangle = 2, Sawtooth = 3, Noise = 4 };

    /** @brief Modulation type */
    enum ModType { None = 0, AM = 1, FM = 2, PM = 3 };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numSamplesGenerated = 0;
        int sampleRate = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SignalGenerator7(QObject *parent = nullptr);
    ~SignalGenerator7() override;

    /** @brief Set sample rate */
    void setSampleRate(int sr);

    /** @brief Set base frequency in Hz */
    void setFrequency(double freq);

    /** @brief Set amplitude (0.0..1.0) */
    void setAmplitude(double amp);

    /** @brief Set waveform shape */
    void setShape(Shape shape);

    /** @brief Set modulation type and parameters */
    void setModulation(ModType type, double modFreq, double modDepth);

    /** @brief Generate numSamples of the configured waveform */
    QVector<double> generate(int numSamples);

    /** @brief Generate morphed waveform between two shapes (alpha 0..1) */
    QVector<double> generateMorphed(int numSamples, Shape from, Shape to,
                                    double alpha);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void generationCompleted(int numSamples, double timeMs);

private:
    int m_sampleRate = 44100;
    double m_frequency = 440.0;
    double m_amplitude = 1.0;
    Shape m_shape = Sine;

    ModType m_modType = None;
    double m_modFreq = 5.0;
    double m_modDepth = 0.5;

    double m_phase = 0.0;       // Running phase accumulator

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Generate one period of a waveform shape */
    double sampleShape(Shape shape, double phase) const;

    /** @brief Apply modulation to a sample */
    double applyModulation(double sample, double t) const;

    /** @brief Generate noise sample (uniform random) */
    double noiseSample() const;
};
