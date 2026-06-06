/**
 * @file DynamicEQ2.h
 * @brief 动态均衡器(频率依赖阈值+包络跟随频段增益+侧链输入) — Dynamic EQ with Frequency-Dependent Threshold, Envelope-Following Band Gain and Sidechain Input
 *
 * 功能: 实现动态均衡器，支持频率依赖阈值、包络跟随器、
 *       多频段增益控制和侧链输入检测。
 *
 * 协作: DynamicCompressor4(动态压缩) / ParametricEQ3(参数均衡) / MultibandComp6(多段压缩)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 动态均衡器(频率依赖阈值+包络跟随+侧链)
 */
class DynamicEQ2 : public QObject {
    Q_OBJECT

public:
    /** @brief Single EQ band parameters */
    struct BandParams {
        double frequency = 1000.0;
        double gain = 0.0;
        double threshold = -20.0;
        double ratio = 4.0;
        double attack = 10.0;
        double release = 100.0;
        double qFactor = 1.0;
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSamples = 0;
        int numBands = 0;
        double sampleRate = 44100.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DynamicEQ2(QObject *parent = nullptr);
    ~DynamicEQ2() override;

    void setSampleRate(double rate);
    void setBands(const QVector<BandParams>& bands);

    /** @brief 处理单声道音频 */
    QVector<double> process(const QVector<double>& input);

    /** @brief 处理带侧链输入 */
    QVector<double> processWithSidechain(const QVector<double>& input,
                                         const QVector<double>& sidechain);

    /** @brief 计算单频段包络 */
    double computeEnvelope(double sample, double attack, double release,
                           double& state) const;

    /** @brief 计算增益缩减量 */
    double gainReduction(double level, double threshold, double ratio) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int numSamples, double peakReduction);

private:
    double m_sampleRate = 44100.0;
    QVector<BandParams> m_bands;

    // Per-band envelope state
    QVector<double> m_envelopeState;
    // Per-band bandpass filter state (biquad)
    struct BiquadState { double x1 = 0, x2 = 0, y1 = 0, y2 = 0; };
    QVector<BiquadState> m_bpState;
    QVector<BiquadState> m_bpSideState;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Design bandpass filter coefficients */
    struct BiquadCoeffs { double b0 = 0, b1 = 0, b2 = 0, a1 = 0, a2 = 0; };
    BiquadCoeffs designBandpass(double freq, double q) const;

    /** @brief Apply biquad filter */
    double applyBiquad(double sample, const BiquadCoeffs& c, BiquadState& s) const;
};
