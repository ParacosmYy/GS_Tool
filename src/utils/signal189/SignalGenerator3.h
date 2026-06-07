/**
 * @file SignalGenerator3.h
 * @brief 信号发生器(正弦/方波/三角/锯齿+AM/FM调制+扫频) — Signal Generator Producing Sine/Square/Triangle/Sawtooth with AM/FM Modulation and Frequency Sweep
 *
 * 功能: 实现多功能信号发生器，支持4种基本波形、AM/FM调制、
 *       线性/对数扫频和参数化波形生成。
 *
 * 协作: MultibandGate2(多频段门控) / Equalizer4(均衡器) / FftEngine2(FFT引擎)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 多功能信号发生器(AM/FM调制+扫频)
 */
class SignalGenerator3 : public QObject {
    Q_OBJECT

public:
    /** @brief 波形类型 */
    enum Waveform { Sine = 0, Square, Triangle, Sawtooth };

    /** @brief 调制类型 */
    enum ModType { None = 0, AM, FM };

    /** @brief 扫频类型 */
    enum SweepType { NoSweep = 0, Linear, Logarithmic };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalGenerations = 0;
        int numSamples = 0;
        double duration = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SignalGenerator3(QObject *parent = nullptr);
    ~SignalGenerator3() override;

    void setSampleRate(double sr);
    void setFrequency(double freq);
    void setAmplitude(double amp);
    void setPhase(double phase);
    void setWaveform(Waveform wf);
    void setModulation(ModType type, double modFreq, double modDepth);
    void setSweep(SweepType type, double startFreq, double endFreq, double duration);

    /** @brief 生成指定样本数的波形 */
    QVector<double> generate(int numSamples);

    /** @brief 生成指定时长的波形 */
    QVector<double> generateDuration(double seconds);

    /** @brief 生成单个采样点 */
    double sample(int index) const;

    /** @brief 混合两路信号 */
    static QVector<double> mix(const QVector<double>& a,
                               const QVector<double>& b, double mixRatio);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void generationCompleted(int numSamples, double duration, double timeMs);

private:
    double m_sampleRate = 44100.0;
    double m_frequency = 440.0;
    double m_amplitude = 1.0;
    double m_phase = 0.0;
    Waveform m_waveform = Sine;

    // Modulation
    ModType m_modType = None;
    double m_modFreq = 5.0;
    double m_modDepth = 0.5;

    // Sweep
    SweepType m_sweepType = NoSweep;
    double m_sweepStartFreq = 20.0;
    double m_sweepEndFreq = 20000.0;
    double m_sweepDuration = 1.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute instantaneous frequency at sample index */
    double instantFreq(int idx, int totalSamples) const;

    /** @brief Generate one cycle of base waveform */
    double baseWaveform(double phase) const;

    /** @brief Apply modulation */
    double applyModulation(double sample, int idx) const;
};
