/**
 * @file NoiseGate10.h
 * @brief 噪声门(迟滞阈值与前瞻缓冲区实现无咔嗒声瞬态保护音频门控) — Noise Gate with Hysteresis Threshold and Lookahead Buffer for Click-free Transient Preservation in Audio Gating
 *
 * 功能: 实现噪声门(Noise gate)，采用迟滞阈值(hysteresis threshold)
 *       与前瞻缓冲区(lookahead buffer)实现无咔嗒声瞬态保护(click-free transient preservation)。
 *
 * 协作: Compressor9(压缩器) / Expander8(扩展器) / EnvelopeDetector7(包络检测)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 噪声门(迟滞阈值与前瞻缓冲区实现无咔嗒声瞬态保护音频门控)
 */
class NoiseGate10 : public QObject {
    Q_OBJECT

public:
    /** @brief Gate processing result */
    struct GateResult {
        QVector<double> output;
        QVector<double> gainEnvelope;   // Gain applied per sample
        int openCount = 0;              // Number of gate-open transitions
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int lastFrameSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit NoiseGate10(QObject *parent = nullptr);
    ~NoiseGate10() override;

    void setThreshold(double dB);         // Open threshold in dB
    void setCloseThreshold(double dB);    // Close threshold in dB (hysteresis)
    void setAttack(double ms);            // Attack time
    void setRelease(double ms);           // Release time
    void setHold(double ms);              // Hold time
    void setLookahead(int samples);       // Lookahead buffer size
    void setSampleRate(double sr);

    /** @brief Process audio through the noise gate */
    GateResult process(const QVector<double>& input);

    /** @brief Reset internal state */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processDone(int frameSize, int openCount, double timeMs);

private:
    double m_threshold = -40.0;     // Open threshold (dB)
    double m_closeThreshold = -46.0;// Close threshold (dB)
    double m_attack = 1.0;          // Attack time (ms)
    double m_release = 50.0;        // Release time (ms)
    double m_hold = 10.0;           // Hold time (ms)
    int m_lookahead = 64;           // Lookahead samples
    double m_sampleRate = 44100.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    // Internal state
    double m_currentGain = 0.0;
    double m_holdCounter = 0.0;
    bool m_gateOpen = false;

    // Lookahead buffer
    QVector<double> m_lookaheadBuf;
    int m_lookaheadPos = 0;

    /** @brief Compute amplitude envelope via peak detection */
    double detectEnvelope(double sample) const;

    /** @brief Compute smooth gain coefficient */
    double smoothCoeff(double timeMs) const;

    /** @brief Convert dB to linear */
    static double dbToLinear(double dB);

    /** @brief Convert linear to dB */
    static double linearToDb(double lin);
};
