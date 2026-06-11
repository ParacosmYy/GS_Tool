/**
 * @file SignalGenerator10.h
 * @brief 信号发生器(任意波形合成与AM/FM/PM调制包络整形实现测试信号产生) — Signal Generator with Arbitrary Waveform Synthesis and AM/FM/PM Modulation with Envelope Shaping for Test Signal Production
 *
 * 功能: 实现信号发生器(signal generator)，采用任意波形合成(arbitrary waveform synthesis)
 *       与AM/FM/PM调制包络整形(AM/FM/PM modulation with envelope shaping)实现测试信号产生(test signal production)。
 *
 * 协作: Reverb10(混响) / SpectrumAnalyzer(频谱分析) / FIRFilter(FIR滤波器)
 */
#pragma once

#include <QObject>
#include <QVector>

class SignalGenerator10 : public QObject {
    Q_OBJECT

public:
    /** @brief Waveform type */
    enum WaveformType {
        Sine, Square, Triangle, Sawtooth, Pulse,
        WhiteNoise, PinkNoise, Arbitrary
    };

    /** @brief Modulation type */
    enum ModType { None, AM, FM, PM };

    /** @brief Generator configuration */
    struct Config {
        WaveformType waveform = Sine;
        double frequency = 1000.0;
        double amplitude = 1.0;
        double phase = 0.0;
        double dcOffset = 0.0;
        double dutyCycle = 0.5;
        ModType modulation = None;
        double modFrequency = 10.0;
        double modDepth = 0.5;
    };

    /** @brief Generation result */
    struct GenResult {
        QVector<double> samples;
        double peakAmplitude = 0.0;
        double rmsAmplitude = 0.0;
        int numSamples = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalGenerations = 0;
        int lastNumSamples = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SignalGenerator10(QObject *parent = nullptr);
    ~SignalGenerator10() override;

    void setSampleRate(double sr);
    void setConfig(const Config& config);

    /** @brief Set arbitrary waveform table */
    void setArbitraryWaveform(const QVector<double>& table);

    /** @brief Generate N samples */
    GenResult generate(int numSamples);

    /** @brief Generate with frequency sweep (chirp) */
    GenResult generateChirp(int numSamples,
                             double startFreq, double endFreq);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void generateDone(int n, double peak, double timeMs);

private:
    double m_sampleRate = 44100.0;
    Config m_config;
    QVector<double> m_arbTable;
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_phaseAccum = 0.0;

    /** @brief Compute one sample of the base waveform at given phase */
    double baseWaveform(double phase) const;

    /** @brief Apply modulation to sample */
    double applyModulation(double sample, int sampleIndex) const;

    /** @brief Generate white noise sample */
    double whiteNoise() const;

    /** @brief Generate pink noise sample (Voss-McCartney) */
    double pinkNoise();

    /** @brief Pink noise state */
    QVector<double> m_pinkRows;
    int m_pinkIndex = 0;
    double m_pinkRunningSum = 0.0;
};
