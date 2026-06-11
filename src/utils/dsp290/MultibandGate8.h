/**
 * @file MultibandGate8.h
 * @brief 多频段门(交叉滤波器组与逐频段滞后控制实现动态范围频率选择性噪声抑制) — Multiband Gate with Crossover Filter Bank and Per-band Hysteresis for Frequency-selective Noise Suppression across Dynamic Ranges
 *
 * 功能: 实现多频段门(multiband gate)，采用交叉滤波器组(crossover filter bank)
 *       与逐频段滞后控制(per-band hysteresis)实现频率选择性噪声抑制(frequency-selective noise suppression)。
 *
 * 协作: LinkwitzRiley4(交叉滤波器) / CompressorBand6(压缩器) / NoiseGate6(噪声门)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 多频段门(交叉滤波器组与逐频段滞后控制实现频率选择性噪声抑制)
 */
class MultibandGate8 : public QObject {
    Q_OBJECT

public:
    /** @brief Per-band gate parameters */
    struct BandParams {
        double threshold = -40.0;       // Gate threshold in dB
        double attack = 1.0;            // Attack time in ms
        double release = 50.0;          // Release time in ms
        double hysteresis = 6.0;        // Hysteresis width in dB
        double range = -80.0;           // Gate floor in dB
        bool bypass = false;
    };

    /** @brief Processing result for one frame */
    struct FrameResult {
        QVector<double> bandGains;      // Current gain per band (linear)
        QVector<double> bandLevels;     // RMS level per band (dB)
        int numBands = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalFrames = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MultibandGate8(QObject *parent = nullptr);
    ~MultibandGate8() override;

    void setSampleRate(double sr);
    void setNumBands(int n);
    void setCrossoverFreqs(const QVector<double>& freqs);
    void setBandParams(int band, const BandParams& params);

    /** @brief Process a single sample, return gated output */
    double processSample(double input);

    /** @brief Process a block of samples */
    QVector<double> processBlock(const QVector<double>& input);

    /** @brief Get current band levels and gains */
    FrameResult getFrameState() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingDone(int numSamples, double timeMs);

private:
    double m_sampleRate = 44100.0;
    int m_numBands = 4;
    Stats m_stats;
    double m_timeSum = 0.0;

    QVector<double> m_crossoverFreqs;
    QVector<BandParams> m_bandParams;

    // Per-band state: Linkwitz-Riley 2nd-order state variables
    struct FilterState {
        double x1 = 0.0, x2 = 0.0;
        double y1 = 0.0, y2 = 0.0;
    };

    // Per-band: LP and HP filter states for each crossover
    struct CrossoverStage {
        FilterState lp, hp;
        double a0 = 0.0, a1 = 0.0, a2 = 0.0;
        double b0 = 1.0, b1 = 0.0, b2 = 0.0;
    };

    QVector<CrossoverStage> m_crossovers;

    // Per-band gate state
    struct GateState {
        double gain = 1.0;              // Current gain (linear)
        double envLevel = 0.0;          // Envelope follower (dB)
        bool gateOpen = false;
        double coeffAttack = 0.0;
        double coeffRelease = 0.0;
    };

    QVector<GateState> m_gateStates;
    QVector<double> m_bandOutputs;

    /** @brief Design Butterworth 2nd-order coefficients for a crossover */
    void designCrossover(double freq, CrossoverStage& stage);

    /** @brief Process one sample through a single filter */
    double filterSample(double input, FilterState& fs,
                        double b0, double b1, double b2,
                        double a1, double a2) const;

    /** @brief Update gate gain for a single band */
    void updateGate(int band, double level);

    /** @brief dB to linear conversion */
    static double dbToLinear(double db);

    /** @brief Linear to dB conversion */
    static double linearToDb(double lin);
};
