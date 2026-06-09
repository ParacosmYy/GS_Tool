/**
 * @file Phaser6.h
 * @brief 移相器(多级全通滤波网络+每级随机LFO相位偏移厚调制) — Phaser with Multi-Stage All-Pass Filter Network and Random LFO Phase Offset Per Stage for Thicker Modulation
 *
 * 功能: 实现移相器(Phaser)音效处理，使用多级全通滤波器网络(multi-stage
 *       all-pass filter network)，每级配置随机LFO相位偏移(random LFO
 *       phase offset)产生更厚的调制效果(thicker modulation)。
 *
 * 协作: Chorus5(合唱) / Flanger4(法兰) / Delay3(延迟)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 移相器(多级全通+随机LFO相位偏移)
 */
class Phaser6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numStages = 0;
        int numSamples = 0;
        int numBlocks = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Phaser6(QObject *parent = nullptr);
    ~Phaser6() override;

    /** @brief Set number of all-pass stages (2..12) */
    void setStages(int stages);

    /** @brief Set LFO rate in Hz */
    void setLFORate(double hz);

    /** @brief Set LFO depth (0..1) */
    void setDepth(double depth);

    /** @brief Set feedback amount (0..0.95) */
    void setFeedback(double fb);

    /** @brief Set mix ratio dry/wet (0..1) */
    void setMix(double mix);

    /** @brief Process a block of mono samples */
    QVector<double> process(const QVector<double>& input);

    /** @brief Reset internal state */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int numSamples, double timeMs);

private:
    int m_stages = 6;
    double m_lfoRate = 0.5;    // Hz
    double m_depth = 0.7;
    double m_feedback = 0.5;
    double m_mix = 0.5;
    double m_sampleRate = 44100.0;

    // Per-stage state
    QVector<double> m_lfoPhase;    // Random phase offset per stage
    QVector<double> m_allpassX1;   // Previous input per stage
    QVector<double> m_allpassY1;   // Previous output per stage
    double m_phase = 0.0;         // Master LFO phase
    double m_feedbackBuf = 0.0;   // Feedback buffer

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Initialize random LFO phase offsets */
    void initPhaseOffsets();

    /** @brief Compute LFO value for given stage */
    double lfoValue(int stage) const;

    /** @brief Single all-pass filter stage */
    double allpass(double input, int stage, double coeff);
};
