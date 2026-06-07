/**
 * @file MultibandGate2.h
 * @brief 多频段门控(完美重建交叉+噪声底跟踪) — Multiband Gate with Perfect-Reconstruction Crossover and Per-Band Noise Floor Tracking
 *
 * 功能: 实现多频段门控处理器，支持完美重建Linkwitz-Riley交叉滤波、
 *       每频段独立噪声底跟踪、自适应攻击/释放包络。
 *
 * 协作: Compressor3(动态压缩) / Equalizer4(均衡器) / Limiter2(限幅器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 多频段门控处理器(完美重建交叉+噪声底跟踪)
 */
class MultibandGate2 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalProcessCalls = 0;
        int numBands = 4;
        int blockSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief 单频段参数 */
    struct BandParams {
        double crossoverFreq = 0.0;
        double threshold = -40.0;      // dB
        double attack = 5.0;           // ms
        double release = 50.0;         // ms
        double noiseFloor = -80.0;     // dB
        double reduction = 0.0;        // dB (current)
    };

    explicit MultibandGate2(QObject *parent = nullptr);
    ~MultibandGate2() override;

    void setSampleRate(double sr);
    void setNumBands(int bands);
    void setCrossoverFreqs(const QVector<double>& freqs);
    void setBandParams(int band, const BandParams& params);

    /** @brief 处理音频块，返回门控后信号 */
    QVector<double> process(const QVector<double>& input);

    /** @brief 获取各频段当前增益(dB) */
    QVector<double> bandGains() const;

    /** @brief 获取各频段噪声底估计(dB) */
    QVector<double> noiseFloorEstimates() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int blockSize, double timeMs);

private:
    double m_sampleRate = 44100.0;
    int m_numBands = 4;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Linkwitz-Riley 2nd-order state */
    struct LRState {
        double x1 = 0.0, x2 = 0.0;
        double y1 = 0.0, y2 = 0.0;
    };

    struct BandState {
        BandParams params;
        LRState lpState1, lpState2;   // cascaded LP
        LRState hpState1, hpState2;   // cascaded HP
        double envelope = 0.0;        // current envelope follower
        double noiseEstimate = -80.0; // tracked noise floor
        double gateGain = 1.0;        // current gate gain (0..1)
    };

    QVector<BandState> m_bands;

    /** @brief Apply Linkwitz-Riley crossover split */
    void crossoverSplit(const QVector<double>& input,
                        QVector<QVector<double>>& bandSignals);

    /** @brief Process 2nd-order LP section */
    double processLP(double x, LRState& s, double coeff_a1, double coeff_a2,
                     double coeff_b0, double coeff_b1, double coeff_b2);

    /** @brief Process 2nd-order HP section */
    double processHP(double x, LRState& s, double coeff_a1, double coeff_a2,
                     double coeff_b0, double coeff_b1, double coeff_b2);

    /** @brief Compute butterworth coefficients for given fc */
    void butterworthCoeffs(double fc, double& a1, double& a2,
                           double& b0, double& b1, double& b2) const;

    /** @brief Envelope follower for one band */
    double followEnvelope(double sample, const BandState& band);

    /** @brief Track noise floor estimate */
    double trackNoiseFloor(double envelope, double currentEstimate);

    /** @brief Compute gate gain from envelope and threshold */
    double computeGateGain(double envelope, double threshold,
                           double noiseFloor) const;

    /** @brief Initialize band states */
    void initBands();
};
