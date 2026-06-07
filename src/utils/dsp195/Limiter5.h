/**
 * @file Limiter5.h
 * @brief 真峰值限制器(sinc插值样本间峰值检测+ISP上限) — True-Peak Limiter with Inter-Sample Peak Detection via Sinc Interpolation and ISP Ceiling
 *
 * 功能: 实现真峰值限制器，支持sinc插值检测样本间峰值、
 *       ISP上限控制、前视延迟补偿和增益平滑。
 *
 * 协作: Compressor3(动态压缩) / Equalizer4(均衡器) / BandPassFilter4(带通滤波)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 真峰值限制器(sinc插值ISP检测+上限控制)
 */
class Limiter5 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSamples = 0;
        double peakInput = 0.0;
        double peakOutput = 0.0;
        double maxISP = 0.0;
        double avgGainReduction = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Limiter5(QObject *parent = nullptr);
    ~Limiter5() override;

    void setCeiling(double dB);
    void setThreshold(double dB);
    void setRelease(double ms);
    void setLookahead(int samples);
    void setOversampleRate(int rate);

    /** @brief Process audio buffer, return limited output */
    QVector<double> process(const QVector<double>& input);

    /** @brief Detect inter-sample peaks via sinc interpolation */
    QVector<double> detectISP(const QVector<double>& input) const;

    /** @brief Compute true-peak level of signal */
    double truePeakLevel(const QVector<double>& input) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int samples, double peakReduction, double timeMs);

private:
    double m_ceilingDb = -1.0;
    double m_thresholdDb = -3.0;
    double m_releaseMs = 50.0;
    int m_lookahead = 64;
    int m_oversampleRate = 4;

    double m_gainSmooth = 1.0;
    QVector<double> m_delayLine;
    int m_delayPos = 0;

    Stats m_stats;
    double m_timeSum = 0.0;
    quint64 m_gainReductionCount = 0;

    /** @brief Design sinc interpolation FIR kernel */
    QVector<double> sincKernel(int taps, int oversample) const;

    /** @brief Apply gain reduction with smooth release */
    double computeGain(double peak, double sampleRate);

    /** @brief Upsample via zero-stuffing and LPF */
    QVector<double> upsample(const QVector<double>& input) const;

    /** @brief Downsample via decimation and LPF */
    QVector<double> downsample(const QVector<double>& input) const;
};
