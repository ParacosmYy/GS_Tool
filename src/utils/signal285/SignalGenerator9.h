/**
 * @file SignalGenerator9.h
 * @brief 信号发生器(FM合成与算子堆叠的多载波波形生成及调制指数控制) — Signal Generator with FM Synthesis and Operator Stacking for Multi-carrier Waveform Generation with Modulation Index Control
 *
 * 功能: 实现信号发生器(signal generator)，采用FM合成(FM synthesis)
 *       与算子堆叠(operator stacking)实现多载波波形生成(multi-carrier waveform generation)及调制指数控制(modulation index control)。
 *
 * 协作: SignalGenerator8(基本波形) / FFT10(FFT分析) / Filter8(滤波器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 信号发生器(FM合成与算子堆叠的多载波波形生成)
 */
class SignalGenerator9 : public QObject {
    Q_OBJECT

public:
    /** @brief Waveform type */
    enum WaveType { Sine = 0, Square, Sawtooth, Triangle, Noise };

    /** @brief FM operator configuration */
    struct FMOperator {
        double frequency = 440.0;     // Carrier/modulator frequency
        double amplitude = 1.0;       // Output amplitude
        double modulationIndex = 1.0; // FM modulation depth
        WaveType waveform = Sine;
        int modulatorIdx = -1;        // Index of modulating operator (-1 = none)
    };

    /** @brief Generation result */
    struct GenResult {
        QVector<double> samples;
        double peakAmplitude = 0.0;
        double rmsLevel = 0.0;
        int numSamples = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numSamples = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SignalGenerator9(QObject *parent = nullptr);
    ~SignalGenerator9() override;

    void setSampleRate(double sr);
    void setOperators(const QVector<FMOperator>& ops);

    /** @brief Generate samples using FM synthesis */
    GenResult generate(int numSamples);

    /** @brief Generate a single waveform cycle */
    QVector<double> generateCycle(WaveType type, double freq, int samplesPerCycle);

    /** @brief Get current operator stack */
    QVector<FMOperator> operators() const { return m_operators; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void generationDone(int n, double peak, double timeMs);

private:
    double m_sampleRate = 44100.0;
    QVector<FMOperator> m_operators;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Phase accumulators for each operator */
    QVector<double> m_phase;

    /** @brief Evaluate one sample of a base waveform */
    double evalWave(WaveType type, double phase) const;

    /** @brief Evaluate the full operator stack for one sample */
    double evalOperatorStack();

    /** @brief Normalize phase to [0, 2π) */
    static double wrapPhase(double phase);
};
