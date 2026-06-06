/**
 * @file TransientShaper3.h
 * @brief 瞬态塑形器(包络差分+瞬态/持续分离+多频带处理) — Transient Shaper with Envelope Differencer, Transient/Sustain Separation and Multiband Processing
 *
 * 功能: 实现瞬态塑形器，支持快速/慢速包络差分、瞬态/持续成分分离、
 *       多频带瞬态处理和增益控制。
 *
 * 协作: Compressor6(动态压缩) / FeatureExtractor2(特征提取) / BiQuadFilter3(双二阶滤波)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 瞬态塑形器(包络差分+多频带处理)
 */
class TransientShaper3 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalFrames = 0;
        int frameSize = 0;
        int sampleRate = 44100;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief 单频带瞬态参数 */
    struct BandParams {
        double attack = 0.01;     // fast envelope time constant (s)
        double sustain = 0.1;     // slow envelope time constant (s)
        double transientGain = 1.0;
        double sustainGain = 1.0;
        double lowFreq = 0.0;     // Hz, 0 = fullband
        double highFreq = 0.0;    // Hz, 0 = fullband
    };

    explicit TransientShaper3(QObject *parent = nullptr);
    ~TransientShaper3() override;

    void setSampleRate(int sr);
    void setBandParams(const QVector<BandParams>& bands);

    /** @brief 处理单帧音频 */
    QVector<double> process(const QVector<double>& frame);

    /** @brief 分离瞬态和持续成分 */
    void separateTransientSustain(const QVector<double>& input,
                                  QVector<double>& transient,
                                  QVector<double>& sustain);

    /** @brief 计算包络(攻击/释放) */
    QVector<double> computeEnvelope(const QVector<double>& input,
                                    double attack, double release) const;

    /** @brief 包络差分提取瞬态 */
    QVector<double> envelopeDifference(const QVector<double>& fast,
                                       const QVector<double>& slow) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void frameProcessed(int frameSize, double peakTransient);

private:
    int m_sampleRate = 44100;
    QVector<BandParams> m_bands;

    Stats m_stats;
    double m_timeSum = 0.0;

    // Envelope follower state per band
    struct EnvState {
        double fastEnv = 0.0;
        double slowEnv = 0.0;
    };
    QVector<EnvState> m_envStates;

    /** @brief Simple 2nd-order crossover split */
    void crossover(const QVector<double>& input,
                   double freq,
                   QVector<double>& low,
                   QVector<double>& high) const;

    /** @brief Apply transient/sustain gains to a band */
    QVector<double> shapeBand(const QVector<double>& input,
                              BandParams& params, EnvState& state);
};
