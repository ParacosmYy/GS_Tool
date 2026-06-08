/**
 * @file MultibandCompressor5.h
 * @brief 多频段动态压缩器(Linkwitz-Riley 4阶交叉+逐频段自动补偿增益) — Multiband Compressor with Linkwitz-Riley 4th-Order Crossover and Per-Band Makeup Gain with Auto-Match
 *
 * 功能: 实现多频段动态压缩，使用Linkwitz-Riley 4阶交叉滤波器分频，
 *       每个频段独立压缩并自动匹配补偿增益。
 *
 * 协作: ButterworthFilter3(巴特沃斯) / PeakDetector4(峰值检测) / EqualLoudness7(等响度)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 多频段动态压缩器(LR4交叉+自动补偿)
 */
class MultibandCompressor5 : public QObject {
    Q_OBJECT

public:
    /** @brief Per-band compressor parameters */
    struct BandParams {
        double threshold = -20.0;   // dB
        double ratio = 4.0;
        double attack = 5.0;        // ms
        double release = 50.0;      // ms
        double makeupGain = 0.0;    // dB (auto-matched when enabled)
        bool autoMatch = true;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalSamples = 0;
        int numBands = 0;
        int blockSize = 0;
        double sampleRate = 44100.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MultibandCompressor5(QObject *parent = nullptr);
    ~MultibandCompressor5() override;

    /** @brief Set sample rate and crossover frequencies */
    void setParameters(double sampleRate, const QVector<double>& crossoverFreqs);

    /** @brief Set per-band compressor parameters */
    void setBandParams(int band, const BandParams& params);

    /** @brief Process a single sample */
    double processSample(double sample);

    /** @brief Process block of samples */
    QVector<double> processBlock(const QVector<double>& samples);

    /** @brief Get per-band RMS levels in dB */
    QVector<double> bandLevels() const;

    /** @brief Get per-band gain reduction in dB */
    QVector<double> gainReduction() const;

    /** @brief Compute Linkwitz-Riley 4th-order coefficients */
    void computeLRCoeffs(double freq, double sampleRate,
                         QVector<double>& b, QVector<double>& a) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void blockProcessed(int numBands, double timeMs);

private:
    int m_numBands = 4;
    double m_sampleRate = 44100.0;

    QVector<BandParams> m_bandParams;

    // LR4 filter state per band (2 cascaded 2nd-order)
    // Each band has LP and HP branches with 2 biquads each
    struct BiquadState {
        double x1 = 0, x2 = 0;
        double y1 = 0, y2 = 0;
    };
    struct LR4State {
        BiquadState lp1, lp2;   // LP path: two cascaded biquads
        BiquadState hp1, hp2;   // HP path: two cascaded biquads
    };
    QVector<LR4State> m_crossoverState;

    // Envelope follower per band
    QVector<double> m_envelope;
    QVector<double> m_gainRed;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Apply single biquad */
    double applyBiquad(double x, const QVector<double>& b,
                       const QVector<double>& a, BiquadState& st) const;

    /** @brief Compute gain reduction for a band */
    double computeGainReduction(int band, double inputLevel) const;

    /** @brief Auto-match makeup gain to maintain output level */
    double autoMatchGain(int band, double inputLevel, double outputLevel) const;
};
