/**
 * @file MultibandCompressor4.h
 * @brief 多频段压缩器(翘曲滤波器动态分频+逐频段前瞻) — Multiband Compressor with Dynamic Band Splitting via Warped Filter and Per-Band Lookahead
 *
 * 功能: 实现多频段动态压缩，支持翘曲滤波器分频、
 *       逐频段前瞻增益计算和动态交叉频率调整。
 *
 * 协作: BiquadFilter6(双二阶滤波器) / DynamicProcessor3(动态处理器) / SpectralAnalyzer5(频谱分析)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 多频段压缩器(翘曲滤波器分频+前瞻)
 */
class MultibandCompressor4 : public QObject {
    Q_OBJECT

public:
    /** @brief Per-band compressor parameters */
    struct BandParams {
        double threshold = -20.0;   // dB
        double ratio = 4.0;
        double attack = 5.0;        // ms
        double release = 50.0;      // ms
        double knee = 6.0;          // dB, soft knee width
        double makeupGain = 0.0;    // dB
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalFrames = 0;
        int numBands = 0;
        int sampleRate = 44100;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MultibandCompressor4(QObject *parent = nullptr);
    ~MultibandCompressor4() override;

    void setSampleRate(int sr);
    void setNumBands(int bands);
    void setBandParams(int band, const BandParams& params);
    void setCrossFreq(int index, double freq);
    void setLookahead(int samples);
    void setWarpFactor(double lambda);

    /** @brief Process a frame of audio samples */
    QVector<double> process(const QVector<double>& input);

    /** @brief Process interleaved stereo */
    QVector<double> processStereo(const QVector<double>& input);

    /** @brief Get per-band RMS levels (dB) */
    QVector<double> bandLevels() const;

    /** @brief Get per-band gain reduction (dB) */
    QVector<double> gainReduction() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void frameProcessed(int bands, double peakReduction, double timeMs);

private:
    int m_sampleRate = 44100;
    int m_numBands = 4;
    int m_lookahead = 64;
    double m_warpFactor = 0.3;

    QVector<BandParams> m_params;
    QVector<double> m_crossFreqs;

    // Per-band state
    QVector<double> m_envLevel;       // envelope follower level
    QVector<double> m_gainReduction;  // current gain reduction per band

    // Warped filter coefficients (1st-order allpass cascade per crossover)
    QVector<QVector<double>> m_apState;   // allpass states per band
    QVector<QVector<double>> m_apCoeff;   // allpass coefficients

    // Lookahead delay lines per band
    QVector<QVector<double>> m_lookaheadBuf;
    QVector<int> m_lookaheadPos;

    // Warped crossover filter state
    struct WarpState {
        double xz = 0.0;  // previous input
        double yz = 0.0;  // previous output
    };
    QVector<QVector<WarpState>> m_warpStates;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Update warped filter coefficients */
    void updateWarpCoeffs();

    /** @brief Apply warped crossover to split into bands */
    QVector<QVector<double>> splitBands(const QVector<double>& input);

    /** @brief Compute gain for one band given level */
    double computeGain(int band, double level) const;

    /** @brief Apply envelope follower */
    double followEnvelope(int band, double input);
};
