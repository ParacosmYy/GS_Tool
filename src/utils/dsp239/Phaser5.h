/**
 * @file Phaser5.h
 * @brief 移相器(全通级联调制+LFO驱动扫频+可配置级数和反馈混合) — Phaser with All-Pass Cascade Modulation and LFO-Driven Sweep with Configurable Stage Count and Feedback Mixing
 *
 * 功能: 实现移相器效果(Phaser effect)，采用全通级联调制(all-pass cascade modulation)通过
 *       多阶二阶全通滤波器产生频率依赖的相移，利用LFO驱动扫频(LFO-driven sweep)自动调制
 *       截止频率，支持可配置级数(configurable stage count)和反馈混合(feedback mixing)。
 *
 * 协作: ChorusEffect4(合唱效果) / DelayEffect3(延迟效果) / BiquadFilter6(双二阶滤波器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 移相器(全通级联调制+LFO驱动扫频+可配置级数和反馈混合)
 */
class Phaser5 : public QObject {
    Q_OBJECT

public:
    /** @brief LFO waveform shape */
    enum LfoShape { Sine = 0, Triangle = 1 };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numStages = 0;
        int blockSize = 0;
        int numBlocks = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Phaser5(QObject *parent = nullptr);
    ~Phaser5() override;

    /** @brief Set number of all-pass stages (2, 4, 6, 8, 12) */
    void setStages(int stages);

    /** @brief Set LFO rate in Hz */
    void setLfoRate(double hz);

    /** @brief Set LFO waveform shape */
    void setLfoShape(LfoShape shape);

    /** @brief Set sweep depth (0.0 to 1.0) */
    void setDepth(double depth);

    /** @brief Set feedback amount (-0.99 to 0.99) */
    void setFeedback(double fb);

    /** @brief Set base frequency in Hz */
    void setBaseFrequency(double hz);

    /** @brief Process a block of mono samples */
    QVector<double> process(const QVector<double>& input);

    /** @brief Process a single sample */
    double processOne(double input);

    /** @brief Reset all filter states */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void blockProcessed(int blockSize, double lfoValue, double timeMs);

private:
    int m_stages = 4;
    double m_lfoRate = 0.5;
    LfoShape m_lfoShape = Sine;
    double m_depth = 0.7;
    double m_feedback = 0.5;
    double m_baseFreq = 1000.0;

    // All-pass filter state per stage
    QVector<double> m_x1;  // previous input per stage
    QVector<double> m_y1;  // previous output per stage

    double m_feedbackSample = 0.0;
    double m_lfoPhase = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Advance LFO and return current value in [0,1] */
    double advanceLfo();

    /** @brief Compute all-pass coefficient for given frequency */
    double allPassCoeff(double freq) const;

    /** @brief Process single all-pass stage */
    double processStage(double input, int stage, double coeff);
};
