/**
 * @file Phaser7.h
 * @brief 相位器(全通级联与正交LFO调制及反馈谐振的扫频相位消除) — Phaser with All-Pass Cascade Modulated by Quadrature LFO and Feedback Resonance for Sweeping Phase Cancellation
 *
 * 功能: 实现相位器(Phaser)，采用全通级联(all-pass cascade)与正交LFO调制(quadrature LFO)
 *       及反馈谐振(feedback resonance)实现扫频相位消除(sweeping phase cancellation)。
 *
 * 协作: Chorus6(合唱) / Flanger5(镶边) / DelayLine4(延迟线)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 相位器(全通级联与正交LFO调制及反馈谐振的扫频相位消除)
 */
class Phaser7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int blockSize = 0;
        int numStages = 0;
        double feedbackLevel = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Phaser7(QObject *parent = nullptr);
    ~Phaser7() override;

    /** @brief Configure phaser: stages(4-12), lfoRate(Hz), depth(0-1), feedback(0-0.95) */
    void setParameters(int stages, double lfoRate, double depth, double feedback);

    /** @brief Process a block of samples */
    QVector<double> process(const QVector<double>& input);

    /** @brief Reset internal state (delay lines, LFO phase) */
    void reset();

    /** @brief Set sample rate */
    void setSampleRate(double sampleRate);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void phaserUpdated(int blockSize, double lfoRate, double timeMs);

private:
    double m_sampleRate = 44100.0;
    int m_stages = 6;
    double m_lfoRate = 0.5;
    double m_depth = 0.7;
    double m_feedback = 0.5;

    // LFO state (quadrature: sine + cosine)
    double m_lfoPhase = 0.0;

    // All-pass filter states per stage
    QVector<double> m_apX1;   // Previous input per stage
    QVector<double> m_apY1;   // Previous output per stage

    // Feedback path
    double m_fbSample = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute all-pass coefficient from normalized frequency */
    double allPassCoeff(double normalizedFreq) const;

    /** @brief Process single sample through all-pass cascade */
    double processSample(double input);

    /** @brief Advance quadrature LFO by one sample */
    void advanceLFO();
};
