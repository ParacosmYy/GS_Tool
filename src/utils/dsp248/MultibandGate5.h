/**
 * @file MultibandGate5.h
 * @brief 多频段门控(完美重建交叉滤波器+前瞻包络预测) — Multiband Gate with Perfect Reconstruction Crossover and Look-Ahead Envelope Prediction per Frequency Band
 *
 * 功能: 实现多频段门控(Multiband Gate)，使用完美重建交叉滤波器
 *       (perfect reconstruction crossover)分离频段，每个频段配备前瞻
 *       包络预测(look-ahead envelope prediction)实现零延迟启闭控制。
 *
 * 协作: BiquadFilter8(双二阶滤波器) / DynamicCompressor5(动态压缩器) / WindowFunction5(窗函数)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 多频段门控(完美重建交叉+前瞻包络预测)
 */
class MultibandGate5 : public QObject {
    Q_OBJECT

public:
    /** @brief Per-band gate parameters */
    struct BandParams {
        double thresholdDb = -40.0;
        double attackMs = 1.0;
        double releaseMs = 50.0;
        double ratio = 10.0;
        bool bypassed = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numBands = 4;
        int blockSize = 0;
        double peakInputDb = -120.0;
        double peakOutputDb = -120.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MultibandGate5(int numBands = 4, QObject *parent = nullptr);
    ~MultibandGate5() override;

    /** @brief Set crossover frequencies between bands (numBands-1 freqs) */
    void setCrossoverFreqs(const QVector<double>& freqs);

    /** @brief Set gate parameters for a specific band */
    void setBandParams(int band, const BandParams& params);

    /** @brief Process a block of samples */
    QVector<double> process(const QVector<double>& input, double sampleRate);

    /** @brief Get per-band envelope levels (dB) */
    QVector<double> bandLevels() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void blockProcessed(int blockSize, double peakDb, double timeMs);

private:
    int m_numBands;
    int m_lookAhead = 64;

    /** @brief Linkwitz-Riley 4th-order crossover state */
    struct CrossoverState {
        double x1 = 0.0, x2 = 0.0, x3 = 0.0, x4 = 0.0;
        double y1 = 0.0, y2 = 0.0, y3 = 0.0, y4 = 0.0;
    };

    /** @brief Gate envelope detector */
    struct GateEnv {
        double envelope = 0.0;
        double gain = 0.0;
        QVector<double> lookAheadBuf;
        int laWritePos = 0;
    };

    QVector<BandParams> m_params;
    QVector<CrossoverState> m_lpState;   // Per-band LP filter states
    QVector<CrossoverState> m_hpState;   // Per-band HP filter states
    QVector<GateEnv> m_envelopes;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Apply Linkwitz-Riley LP stage */
    double processLP(CrossoverState& s, double x, double coeff);

    /** @brief Apply Linkwitz-Riley HP stage */
    double processHP(CrossoverState& s, double x, double coeff);

    /** @brief Compute LR crossover coefficient */
    double lrCoeff(double freq, double sampleRate) const;

    /** @brief Detect envelope with look-ahead */
    double detectEnvelope(GateEnv& env, double sample, double sampleRate);

    /** @brief Compute gate gain from envelope */
    double computeGateGain(double envDb, const BandParams& p) const;

    /** @brief Linear amplitude to dB */
    double toDb(double linear) const;

    /** @brief dB to linear amplitude */
    double fromDb(double db) const;
};
