/**
 * @file Chorus8.h
 * @brief 合唱效果器(多声部微调与随机相位初始化的厚重立体声合唱纹理生成) — Chorus with Ensemble Multi-voice Detuning and Random Phase Initialization for Thick Stereo Chorus Texture
 *
 * 功能: 实现合唱效果器(Chorus effect)，采用多声部微调(ensemble multi-voice detuning)
 *       与随机相位初始化(random phase initialization)实现厚重立体声合唱纹理生成(thick stereo chorus texture)。
 *
 * 协作: Phaser7(移相器) / Flanger6(镶边器) / Reverb9(混响)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 合唱效果器(多声部微调与随机相位初始化)
 */
class Chorus8 : public QObject {
    Q_OBJECT

public:
    /** @brief Processing result */
    struct ChorusResult {
        QVector<double> outputLeft;
        QVector<double> outputRight;
        double peakLevel = 0.0;
        double rmsLevel = 0.0;
    };

    /** @brief Single voice parameters */
    struct VoiceConfig {
        double rate = 0.0;          // LFO frequency (Hz)
        double depth = 0.0;         // Modulation depth (ms)
        double delay = 0.0;         // Base delay (ms)
        double pan = 0.0;           // Pan position (-1 to 1)
        double gain = 1.0;          // Voice gain
        double phase = 0.0;         // Initial LFO phase (radians)
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVoices = 0;
        int numSamples = 0;
        double sampleRate = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Chorus8(QObject *parent = nullptr);
    ~Chorus8() override;

    /** @brief Set sample rate in Hz */
    void setSampleRate(double rate);

    /** @brief Set base delay in milliseconds */
    void setBaseDelay(double ms);

    /** @brief Set modulation depth in milliseconds */
    void setDepth(double ms);

    /** @brief Set LFO rate in Hz */
    void setRate(double hz);

    /** @brief Set number of chorus voices (2-12) */
    void setNumVoices(int n);

    /** @brief Set feedback amount (0.0 - 0.95) */
    void setFeedback(double fb);

    /** @brief Set dry/wet mix (0.0 - 1.0) */
    void setMix(double mix);

    /** @brief Process mono input to stereo chorus output */
    ChorusResult process(const QVector<double>& input);

    /** @brief Process stereo input to stereo chorus output */
    ChorusResult processStereo(const QVector<double>& inputLeft,
                                const QVector<double>& inputRight);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingDone(int numSamples, int numVoices, double timeMs);

private:
    double m_sampleRate = 44100.0;
    double m_baseDelay = 7.0;
    double m_depth = 3.0;
    double m_rate = 0.8;
    int m_numVoices = 4;
    double m_feedback = 0.3;
    double m_mix = 0.5;

    Stats m_stats;
    double m_timeSum = 0.0;

    QVector<VoiceConfig> m_voices;
    QVector<double> m_delayBuffer;
    int m_writePos = 0;
    int m_delayLineSize = 0;

    /** @brief Initialize voice configurations with random phases */
    void initVoices();

    /** @brief Ensure delay line is large enough */
    void resizeDelayLine(double maxDelayMs);

    /** @brief Read from delay line with fractional sample position */
    double readFractional(double position) const;

    /** @brief Advance LFO for a voice, return modulated delay in samples */
    double computeModulatedDelay(int voiceIdx, int sampleIdx) const;
};
