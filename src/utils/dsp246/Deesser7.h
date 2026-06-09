/**
 * @file Deesser7.h
 * @brief 去齿音处理器(嘶嘶音频段峰值检测+侧链滤波动态处理) — De-esser with Spectral Peak Detection in Sibilance Band and Sidechain-Filtered Dynamics Processing
 *
 * 功能: 实现去齿音处理器(De-esser)，通过嘶嘶音频段(sibilance band)的
 *       频谱峰值检测(spectral peak detection)识别齿音，使用侧链滤波
 *       (sidechain filtering)驱动的动态处理(dynamics processing)进行抑制。
 *
 * 协作: DynamicCompressor6(动态压缩器) / Equalizer5(均衡器) / NoiseGate4(噪声门)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 去齿音处理器(嘶嘶音频段峰值检测+侧链滤波动态处理)
 */
class Deesser7 : public QObject {
    Q_OBJECT

public:
    /** @brief Processing parameters */
    struct Params {
        double threshold = -20.0;    // Detection threshold in dB
        double frequency = 6000.0;   // Sibilance center frequency in Hz
        double bandwidth = 3000.0;   // Sibilance bandwidth in Hz
        double ratio = 4.0;          // Compression ratio
        double attack = 0.5;         // Attack time in ms
        double release = 50.0;       // Release time in ms
        double makeupGain = 0.0;     // Output makeup gain in dB
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numFrames = 0;
        int sibilanceDetections = 0;
        double avgReduction = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Deesser7(QObject *parent = nullptr);
    ~Deesser7() override;

    /** @brief Set processing parameters */
    void setParams(const Params& params);

    /** @brief Set sample rate */
    void setSampleRate(double rate);

    /** @brief Process a frame of audio samples */
    QVector<double> process(const QVector<double>& input);

    /** @brief Reset internal state (filters, envelope) */
    void reset();

    const Params& params() const { return m_params; }
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int frameSize, double timeMs);

private:
    Params m_params;
    double m_sampleRate = 44100.0;

    // Band-pass filter state for sibilance detection
    QVector<double> m_bpX;   // Input delay line
    QVector<double> m_bpY;   // Output delay line
    double m_bpA[3] = {};    // Denominator coefficients
    double m_bpB[3] = {};    // Numerator coefficients

    // Envelope follower state
    double m_env = 0.0;
    double m_attackCoeff = 0.0;
    double m_releaseCoeff = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;
    double m_reductionSum = 0.0;

    /** @brief Design band-pass filter for sibilance band */
    void designBandpass();

    /** @brief Apply band-pass filter to single sample */
    double bandpassSample(double x);

    /** @brief Compute gain reduction from detected level */
    double computeGainReduction(double levelDb) const;
};
