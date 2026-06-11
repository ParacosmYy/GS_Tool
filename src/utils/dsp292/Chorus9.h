/**
 * @file Chorus9.h
 * @brief 合唱效果器(多声部失谐合唱与随机LFO相位偏移立体声宽度调制实现丰富空间增厚) — Chorus with Multi-voice Detune and Stereo Width Modulation via Randomized LFO Phase Offsets for Rich Spatial Thickening
 *
 * 功能: 实现合唱效果器(Chorus effect)，采用多声部失谐合唱(multi-voice detune chorus)
 *       与随机LFO相位偏移(randomized LFO phase offsets)立体声宽度调制(stereo width modulation)实现丰富空间增厚(rich spatial thickening)。
 *
 * 协作: Flanger8(Flanger效果) / Phaser7(Phaser效果) / Delay6(延迟效果)
 */
#pragma once

#include <QObject>
#include <QVector>

class Chorus9 : public QObject {
    Q_OBJECT

public:
    /** @brief Audio processing result with stereo output */
    struct ChorusResult {
        QVector<double> leftOut;
        QVector<double> rightOut;
        double peakLevel = 0.0;
    };

    /** @brief Single voice configuration */
    struct VoiceConfig {
        double delayMs = 0.0;           // Base delay in ms
        double depth = 0.0;             // LFO depth in ms
        double rate = 0.0;              // LFO frequency in Hz
        double phaseOffset = 0.0;       // LFO phase offset (0-2π)
        double pan = 0.0;               // Pan position (-1 left, +1 right)
        double gain = 1.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalSamples = 0;
        int numVoices = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Chorus9(QObject *parent = nullptr);
    ~Chorus9() override;

    void setSampleRate(double sr);
    void setNumVoices(int n);
    void setBaseDelay(double ms);
    void setDepth(double ms);
    void setRate(double hz);
    void setStereoWidth(double width);  // 0 = mono, 1 = full stereo
    void setMix(double mix);            // Dry/wet mix (0-1)

    /** @brief Process mono or stereo input through chorus */
    ChorusResult process(const QVector<double>& input);
    ChorusResult processStereo(const QVector<double>& leftIn,
                                const QVector<double>& rightIn);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processDone(int numSamples, double peakLevel, double timeMs);

private:
    double m_sampleRate = 44100.0;
    int m_numVoices = 4;
    double m_baseDelay = 7.0;           // ms
    double m_depth = 3.0;               // ms
    double m_rate = 0.8;                // Hz
    double m_stereoWidth = 0.8;
    double m_mix = 0.5;
    Stats m_stats;
    double m_timeSum = 0.0;

    // Voice state
    QVector<VoiceConfig> m_voices;
    QVector<double> m_lfoPhase;         // Current LFO phase per voice
    QVector<QVector<double>> m_delayBuffer; // Circular delay buffer per voice
    QVector<int> m_writePos;

    /** @brief Initialize voice configurations with random phases */
    void initVoices();

    /** @brief Compute modulated delay in samples for a voice */
    double modulatedDelay(int voiceIdx) const;

    /** @brief Read from delay buffer with fractional delay (linear interpolation) */
    double readBuffer(int voiceIdx, double delaySamples) const;

    /** @brief Write sample to delay buffer */
    void writeBuffer(int voiceIdx, double sample);
};
